-- PostgreSQL database initialization script for Matrecomm Assignment

-- Drop existing tables if they exist
DROP TABLE IF EXISTS file_chunks;
DROP TABLE IF EXISTS uploaded_files;
DROP TABLE IF EXISTS users;

-- Create UUID extension
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- 1. User and Password table
CREATE TABLE users (
	user_id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
	username VARCHAR(50) UNIQUE NOT NULL,
	password_hash VARCHAR(255) NOT NULL,
	created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
	-- last_login TIMESTAMP WITH TIME ZONE
);

-- 2. Uploaded file ID and index table
CREATE TABLE uploaded_files (
	file_id SERIAL PRIMARY KEY,
	user_id UUID NOT NULL,
	file_name VARCHAR(255) NOT NULL,
	file_size BIGINT NOT NULL,
	file_type VARCHAR(50),
	upload_date TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
	last_modified TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
	CONSTRAINT fk_user FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- 3. Uploaded file chunks table
CREATE TABLE file_chunks (
	chunk_id SERIAL PRIMARY KEY,
	file_id INTEGER NOT NULL,
	chunk_index INTEGER NOT NULL,
	chunk_data BYTEA NOT NULL,
	UNIQUE (file_id, chunk_index),
	CONSTRAINT fk_file FOREIGN KEY (file_id) REFERENCES uploaded_files(file_id) ON DELETE CASCADE
);

-- Create indexes for better query performance
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_uploaded_files_user_id ON uploaded_files(user_id);
CREATE INDEX idx_file_chunks_file_id ON file_chunks(file_id);
