#include "interfaces.hpp"
#include "back/users.hpp"
#include <iostream>
#include <map>

using namespace std;

int pick_floated(sqlite3 *db) {
    map<string, int> floated_courses;
    Student::list_floated_courses(db, floated_courses);
    return pick(floated_courses);
}

int pick_unfloated(sqlite3 *db) {
    map<string, int> unfloated_courses;
    Student::list_unfloated_courses(db, unfloated_courses);
    return pick(unfloated_courses);
}

int pick_course(sqlite3 *db) {
    map<string, pair<int, bool>> courses;
    Student::list_courses(db, courses);
    return pick(courses);
}

void student_interface(Student student) {
    vector<string> opts = {"register for a course", "deregister for a course", "view registered courses", "check course grade"};
    while (1) {
        int choice = pick(opts);
        if (choice == 0) {
            return;
        } else if (choice == 1) {
            student.reg(pick_floated(student.db_p()));
        } else if (choice == 2) {
            student.dereg(pick_unfloated(student.db_p()));
        } else if (choice == 3) {
            vector<pair<int, string>> regd_courses;
            student.list_reg(regd_courses);
            for (auto course : regd_courses) {
                cout << course.second << "\n";
            }
        } else if (choice == 4) {
            vector<pair<int, string>> regd_courses;
            student.list_reg(regd_courses);
            int course_id = pick(regd_courses);
            
            string grade;
            if (student.check_grade(course_id, grade)) {
                cout << "Couldn't check grade. It is probably not graded yet.\n";
            }
            cout << "Your grade was " << grade << ".\n";
        }
    }
}

void faculty_interface(Faculty faculty) {
    vector<string> opts = {"view teaching courses (ungraded)", "list students registered for course", "grade course"};

    while (1) {
        int choice = pick(opts);

        if (choice == 0) {
            break;
        }

        if (choice == 1) {
            vector<pair<int, string>> courses;
            faculty.teaching_courses(courses);
            for (auto course : courses) {
                cout << course.second << "\n";
            }
        } else if (choice == 2) {
            vector<pair<int, string>> courses;
            int course_id = pick(courses);
            vector<pair<int, string>> regd_students;
            faculty.list_reg_students(course_id, regd_students);
            for (auto student : regd_students) {
                cout << student.second << "\n";
            }
        } else {
            vector<pair<int, string>> courses;
            int course_id = pick(courses);
            vector<pair<int, string>> regd_students;
            faculty.list_reg_students(course_id, regd_students);

            map<int, string> grades;
            for (auto student : regd_students) {
                cout << "Enter grade for " << student.second << ":\n";
                string grade;
                cin >> grade;
                while (possible_grades.find(grade) != possible_grades.end()) {
                    cout << "Invalid grade.\n";
                    cout << "Grade must be one of A+, A, A-, B, B-, C, C-, D, FR, FS\n";
                    cout << "Enter grade for " << student.second << ":\n";
                    cin >> grade;
                }
                grades[student.first] = grade;
            }
            faculty.grade_course(course_id, grades);
        }
    }
}

void admin_interface(Admin admin) {
    vector<string> opts = {"add course", "delete course", "set course floating", "check if course is floating",
        "add user", "delete user", "register student for course", "deregister student from course"};
    while (1) {
        int choice = pick(opts);
        if (choice == 1) {
            string course_code;
            string faculty_name;
            cout << "Enter course code\n";
            cin >> course_code;
            cout << "Enter faculty username\n";
            cin >> faculty_name;

            USER_TYPE user_type;
            if (User::get_user_type(admin.db_p(), faculty_name.c_str(), user_type)) {
                cout << "Invalid username.\n";
                continue;
            }
            if (user_type != USER_FACULTY) {
                cout << "Username is not of a faculty member.\n";
                continue;
            }
            int faculty_id;
            User::get_user_id(admin.db_p(), faculty_name.c_str(), faculty_id);
            admin.add_course((char *) course_code.c_str(), faculty_id);
        } else if (choice == 2) {
            admin.del_course(pick_course(admin.db_p()));
        } else if (choice == 3) {
            admin.set_floated(pick_unfloated(admin.db_p()), true);
        } else if (choice == 4) {
            bool is_floating;
            admin.is_floated(pick_course(admin.db_p()), is_floating);
            cout << (is_floating ? "Floating\n" : "Not floating\n");
        } else if (choice == 5) {
            vector<string> opts = {"student", "faculty", "admin"};
            int choice = pick(opts);
            USER_TYPE type;
            if (choice == 1) {
                type = USER_STUDENT;
            } else if (choice == 2) {
                type = USER_FACULTY;
            } else {
                type = USER_ADMIN;
            }
            string username;
            cout << "Enter username:\n";
            cin >> username;
            if (admin.add_user(username.c_str(), type)) {
                cout << "user already exists.\n";
            }
        } else if (choice == 6) {
            string username;
            cout << "Enter username:\n";
            cin >> username;

            if (admin.del_user((char *) username.c_str())) {
                cout << "Invalid username.\n";
                continue;
            }
        } else if (choice == 7) {
            int course_id = pick_floated(admin.db_p());

            string username;
            cout << "Enter student username:\n";
            cin >> username;

            USER_TYPE user_type;
            if (User::get_user_type(admin.db_p(), username.c_str(), user_type)) {
                cout << "Invalid username.\n";
                continue;
            }
            if (user_type != USER_STUDENT) {
                cout << "Username is not of a student.\n";
                continue;
            }
            int student_id;
            User::get_user_id(admin.db_p(), username.c_str(), student_id);
            int res =  admin.reg_student(student_id, course_id);
            if (res == 1) {
                cout << "Student is already registered for course.\n";
            } else if (res == 2) {
                cout << "Course is not floated.\n";
            } else {
                cout << "Student registered for course.\n";
            }
        } else if (choice == 8) {
            int course_id = pick_floated(admin.db_p());

            string username;
            cout << "Enter student username:\n";
            cin >> username;

            USER_TYPE user_type;
            if (User::get_user_type(admin.db_p(), username.c_str(), user_type)) {
                cout << "Invalid username.\n";
                continue;
            }
            if (user_type != USER_STUDENT) {
                cout << "Username is not of a student.\n";
                continue;
            }
            int student_id;
            User::get_user_id(admin.db_p(), username.c_str(), student_id);
            int res =  admin.reg_student(student_id, course_id);
            if (res == 1) {
                cout << "Course is already graded / Student is already not registered for course.\n";
            } else {
                cout << "Student deregistered from course.\n";
            }
        }
    }
}

