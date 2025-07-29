#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/LOB.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

enum task_t {
	ADD_USER,
	LOGIN_USER,
	UPLOAD_FILE,
	DOWNLOAD_FILE,
	DELETE_FILE,
	LIST_FILES,
	SHOW_ERROR,
};

const std::string CONNECTION_STRING =
"host=asia-south1.509ecc4c-201c-4248-a364-c059af51f5c4.gcp.yugabyte.cloud port=5433 user=admin password=gcW10R2_HiftI07pc-D0TzghDVs1mp dbname=yugabyte sslmode=verify-full sslrootcert=root.crt";

#include <vector>
#include <cmath>
#include <Poco/File.h>

void uploadFile(Poco::Data::Session& session, std::string& user_id, std::string filename, const std::string& filepath) {
	// 1. Open the file in binary mode to read its contents
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filepath << std::endl;
		return;
	}

	// 2. Read the entire file into a memory buffer
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<unsigned char> buffer(size);
	if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
		std::cerr << "Error: Could not read file." << std::endl;
		return;
	}
	file.close();

	try {
		using namespace Poco::Data::Keywords;

		int user_id_int = std::stoi(user_id);
		Poco::Int64 size64 = static_cast<Poco::Int64>(size);
		// 3. Prepare the SQL statement with placeholders
		Poco::Data::Statement insert_file(session);
		insert_file << "INSERT INTO uploaded_files (user_id, file_name, file_size) VALUES ($1, $2, $3)",
			use(user_id_int),
			use(filename),
			use(size64),
			now;

		int file_id;
		session << "SELECT file_id FROM uploaded_files WHERE user_id = $1 AND file_name = $2",
			use(user_id_int),
			use(filename),
			into(file_id),
			now;

		Poco::Data::BLOB blob(buffer.data());

		Poco::Data::Statement insert_binary(session);
		insert_binary << "INSERT INTO file_chunks (file_id, chunk_data) VALUES ($1, $2)",
			// 4. Bind the filename string to the first placeholder
			use(file_id),
			// 5. Bind the binary data from the buffer as a BLOB
			use(blob);
		// 6. Execute the statement
		insert_binary.execute();

		std::cout << "Successfully inserted image: " << filepath << std::endl;
	} catch (const Poco::Exception& e) {
		std::cerr << "Database error: " << e.displayText() << std::endl;
	}
}

int main(int argc, char* argv[]) {
	// Read arguments
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <command> <args...>" << std::endl;
		return 1;
	}
	std::string task = argv[1];
	std::string arg1;
	std::string arg2;

	task_t task_code;
	// Identify the task based on the first argument
	if (task == "adduser") {
		task_code = ADD_USER;
	} else if (task == "login") {
		task_code = LOGIN_USER;
	} else if (std::regex_match(task, std::regex("up(load)?"))) {
		task_code = UPLOAD_FILE;
	} else if (std::regex_match(task, std::regex("down(load)?"))) {
		task_code = DOWNLOAD_FILE;
	} else if (std::regex_match(task, std::regex("del(ete)?"))) {
		task_code = DELETE_FILE;
	} else if (std::regex_match(task, std::regex("l(ist)?"))) {
		task_code = LIST_FILES;
	} else {
		task_code = SHOW_ERROR;
	}

	// First check if ".login" file exists
	std::ifstream login_file(".login");
	if (login_file) {
		std::string user_id;
		std::getline(login_file, user_id);
		login_file.close();
		std::cout << "User ID: " << user_id << std::endl;   // Debugging output
	} else if (task_code != ADD_USER && task_code != LOGIN_USER) {
		std::cerr << "No login file found. Please log in first using:" << std::endl
			<< '\t' << argv[0] << " adduser <username> <password>" << std::endl
			<< '\t' << argv[0] << " login <username> <password>" << std::endl;
		return 1; // Return an error code
	}
	// Register the connector once at the start
	Poco::Data::PostgreSQL::Connector::registerConnector();
	try {
		// std::cout << "Using connection string: " << CONNECTION_STRING << std::endl;
		std::cout << ">>>> Connecting to File Server..." << std::endl;

		// 1. Create the session directly on the stack. No 'new' needed.
		Session session("PostgreSQL", CONNECTION_STRING);

		std::cout << ">>>> Successfully connected!" << std::endl;

		// Do the task
		if (task_code == ADD_USER) {
			std::string& username = arg1 = argv[2];
			std::string& password = arg2 = argv[3];
			std::cout << "Adding user: " << username << " with password: " << password << std::endl;
			session << "INSERT INTO users (username, password_hash) VALUES('" + username + "', '" + password + "')", Poco::Data::Keywords::now;
		} else if (task_code == LOGIN_USER) {
			std::string& username = arg1 = argv[2];
			std::string& password = arg2 = argv[3];
			std::cout << "Logging in user: " << username << " with password: " << password << std::endl;
			// Check if user exists
			Poco::Data::Statement select(session);
			select << "SELECT user_id, password_hash FROM users WHERE username = '" + username + "'", Poco::Data::Keywords::now;
			Poco::Data::RecordSet rs(select);
			if (rs.rowCount() != 0) {
				std::string user_id = rs["user_id"].convert<std::string>();
				std::string stored_password = rs["password_hash"].convert<std::string>();
				if (password == stored_password) {
					// Create a file called .login that stored the userID and password
					std::ofstream output_file(".login");
					// Check if the file was successfully opened for writing
					if (output_file.is_open()) {
						// Write the two strings, each on a new line
						output_file << user_id << std::endl;
						// Close the file
						output_file.close();
						std::cout << "Login successful for user: " << username << std::endl;
					} else {
						std::cerr << "Error: Unable to open file for writing." << std::endl;
						return 1; // Return an error code
					}
				} else {
					std::cerr << "Incorrect password for user: " << username << std::endl;
				}
			} else {
				std::cerr << "User not found: " << username << std::endl;
				Poco::Data::PostgreSQL::Connector::unregisterConnector();
				return 1;
			}
		} else if (task_code == UPLOAD_FILE) {
			if (argc < 4) {
				std::cerr << "Usage: " << argv[0] << " upload <filename_after_upload> <path/to/file>" << std::endl;
				return 1;
			}
			std::string& filename = arg1 = argv[2];
			std::string& filepath = arg2 = argv[3];

			// Read user_id from .login file
			std::ifstream login_file(".login");
			std::string user_id;
			if (login_file >> user_id) {
				std::cout << "Uploading file: " << filename << " from path: " << filepath << std::endl;
				uploadFile(session, user_id, filename, filepath);
			} else {
				std::cerr << "Error: User not logged in. Please log in first." << std::endl;
				return 1;
			}
		} else if (task_code == DOWNLOAD_FILE) {
			std::string& file_id = arg1 = argv[2];
			std::string& user_id = arg2 = argv[3];
			std::cout << "Downloading file: " << file_id << " for user: " << user_id << std::endl;
		} else if (task_code == DELETE_FILE) {
			std::string& file_id = arg1 = argv[2];
			std::string user_id;
			std::ifstream login_file(".login");
			if (login_file) {
				std::getline(login_file, user_id);
				login_file.close();
				std::cout << "User ID: " << user_id << std::endl;   // Debugging output
			} else if (task_code != ADD_USER && task_code != LOGIN_USER) {
				std::cerr << "No login file found. Please log in first using:" << std::endl
					<< '\t' << argv[0] << " adduser <username> <password>" << std::endl
					<< '\t' << argv[0] << " login <username> <password>" << std::endl;
				exit(1); // Return an error code
			}
			std::string file_name;
			std::cout << "Deleting file: " << file_id << " for user: " << user_id << std::endl;
			session << "SELECT file_name FROM uploaded_files WHERE file_id = $1 AND user_id = $2",
				Poco::Data::Keywords::use(file_id),
				Poco::Data::Keywords::use(user_id),
				Poco::Data::Keywords::into(file_name),
				Poco::Data::Keywords::now;
			session << "DELETE FROM uploaded_files WHERE file_id = $1 AND user_id = $2",
				Poco::Data::Keywords::use(file_id),
				Poco::Data::Keywords::use(user_id),
				Poco::Data::Keywords::now;
			session << "DELETE FROM file_chunks WHERE file_id = $1",
				Poco::Data::Keywords::use(file_id),
				Poco::Data::Keywords::now;
			std::cout << "Successfully deleted " << file_name << " of file_id: " << file_id << std::endl;
		} else if (task_code == LIST_FILES) {
			std::string user_id;
			std::ifstream login_file(".login");
			if (login_file) {
				std::getline(login_file, user_id);
				login_file.close();
				std::cout << "User ID: " << user_id << std::endl;   // Debugging output
			} else if (task_code != ADD_USER && task_code != LOGIN_USER) {
				std::cerr << "No login file found. Please log in first using:" << std::endl
					<< '\t' << argv[0] << " adduser <username> <password>" << std::endl
					<< '\t' << argv[0] << " login <username> <password>" << std::endl;
				exit(1); // Return an error code
			}
			// Display all files uploaded by the user
			std::cout << "Listing files for user: " << user_id << std::endl;
			std::string output;
			session << "SELECT * FROM uploaded_files WHERE user_id = $1",
				Poco::Data::Keywords::use(user_id),
				Poco::Data::Keywords::into(output),
				Poco::Data::Keywords::now;
			std::cout << output << std::endl; // Display the output
		} else {
			std::cerr << "Invalid command: " << task << std::endl;
			return 1;
		}
		// Unregister the connector before exiting
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		return 0;
	} catch (const Poco::Exception& e) {    // Catches the specific Poco exceptions
		std::cerr << "Poco Error: " << e.displayText() << std::endl;
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		return 1;
	} catch (const std::exception& e) {
		std::cerr << "Standard Error: " << e.what() << std::endl;
		Poco::Data::PostgreSQL::Connector::unregisterConnector();
		return 1;
	}

}