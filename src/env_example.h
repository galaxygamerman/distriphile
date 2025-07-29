#pragma once

#include <string>

const extern std::string HOST = "your-hostname-or-ip";
const extern std::string PORT = "5433";
const extern std::string USER = "admin";
const extern std::string PASSWORD = "your-db-password";
const extern std::string DBNAME = "your-db-name";
const extern std::string SSLMODE = "verify-full";
const extern std::string SSLROOTCERT = "/absolute/path/to/your/root.crt";

std::string CONNECTION_STRING() {
	return "host=" + HOST
		+ " port=" + PORT
		+ " user=" + USER
		+ " password=" + PASSWORD
		+ " dbname=" + DBNAME
		+ " sslmode=" + SSLMODE
		+ " sslrootcert=" + SSLROOTCERT;
}