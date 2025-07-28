#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <Poco/Data/PostgreSQL/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>

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
    if (std::regex_match(task, std::regex("add(user)?"))) {
        task_code = ADD_USER;
    } else if (std::regex_match(task, std::regex("l(ogin)?"))) {
        task_code = LOGIN_USER;
    } else if (std::regex_match(task, std::regex("up(load)?"))) {
        task_code = UPLOAD_FILE;
    } else if (std::regex_match(task, std::regex("down(load)?"))) {
        task_code = DOWNLOAD_FILE;
    } else if (std::regex_match(task, std::regex("del(ete)?"))) {
        task_code = DELETE_FILE;
    } else if (std::regex_match(task, std::regex("list"))) {
        task_code = LIST_FILES;
    } else {
        task_code = SHOW_ERROR;
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
            session << "INSERT INTO users (username, password) VALUES(?, ?)",
                use(username), use(password), Poco::Data::Keywords::now;
        } else if (task_code == LOGIN_USER) {
            std::string& username = arg1 = argv[2];
            std::string& password = arg2 = argv[3];
            std::cout << "Logging in user: " << username << " with password: " << password << std::endl;
            // Check if user exists
            Poco::Data::Statement select(session);
            select << "SELECT user_id, password FROM users WHERE username = ?", use(username), Poco::Data::Keywords::now;
            Poco::Data::RecordSet rs(select);
            if (rs.rowCount() != 0) {
                std::string user_id = rs["user_id"].convert<std::string>();
                std::string stored_password = rs["password"].convert<std::string>();
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
            std::string& filename = arg1 = argv[2];
            std::string& user = arg2 = argv[3];
            std::cout << "Uploading file: " << filename << std::endl;
        } else if (task_code == DOWNLOAD_FILE) {
            std::string& file_id = arg1 = argv[2];
            std::string& user_id = arg2 = argv[3];
            std::cout << "Downloading file: " << file_id << " for user: " << user_id << std::endl;
        } else if (task_code == DELETE_FILE) {
            std::string& file_id = arg1 = argv[2];
            std::string& user_id = arg2 = argv[3];
            std::cout << "Deleting file: " << file_id << " for user: " << user_id << std::endl;
        } else if (task_code == LIST_FILES) {
            std::string& user_id = arg1 = argv[2];
            std::cout << "Listing files for user: " << user_id << std::endl;
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