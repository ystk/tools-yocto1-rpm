CREATE TABLE Packages (
  i	INTEGER UNIQUE PRIMARY KEY NOT NULL,
  h	BLOB NOT NULL
);
CREATE TEMP TRIGGER insert_Packages AFTER INSERT ON Packages
  BEGIN
    INSERT INTO Nvra (k,v)	VALUES ( new.h, new.rowid );
  END;
CREATE TEMP TRIGGER delete_Packages BEFORE DELETE ON Packages
  BEGIN
    DELETE FROM Nvra WHERE v = old.rowid;
  END;

CREATE TABLE Nvra (
  k	TEXT PRIMARY KEY NOT NULL,
  v	INTEGER REFERENCES Packages(i) ON UPDATE RESTRICT ON DELETE RESTRICT
);
