CREATE TABLE SENSOR (
	ID TEXT NOT NULL PRIMARY KEY,
	FOV FLOAT NOT NULL,
	IFOV FLOAT NOT NULL,
	FREQ FLOAT NOT NULL,
	SCAN_RATE FLOAT NOT NULL,
	PLANNING NUMERIC,
	SPEED FLOAT
);

CREATE TABLE MISSION (
	ID TEXT NOT NULL PRIMARY KEY,
	ID_SENSOR TEXT,
	RINEX BLOB,
	RINEX_NAME TEXT,
	NAME TEXT UNIQUE,
	FOREIGN KEY(ID_SENSOR) REFERENCES SENSOR(ID) ON DELETE SET NULL
);

CREATE TABLE CLOUD_SAMPLE (
	ID TEXT NOT NULL PRIMARY KEY,
	URI TEXT NOT NULL
);

CREATE TABLE STRIP_RAW_DATA (
	ID TEXT NOT NULL PRIMARY KEY,
	FOLDER TEXT NOT NULL
);

CREATE TABLE _META_COLUMNS_ (
	CONTROL INT,
	OBJECT INT,
	REF TEXT,
	TARGET TEXT,
	UNIQUE (CONTROL, OBJECT, REF)
);

INSERT INTO _META_COLUMNS_ VALUES (2, 2, 'A_VOL_QT', '');
INSERT INTO _META_COLUMNS_ VALUES (2, 2, 'A_VOL_CS', '');