# MohammedImaadIqbal_SCEM_C++_Task2
presents...  
# DistriPhile
A distributed file sharing application and platform.
Made fully with POCO as the only dependency, and engineered entirely around the YugaByte database (and by extension, PostgreSQL as well).

## Building this app
- Initialise your yugabyte database first with the script in [initDB.sql](initDB.sql)
- Edit the properties in [src/env.h](src/env_example.h). The project has a [src/env_example.h](<src/env_example.h>) for you to copy and edit.
- Build the app using cmake:
	```bash
	cmake -B build && cmake --build build
	```

## Using the app:
### Commands:
- adduser: Add yourself as a user to the database
- login: Login into your existing account
- upload: Upload your file to the server
- download: Download any file from the server
- list: List all the files uploaded by you
- delete: Delete a specific file from the server
### Aliases:
- up: Same as upload
- down: Same as download
- l: Same as list
- del: Same as delete
### Syntax:
```bash
./build/app
./build/app adduser <username> <password>
./build/app login <username> <password>
./build/app upload <name_to_be_given_in_server> <path/to/file>
./build/app download <file_id_to_downloaded> <path/to/file>
./build/app list
./build/app delete <file_id_to_deleted>
```