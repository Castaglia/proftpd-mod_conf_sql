# Example PostgresqlSQL schema

DROP TABLE ftpctxt;
CREATE TABLE ftpctxt (
  id SERIAL,
  parent_id INTEGER,
  name VARCHAR(255),
  value VARCHAR(255),
  PRIMARY KEY (id)
);

DROP TABLE ftpconf;
CREATE TABLE ftpconf (
  id SERIAL,
  name VARCHAR(255) NOT NULL,
  value TEXT,
  PRIMARY KEY (id)
);

DROP TABLE ftpmap;
CREATE TABLE ftpmap (
  conf_id INTEGER NOT NULL,
  ctxt_id INTEGER  NOT NULL
);

