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

CREATE TABLE MISSION (
	ID TEXT NOT NULL PRIMARY KEY,
	ID_CAMERA TEXT,
	RINEX BLOB,
	RINEX_NAME TEXT,
	NAME TEXT UNIQUE,
	FOREIGN KEY(ID_CAMERA) REFERENCES CAMERA(ID) ON DELETE SET NULL
);

CREATE TABLE UNITS (
	OBJECT INTEGER NOT NULL,
	UNIT_CODE INTEGER NOT NULL,
	UNIT TEXT NOT NULL
);