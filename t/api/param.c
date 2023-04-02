/*
 * ProFTPD - mod_conf_sql testsuite
 * Copyright (c) 2016-2022 TJ Saunders <tj@castaglia.org>
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

/* Param API tests. */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = make_sub_pool(NULL);
  }
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
  }
}

/* Expected format of the conf parameter:
 *
 *   conf=<table>[:id,name,value][:where=<clause>]
 */
START_TEST (param_parse_conf_test) {
  int res;
  char *table, *id_col, *name_col, *value_col, *where, *param, *expected;
  pr_table_t *params;

  mark_point();
  res = sqlconf_param_parse_conf(NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_conf(p, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null params");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  params = pr_table_alloc(p, 1);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null id_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null name_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col, NULL,
    NULL);
  ck_assert_msg(res < 0, "Failed to handle null value_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, NULL);
  ck_assert_msg(res < 0, "Failed to handle null where");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* No "conf" parameter. */
  pr_table_empty(params);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  ck_assert_msg(table == NULL, "Expected null, got table '%s'", table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(name_col == NULL, "Expected null, got name_col '%s'", name_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* Empty "conf" parameter. */
  pr_table_empty(params);

  param = "";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  ck_assert_msg(table == NULL, "Expected null, got table '%s'", table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(name_col == NULL, "Expected null, got name_col '%s'", name_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "conf" parameter with just the table name. */
  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "table";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(name_col == NULL, "Expected null, got name_col '%s'", name_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "conf" parameter with table name and WHERE clause, no column names. */
  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "table::WHERE=bar";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(name_col == NULL, "Expected null, got name_col '%s'", name_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  expected = "WHERE=bar";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* "conf" parameter with table name, and column names, and no WHERE clause. */
  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "my_table:my_id_col,my_name_col,my_val_col";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  expected = "my_table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_id_col";
  ck_assert_msg(id_col != NULL, "Expected id_col, got null");
  ck_assert_msg(strcmp(id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, id_col);
  expected = "my_name_col";
  ck_assert_msg(name_col != NULL, "Expected name_col, got null");
  ck_assert_msg(strcmp(name_col, expected) == 0, "Expected '%s', got '%s'",
    expected, name_col);
  expected = "my_val_col";
  ck_assert_msg(value_col != NULL, "Expected value_col, got null");
  ck_assert_msg(strcmp(value_col, expected) == 0, "Expected '%s', got '%s'",
    expected, value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "conf" parameter with table name, column names, and WHERE clause. */
  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "my_table:my_id_col,my_name_col,my_val_col:WHERE=barbaz";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  expected = "my_table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_id_col";
  ck_assert_msg(id_col != NULL, "Expected id_col, got null");
  ck_assert_msg(strcmp(id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, id_col);
  expected = "my_name_col";
  ck_assert_msg(name_col != NULL, "Expected name_col, got null");
  ck_assert_msg(strcmp(name_col, expected) == 0, "Expected '%s', got '%s'",
    expected, name_col);
  expected = "my_val_col";
  ck_assert_msg(value_col != NULL, "Expected value_col, got null");
  ck_assert_msg(strcmp(value_col, expected) == 0, "Expected '%s', got '%s'",
    expected, value_col);
  expected = "WHERE=barbaz";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* "conf" parameter with table name, column names, and a complex WHERE
   * clause.
   *
   * Note: To generate the URI-escaped WHERE clause, I used Perl:
   *
   *  $ perl -MURI::Escape=uri_escape -e 'print uri_escape(...);'
   */
  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "my_table:my_id_col,my_name_col,my_val_col:WHERE=foo%20%3D%201%20AND%20bar%20%3D%20%27baz%27";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse conf param: %s", strerror(errno));
  expected = "my_table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_id_col";
  ck_assert_msg(id_col != NULL, "Expected id_col, got null");
  ck_assert_msg(strcmp(id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, id_col);
  expected = "my_name_col";
  ck_assert_msg(name_col != NULL, "Expected name_col, got null");
  ck_assert_msg(strcmp(name_col, expected) == 0, "Expected '%s', got '%s'",
    expected, name_col);
  expected = "my_val_col";
  ck_assert_msg(value_col != NULL, "Expected value_col, got null");
  ck_assert_msg(strcmp(value_col, expected) == 0, "Expected '%s', got '%s'",
    expected, value_col);
  expected = "WHERE=foo = 1 AND bar = 'baz'";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* Malformed "conf" parameters. */
  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "foo:bar";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res < 0, "Failed to handle invalid conf param '%s': %s",
    param, strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  table = id_col = name_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "foo::bar";
  pr_table_add(params, pstrdup(p, "conf"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_conf(p, params, &table, &id_col, &name_col,
    &value_col, &where);
  ck_assert_msg(res < 0, "Failed to handle invalid conf param '%s': %s",
    param, strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pr_table_empty(params);
  pr_table_free(params);
}
END_TEST

/* Expected format of the ctx parameter:
 *
 *   ctx=<table>[:id,parent_id,type,value][:where=<clause>]
 */
START_TEST (param_parse_ctx_test) {
  int res;
  char *table, *id_col, *parent_id_col, *type_col, *value_col, *where;
  char *param, *expected;
  pr_table_t *params;

  mark_point();
  res = sqlconf_param_parse_ctx(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_ctx(p, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null params");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  params = pr_table_alloc(p, 1);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, NULL, NULL, NULL, NULL,
    NULL);
  ck_assert_msg(res < 0, "Failed to handle null id_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, NULL, NULL, NULL,
    NULL);
  ck_assert_msg(res < 0, "Failed to handle null parent_id_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null type_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null value_col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, NULL);
  ck_assert_msg(res < 0, "Failed to handle null where");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* No "ctx" parameter. */
  pr_table_empty(params);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  ck_assert_msg(table == NULL, "Expected null, got table '%s'", table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(parent_id_col == NULL, "Expected null, got parent_id_col '%s'",
    parent_id_col);
  ck_assert_msg(type_col == NULL, "Expected null, got type_col '%s'", type_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* Empty "ctx" parameter. */
  pr_table_empty(params);

  param = "";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  ck_assert_msg(table == NULL, "Expected null, got table '%s'", table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(parent_id_col == NULL, "Expected null, got parent_id_col '%s'",
    parent_id_col);
  ck_assert_msg(type_col == NULL, "Expected null, got type_col '%s'", type_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "ctx" parameter with just the table name. */
  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "table";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(parent_id_col == NULL, "Expected null, got parent_id_col '%s'",
    parent_id_col);
  ck_assert_msg(type_col == NULL, "Expected null, got type_col '%s'", type_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "ctx" parameter with table name and WHERE clause, no column names. */
  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "table::WHERE=bar";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  ck_assert_msg(id_col == NULL, "Expected null, got id_col '%s'", id_col);
  ck_assert_msg(parent_id_col == NULL, "Expected null, got parent_id_col '%s'",
    parent_id_col);
  ck_assert_msg(type_col == NULL, "Expected null, got type_col '%s'", type_col);
  ck_assert_msg(value_col == NULL, "Expected null, got value_col '%s'",
    value_col);
  expected = "WHERE=bar";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* "ctx" parameter with table name, column names, and no WHERE clause. */
  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "my_table:my_id_col,my_parentid_col,my_type_col,my_val_col";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  expected = "my_table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_id_col";
  ck_assert_msg(id_col != NULL, "Expected id_col, got null");
  ck_assert_msg(strcmp(id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, id_col);
  expected = "my_parentid_col";
  ck_assert_msg(parent_id_col != NULL, "Expected parent_id_col, got null");
  ck_assert_msg(strcmp(parent_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, parent_id_col);
  expected = "my_type_col";
  ck_assert_msg(type_col != NULL, "Expected type_col, got null");
  ck_assert_msg(strcmp(type_col, expected) == 0, "Expected '%s', got '%s'",
    expected, type_col);
  expected = "my_val_col";
  ck_assert_msg(value_col != NULL, "Expected value_col, got null");
  ck_assert_msg(strcmp(value_col, expected) == 0, "Expected '%s', got '%s'",
    expected, value_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "ctx" parameter with table name, column names, and WHERE clause. */
  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "my_table:my_id_col,my_parentid_col,my_type_col,my_val_col:WHERE=barbaz";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  expected = "my_table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_id_col";
  ck_assert_msg(id_col != NULL, "Expected id_col, got null");
  ck_assert_msg(strcmp(id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, id_col);
  expected = "my_parentid_col";
  ck_assert_msg(parent_id_col != NULL, "Expected parent_id_col, got null");
  ck_assert_msg(strcmp(parent_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, parent_id_col);
  expected = "my_type_col";
  ck_assert_msg(type_col != NULL, "Expected type_col, got null");
  ck_assert_msg(strcmp(type_col, expected) == 0, "Expected '%s', got '%s'",
    expected, type_col);
  expected = "my_val_col";
  ck_assert_msg(value_col != NULL, "Expected value_col, got null");
  ck_assert_msg(strcmp(value_col, expected) == 0, "Expected '%s', got '%s'",
    expected, value_col);
  expected = "WHERE=barbaz";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* "ctx" parameter with table name, column names, and a complex WHERE
   * clause.
   *
   * Note: To generate the URI-escaped WHERE clause, I used Perl:
   *
   *  $ perl -MURI::Escape=uri_escape -e 'print uri_escape(...);'
   */
  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "my_table:my_id_col,my_parentid_col,my_type_col,my_val_col:WHERE=foo+%3D+1+AND+bar+%3D+%27baz%27";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res == 0, "Failed to parse ctx param: %s", strerror(errno));
  expected = "my_table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_id_col";
  ck_assert_msg(id_col != NULL, "Expected id_col, got null");
  ck_assert_msg(strcmp(id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, id_col);
  expected = "my_parentid_col";
  ck_assert_msg(parent_id_col != NULL, "Expected parent_id_col, got null");
  ck_assert_msg(strcmp(parent_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, parent_id_col);
  expected = "my_type_col";
  ck_assert_msg(type_col != NULL, "Expected type_col, got null");
  ck_assert_msg(strcmp(type_col, expected) == 0, "Expected '%s', got '%s'",
    expected, type_col);
  expected = "my_val_col";
  ck_assert_msg(value_col != NULL, "Expected value_col, got null");
  ck_assert_msg(strcmp(value_col, expected) == 0, "Expected '%s', got '%s'",
    expected, value_col);
  expected = "WHERE=foo = 1 AND bar = 'baz'";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* Malformed "ctx" parameters. */
  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "foo:bar";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res < 0, "Failed to handle invalid ctx param '%s': %s",
    param, strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  table = id_col = parent_id_col = type_col = value_col = where = NULL;
  pr_table_empty(params);

  param = "foo::bar";
  pr_table_add(params, pstrdup(p, "ctx"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_ctx(p, params, &table, &id_col, &parent_id_col,
    &type_col, &value_col, &where);
  ck_assert_msg(res < 0, "Failed to handle invalid ctx param '%s': %s",
    param, strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pr_table_empty(params);
  pr_table_free(params);
}
END_TEST

/* Expected format of the map parameter:
 *
 *   map=<table>[:conf_id,ctx_id][:where=<clause>]
 */
START_TEST (param_parse_map_test) {
  int res;
  char *table, *conf_id_col, *ctx_id_col, *where, *param, *expected;
  pr_table_t *params;

  mark_point();
  res = sqlconf_param_parse_map(NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_map(p, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null params");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  params = pr_table_alloc(p, 1);

  mark_point();
  res = sqlconf_param_parse_map(p, params, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null conf ID col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null ctx ID col");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    NULL);
  ck_assert_msg(res < 0, "Failed to handle null where");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  /* No "map" parameter. */
  pr_table_empty(params);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  ck_assert_msg(table == NULL, "Expected null, got table '%s'", table);
  ck_assert_msg(conf_id_col == NULL, "Expected null, got conf_id_col '%s'",
    conf_id_col);
  ck_assert_msg(ctx_id_col == NULL, "Expected null, got ctx_id_col '%s'",
    ctx_id_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* Empty "map" parameter. */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  ck_assert_msg(table == NULL, "Expected null, got table '%s'", table);
  ck_assert_msg(conf_id_col == NULL, "Expected null, got conf_id_col '%s'",
    conf_id_col);
  ck_assert_msg(ctx_id_col == NULL, "Expected null, got ctx_id_col '%s'",
    ctx_id_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "map" parameter with just the table name. */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "table";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  ck_assert_msg(conf_id_col == NULL, "Expected null, got conf_id_col '%s'",
    conf_id_col);
  ck_assert_msg(ctx_id_col == NULL, "Expected null, got ctx_id_col '%s'",
    ctx_id_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "map" parameter table name and WHERE clause, no column names. */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "table::WHERE=foo";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  ck_assert_msg(conf_id_col == NULL, "Expected null, got conf_id_col '%s'",
    conf_id_col);
  ck_assert_msg(ctx_id_col == NULL, "Expected null, got ctx_id_col '%s'",
    ctx_id_col);
  expected = "WHERE=foo";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* "map" parameter table name, column names, and no WHERE clause. */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "table:my_conf_id,my_ctx_id";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_conf_id";
  ck_assert_msg(conf_id_col != NULL, "Expected conf_id_col, got null");
  ck_assert_msg(strcmp(conf_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, conf_id_col);
  expected = "my_ctx_id";
  ck_assert_msg(ctx_id_col != NULL, "Expected ctx_id_col, got null");
  ck_assert_msg(strcmp(ctx_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, ctx_id_col);
  ck_assert_msg(where == NULL, "Expected null, got where '%s'", where);

  /* "map" parameter table name, column names, and WHERE clause. */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "table:my_conf_id,my_ctx_id:WHERE=foo";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_conf_id";
  ck_assert_msg(conf_id_col != NULL, "Expected conf_id_col, got null");
  ck_assert_msg(strcmp(conf_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, conf_id_col);
  expected = "my_ctx_id";
  ck_assert_msg(ctx_id_col != NULL, "Expected ctx_id_col, got null");
  ck_assert_msg(strcmp(ctx_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, ctx_id_col);
  expected = "WHERE=foo";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* "map" parameter table name, column names, and a complex WHERE clause.
   *
   * Note: To generate the URI-escaped WHERE clause, I used Perl:
   *
   *  $ perl -MURI::Escape=uri_escape -e 'print uri_escape(...);'
   */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "table:my_conf_id,my_ctx_id:WHERE=foo+%3D+1%20AND%20bar%20%3D+%27baz%27";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res == 0, "Failed to parse map param: %s", strerror(errno));
  expected = "table";
  ck_assert_msg(table != NULL, "Expected table, got null");
  ck_assert_msg(strcmp(table, expected) == 0, "Expected '%s', got '%s'",
    expected, table);
  expected = "my_conf_id";
  ck_assert_msg(conf_id_col != NULL, "Expected conf_id_col, got null");
  ck_assert_msg(strcmp(conf_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, conf_id_col);
  expected = "my_ctx_id";
  ck_assert_msg(ctx_id_col != NULL, "Expected ctx_id_col, got null");
  ck_assert_msg(strcmp(ctx_id_col, expected) == 0, "Expected '%s', got '%s'",
    expected, ctx_id_col);
  expected = "WHERE=foo = 1 AND bar = 'baz'";
  ck_assert_msg(where != NULL, "Expected where, got null");
  ck_assert_msg(strcmp(where, expected) == 0, "Expected '%s', got '%s'",
    expected, where);

  /* Malformed "map" parameters. */
  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "foo:bar";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res < 0, "Failed to handle invalid map param '%s': %s",
    param, strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  table = conf_id_col = ctx_id_col = where = NULL;
  pr_table_empty(params);

  param = "foo::bar";
  pr_table_add(params, pstrdup(p, "map"), pstrdup(p, param), 0);

  mark_point();
  res = sqlconf_param_parse_map(p, params, &table, &conf_id_col, &ctx_id_col,
    &where);
  ck_assert_msg(res < 0, "Failed to handle invalid map param '%s': %s",
    param, strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pr_table_empty(params);
  pr_table_free(params);
}
END_TEST

Suite *tests_get_param_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("param");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, param_parse_conf_test);
  tcase_add_test(testcase, param_parse_ctx_test);
  tcase_add_test(testcase, param_parse_map_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
