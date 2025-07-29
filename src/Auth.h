#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <Poco/Data/Session.h>

const extern std::filesystem::path path_to_login_file;

std::string getUserIdfromLoginFile();

void addUserToDatabase(Poco::Data::Session& session, std::string& username, std::string& password);

void loginUser(Poco::Data::Session& session, const std::string& username, const std::string& password);
