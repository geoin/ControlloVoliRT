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

CREATE TABLE STATION (
	ID TEXT NOT NULL PRIMARY KEY,
	ID_MISSION NOT NULL,
	RINEX BLOB,
	NAME TEXT,
	FOREIGN KEY(ID_MISSION) REFERENCES MISSION(ID) ON DELETE CASCADE
);

CREATE TABLE ASSETTI (
	ID TEXT NOT NULL,
	STRIP TEXT NOT NULL,
	NAME TEXT NOT NULL,
	PX FLOAT NOT NULL,
	PY FLOAT NOT NULL,
	PZ FLOAT NOT NULL,
	OMEGA FLOAT NOT NULL,
	PHI FLOAT NOT NULL,
	KAPPA FLOAT NOT NULL,
	PRIMARY KEY (STRIP, NAME)
);