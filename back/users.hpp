#ifndef _USERS_HPP
#define _USERS_HPP

#include <string>
#include <vector>
#include <set>
#include <map>
#include <utility>

#include <sqlite3.h>

extern std::set<std::string> possible_grades;

// in the form {"A+", 10}
typedef std::pair<std::string, int> grade_t;

enum USER_TYPE {
    USER_STUDENT,
    USER_FACULTY,
    USER_ADMIN
};

class User {

protected:
    int id;
    std::string username;
    sqlite3 *db;

public:
    User(const char *user_name, sqlite3 *db_p);

    // 0: success
    // 1: invalid username
    // -1: other error
    static int get_user_type(sqlite3 *db, const char *user_name, USER_TYPE &type);

    // 0: success
    // 1: invalid username
    // -1: other error
    static int get_user_id(sqlite3 *db, const char *user_name, int &user_id);

    int get_id();
    std::string get_username();

    sqlite3 *db_p();

};

class Student: public User {
public:
    using User::User;

    // 0: success
    // 1: already registered
    // 2: invalid course/student / course not floated
    // -1: other error
    static int reg(sqlite3 *db, int student_id, int course_id);

    // 0: success
    // 1: invalid course/student / already graded / never registered
    // -1: other error
    static int dereg(sqlite3 *db, int student_id, int course_id);

    // 0: success
    // -1: other error
    static int list_courses(sqlite3 *db, std::map<std::string, std::pair<int, bool>>& courses);

    // 0: success
    // -1: other error
    static int list_floated_courses(sqlite3 *db, std::map<std::string, int>& courses);

    // 0: success
    // -1: other error
    static int list_unfloated_courses(sqlite3 *db, std::map<std::string, int>& courses);

    // 0: success
    // 1: already registered
    // 2: invalid course/student / course not floated
    // -1: other error
    int reg(int course_id);

    // 0: success
    // 1: invalid course/student / already graded / never registered
    // -1: other error
    int dereg(int course_id);

    // 0: success
    // -1: unknown error, courses may be incomplete/empty
    //
    // courses: Vec<(course_code, course_id)>
    // courses is cleared and refilled
    int list_reg(std::vector<std::pair<int, std::string>>& courses);

    // 0: success, "A+" to "FR"/"FS" written to grade
    // 1: invalid course / not registered / not graded
    // -1: other error
    int check_grade(int course_id, std::string &grade);
};

class Admin: public User {
public:
    using User::User;

    // 0: success
    // 1: faculty does not exist
    int add_course(char name[6], int faculty_id);

    // 0: success
    // 1: course does not exist
    int del_course(int course_id);

    // 0: success
    // 1: course does not exist
    int is_floated(int course_id, bool &floated);

    // 0: success
    // 1: course does not exist
    int set_floated(int course_id, bool floated);

    // 0: success
    // 1: invalid user_type (not in {0, 1, 2})
    // 2: username already exists
    //
    // user_type: 
    //  0: student
    //  1: faculty
    //  2: admin
    int add_user(const char *username, USER_TYPE user_type);

    // 0: success
    // 1: student doesn't exist
    int del_user(char *username);
    int del_user(int user_id);

    // 0: success
    // 1: already registered
    // 2: invalid course/student / course not floated
    // 3: other error
    int reg_student(int student_id, int course_id);

    // 0: success
    // 1: invalid course/student / already graded / never registered
    // 2: other error
    int dereg_student(int student_id, int course_id);
};

class Faculty: public User {
public:
    using User::User;

    // 0: success
    // -1: unknown error, courses may be incomplete/empty
    //
    // courses: Vec<(course_code, course_id)>
    // courses is cleared and refilled
    // only non-graded courses are shown
    int teaching_courses(std::vector<std::pair<int, std::string>>& courses);

    // 0: success
    // 1: course doesn't exist
    // -1: unknown error
    //
    // output written to instructor
    int get_instructor_id(int course_id, int &instructor);

    // 0: success
    // 1: course doesn't exist
    // -1: unknown error
    //
    // output written to graded
    int check_graded(int course_id, bool &graded);

    // 0: success
    // 1: course doesn't exist
    // 2: course not taught by instructor
    // -1: unknown error
    //
    // students is cleared and refilled
    // Vec<(student_id, username)>
    int list_reg_students(int course_id, std::vector<std::pair<int, std::string>>& students);

    // grades: maps student_id to grade ("A+" to "FS")
    // 
    // Return value:
    // 0: success
    // 1: course doesn't exist
    // 2: course not taught by instructor
    // 3: course already graded
    // 4: `grades` is not equal to list of students
    // 5: invalid grade found
    // -1: unknown error
    int grade_course(int course_id, std::map<int, std::string>& grades);
};

#endif
