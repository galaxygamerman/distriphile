cmake_minimum_required(VERSION 3.10)
project(MyPocoApp CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Diagnostic Step 1: Find PostgreSQL with verbose output ---
message(STATUS "--> Searching for PostgreSQL...")
# We make it optional here so we can write our own error message
find_package(PostgreSQL)

# --- Diagnostic Step 2: Check if it was found and print details ---
if(PostgreSQL_FOUND)
    message(STATUS "--> SUCCESS: Found PostgreSQL.")
    message(STATUS "    Include Dirs: ${PostgreSQL_INCLUDE_DIRS}")
    message(STATUS "    Libraries: ${PostgreSQL_LIBRARIES}")
else()
    message(FATAL_ERROR "--> FAILURE: Could NOT find the PostgreSQL client library (libpq).
    1. Make sure the development package is installed (e.g., 'libpq-dev', 'postgresql-devel').
    2. If it's in a non-standard path, you must provide a hint to CMake (see instructions).")
endif()

# The rest of the script will only run if PostgreSQL was found
message(STATUS "--> Searching for Poco libraries...")
find_package(Poco REQUIRED Foundation Data DataPostgreSQL)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE Poco::Foundation Poco::Data Poco::DataPostgreSQL)

message(STATUS "--> CMake configuration finished successfully.")