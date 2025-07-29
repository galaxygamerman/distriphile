#include "Commands.h"
#include <regex>

task_t parse_task(const std::string& task){
if (task == "adduser") {
	return ADD_USER;
} else if (task == "login") {
	return LOGIN_USER;
} else if (std::regex_match(task, std::regex("up(load)?"))) {
	return UPLOAD_FILE;
} else if (std::regex_match(task, std::regex("down(load)?"))) {
	return DOWNLOAD_FILE;
} else if (std::regex_match(task, std::regex("del(ete)?"))) {
	return DELETE_FILE;
} else if (std::regex_match(task, std::regex("l(ist)?"))) {
	return LIST_FILES;
} else {
	return SHOW_ERROR;
}
}