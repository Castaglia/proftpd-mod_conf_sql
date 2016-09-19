/*
 * ProFTPD - mod_conf_sql Param implementation
 * Copyright (c) 2016 TJ Saunders
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#include "mod_conf_sql.h"
#include "param.h"
#include "uri.h"

/* Expected format of the conf parameter:
 *
 *   conf=<table>[:id,name,value][:where=<clause>]
 */
int sqlconf_param_parse_conf(pool *p, pr_table_t *params, char **table,
    char **id_col, char **name_col, char **value_col, char **where) {
  const void *v, *val, *cols_val;
  size_t sz, vsz, valsz, cols_valsz;
  char *ptr;

  if (p == NULL ||
      params == NULL ||
      table == NULL ||
      id_col == NULL ||
      name_col == NULL ||
      value_col == NULL ||
      where == NULL) {
    errno = EINVAL;
    return -1;
  }

  v = pr_table_get(params, "conf", &vsz);
  if (v == NULL) {
    *table = *id_col = *name_col = *value_col = *where = NULL;
    return 0;
  }

  /* Ignore empty values.  Remember that table value lengths include the NUL. */
  if (vsz < 2) {
    *table = *id_col = *name_col = *value_col = *where = NULL;
    return 0;
  }

  val = v;
  valsz = vsz-1;

  ptr = memchr(v, ':', vsz);
  if (ptr == NULL) {
    /* Just the table name, then. */
    sz = vsz;
    *table = pstrndup(p, v, sz);
    *id_col = *name_col = *value_col = *where = NULL;

    return 0;
  }

  sz = ptr - ((char *) v);
  *table = pstrndup(p, v, sz);

  *id_col = *name_col = *value_col = *where = NULL;

  v = ptr + 1;
  vsz = vsz - sz - 1;
  ptr = memchr(v, ':', vsz);
  if (ptr != NULL) {
    cols_val = v;
    cols_valsz = 0;

    /* Handle the case where ptr is pointing AT v, i.e. v is pointing at
     * ":...".
     */
    if (ptr != v) {
      cols_valsz = ptr - (char *) v;
    }

  } else {
    cols_val = v;
    cols_valsz = vsz;
  }

  if (cols_valsz != 0) {
    ptr = memchr(cols_val, ',', cols_valsz);
    if (ptr == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'conf' parameter '%.*s': missing column names",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = ptr - (char *) cols_val;
    *id_col = pstrndup(p, cols_val, sz);

    cols_val = ptr + 1;
    cols_valsz = cols_valsz - sz - 1;
    ptr = memchr(cols_val, ',', cols_valsz);
    if (ptr == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'conf' parameter '%.*s': missing column names",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = ptr - (char *) cols_val;
    *name_col = pstrndup(p, cols_val, sz);

    cols_val = ptr + 1;
    cols_valsz = cols_valsz - sz - 1;
    ptr = memchr(cols_val, ':', cols_valsz);
    if (ptr == NULL) {
      *value_col = pstrndup(p, cols_val, cols_valsz);

    } else {
      *value_col = pstrndup(p, cols_val, sz);
    }

    ptr = memchr((char *) v + (char *) cols_valsz, ':', vsz - cols_valsz);
  }

  if (ptr != NULL) {
    size_t wheresz = 0;

    /* Possible WHERE clause included. */
    if (strncasecmp(ptr+1, "where=", 6) != 0) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'conf' parameter '%.*s': bad WHERE clause",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = strlen(ptr+1);
    sqlconf_uri_urldecode(p, ptr+1, sz, where, &wheresz);
  }

  return 0;
}

/* Expected format of the ctx parameter:
 *
 *   ctx=<table>[:id,parent_id,type,value][:where=<clause>]
 */
int sqlconf_param_parse_ctx(pool *p, pr_table_t *params, char **table,
    char **id_col, char **parent_id_col, char **type_col, char **value_col,
    char **where) {
  const void *v, *val, *cols_val;
  size_t sz, vsz, valsz, cols_valsz;
  char *ptr;

  if (p == NULL ||
      params == NULL ||
      table == NULL ||
      id_col == NULL ||
      parent_id_col == NULL ||
      type_col == NULL ||
      value_col == NULL ||
      where == NULL) {
    errno = EINVAL;
    return -1;
  }

  v = pr_table_get(params, "ctx", &vsz);
  if (v == NULL) {
    *table = *id_col = *parent_id_col = *type_col = *value_col = *where = NULL;
    return 0;
  }

  /* Ignore empty values.  Remember that table value lengths include the NUL. */
  if (vsz < 2) {
    *table = *id_col = *parent_id_col = *type_col = *value_col = *where = NULL;
    return 0;
  }

  val = v;
  valsz = vsz-1;

  ptr = memchr(v, ':', vsz);
  if (ptr == NULL) {
    /* Just the table name, then. */
    sz = vsz;
    *table = pstrndup(p, v, sz);
    *id_col = *parent_id_col = *type_col = *value_col = *where = NULL;

    return 0;
  }

  sz = ptr - ((char *) v);
  *table = pstrndup(p, v, sz);

  *id_col = *parent_id_col = *type_col = *value_col = *where = NULL;

  v = ptr + 1;
  vsz = vsz - sz - 1;
  ptr = memchr(v, ':', vsz);
  if (ptr != NULL) {
    cols_val = v;
    cols_valsz = 0;

    /* Handle the case where ptr is pointing AT v, i.e. v is pointing at
     * ":...".
     */
    if (ptr != v) {
      cols_valsz = ptr - (char *) v;
    }

  } else {
    cols_val = v;
    cols_valsz = vsz;
  }

  if (cols_valsz != 0) {
    ptr = memchr(cols_val, ',', cols_valsz);
    if (ptr == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'ctx' parameter '%.*s': missing column names",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = ptr - (char *) cols_val;
    *id_col = pstrndup(p, cols_val, sz);

    cols_val = ptr + 1;
    cols_valsz = cols_valsz - sz - 1;
    ptr = memchr(cols_val, ',', cols_valsz);
    if (ptr == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'ctx' parameter '%.*s': missing column names",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = ptr - (char *) cols_val;
    *parent_id_col = pstrndup(p, cols_val, sz);

    cols_val = ptr + 1;
    cols_valsz = cols_valsz - sz - 1;
    ptr = memchr(cols_val, ',', cols_valsz);
    if (ptr == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'ctx' parameter '%.*s': missing column names",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = ptr - (char *) cols_val;
    *type_col = pstrndup(p, cols_val, sz);

    cols_val = ptr + 1;
    cols_valsz = cols_valsz - sz - 1;
    ptr = memchr(cols_val, ':', cols_valsz);
    if (ptr == NULL) {
      *value_col = pstrndup(p, cols_val, cols_valsz);

    } else {
      *value_col = pstrndup(p, cols_val, sz);
    }

    ptr = memchr((char *) v + (char *) cols_valsz, ':', vsz - cols_valsz);
  }

  if (ptr != NULL) {
    size_t wheresz = 0;

    /* Possible WHERE clause included. */
    if (strncasecmp(ptr+1, "where=", 6) != 0) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'ctx' parameter '%.*s': bad WHERE clause",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = strlen(ptr+1);
    sqlconf_uri_urldecode(p, ptr+1, sz, where, &wheresz);
  }

  return 0;
}

/* Expected format of the map parameter:
 *
 *   map=<table>[:conf_id,ctx_id][:where=<clause>]
 */
int sqlconf_param_parse_map(pool *p, pr_table_t *params, char **table,
    char **conf_id_col, char **ctx_id_col, char **where) {
  const void *v, *val, *cols_val;
  size_t sz, vsz, valsz, cols_valsz;
  char *ptr;

  if (p == NULL ||
      params == NULL ||
      table == NULL ||
      conf_id_col == NULL ||
      ctx_id_col == NULL ||
      where == NULL) {
    errno = EINVAL;
    return -1;
  }

  v = pr_table_get(params, "map", &vsz);
  if (v == NULL) {
    *table = *conf_id_col = *ctx_id_col = *where = NULL;
    return 0;
  }

  /* Ignore empty values.  Remember that table value lengths include the NUL. */
  if (vsz < 2) {
    *table = *conf_id_col = *ctx_id_col = *where = NULL;
    return 0;
  }

  val = v;
  valsz = vsz-1;

  ptr = memchr(v, ':', vsz);
  if (ptr == NULL) {
    /* Just the table name, then. */
    sz = vsz-1;
    *table = pstrndup(p, v, sz);
    *conf_id_col = *ctx_id_col = *where = NULL;

    return 0;
  }

  sz = ptr - ((char *) v);
  *table = pstrndup(p, v, sz);

  *conf_id_col = *ctx_id_col = *where = NULL;

  v = ptr + 1;
  vsz = vsz - sz - 1;
  ptr = memchr(v, ':', vsz);
  if (ptr != NULL) {
    cols_val = v;
    cols_valsz = 0;

    /* Handle the case where ptr is pointing AT v, i.e. v is pointing at
     * ":...".
     */
    if (ptr != v) {
      cols_valsz = ptr - (char *) v;
    }

  } else {
    cols_val = v;
    cols_valsz = vsz;
  }

  if (cols_valsz != 0) {
    ptr = memchr(cols_val, ',', cols_valsz);
    if (ptr == NULL) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'map' parameter '%.*s': missing column names",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = ptr - (char *) cols_val;
    *conf_id_col = pstrndup(p, cols_val, sz);

    cols_val = ptr + 1;
    cols_valsz = cols_valsz - sz - 1;
    ptr = memchr(cols_val, ':', cols_valsz);
    if (ptr == NULL) {
      *ctx_id_col = pstrndup(p, cols_val, cols_valsz);

    } else {
      *ctx_id_col = pstrndup(p, cols_val, sz);
    }

    ptr = memchr((char *) v + (char *) cols_valsz, ':', vsz - cols_valsz);
  }

  if (ptr != NULL) {
    size_t wheresz = 0;

    /* Possible WHERE clause included. */
    if (strncasecmp(ptr+1, "where=", 6) != 0) {
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted 'map' parameter '%.*s': bad WHERE clause",
        (int) valsz, (char *) val);
      errno = EINVAL;
      return -1;
    }

    sz = strlen(ptr+1);
    sqlconf_uri_urldecode(p, ptr+1, sz, where, &wheresz);
  }

  return 0;
}
