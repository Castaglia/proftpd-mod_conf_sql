CREATE TABLE ftpctx IF NOT EXISTS (
  id SERIAL,
  parent_id INTEGER,
  name TEXT,
  type TEXT,
  value TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE ftpconf IF NOT EXISTS (
  id SERIAL,
  name TEXT NOT NULL,
  value TEXT,
  PRIMARY KEY (id)
);

CREATE TABLE ftpmap IF NOT EXISTS (
  conf_id INTEGER NOT NULL,
  ctx_id INTEGER NOT NULL
);
