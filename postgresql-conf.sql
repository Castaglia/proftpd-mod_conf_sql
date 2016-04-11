# Example PostgresqlSQL schema
CREATE TABLE IF NOT EXISTS ftpctxt (
  id SERIAL,
  parent_id INTEGER,
  name VARCHAR(255),
  value VARCHAR(255),
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS ftpconf (
  id SERIAL,
  name VARCHAR(255) NOT NULL,
  value TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS ftpmap (
  conf_id INTEGER NOT NULL,
  ctxt_id INTEGER  NOT NULL
);

