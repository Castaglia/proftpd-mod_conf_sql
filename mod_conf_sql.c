/*
 * ProFTPD: mod_conf_sql -- a module for reading configurations from SQL tables
 * Copyright (c) 2003-2016 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * This is mod_conf_sql, contrib software for proftpd 1.2 and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * -----DO NOT EDIT BELOW THIS LINE-----
 * $Archive: mod_conf_sql.a$
 */

#include "mod_conf_sql.h"
#include "mod_sql.h"
#include "uri.h"
#include "param.h"

#define CONF_SQL_URI_SCHEME		"sql"
#define CONF_SQL_URI_PREFIX		CONF_SQL_URI_SCHEME "://"
#define CONF_SQL_URI_PREFIX_LEN		6

/* Fake fd number for FSIO needs. */
#define CONF_SQL_FILENO		2746

struct {
  const char *username;
  const char *password;
  const char *server;
  const char *database;

} sqlconf_db;

struct {
  const char *table;
  const char *id_col;

  const char *parent_id_col;
  const char *type_col;
  const char *value_col;

  const char *where;
  const char *base_id;

} sqlconf_ctxs;

struct {
  const char *table;
  const char *id_col;
  const char *name_col;
  const char *value_col;

  const char *where;

} sqlconf_confs;

struct {
  const char *table;
  const char *conf_id_col;
  const char *ctx_id_col;

  const char *where;

} sqlconf_maps;

module conf_sql_module;
pool *conf_sql_pool = NULL;

static array_header *sqlconf_conf = NULL;
static unsigned int sqlconf_confi = 0;

static const char *trace_channel = "conf_sql";

/* Prototypes */
static int sqlconf_read_ctx(pool *p, int ctx_id, int isbase);
static void sqlconf_register(void);

static int sqlconf_parse_ctx_param(pool *p, pr_table_t *params) {
  int res;
  char *table, *id_col, *parent_id_col, *type_col, *value_col, *where;

  /* Defaults */
  sqlconf_ctxs.table = CONF_SQL_CTX_DEFAULT_TABLE_NAME;
  sqlconf_ctxs.id_col = CONF_SQL_CTX_DEFAULT_ID_COL_NAME;
  sqlconf_ctxs.parent_id_col = CONF_SQL_CTX_DEFAULT_PARENT_ID_COL_NAME;
  sqlconf_ctxs.type_col = CONF_SQL_CTX_DEFAULT_TYPE_COL_NAME;
  sqlconf_ctxs.value_col = CONF_SQL_CTX_DEFAULT_VALUE_COL_NAME;
  sqlconf_ctxs.where = NULL;

  table = id_col = parent_id_col = type_col = value_col = where = NULL;

  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  if (res < 0) {
    return -1;
  }

  if (table != NULL) {
    sqlconf_ctxs.table = table;
  }

  if (id_col != NULL) {
    sqlconf_ctxs.id_col = id_col;
  }

  if (parent_id_col != NULL) {
    sqlconf_ctxs.parent_id_col = parent_id_col;
  }

  if (type_col != NULL) {
    sqlconf_ctxs.type_col = type_col;
  }

  if (value_col != NULL) {
    sqlconf_ctxs.value_col = value_col;
  }

  if (where != NULL) {
    sqlconf_ctxs.where = where;
  }

  return 0;
}

static int sqlconf_parse_conf_param(pool *p, pr_table_t *params) {
  int res;
  char *table, *id_col, *name_col, *value_col, *where;

  /* Defaults */
  sqlconf_confs.table = CONF_SQL_CONF_DEFAULT_TABLE_NAME;
  sqlconf_confs.id_col = CONF_SQL_CONF_DEFAULT_ID_COL_NAME;
  sqlconf_confs.name_col = CONF_SQL_CONF_DEFAULT_NAME_COL_NAME;
  sqlconf_confs.value_col = CONF_SQL_CONF_DEFAULT_VALUE_COL_NAME;
  sqlconf_confs.where = NULL;

  table = id_col = name_col = value_col = where = NULL;

  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  if (res < 0) {
    return -1;
  }

  if (table != NULL) {
    sqlconf_confs.table = table;
  }

  if (id_col != NULL) {
    sqlconf_confs.id_col = id_col;
  }

  if (name_col != NULL) {
    sqlconf_confs.name_col = name_col;
  }

  if (value_col != NULL) {
    sqlconf_confs.value_col = value_col;
  }

  if (where != NULL) {
    sqlconf_confs.where = where;
  }

  return 0;
}

static int sqlconf_parse_map_param(pool *p, pr_table_t *params) {
  int res;
  char *table, *conf_id_col, *ctx_id_col, *where;

  /* Defaults */
  sqlconf_maps.table = CONF_SQL_MAP_DEFAULT_TABLE_NAME;
  sqlconf_maps.conf_id_col = CONF_SQL_MAP_DEFAULT_CONF_ID_COL_NAME;
  sqlconf_maps.ctx_id_col = CONF_SQL_MAP_DEFAULT_CTX_ID_COL_NAME;
  sqlconf_maps.where = NULL;

  table = conf_id_col = ctx_id_col = where = NULL;

  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  if (res < 0) {
    return -1;
  }

  if (table != NULL) {
    sqlconf_maps.table = table;
  }

  if (conf_id_col != NULL) {
    sqlconf_maps.conf_id_col = conf_id_col;
  }

  if (ctx_id_col != NULL) {
    sqlconf_maps.ctx_id_col = ctx_id_col;
  }

  if (where != NULL) {
    sqlconf_maps.where = where;
  }

  return 0;
}

/* Expected format of the URI:
 *
 * sql://dbuser:dbpass@dbserver[:dbport]/dbname?
 *   [database=<dbname>]
 *   &ctx:<table>[:id,parent_id,key,value][:where=<clause>]\
 *   &conf:<table>[:id,key,value][:where=<clause>]\
 *   &map:<table>[:conf_id,ctx_id][:where=<clause>]\
 *   [&base_id=<name>]
 */
static int sqlconf_parse_uri(pool *p, const char *uri, char **driver,
    int *use_tracing) {
  int res, xerrno;
  char *host = NULL, *path = NULL, *username, *password;
  unsigned int port = 0;
  pr_table_t *params = NULL;
  const void *v;

  params = pr_table_alloc(p, 1);

  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  if (res < 0) {
    xerrno = errno;

    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": failed parsing connect portion of URI '%.200s': %s", uri,
      strerror(xerrno));

    pr_table_free(params);
    errno = xerrno;
    return -1;
  }

  if (port != 0) {
    char portnum[32];

    memset(portnum, '\0', sizeof(portnum));
    snprintf(portnum, sizeof(portnum)-1, "%u", port);
    sqlconf_db.server = pstrcat(p, host, ":", portnum, NULL);

  } else {
    sqlconf_db.server = pstrdup(p, host);
  }

  sqlconf_db.username = pstrdup(p, username);
  sqlconf_db.password = pstrdup(p, password);

  /* Advance one character past the path separator to get the database/schema
   * name.
   */
  if (path != NULL) {
    sqlconf_db.database = pstrdup(p, path + 1);
  }

  v = pr_table_get(params, "database", NULL);
  if (v != NULL) {
    sqlconf_db.database = v;
  }

  v = pr_table_get(params, "tracing", NULL);
  if (v != NULL) {
    res = pr_str_is_boolean(v);
    if (res == TRUE) {
      *use_tracing = TRUE;
      pr_trace_use_stderr(*use_tracing);

      /* TODO: Make the trace level a param as well. */
      pr_trace_set_levels(trace_channel, 1, 20);
    }
  }

  pr_trace_msg(trace_channel, 6, "db.username = %s",
    sqlconf_db.username ? sqlconf_db.username : "(none)");
  pr_trace_msg(trace_channel, 6, "db.server = %s",
    sqlconf_db.server ? sqlconf_db.server : "(none)");
  pr_trace_msg(trace_channel, 6, "db.database = %s",
    sqlconf_db.database ? sqlconf_db.database : "(none)");

  if (sqlconf_parse_ctx_param(p, params) < 0) {
    xerrno = errno;

    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": failed parsing context table portion of URI '%.100s': %s", uri,
      strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_trace_msg(trace_channel, 6, "ctx.table = %s", sqlconf_ctxs.table);
  pr_trace_msg(trace_channel, 6, "ctx.id_col = %s", sqlconf_ctxs.id_col);
  pr_trace_msg(trace_channel, 6, "ctx.parent_id_col = %s",
    sqlconf_ctxs.parent_id_col);
  pr_trace_msg(trace_channel, 6, "ctx.type_col = %s", sqlconf_ctxs.type_col);
  pr_trace_msg(trace_channel, 6, "ctx.value_col = %s", sqlconf_ctxs.value_col);
  pr_trace_msg(trace_channel, 6, "ctx.where = %s",
    sqlconf_ctxs.where ? sqlconf_ctxs.where : "(none)");

  if (sqlconf_parse_conf_param(p, params) < 0) {
    xerrno = errno;

    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": failed parsing directive table portion of URI '%.100s': %s", uri,
      strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_trace_msg(trace_channel, 6, "conf.table = %s", sqlconf_confs.table);
  pr_trace_msg(trace_channel, 6, "conf.id_col = %s", sqlconf_confs.id_col);
  pr_trace_msg(trace_channel, 6, "conf.name_col = %s", sqlconf_confs.name_col);
  pr_trace_msg(trace_channel, 6, "conf.value_col = %s",
    sqlconf_confs.value_col);
  pr_trace_msg(trace_channel, 6, "conf.where = %s",
    sqlconf_confs.where ? sqlconf_confs.where : "(none)");

  if (sqlconf_parse_map_param(p, params) < 0) {
    xerrno = errno;

    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": failed parsing map table portion of URI '%.100s': %s", uri,
      strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  pr_trace_msg(trace_channel, 6, "map.table = %s", sqlconf_maps.table);
  pr_trace_msg(trace_channel, 6, "map.conf_id_col = %s",
    sqlconf_maps.conf_id_col);
  pr_trace_msg(trace_channel, 6, "map.ctx_id_col = %s",
    sqlconf_maps.ctx_id_col);
  pr_trace_msg(trace_channel, 6, "map.where = %s",
    sqlconf_maps.where ? sqlconf_maps.where : "(none)");

  v = pr_table_get(params, "base_id", NULL);
  if (v != NULL) {
    sqlconf_ctxs.base_id = v;
  }

  pr_trace_msg(trace_channel, 6, "ctxs.base_id = %s",
    sqlconf_ctxs.base_id ? sqlconf_ctxs.base_id : "(none)");

  /* Look for a specific database backend/driver to use. */
  v = pr_table_get(params, "driver", NULL);
  if (v != NULL) {
    *driver = pstrdup(p, (char *) v);
    pr_trace_msg(trace_channel, 6, "driver = %s", *driver);
  }

  if (*use_tracing) {
    pr_trace_set_levels(trace_channel, 0, 0);
    pr_trace_use_stderr(FALSE);
  }

  return 0;
}

/* SQL functions
 */

/* Note: mod_sql.c doesn't expose this function, so we'll need our own copy
 * of it.
 */
static cmd_rec *sqlconf_cmd_alloc(pool *p, unsigned int argc, ...) {
  pool *sub_pool = NULL;
  cmd_rec *cmd = NULL;
  va_list args;
  register unsigned int i = 0;

  sub_pool = make_sub_pool(p);
  cmd = pcalloc(sub_pool, sizeof(cmd_rec));
  cmd->argc = argc;
  cmd->stash_index = -1;
  cmd->pool = sub_pool;

  cmd->argv = pcalloc(sub_pool, sizeof(void *) * (argc + 1));
  cmd->tmp_pool = sub_pool;

  va_start(args, argc);

  for (i = 0; i < argc; i++) {
    cmd->argv[i] = (void *) va_arg(args, char *);
  }
  va_end(args);

  return cmd;
}

static modret_t *sqlconf_dispatch(cmd_rec *cmd, char *name) {
  cmdtable *cmdtab;
  modret_t *res;

  cmdtab = pr_stash_get_symbol(PR_SYM_HOOK, name, NULL, NULL);
  if (cmdtab == NULL) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": unable to find SQL hook symbol '%s'", name);
    errno = ENOENT;
    return PR_ERROR(cmd);
  }

  res = pr_module_call(cmdtab->m, cmdtab->handler, cmd);
  if (MODRET_ISERROR(res)) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION ": '%s' error: %s", name,
      res->mr_message);
    return res;
  }

  return res;
}

/* Database-reading routines
 */

static int sqlconf_read_ctx_ctxs(pool *p, int ctx_id) {
  cmd_rec *cmd = NULL;
  modret_t *res = NULL;
  sql_data_t *sd = NULL;
  char *where = NULL;

  register unsigned int i = 0;
  char idstr[64] = {'\0'};

  snprintf(idstr, sizeof(idstr)-1, "%d", ctx_id);
  idstr[sizeof(idstr)-1] = '\0';

  if (sqlconf_ctxs.where == NULL) {
    where = pstrcat(p, sqlconf_ctxs.parent_id_col, " = ", idstr, NULL);

  } else {
    where = pstrcat(p, sqlconf_ctxs.parent_id_col, " = ", idstr, " AND ",
      sqlconf_ctxs.where, NULL);
  }

  cmd = sqlconf_cmd_alloc(p, 4, "sqlconf", sqlconf_ctxs.table,
    sqlconf_ctxs.id_col, where);

  res = sqlconf_dispatch(cmd, "sql_select");
  if (MODRET_ISERROR(res)) {
    int xerrno = errno;
    const char *errmsg;

    errmsg = MODRET_ERRMSG(res);
    pr_trace_msg(trace_channel, 9, "SQL SELECT error: %s",
      errmsg ? errmsg : "(unknown)");

    errno = xerrno;
    return -1;
  }

  sd = res->data;

  for (i = 0; i < sd->rnum; i++) {
    sqlconf_read_ctx(p, atoi(sd->data[i]), FALSE);
  }

  return 0;
}

static int sqlconf_read_conf(pool *p, int ctx_id) {
  cmd_rec *cmd = NULL;
  modret_t *res = NULL;
  sql_data_t *sd = NULL;
  char *query = NULL;

  register unsigned int i = 0;
  char idstr[64] = {'\0'};

  snprintf(idstr, sizeof(idstr)-1, "%d", ctx_id);
  idstr[sizeof(idstr)-1] = '\0';

  if (sqlconf_confs.where == NULL) {
    query = pstrcat(p, sqlconf_confs.name_col, ", ", sqlconf_confs.value_col,
      " FROM ", sqlconf_confs.table, " INNER JOIN ", sqlconf_maps.table,
      " ON ", sqlconf_confs.table, ".", sqlconf_confs.id_col, " = ",
      sqlconf_maps.table, ".", sqlconf_maps.conf_id_col, " WHERE ",
      sqlconf_maps.table, ".", sqlconf_maps.ctx_id_col, " = ", idstr, NULL);

  } else {
    query = pstrcat(p, sqlconf_confs.name_col, ", ", sqlconf_confs.value_col,
      " FROM ", sqlconf_confs.table, " INNER JOIN ", sqlconf_maps.table,
      " ON ", sqlconf_confs.table, ".", sqlconf_confs.id_col, " = ",
      sqlconf_maps.table, ".", sqlconf_maps.conf_id_col, " WHERE ",
      sqlconf_maps.table, ".", sqlconf_maps.ctx_id_col, " = ", idstr,
      " AND ", sqlconf_confs.where, NULL);
  }

  cmd = sqlconf_cmd_alloc(p, 2, "sqlconf", query);

  res = sqlconf_dispatch(cmd, "sql_select");
  if (MODRET_ISERROR(res)) {
    int xerrno = errno;
    const char *errmsg;

    errmsg = MODRET_ERRMSG(res);
    pr_trace_msg(trace_channel, 9, "SQL SELECT error: %s",
      errmsg ? errmsg : "(unknown)");

    errno = xerrno;
    return -1;
  }

  sd = res->data;

  for (i = 0; i < sd->rnum; i++) {
    char *str;

    str = pstrcat(conf_sql_pool, sd->data[(i * sd->fnum)], " ",
      sd->data[(i * sd->fnum) + 1], "\n", NULL);
    *((char **) push_array(sqlconf_conf)) = str;
  }

  return 0;
}

static int sqlconf_read_ctx(pool *p, int ctx_id, int isbase) {
  cmd_rec *cmd = NULL;
  modret_t *res = NULL;
  sql_data_t *sd = NULL;
  char *where = NULL;

  char idstr[64] = {'\0'};
  char *ctx_key = NULL, *ctx_val = NULL;

  snprintf(idstr, sizeof(idstr)-1, "%d", ctx_id);
  idstr[sizeof(idstr)-1] = '\0';

  if (sqlconf_ctxs.where == NULL) {
    where = pstrcat(p, sqlconf_ctxs.id_col, " = ", idstr, NULL);

  } else {
    where = pstrcat(p, sqlconf_ctxs.id_col, " = ", idstr, " AND ",
      sqlconf_ctxs.where, NULL);
  }

  cmd = sqlconf_cmd_alloc(p, 4, "sqlconf", sqlconf_ctxs.table,
    pstrcat(p, sqlconf_ctxs.type_col, ", ", sqlconf_ctxs.value_col, NULL),
    where);

  res = sqlconf_dispatch(cmd, "sql_select");
  if (MODRET_ISERROR(res)) {
    pr_log_debug(DEBUG4, MOD_CONF_SQL_VERSION
      ": notice: context ID (%d) has no associated key/value", ctx_id);
    errno = ENOENT;
    return -1;
  }

  sd = res->data;

  if (sd->rnum > 1) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": error: multiple key/values returned for given context ID (%d)",
      ctx_id);
    errno = EINVAL;
    return -1;
  }

  ctx_key = sd->data[0];
  ctx_val = sd->data[1];

  if (ctx_key &&
      !isbase) {
    *((char **) push_array(sqlconf_conf)) = pstrcat(conf_sql_pool, "<",
      ctx_key, ctx_val ? " " : "", ctx_val ? ctx_val : "", ">\n", NULL);
  }

  if (sqlconf_read_conf(p, ctx_id) < 0) {
    return -1;
  }

  if (sqlconf_read_ctx_ctxs(p, ctx_id) < 0) {
    return -1;
  }

  if (ctx_key &&
      !isbase) {
    *((char **) push_array(sqlconf_conf)) = pstrcat(conf_sql_pool, "</",
      ctx_key, ">\n", NULL);
  }

  return 0;
}

static int sqlconf_close_db(pool *p, int use_tracing) {
  int res = 0, xerrno = 0;
  cmd_rec *cmd = NULL;
  modret_t *mr = NULL;

  /* Close the connection. */
  cmd = sqlconf_cmd_alloc(p, 2, "sqlconf", "1");
  mr = sqlconf_dispatch(cmd, "sql_close_conn");
  destroy_pool(cmd->pool);
  if (MODRET_ISERROR(mr)) {
    const char *errmsg;

    errmsg = MODRET_ERRMSG(mr);
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": error closing database connection: %s",
      errmsg ? errmsg : strerror(errno));

    xerrno = EINVAL;
    res = -1;
  }

  if (res == 0) {
    /* Cleanup the SQL subsystem. */
    cmd = sqlconf_cmd_alloc(p, 0);
    mr = sqlconf_dispatch(cmd, "sql_cleanup");
    destroy_pool(cmd->pool);
    if (MODRET_ISERROR(mr)) {
      const char *errmsg;

      errmsg = MODRET_ERRMSG(mr);
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": error cleaning up SQL system: %s",
        errmsg ? errmsg : strerror(errno));

      xerrno = EINVAL;
      res = -1;
    }
  }

  if (use_tracing) {
    pr_trace_use_stderr(FALSE);
    pr_trace_set_levels(trace_channel, 0, 0);
  }

  errno = xerrno;
  return res;
}

/* Construct the configuration file from the database contents. */
static int sqlconf_read_db(pool *p, char *driver, int use_tracing) {
  int id = 0;
  cmd_rec *cmd = NULL;
  modret_t *res = NULL;
  sql_data_t *sd = NULL;
  const char *username, *password, *dsn;
  char *where, *which_id = NULL;

  if (use_tracing) {
    pr_trace_use_stderr(TRUE);

    /* TODO: Make the trace level a param as well. */
    pr_trace_set_levels(trace_channel, 1, 20);
  }

  /* Load the SQL backend module we'll be using. */
  if (driver == NULL) {
    cmd = sqlconf_cmd_alloc(p, 0);

  } else {
    pr_trace_msg(trace_channel, 9, "reading database using driver '%s'",
      driver);

    /* The mod_sql_sqlite module uses a backend name of "sqlite3"; check
     * the driver name to see if that what was intended.
     */
    if (strcasecmp(driver, "sqlite") == 0) {
      driver = pstrdup(p, "sqlite3");
    }

    cmd = sqlconf_cmd_alloc(p, 1, driver);
  }

  res = sqlconf_dispatch(cmd, "sql_load_backend");
  destroy_pool(cmd->pool);

/* XXX What if mod_sql is not built/loaded? */
/* XXX What if desired backend is not built/loaded? */

  /* Prepare the SQL subsystem. */
  cmd = sqlconf_cmd_alloc(p, 1, make_sub_pool(p));
  res = sqlconf_dispatch(cmd, "sql_prepare");
  destroy_pool(cmd->pool);

/* XXX What if this preparation fails? */

  /* Define the connection we'll be making. */
  username = sqlconf_db.username;
  password = sqlconf_db.password;
  if (sqlconf_db.database != NULL) {
    dsn = pstrcat(p, sqlconf_db.database, "@", sqlconf_db.server, NULL);

  } else {
    dsn = sqlconf_db.server;
  }

  cmd = sqlconf_cmd_alloc(p, 4, "sqlconf", username, password, dsn);
  res = sqlconf_dispatch(cmd, "sql_define_conn");
  destroy_pool(cmd->pool);
  if (MODRET_ISERROR(res)) {
    const char *errmsg;

    errmsg = MODRET_ERRMSG(res);
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": error defining database connection: %s",
      errmsg ? errmsg : strerror(errno));

    if (use_tracing) {
      pr_trace_set_levels(trace_channel, 0, 0);
      pr_trace_use_stderr(FALSE);
    }

    errno = EINVAL;
    return -1;
  }

  /* Open a connection to the database. */
  cmd = sqlconf_cmd_alloc(p, 1, "sqlconf");
  res = sqlconf_dispatch(cmd, "sql_open_conn");
  destroy_pool(cmd->pool);
  if (MODRET_ISERROR(res)) {
    const char *errmsg;

    errmsg = MODRET_ERRMSG(res);
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": error opening database connection: %s",
      errmsg ? errmsg : strerror(errno));

    errno = EINVAL;
    return -1;
  }

  /* Do the database digging. To start things off, we need to find the
   * "server config"/default context.  If we've been given a base context,
   * look for the ID of the context with that name, otherwise, look for the
   * context whose ID is NULL.
   */
  if (sqlconf_ctxs.base_id == NULL) {
    where = pstrcat(p, sqlconf_ctxs.parent_id_col, " IS NULL", NULL);
    which_id = "default";

  } else {
    where = pstrcat(p, sqlconf_ctxs.id_col, " = ", sqlconf_ctxs.base_id, NULL);
    which_id = "base";
  }

  cmd = sqlconf_cmd_alloc(p, 4, "sqlconf", sqlconf_ctxs.table,
    sqlconf_ctxs.id_col, where);

  res = sqlconf_dispatch(cmd, "sql_select");
  if (MODRET_ISERROR(res)) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": error retrieving %s context ID", which_id);

    (void) sqlconf_close_db(p, use_tracing);
    errno = ENOENT;
    return -1;
  }

  sd = res->data;

  /* We only want _one_ unique base context.  Any more than that is a
   * configuration error in the database.
   *
   * However, if there is NO unique base context, then we ASSUME that the
   * configuration data is empty, akin to an empty config file.
   */
  if (sd->rnum != 0 &&
      sd->fnum != 0) {
    if (sd->rnum != 1 &&
        sd->fnum != 1) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": retrieving %s context failed: bad/non-unique results", which_id);

      (void) sqlconf_close_db(p, use_tracing);
      errno = ENOENT;
      return -1;
    }

    if (sd->data == NULL ||
        sd->data[0] == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": retrieving %s context failed: no matching results", which_id);

      (void) sqlconf_close_db(p, use_tracing);
      errno = ENOENT;
      return -1;
    }

    id = atoi(sd->data[0]);
  }

  destroy_pool(cmd->pool);

  sqlconf_conf = make_array(conf_sql_pool, 1, sizeof(char *));
  if (sd->rnum == 1 &&
      sd->fnum == 1) {
    sqlconf_read_ctx(p, id, TRUE);
  }

  if (sqlconf_close_db(p, use_tracing) < 0) {
    return -1;
  }

  return 0;
}

/* FSIO callbacks
 */

static int sqlconf_fsio_fstat(pr_fh_t *fh, int fd, struct stat *st) {
  if (fd == CONF_SQL_FILENO) {
    /* Set a default "block size". */
    st->st_blksize = 4096;

    return 0;
  }

  return fstat(fd, st);
}

static int sqlconf_fsio_lstat(pr_fs_t *fs, const char *path, struct stat *st) {
  /* Is this a path that we can use? */
  if (strncmp(CONF_SQL_URI_PREFIX, path, CONF_SQL_URI_PREFIX_LEN) == 0) {
    return 0;
  }

  return lstat(path, st);
}

static int sqlconf_fsio_stat(pr_fs_t *fs, const char *path, struct stat *st) {
  /* Is this a path that we can use? */
  if (strncmp(CONF_SQL_URI_PREFIX, path, CONF_SQL_URI_PREFIX_LEN) == 0) {
    return 0;
  }

  return stat(path, st);
}

static int sqlconf_fsio_open(pr_fh_t *fh, const char *path, int flags) {

  /* Is this a path that we can use? */
  if (strncmp(CONF_SQL_URI_PREFIX, path, CONF_SQL_URI_PREFIX_LEN) == 0) {
    pool *p;
    char *driver = NULL, *uri;
    int use_tracing = FALSE;

    p = conf_sql_pool;
    uri = pstrdup(p, path);

    /* Parse through the given URI, breaking out the needed pieces. */
    if (sqlconf_parse_uri(p, uri, &driver, &use_tracing) < 0) {
      return -1;
    }

    if (sqlconf_conf == NULL &&
        sqlconf_read_db(p, driver, use_tracing) < 0) {
      return -1;
    }

    /* Return a fake file descriptor. */
    return CONF_SQL_FILENO;
  }

  /* Default normal open. */
  return open(path, flags, PR_OPEN_MODE);
}

static int sqlconf_fsio_close(pr_fh_t *fh, int fd) {
  if (fd == CONF_SQL_FILENO) {
    return 0;
  }

  return close(fd);
}

static int sqlconf_fsio_read(pr_fh_t *fh, int fd, char *buf, size_t buflen) {

  /* Make sure this filehandle is for this module before trying to use it. */
  if (fd == CONF_SQL_FILENO &&
      fh->fh_path != NULL &&
      strncmp(CONF_SQL_URI_PREFIX, fh->fh_path, CONF_SQL_URI_PREFIX_LEN) == 0) {

    if (sqlconf_conf == NULL) {
      errno = ENOENT;
      return -1;
    }

    if (sqlconf_confi < sqlconf_conf->nelts) {
      char **lines = sqlconf_conf->elts;
     
      /* Read from our built-up buffer, until there are no more lines to be
       * read.
       */
      memcpy(buf, lines[sqlconf_confi++], buflen);
      return strlen(buf);
    }

    return 0;
  }

  /* Default normal read. */
  return read(fd, buf, buflen);
}

/* Event handlers
 */

static void sqlconf_postparse_ev(const void *event_data, void *user_data) {

  /* Unregister the registered FS. */
  if (pr_unregister_fs("sql://") < 0) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION ": error unregistering fs: %s",
      strerror(errno));

  } else {
    pr_log_debug(DEBUG8, MOD_CONF_SQL_VERSION ": fs unregistered");
  }

  /* Destroy the module pool. */
  if (conf_sql_pool) {
    destroy_pool(conf_sql_pool);
    conf_sql_pool = NULL;
  }
}

static void sqlconf_restart_ev(const void *event_data, void *user_data) {

  /* Register the FS object. */
  sqlconf_register();
}

/* Initialization functions
 */

static void sqlconf_register(void) {
  pr_fs_t *fs = NULL;

  conf_sql_pool = make_sub_pool(permanent_pool);

  /* Register a FS object, with which we will watch for 'sql://' files
   * being opened, and intercept them.
   */
  fs = pr_register_fs(conf_sql_pool, "sqlconf", "sql://");
  if (fs == NULL) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION ": error registering fs: %s",
      strerror(errno));
    return;
  }
  pr_log_debug(DEBUG10, MOD_CONF_SQL_VERSION ": registered 'sqlconf' fs");

  /* Add the module's custom FS callbacks here. This module does not
   * provide callbacks for most of the operations.
   */
  fs->fstat = sqlconf_fsio_fstat;
  fs->lstat = sqlconf_fsio_lstat;
  fs->open = sqlconf_fsio_open;
  fs->close = sqlconf_fsio_close;
  fs->read = sqlconf_fsio_read;
  fs->stat = sqlconf_fsio_stat;
}

static int sqlconf_init(void) {

  /* Register the FS object. */
  sqlconf_register();

  /* Register event handlers. */
  pr_event_register(&conf_sql_module, "core.postparse", sqlconf_postparse_ev,
    NULL);
  pr_event_register(&conf_sql_module, "core.restart", sqlconf_restart_ev,
    NULL);

  return 0;
}

/* Module API tables
 */

module conf_sql_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "conf_sql",

  /* Module configuration handler table */
  NULL,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  sqlconf_init,

  /* Session initialization function */
  NULL,

  /* Module version */
  MOD_CONF_SQL_VERSION
};
