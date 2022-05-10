#include "back/users.hpp"

#include <vector>
#include <string>
#include <map>

int pick(std::vector<std::string> &opts);
int pick(std::vector<std::pair<int, std::string>> &opts);
int pick(std::map<std::string, int> &opts);
int pick(std::map<std::string, std::pair<int, bool>> &opts);

void student_interface(Student student);
void faculty_interface(Faculty faculty);
void admin_interface(Admin admin);
