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

/* URI API tests. */

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

START_TEST (uri_parse_args_test) {
  int res;
  const char *uri;
  char *host = NULL, *path = NULL, *username = NULL, *password = NULL;
  unsigned int port = 0;

  mark_point();
  res = sqlconf_uri_parse(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_parse(p, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null URI");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "foo";
  res = sqlconf_uri_parse(p, uri, NULL, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null host");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_parse(p, uri, &host, NULL, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null port");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_parse(p, uri, &host, &port, NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null path");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null username");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null password");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    NULL);
  ck_assert_msg(res < 0, "Failed to handle null params");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (uri_parse_scheme_test) {
  int res;
  const char *uri;
  char *host = NULL, *path = NULL, *username = NULL, *password = NULL;
  unsigned int port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  uri = "foo";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s'", uri);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "foobarbaz";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s'", uri);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "sql://";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s'", uri);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pr_table_free(params);
}
END_TEST

START_TEST (uri_parse_host_test) {
  int res;
  const char *uri;
  char *expected, *host = NULL, *path, *username = NULL, *password = NULL;
  unsigned int port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  uri = "sql://castaglia.org";
  expected = "castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(host != NULL, "Failed to parse host out of URI '%s'", uri);
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);

  mark_point();
  uri = "sql://127.0.0.1";
  expected = "127.0.0.1";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(host != NULL, "Failed to parse host out of URI '%s'", uri);
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);

  mark_point();
  uri = "sql://[::1]";
  expected = "::1";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(host != NULL, "Failed to parse host out of URI '%s'", uri);
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);

  mark_point();
  uri = "sql://[::1";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Parsed URI '%s' unexpectedly", uri);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "sql:///path/to/some/file";
  expected = "/path/to/some/file";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(host != NULL, "Failed to parse host out of URI '%s'", uri);
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);

  pr_table_free(params);
}
END_TEST

START_TEST (uri_parse_port_test) {
  int res;
  const char *uri;
  char *host = NULL, *path = NULL, *username = NULL, *password = NULL;
  unsigned int expected, port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  uri = "sql://castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(port == 0, "Failed to parse port out of URI '%s'", uri);

  mark_point();
  uri = "sql://castaglia.org:3567";
  expected = 3567;
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(port == expected, "Expected %u, got %u", expected, port);

  mark_point();
  uri = "sql://castaglia.org:foo";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s': %s", uri,
    strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "sql://castaglia.org:0";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s': %s", uri,
    strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "sql://castaglia.org:70000";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s': %s", uri,
    strerror(errno));
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pr_table_free(params);
}
END_TEST

START_TEST (uri_parse_userinfo_test) {
  int res;
  const char *uri;
  char *expected, *host = NULL, *path, *username = NULL, *password = NULL;
  unsigned int port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  uri = "sql://castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(username == NULL, "Expected null username, got %s", username);
  ck_assert_msg(password == NULL, "Expected null password, got %s", password);

  mark_point();
  uri = "sql://user@castaglia.org";
  expected = "user";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(username == NULL, "Expected null username, got %s", username);
  ck_assert_msg(password == NULL, "Expected null password, got %s", password);

  mark_point();
  uri = "sql://user:pass@castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));

  expected = "user";
  ck_assert_msg(username != NULL, "Expected username, got null");
  ck_assert_msg(strcmp(username, expected) == 0, "Expected '%s', got '%s'",
    expected, username);

  expected = "pass";
  ck_assert_msg(password != NULL, "Expected password, got null");
  ck_assert_msg(strcmp(password, expected) == 0, "Expected '%s', got '%s'",
    expected, password);

  mark_point();
  uri = "sql://user:@castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));

  expected = "user";
  ck_assert_msg(username != NULL, "Expected username, got null");
  ck_assert_msg(strcmp(username, expected) == 0, "Expected '%s', got '%s'",
    expected, username);

  expected = "";
  ck_assert_msg(password != NULL, "Expected password, got null");
  ck_assert_msg(strcmp(password, expected) == 0, "Expected '%s', got '%s'",
    expected, password);

  mark_point();
  uri = "sql://@castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to handle parse URI '%s': %s", uri,
    strerror(errno));
  ck_assert_msg(username == NULL, "Expected null username, got %s", username);
  ck_assert_msg(password == NULL, "Expected null password, got %s", password);

  pr_table_free(params);
}
END_TEST

START_TEST (uri_parse_path_test) {
  int res;
  const char *uri;
  char *expected, *host = NULL, *path = NULL, *username, *password;
  unsigned int port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  uri = "sql://castaglia.org";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(path == NULL, "Expected null path, got %s", path);

  mark_point();
  uri = "sql://castaglia.org/path";
  expected = "/path";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(path != NULL, "Expected path, got null");
  ck_assert_msg(strcmp(path, expected) == 0, "Expected '%s', got '%s'", expected,
    path);

  mark_point();
  uri = "sql://castaglia.org:1234/path/to/resource";
  expected = "/path/to/resource";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(path != NULL, "Expected path, got null");
  ck_assert_msg(strcmp(path, expected) == 0, "Expected '%s', got '%s'", expected,
    path);

  mark_point();
  uri = "sql://castaglia.org:1234/path/to/resource?key=val";
  expected = "/path/to/resource";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  ck_assert_msg(path != NULL, "Expected path, got null");
  ck_assert_msg(strcmp(path, expected) == 0, "Expected '%s', got '%s'", expected,
    path);

  pr_table_empty(params);
  pr_table_free(params);
}
END_TEST

START_TEST (uri_parse_params_test) {
  int res;
  const char *uri;
  char *host = NULL, *path = NULL, *username, *password;
  unsigned int port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  uri = "sql://castaglia.org?foo";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s'", uri);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  pr_table_empty(params);
  uri = "sql://castaglia.org?foo=";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  res = pr_table_count(params);
  ck_assert_msg(res == 1, "Expected 1 parameter, got %d", res);

  mark_point();
  uri = "sql://castaglia.org?foo=&";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res < 0, "Failed to handle invalid URI '%s'", uri);
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  uri = "sql://castaglia.org?foo=bar&foo=baz";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  res = pr_table_count(params);
  ck_assert_msg(res == 1, "Expected 1 parameter, got %d", res);

  mark_point();
  uri = "sql://castaglia.org?foo=bar&baz=quxx&foo=baz";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  res = pr_table_count(params);
  ck_assert_msg(res == 2, "Expected 2 parameters, got %d", res);

  pr_table_empty(params);
  pr_table_free(params);
}
END_TEST

START_TEST (uri_parse_real_uris_test) {
  int res;
  const char *uri;
  char *host = NULL, *path = NULL, *username = NULL, *password = NULL;
  char *expected;
  unsigned int port = 0;
  pr_table_t *params = NULL;

  params = pr_table_alloc(p, 1);

  mark_point();
  host = path = username = password = NULL;
  port = 0;
  pr_table_empty(params);
  uri = "sql://user:pass@server:12345/dbname";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  expected = "server";
  ck_assert_msg(host != NULL, "Expected host, got null");
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);
  expected = "/dbname";
  ck_assert_msg(path != NULL, "Expected path, got null");
  ck_assert_msg(strcmp(path, expected) == 0, "Expected '%s', got '%s'",
    expected, path);
  expected = "user";
  ck_assert_msg(username != NULL, "Expected username, got null");
  ck_assert_msg(strcmp(username, expected) == 0, "Expected '%s', got '%s'",
    expected, username);
  expected = "pass";
  ck_assert_msg(password != NULL, "Expected password, got null");
  ck_assert_msg(strcmp(password, expected) == 0, "Expected '%s', got '%s'",
    expected, password);
  ck_assert_msg(port == 12345, "Expected 12345, got %u", port);
  res = pr_table_count(params);
  ck_assert_msg(res == 0, "Expected 0 parameters, got %d", res);

  mark_point();
  host = path = username = password = NULL;
  port = 0;
  pr_table_empty(params);
  uri = "sql:///path/to/sqlite.db?database=dbname";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  expected = "/path/to/sqlite.db";
  ck_assert_msg(host != NULL, "Expected host, got null");
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);
  ck_assert_msg(path == NULL, "Expected null, got path '%s'", path);
  ck_assert_msg(username == NULL, "Expected null, got username '%s'", username);
  ck_assert_msg(password == NULL, "Expected null, got password '%s'", password);
  ck_assert_msg(port == 0, "Expected 0, got %u", port);
  res = pr_table_count(params);
  ck_assert_msg(res == 1, "Expected 1 parameter, got %d", res);

  /* Note that this will not parse as expected.  Using an absolute path as
   * the hostname makes it hard to determine the URL path, as the same
   * path separators are used.
   */
  mark_point();
  host = path = username = password = NULL;
  port = 0;
  pr_table_empty(params);
  uri = "sql:///path/to/sqlite.db/dbname";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  expected = "/path/to/sqlite.db/dbname";
  ck_assert_msg(host != NULL, "Expected host, got null");
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);
  ck_assert_msg(path == NULL, "Expected null, got path '%s'", path);
  ck_assert_msg(username == NULL, "Expected null, got username '%s'", username);
  ck_assert_msg(password == NULL, "Expected null, got password '%s'", password);
  ck_assert_msg(port == 0, "Expected 0, got %u", port);
  res = pr_table_count(params);
  ck_assert_msg(res == 0, "Expected 0 parameters, got %d", res);

  mark_point();
  host = path = username = password = NULL;
  port = 0;
  pr_table_empty(params);
  uri = "sql://foo:bar@localhost?database=proftpd&ctx=vhostctx&conf=vhostconf&map=vhostmap&base_id=7";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  expected = "localhost";
  ck_assert_msg(host != NULL, "Expected host, got null");
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);
  ck_assert_msg(path == NULL, "Expected null, got path '%s'", path);
  expected = "foo";
  ck_assert_msg(username != NULL, "Expected username, got null");
  ck_assert_msg(strcmp(username, expected) == 0, "Expected '%s', got '%s'",
    expected, username);
  expected = "bar";
  ck_assert_msg(password != NULL, "Expected password, got null");
  ck_assert_msg(strcmp(password, expected) == 0, "Expected '%s', got '%s'",
    expected, password);
  ck_assert_msg(port == 0, "Expected 0, got %u", port);
  res = pr_table_count(params);
  ck_assert_msg(res == 5, "Expected 5 parameters, got %d", res);

  mark_point();
  host = path = username = password = NULL;
  port = 0;
  pr_table_empty(params);
  uri = "sql:///Users/tj/git/proftpd-mod_conf_sql/proftpd.db?ctx=ftpctx:id,parent_id,name,type,value&conf=ftpconf:id,type,value";
  res = sqlconf_uri_parse(p, uri, &host, &port, &path, &username, &password,
    params);
  ck_assert_msg(res == 0, "Failed to parse URI '%s': %s", uri, strerror(errno));
  expected = "/Users/tj/git/proftpd-mod_conf_sql/proftpd.db";
  ck_assert_msg(host != NULL, "Expected host, got null");
  ck_assert_msg(strcmp(host, expected) == 0, "Expected '%s', got '%s'",
    expected, host);
  ck_assert_msg(path == NULL, "Expected null, got path '%s'", path);
  ck_assert_msg(username == NULL, "Expected null, got username");
  ck_assert_msg(password == NULL, "Expected null, got password");
  ck_assert_msg(port == 0, "Expected 0, got %u", port);
  res = pr_table_count(params);
  ck_assert_msg(res == 2, "Expected 2 parameters, got %d", res);

  pr_table_empty(params);
  pr_table_free(params);
}
END_TEST

START_TEST (uri_urldecode_test) {
  int res;
  const char *src;
  char *dst = NULL, *expected;
  size_t dstsz, expectedsz;

  mark_point();
  res = sqlconf_uri_urldecode(NULL, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_urldecode(p, NULL, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null src");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  src = "foo+bar%20baz";

  mark_point();
  res = sqlconf_uri_urldecode(p, src, 0, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null dst");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = sqlconf_uri_urldecode(p, src, 0, &dst, NULL);
  ck_assert_msg(res < 0, "Failed to handle null dstsz");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  dst = NULL;
  dstsz = 6789;

  mark_point();
  res = sqlconf_uri_urldecode(p, src, 0, &dst, &dstsz);
  ck_assert_msg(res == 0, "Failed to handle zero-length src: %s",
    strerror(errno));
  expected = "";
  ck_assert_msg(dst != NULL, "Expected dst, got null");
  ck_assert_msg(strcmp(dst, expected) == 0, "Expected '%s', got '%s'",
    expected, dst);
  ck_assert_msg(dstsz == 0, "Expected 0, got %lu", (unsigned long) dstsz);

  dst = NULL;
  dstsz = 0;

  mark_point();
  res = sqlconf_uri_urldecode(p, src, strlen(src), &dst, &dstsz);
  ck_assert_msg(res == 0, "Failed to handle src '%s': %s", src,
    strerror(errno));
  expected = "foo bar baz";
  ck_assert_msg(dst != NULL, "Expected dst, got null");
  ck_assert_msg(strcmp(dst, expected) == 0, "Expected '%s', got '%s'",
    expected, dst);

  expectedsz = strlen(expected);
  ck_assert_msg(dstsz == expectedsz, "Expected %lu, got %lu",
    (unsigned long) expectedsz, (unsigned long) dstsz);
}
END_TEST

Suite *tests_get_uri_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("uri");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, uri_parse_args_test);
  tcase_add_test(testcase, uri_parse_scheme_test);
  tcase_add_test(testcase, uri_parse_host_test);
  tcase_add_test(testcase, uri_parse_port_test);
  tcase_add_test(testcase, uri_parse_userinfo_test);
  tcase_add_test(testcase, uri_parse_path_test);
  tcase_add_test(testcase, uri_parse_params_test);
  tcase_add_test(testcase, uri_parse_real_uris_test);

  tcase_add_test(testcase, uri_urldecode_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
