#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include "Auth.h"
#include "Commands.h"
#include "Transmission.h"
#include "env.h" // Contains the connection string

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;
using Poco::Data::RecordSet;

int main(int argc, char* argv[]) {
	// Read arguments
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <command> <args...>" << std::endl;
		return 1;
	}
	std::string task = argv[1];

	task_t task_code = parse_task(task);

	// First check if .login file exists
	if (getUserIdfromLoginFile() == ""
		&& task_code != ADD_USER
		&& task_code != LOGIN_USER) {
		std::cerr << "No login file found. Please log in first using:" << std::endl
			<< '\t' << argv[0] << " adduser <username> <password>" << std::endl
			<< '\t' << argv[0] << " login <username> <password>" << std::endl;
		return 1; // Return an error code
	}
	// Register the connector once at the start
	Poco::Data::PostgreSQL::Connector::registerConnector();
	try {
		std::cout << ">>>> Connecting to File Server..." << std::endl;

		// 1. Create the session directly on the stack. No 'new' needed.
		Session session("PostgreSQL", CONNECTION_STRING());

		std::cout << ">>>> Successfully connected!" << std::endl;

		// Do the task
		if (task_code == ADD_USER) {
			std::string username = argv[2];
			std::string password = argv[3];
			addUserToDatabase(session, username, password);
			loginUser(session, username, password);
		} else if (task_code == LOGIN_USER) {
			std::string username = argv[2];
			std::string password = argv[3];
			loginUser(session, username, password);
		} else if (task_code == UPLOAD_FILE) {
			if (argc < 4) {
				std::cerr << "Usage: " << argv[0] << " upload <filename_after_upload> <path/to/file>" << std::endl;
				return 1;
			}
			std::string filename = argv[2];
			std::string filepath = argv[3];

			// Read user_id from .login file
			std::string user_id = getUserIdfromLoginFile();
			if (user_id.empty()) {
				std::cerr << "Error: User not logged in. Please log in first." << std::endl;
				return 1;
			}
			uploadFile(session, user_id, filename, filepath);
		} else if (task_code == DOWNLOAD_FILE) {
			if (argc < 4) {
				std::cerr << "Usage: " << argv[0] << " download <file_id> <path/to/save>" << std::endl;
				return 1;
			}
			int file_id_to_download = std::stoi(argv[2]);
			std::string save_path = argv[3];
			downloadFile(session, file_id_to_download, save_path);
		} else if (task_code == DELETE_FILE) {
			std::string file_id = argv[2];
			std::string user_id = getUserIdfromLoginFile();
			if (user_id.empty()) {
				std::cerr << "No login file found. Please log in first using:" << std::endl
					<< '\t' << argv[0] << " adduser <username> <password>" << std::endl
					<< '\t' << argv[0] << " login <username> <password>" << std::endl;
				return 1; // Return an error code
			}
			std::string file_name;
			deleteFile(file_id, user_id, session, file_name);
		} else if (task_code == LIST_FILES) {
			std::string user_id = getUserIdfromLoginFile();
			if (user_id.empty()) {
				std::cerr << "No login file found. Please log in first using:" << std::endl
					<< '\t' << argv[0] << " adduser <username> <password>" << std::endl
					<< '\t' << argv[0] << " login <username> <password>" << std::endl;
				return 1; // Return an error code
			}
			// Display all files uploaded by the user
			listFiles(session, user_id);
		} else {
			std::cerr << "Invalid command: " << task << std::endl;
			return 1;
		}
		// Unregister the connector before exiting
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
	} catch (const Poco::Exception& e) {    // Catches the specific Poco exceptions
		std::cerr << "Poco Error: " << e.displayText() << std::endl;
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "Standard Error: " << e.what() << std::endl;
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		return 1;
	} catch (...) {
		std::cerr << "Unknown Error occurred." << std::endl;
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		return 1;
	}
	return 0;
}
