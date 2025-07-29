# Bump the version for better target handling
cmake_minimum_required(VERSION 3.12)
project(MyPocoApp CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Step 1: Find PostgreSQL ---
message(STATUS "--> Searching for PostgreSQL...")
find_package(PostgreSQL REQUIRED)
message(STATUS "--> Found PostgreSQL with Includes: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "    and Libraries: ${PostgreSQL_LIBRARIES}")

# Check if the modern 'PostgreSQL::client' target was created by find_package.
# If not, we create it manually using the variables that were found.
if(NOT TARGET PostgreSQL::client)
	message(STATUS "--> 'PostgreSQL::client' target not found. Creating it manually.")
	
	# Create an INTERFACE library to group the usage requirements
	add_library(PostgreSQL::client INTERFACE IMPORTED)
	
	# Set the properties for the new target using the found variables
	target_include_directories(PostgreSQL::client INTERFACE ${PostgreSQL_INCLUDE_DIRS})
	target_link_libraries(PostgreSQL::client INTERFACE ${PostgreSQL_LIBRARIES})
else()
	message(STATUS "--> Modern 'PostgreSQL::client' target was found automatically.")
endif()

# --- Step 2: Find Poco ---
message(STATUS "--> Searching for Poco libraries...")
find_package(Poco REQUIRED Foundation Data DataPostgreSQL)
message(STATUS "--> Found Poco with Includes: ${Poco_INCLUDE_DIRS}")
message(STATUS "    and Libraries: ${Poco_LIBRARIES}")

# --- Step 3: Build Application ---
add_executable(app 
	src/main.cpp
	src/Auth.cpp
	src/Commands.cpp
	src/Transmission.cpp
)
target_link_libraries(app PRIVATE
	Poco::Foundation
	Poco::Data
	Poco::DataPostgreSQL
)

message(STATUS "--> CMake configuration finished successfully.")