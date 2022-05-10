#include "users.hpp"
#include <string>
#include <utility>

int Student::reg(sqlite3 *db, int student_id, int course_id) {
    // register for course if course exists and is not floated
    char *sql2 = (char *)("INSERT INTO registered (student_id, course_id) SELECT ?SID, ?CID "
                "WHERE EXISTS (SELECT 1 FROM courses WHERE course_id = ?CID AND floated = 0)");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql2, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_int(stmt, 2, course_id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result == SQLITE_CONSTRAINT) {
        // Already registered
        return 1;
    }
    if (result != SQLITE_DONE) {
        // sql error
        return -1;
    }

    // if no changes, such a floated course does not exist
    return sqlite3_changes(db) ? 0 : 2;
}

int Student::dereg(sqlite3 *db, int student_id, int course_id) {
    // dereg if course exists and is not graded
    char *sql = (char *)("DELETE FROM registered WHERE student_id = ?SID AND course_id = ?CID "
            "WHERE EXISTS (SELECT 1 FROM courses WHERE course_id = ?CID AND graded = 0)");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, student_id);
    sqlite3_bind_int(stmt, 2, course_id);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        // sql error
        return -1;
    }

    // if no changes, then didn't delete anything
    return sqlite3_changes(db) ? 0 : 1;
}

int Student::list_courses(sqlite3 *db, std::map<std::string, std::pair<int, bool>> &courses) {
    courses.clear();
    char *sql = (char *)("SELECT course_code, course_id, floated FROM courses");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    int result = sqlite3_step(stmt);

    while (result == SQLITE_ROW) {
        std::string code = (char *) sqlite3_column_text(stmt, 0);
        int id = sqlite3_column_int(stmt, 1);
        bool floated = sqlite3_column_int(stmt, 1);
        courses[code] = {id, floated};
    }
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE ? 0 : -1;
}

int Student::list_floated_courses(sqlite3 *db, std::map<std::string, int> &floated_courses) {
    std::map<std::string, std::pair<int, bool>> all_courses;
    if (list_courses(db, all_courses)) {
        return -1;
    }

    for (auto it = all_courses.begin(); it != all_courses.end(); it++) {
        if (it->second.second) {
            continue;
        }
        floated_courses[it->first] = it->second.first;
    }
    return 0;
}

int Student::list_unfloated_courses(sqlite3 *db, std::map<std::string, int> &floated_courses) {
    std::map<std::string, std::pair<int, bool>> all_courses;
    if (list_courses(db, all_courses)) {
        return -1;
    }

    for (auto it = all_courses.begin(); it != all_courses.end(); it++) {
        if (!it->second.second) {
            continue;
        }
        floated_courses[it->first] = it->second.first;
    }
    return 0;
}

int Student::reg(int course_id) {
    return Student::reg(db, id, course_id);
}

int Student::dereg(int course_id) {
    return Student::reg(db, id, course_id);
}

int Student::list_reg(std::vector<std::pair<int, std::string>>& courses) {
    courses.clear();

    char *sql = (char *)("SELECT course_id, course_code FROM courses WHERE course_id IN "
            "(SELECT course_id FROM registered WHERE student_id = ?)");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);

    int result = sqlite3_step(stmt);
    while (result == SQLITE_ROW) {
        auto first = sqlite3_column_int(stmt, 0);
        auto second = (char *) sqlite3_column_text(stmt, 1);
        courses.push_back(std::make_pair(first, second));

        result = sqlite3_step(stmt);
    }
    
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE ? 0 : -1;
}

int Student::check_grade(int course_id, std::string &grade) {
    char *sql = (char *)("SELECT grade FROM grades WHERE reg_id IN "
            "(SELECT id FROM registered WHERE student_id = ? AND course_id = ?)");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_int(stmt, 2, course_id);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        // either empty, or sql error
        sqlite3_finalize(stmt);
        return result == SQLITE_DONE ? 1 : -1;
    }

    grade = (char *) sqlite3_column_text(stmt, 0);
    sqlite3_finalize(stmt);

    return 0;
}
