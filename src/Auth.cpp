#include "Auth.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/RecordSet.h>

const extern std::filesystem::path path_to_login_file = "/home/galaxygamerman/matrecomm_assignment/.login";

std::string getUserIdfromLoginFile() {
	std::ifstream login_file(path_to_login_file);
	if (login_file) {
		std::string user_id;
		std::getline(login_file, user_id);
		login_file.close();
		return user_id;
	}
	return "";
}

void addUserToDatabase(Poco::Data::Session& session, std::string& username, std::string& password){
		session << "INSERT INTO users (username, password_hash) VALUES('" + username + "', '" + password + "')", Poco::Data::Keywords::now;
		std::cout << "Added user: " << username << " with password: " << password << std::endl;
}

void loginUser(Poco::Data::Session& session, const std::string& username, const std::string& password) {
	// Check if user exists
	Poco::Data::Statement select(session);
	select << "SELECT user_id, password_hash FROM users WHERE username = '" + username + "'", Poco::Data::Keywords::now;
	Poco::Data::RecordSet rs(select);
	if (rs.rowCount() != 0) {
		std::string user_id = rs["user_id"].convert<std::string>();
		std::string stored_password = rs["password_hash"].convert<std::string>();
		if (password == stored_password) {
			// Create a file called .login that stored the userID and password
			std::ofstream output_file(path_to_login_file);
			// Check if the file was successfully opened for writing
			if (output_file.is_open()) {
				output_file << user_id << std::endl;
				output_file.close();
				std::cout << "Login successful for user: " << username << std::endl;
			} else {
				std::cerr << "Error: Unable to open file for writing." << std::endl;
				throw std::runtime_error("Unable to open login file for writing");
			}
		} else {
			std::cerr << "Incorrect password for user: " << username << std::endl;
		}
	} else {
		std::cerr << "User not found: " << username << std::endl;
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		throw std::runtime_error("User not found");
	}
}