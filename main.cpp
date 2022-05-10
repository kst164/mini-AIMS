#include <sqlite3.h>
#include <iostream>
#include <sstream>

#include "back/users.hpp"
#include "interfaces.hpp"

using namespace std;

// Creates a single admin, named "A01"
void create_admin(sqlite3 *db) {
    char *errmsg = NULL;

    sqlite3_exec(db, "INSERT INTO users (username) VALUES (\"A01\")", NULL, NULL, &errmsg);
    if (errmsg) {
        cout << errmsg << "\n";
    } else {
        cout << "Worked\n";
    }

    sqlite3_exec(db, "INSERT INTO admins (id) VALUES (1)", NULL, NULL, &errmsg);
    if (errmsg) {
        cout << errmsg << "\n";
    } else {
        cout << "Worked\n";
    }
}

// Pick a string, returns 1 + chosen index
//
// 0: go back
// else returns i => picked opts[i-1]
int pick(vector<string> &opts) {
    if (opts.size() == 0) {
        return 0;
    }

    while (1) {
        cout << "\n\n";
        for (int i = 1; i <= opts.size(); i++) {
            cout << i << ":  " << opts[i - 1] << "\n";
        }
        cout << "q:  quit\n";
        cout << "b:  go back\n";

        cout << "\nPick an option (0 to print opts again): ";
        string inp;
        cin >> inp;

        stringstream ss(inp);
        int choice;

        if ((ss >> choice).fail()) {
            if (inp == "q") {
                cout << "Exiting\n";
                exit(0);
            } else if (inp == "b") {
                return 0;
            } else {
                cout << "Not an option. Pick again\n";
                continue;
            }
        }
        if (choice < 0 || choice > opts.size()) {
            cout << "Not an option. Pick again\n";
            continue;
        }
        if (choice != 0) {
            return choice;
        }
    }
}

// Pick a string, return corresponding int (ignore index)
int pick(vector<pair<int, string>> &opts) {
    if (opts.size() == 0) {
        return 0;
    }

    while (1) {
        cout << "\n\n";
        for (int i = 1; i <= opts.size(); i++) {
            cout << i << ":  " << opts[i - 1].second << "\n";
        }
        cout << "q:  quit\n";
        cout << "b:  go back\n";

        cout << "\nPick an option (0 to print opts again): ";
        string inp;
        cin >> inp;

        for (auto course: opts) {
            if (course.second == inp) {
                return course.first;
            }
        }
        cout << "Not an option.\n";
    }
}

// asks user to pick a string, return corresponding int
int pick(map<string, int> &opts) {
    if (opts.size() == 0) {
        return 0;
    }

    while (1) {
        cout << "\n\n";
        for (auto it = opts.begin(); it != opts.end(); it++) {
            cout << it->first << "\n";
        }

        cout << "\nPick an option (0 to print opts again): ";
        string inp;
        cin >> inp;

        if (opts.find(inp) == opts.end()) {
            cout << "Not an option.\n";
        }
        return opts[inp];
    }
}

// asks user to pick a string, return corresponding int (ignore bool)
int pick(map<string, pair<int, bool>> &opts) {
    if (opts.size() == 0) {
        return 0;
    }

    while (1) {
        cout << "\n\n";
        for (auto it = opts.begin(); it != opts.end(); it++) {
            cout << it->first << "\n";
        }

        cout << "\nPick an option (0 to print opts again): ";
        string inp;
        cin >> inp;

        if (opts.find(inp) == opts.end()) {
            cout << "Not an option.\n";
        }
        return opts[inp].first;
    }
}

void interface(USER_TYPE type, sqlite3 *db, char *username) {
    if (type == USER_STUDENT) {
        cout << "STUDENT\n";
        Student student(username, db);
        student_interface(student);
    } else if (type == USER_FACULTY) {
        cout << "FACULTY\n";
        Faculty faculty(username, db);
        faculty_interface(faculty);
    } else if (type == USER_ADMIN) {
        cout << "ADMIN\n";
        Admin admin(username, db);
        admin_interface(admin);
    } else {
        cout << "NOTHING\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./a.out <username>\n";
        return 1;
    }

    char *db_name = (char *)("db.sqlite");
    sqlite3 *db;
    sqlite3_open(db_name, &db);
    sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL); // Not enabled by default in sqlite

    USER_TYPE type;

    int r = User::get_user_type(db, argv[1], type);

    if (r == 1) {
        cout << "Invalid username\n";
        return 1;
    } else if (r == -1) {
        cout << "SQL ERROR\n";
        return 1;
    }

    interface(type, db, argv[1]);

    sqlite3_close(db);
    return 0;
}
