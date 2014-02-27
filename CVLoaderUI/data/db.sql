CREATE TABLE DEM (
	ID TEXT NOT NULL PRIMARY KEY,
	URI TEXT NOT NULL
);

CREATE TABLE JOURNAL (
	ID TEXT NOT NULL PRIMARY KEY,
	DATE INTEGER NOT NULL,
	URI TEXT NOT NULL,
	NOTE TEXT NOT NULL,
	CONTROL INTEGER NOT NULL,
	OBJECT INTEGER,
	FOREIGN KEY(CONTROL) REFERENCES ENU_CONTROL(ID),
	FOREIGN KEY(OBJECT) REFERENCES ENU_OBJECT(ID)
);

CREATE TABLE PROJECT (
	NAME TEXT NOT NULL PRIMARY KEY,
	NOTE TEXT NOT NULL,
	SCALE TEXT,
	TYPE INTEGER
);

CREATE TABLE ENU_CONTROL (
	ID INTEGER NOT NULL PRIMARY KEY,
	VALUE TEXT NOT NULL
);

CREATE TABLE ENU_OBJECT (
	ID INTEGER NOT NULL PRIMARY KEY,
	VALUE TEXT NOT NULL
);

