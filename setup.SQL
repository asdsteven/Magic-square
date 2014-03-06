#Create a empty database
DROP DATABASE IF EXISTS db;
CREATE DATABASE db;
USE db;

#Create tables
CREATE TABLE jobs (
        priority INT DEFAULT 0,
        c VARBINARY(36),
        PRIMARY KEY (c)
) ENGINE = MYISAM;
CREATE TABLE solutions3 (c INT, PRIMARY KEY (c)) ENGINE = MYISAM;
CREATE TABLE solutions4 (c INT, PRIMARY KEY (c)) ENGINE = MYISAM;
CREATE TABLE solutions5 (c BIGINT, PRIMARY KEY (c)) ENGINE = MYISAM;
CREATE TABLE solutions6 (c BIGINT, d BIGINT PRIMARY KEY (c, d)) ENGINE = MYISAM;

#Initialize jobs for dimension 3, 4, 5 and 6 respectively
INSERT INTO jobs (c) VALUES (REPEAT("0", 9)), (REPEAT("0", 16)),
        (REPEAT("0", 25)), (REPEAT("0", 36));

#Create user and grant privileges for security measures
DROP USER 'client';
CREATE USER 'client' IDENTIFIED BY '1234';
REVOKE ALL PRIVILEGES, GRANT OPTION FROM 'client' IDENTIFIED BY '1234';
GRANT UPDATE (priority), DELETE ON jobs TO 'client' IDENTIFIED BY '1234';
GRANT SELECT, INSERT, LOCK TABLES ON * TO 'client' IDENTIFIED BY '1234';
