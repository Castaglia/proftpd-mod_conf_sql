CREATE DATABASE proftpd;
USE proftpd;

CREATE TABLE IF NOT EXISTS ftpctx (
  id SERIAL,
  parent_id INTEGER,
  name TEXT,
  type TEXT,
  value TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS ftpconf (
  id SERIAL,
  name TEXT NOT NULL,
  value TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS ftpmap (
  conf_id INTEGER NOT NULL,
  ctx_id INTEGER NOT NULL
);
