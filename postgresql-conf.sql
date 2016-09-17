# Example PostgresqlSQL schema

CREATE TABLE ftpctx IF NOT EXISTS (
  id SERIAL,
  parent_id INTEGER,
  name VARCHAR(255),
  value VARCHAR(255),
  PRIMARY KEY (id)
);

CREATE TABLE ftpconf IF NOT EXISTS (
  id SERIAL,
  name VARCHAR(255) NOT NULL,
  value TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE ftpmap IF NOT EXISTS (
  conf_id INTEGER NOT NULL,
  ctx_id INTEGER NOT NULL
);
