#include "users.hpp"

// if v!=0, return v
#define zero_else_return(v) { auto x = (v); if (x) return x; }

int Faculty::teaching_courses(std::vector<std::pair<int, std::string>>& courses) {
    courses.clear();

    char *sql = (char *)("SELECT course_id, course_code FROM courses WHERE instructor_id = ? AND graded = 0");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);

    int result = sqlite3_step(stmt);
    while (result == SQLITE_ROW) {
        std::pair<int, std::string> course;
        course.first = sqlite3_column_int(stmt, 0);
        course.second = (char *) sqlite3_column_text(stmt, 1);
        courses.push_back(course);

        result = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE ? 0 : 1;
}

int Faculty::get_instructor_id(int course_id, int &instructor_id) {
    char *sql = (char *)("SELECT instructor_id FROM courses WHERE course_id = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, course_id);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        // done => no row => no such course
        return result == SQLITE_DONE ? 1 : -1;
    }

    instructor_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return 0;
}

int Faculty::check_graded(int course_id, bool &is_graded) {
    char *sql = (char *)("SELECT graded FROM courses WHERE course_id = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, course_id);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        // done => no row => no such course
        return result == SQLITE_DONE ? 1 : -1;
    }

    is_graded = (bool) sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return 0;
}

int Faculty::list_reg_students(int course_id, std::vector<std::pair<int, std::string>>& students) {

    // Only instructor (and faculty) should be able to see students
    int instructor_id;
    int res = get_instructor_id(course_id, instructor_id);
    if (res) {
        return res;
    }
    if (instructor_id != id) {
        return 2;
    }

    char *sql2 = (char *)("SELECT id, username FROM users INNER JOIN registered"
            "ON users.id = registered.student_id WHERE registered.course_id = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql2, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, course_id);

    int result = sqlite3_step(stmt);
    while (result == SQLITE_ROW) {
        auto first = sqlite3_column_int(stmt, 0);
        auto second = (char *) sqlite3_column_text(stmt, 1);
        students.push_back(std::make_pair(first, second));

        result = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE ? 0 : -1;
}

int Faculty::grade_course(int course_id, std::map<int, std::string> &grades) {
    std::vector<std::pair<int, std::string>> students;
    int res = list_reg_students(course_id, students);
    if (res) {
        return res;
    }

    // only grade once
    bool is_graded;
    res = check_graded(course_id, is_graded);
    if (res) {
        return res;
    }
    if (is_graded) {
        return 3;
    }

    // check that set of students in db and input are same

    if (students.size() != grades.size()) {
        return 4;
    }

    std::set<int> students_set_in_db;
    for (auto it = students.begin(); it != students.end(); it++) {
        students_set_in_db.insert(it->first);
    }

    std::set<int> students_set_in_input;
    for (auto it = grades.begin(); it != grades.end(); it++) {
        students_set_in_input.insert(it->first);

        // iterating anyways, so checking that all the grades are valid
        if (possible_grades.count(it->second) == 0) {
            return 5;
        }
    }

    if (students_set_in_db != students_set_in_input) {
        return 4;
    }

    // set of students is correct
    // grades are all valid
    // All is good, can write grades to database

    for (auto it = grades.begin(); it != grades.end(); it++) {
        auto student_id = it->first;
        auto student_grade = it->second;

        char *sql = (char *)("INSERT OR REPLACE INTO grades (reg_id, grade) VALUES "
                "((SELECT id FROM registered WHERE student_id = ? AND course_id = ?), ?)");

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_int(stmt, 0, student_id);
        sqlite3_bind_int(stmt, 1, course_id);
        sqlite3_bind_text(stmt, 2, student_grade.c_str(), -1, NULL);

        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result != SQLITE_DONE) {
            return -1;
        }
    }

    // All grades inserted, now mark course as graded

    char *sql = (char *)("UPDATE courses SET graded = 1 WHERE course_id = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 0, course_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return result == SQLITE_DONE ? 0 : -1;
}

