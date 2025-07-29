#include "Transmission.h"

#include <vector>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <Poco/File.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/LOB.h>

using namespace Poco::Data::Keywords;

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

		std::cout << "Successfully uploaded file as '" << filename << "' from path: " << filepath << std::endl;
	} catch (const Poco::Exception& e) {
		std::cerr << "Database error: " << e.displayText() << std::endl;
	}
}

void downloadFile(Poco::Data::Session& session, int file_id, const std::filesystem::path& save_path) {
	try {
		// --- 1. Get the file's name from its metadata ---
		std::string filename;
		Poco::Data::Statement select_meta(session);
		select_meta << "SELECT file_name FROM uploaded_files WHERE file_id = $1",
			use(file_id),
			into(filename),
			now;

		if (filename.empty()) {
			std::cerr << "Error: File with ID " << file_id << " not found." << std::endl;
			return;
		}

		std::filesystem::path full_save_path = std::filesystem::path(save_path) / filename;
		std::cout << "Downloading file to: " << full_save_path << std::endl;

		std::filesystem::create_directories(std::filesystem::path(save_path));
		// --- 2. Open the local output file for writing in binary mode ---
		std::ofstream output_file(full_save_path, std::ios::binary);
		if (!output_file.is_open()) {
			std::cerr << "Error: Could not create file at " << full_save_path << std::endl;
			return;
		}

		// --- 3. Retrieve all file chunks in the correct order ---
		Poco::Data::Statement select_chunks(session);
		select_chunks << "SELECT chunk_data FROM file_chunks WHERE file_id = $1 ORDER BY file_chunks.chunk_id ASC",
			use(file_id);
		select_chunks.execute();

		Poco::Data::RecordSet rs(select_chunks);
		if (rs.rowCount() == 0) {
			std::cerr << "Error: No data found for file ID " << file_id << ". The file may be empty or corrupt." << std::endl;
			return;
		}

		// --- 4. Loop through the chunks and write them to the local file ---
		for (auto& row : rs) {
			// --- FIX: Extract the raw binary data into a std::string ---
			// The database driver is likely returning the BYTEA data as a raw string.
			std::string chunk_data = row["chunk_data"].convert<std::string>();

			// Write the contents of the string directly to the output file.
			output_file.write(chunk_data.data(), chunk_data.size());
		}

		output_file.close();
		std::cout << "✅ File downloaded successfully." << std::endl;

	} catch (const Poco::Exception& e) {
		std::cerr << "Database error during download: " << e.displayText() << std::endl;
	}
}

void deleteFile(std::string& file_id, std::string& user_id, Poco::Data::Session& session, std::string& file_name) {
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
}

void listFiles(Poco::Data::Session& session, std::string& user_id) {
	using namespace Poco::Data::Keywords;

	// 1. Prepare the statement
	Poco::Data::Statement select(session);
	select << "SELECT file_id, file_name, file_size FROM uploaded_files WHERE user_id = $1",
		use(user_id);

	// 2. Execute the statement.
	select.execute();

	// 3. Create a RecordSet, which fetches all results from the query.
	Poco::Data::RecordSet rs(select);

	if (rs.rowCount() == 0) {
		std::cout << "No files found for user ID " << user_id << "." << std::endl;
		return;
	}

	std::cout << "--- Files for User ID: " << user_id << " ---" << std::endl;

	// 4. Iterate through the RecordSet and print each row's data.
	for (auto& row : rs) {
		int file_id = row["file_id"].convert<int>();
		std::string file_name = row["file_name"].convert<std::string>();
		Poco::Int64 file_size = row["file_size"].convert<Poco::Int64>();

		std::cout << "ID: " << file_id
			<< ", Name: " << file_name
			<< ", Size: " << file_size << " bytes" << std::endl;
	}
}