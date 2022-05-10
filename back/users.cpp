#include "users.hpp"
#include <sstream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <iostream>

std::set<std::string> possible_grades = {"FS", "FR", "D",  "C-", "C", "B-", "B",  "A-", "A",  "A+"};

User::User(const char *user_name, sqlite3 *db_p) {
    username = user_name;
    db = db_p;

    char *sql = (char *)("SELECT id FROM users WHERE username = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, user_name, -1, NULL);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        std::cout << result << "\n";
        std::cout << "User constructor given invalid username\n";
        throw;
    }
    id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
}

int User::get_user_type(sqlite3 *db, const char *user_name, USER_TYPE &type) {
    char *tables[] = {(char *) "students", (char *) "faculty", (char *) "admins"};

    for (int i = 0; i < 3; i++) {
        std::ostringstream ss;
        ss << "SELECT 1 FROM " << tables[i] << " INNER JOIN users ON " << tables[i] << ".id = users.id WHERE username = ?";
        std::string sql = ss.str();

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, user_name, -1, NULL);

        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result == SQLITE_ROW) {
            if (i == 0) {
                type = USER_STUDENT;
            } else if (i == 1) {
                type = USER_FACULTY;
            } else {
                type = USER_ADMIN;
            }
            return 0;
        } else if (result != SQLITE_DONE) {
            // sql error
            return -1;
        }
    }
    return 1;
}

int User::get_user_id(sqlite3 *db, const char *user_name, int &user_id) {
    char *sql = (char *)("SELECT id FROM users WHERE username = ?");

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, user_name, -1, NULL);

    int result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return result == SQLITE_DONE ? 1 : -1;
    }

    user_id = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return 0;
}

int User::get_id() {
    return id;
}

std::string User::get_username() {
    return username;
}

sqlite3 * User::db_p() {
    return db;
}
