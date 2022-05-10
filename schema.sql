PRAGMA foreign_keys = ON;

CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL
);

CREATE TABLE students (
    id INTEGER PRIMARY KEY,
    FOREIGN KEY(id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE faculty (
    id INTEGER PRIMARY KEY,
    FOREIGN KEY(id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE admins (
    id INTEGER PRIMARY KEY,
    FOREIGN KEY(id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE TABLE courses (
    course_id INTEGER PRIMARY KEY AUTOINCREMENT,
    course_code CHAR(6) NOT NULL,
    instructor_id INTEGER NOT NULL,
    floated INTEGER NOT NULL CHECK (floated IN (0, 1)),
    graded INTEGER NOT NULL CHECK (graded IN (0, 1)),
    CHECK(floated = 0 OR graded = 0),
    FOREIGN KEY(instructor_id) REFERENCES faculty(faculty_id) ON DELETE CASCADE
);

CREATE TABLE registered (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id INTEGER,
    course_id INTEGER,
    UNIQUE(student_id, course_id),
    FOREIGN KEY (student_id) REFERENCES students(id) ON DELETE CASCADE,
    FOREIGN KEY (course_id) REFERENCES course(id) ON DELETE CASCADE
);

CREATE TABLE grades (
    reg_id INTEGER PRIMARY KEY,
    grade VARCHAR(2) CHECK (grade IN ("A+", "A", "A-", "B+", "B-", "C", "C-", "D", "FR", "FS")),
    FOREIGN KEY (reg_id) REFERENCES registered(id) ON DELETE CASCADE
);
