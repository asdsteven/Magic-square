SET GLOBAL max_allowed_packet=1073741824;

#Create a empty database
DROP DATABASE IF EXISTS db;
CREATE DATABASE db;
USE db;

#Create tables
CREATE TABLE jobs (
	priority INT DEFAULT 0,
	c VARBINARY(36)
) ENGINE = MYISAM;
CREATE TABLE solutions3 (c BINARY(9)) ENGINE = MYISAM;
CREATE TABLE solutions4 (c BINARY(16)) ENGINE = MYISAM;
CREATE TABLE solutions5 (c BINARY(25)) ENGINE = MYISAM;
CREATE TABLE solutions6 (c BINARY(36)) ENGINE = MYISAM;

#Initialize jobs for dimension 3, 4, 5 and 6 respectively
INSERT INTO jobs (c) VALUES
	(REPEAT("a", 9)),
	(REPEAT("a", 16)),
	(REPEAT("a", 25)),
	(REPEAT("a", 36));

#Create user and grant privileges for security measures
DROP USER 'client';
CREATE USER 'client' IDENTIFIED BY '1234';
GRANT UPDATE (priority), DELETE ON jobs TO 'client' IDENTIFIED BY '1234';
GRANT SELECT, INSERT, LOCK TABLES ON * TO 'client' IDENTIFIED BY '1234';
