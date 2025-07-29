#pragma once

#include <vector>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <Poco/File.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>

void uploadFile(Poco::Data::Session& session, std::string& user_id, std::string filename, const std::string& filepath);

void downloadFile(Poco::Data::Session& session, int file_id, const std::filesystem::path& save_path);

void deleteFile(Poco::Data::Session& session, std::string& user_id, std::string& file_id, std::string& file_name);

void listFiles(Poco::Data::Session& session, std::string& user_id);
