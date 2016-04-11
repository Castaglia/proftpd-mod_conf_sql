# Example SQLite schema
CREATE TABLE IF NOT EXISTS ftpctxt (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    parent_id INTEGER UNSIGNED,
    name VARCHAR(255),
    type VARCHAR(255),
    info VARCHAR(255)
);

CREATE TABLE IF NOT EXISTS ftpconf (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type VARCHAR(255) NOT NULL,
    info BLOB
);

CREATE TABLE IF NOT EXISTS ftpmap (
    conf_id INTEGER UNSIGNED NOT NULL,
    ctxt_id INTEGER UNSIGNED NOT NULL
);
