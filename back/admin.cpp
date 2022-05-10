#include "users.hpp"
#include <string>

int Admin::add_course(char name[7], int faculty_id) {
    char *sql = (char *)("INSERT INTO courses (course_code, instructor_id, floated, graded) "
                        "VALUES (?, ?, 0, 0)");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, name, -1, NULL);
    sqlite3_bind_int(stmt, 2, faculty_id);

    int result = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (result == SQLITE_DONE) {
        return 0;
    } else {
        // Most likely SQLITE_CONSTRAINT, but could be something else
        return 1;
    }
}

int Admin::del_course(int course_id) {
    char *sql = (char *)("DELETE FROM courses WHERE course_id = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, course_id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // If changes were made, then success
    return sqlite3_changes(db) ? 0 : 1;
}

// 0: success
// 1: course does not exist
int Admin::is_floated(int course_id, bool &floated) {
    char *sql = (char *)("SELECT floated FROM courses WHERE course_id = ? LIMIT 1");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, course_id);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        // Most likely no such course
        return 1;
    }

    floated = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    return 0;
}

// 0: success
// 1: course does not exist / already graded
int Admin::set_floated(int course_id, bool floated) {
    char *sql = (char *)("UPDATE courses SET floated = ? WHERE course_id = ? AND graded = 0");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, floated);
    sqlite3_bind_int(stmt, 2, course_id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqlite3_changes(db) ? 0 : 1;
}

// 0: success
// 1: invalid user_type
// 2: username already exists
//
// user_type: 
//  0: student
//  1: faculty
//  2: admin
int Admin::add_user(const char *username, USER_TYPE user_type) {
    if (user_type < 0 || user_type > 2) {
        return 1;
    }

    char *sql1 = (char *)("INSERT INTO users (username) VALUES (?) RETURNING (id)");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql1, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, NULL);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        return 2;
    }
    int user_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    std::string sql2 = "INSERT INTO ";
    if (user_type == USER_STUDENT) {
        sql2 += "students";
    } else if (user_type == USER_FACULTY) {
        sql2 += "faculty";
    } else {
        sql2 += "admins";
    }
    sql2 += " VALUES (?)";

    sqlite3_prepare_v2(db, sql2.c_str(), -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0;
}

// 0: success
// 1: student doesn't exist
int Admin::del_user(char *username) {
    char *sql1 = (char *)("SELECT id FROM users WHERE username = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql1, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, NULL);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        return 1;
    }
    int user_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    return del_user(user_id);
}
int Admin::del_user(int user_id) {
    char *sql = (char *)("DELETE FROM users WHERE id = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, user_id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return sqlite3_changes(db) ? 0 : 1;
}

// 0: success
// 1: already registered
// 2: invalid course/student / course not floated
// 3: other error
int Admin::reg_student(int student_id, int course_id) {
    return Student::reg(db, student_id, course_id);
}

// 0: success
// 1: invalid course/student / already graded / never registered
// 2: other error
int Admin::dereg_student(int student_id, int course_id) {
    return Student::dereg(db, student_id, course_id);
}
