#pragma once

#include <string>

enum task_t {
	ADD_USER,
	LOGIN_USER,
	UPLOAD_FILE,
	DOWNLOAD_FILE,
	DELETE_FILE,
	LIST_FILES,
	SHOW_ERROR,
};

task_t parse_task(const std::string& task);