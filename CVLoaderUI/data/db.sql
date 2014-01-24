CREATE TABLE CAMERA (
	ID TEXT NOT NULL PRIMARY KEY,
	FOC FLOAT NOT NULL,
	DIMX FLOAT NOT NULL,
	DIMY FLOAT NOT NULL,
	DPIX FLOAT NOT NULL,
	XP FLOAT NOT NULL,
	YP FLOAT NOT NULL,
	SERIAL_NUMBER TEXT,
	MODEL TEXT,
	DESCR TEXT,
	PLANNING NUMERIC
);

CREATE TABLE JOURNAL (
	ID TEXT NOT NULL PRIMARY KEY,
	DATE INTEGER NOT NULL,
	URI TEXT NOT NULL,
	NOTE TEXT NOT NULL,
	CONTROL INTEGER NOT NULL,
	OBJECT INTEGER
);

CREATE TABLE DEM (
	ID TEXT NOT NULL PRIMARY KEY,
	URI TEXT NOT NULL
);

CREATE TABLE MISSION (
	ID TEXT NOT NULL PRIMARY KEY,
	ID_CAMERA TEXT,
	RINEX BLOB,
	NAME TEXT
);

CREATE TABLE STATION (
	ID TEXT NOT NULL PRIMARY KEY,
	ID_MISSION NOT NULL,
	RINEX BLOB,
	NAME TEXT
);