# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Name your project
project(MyPocoApp CXX)

# Set the C++ standard to a modern version (e.g., C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Poco library, specifically the "Foundation" component
# This is where DateTime and DateTimeFormatter reside.
# The REQUIRED keyword will cause CMake to fail if Poco is not found.
find_package(Poco REQUIRED Foundation)

# Add your executable from a source file (e.g., main.cpp)
add_executable(app MohammedImaadIqbal_SCEM_C++_Task2.cpp)

# Link the Poco Foundation library to your executable.
# This automatically handles include paths and linker flags.
target_link_libraries(app PRIVATE Poco::Foundation)