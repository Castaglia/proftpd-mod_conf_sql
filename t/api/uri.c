/*
 * ProFTPD - mod_conf_sql testsuite
 * Copyright (c) 2016 TJ Saunders <tj@castaglia.org>
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

START_TEST (uri_parse_test) {

  /* URLs to test:
   *
   *  foo
   *  foobarbaz
   *  sql://
   *  sql://host
   *  sql://host:port
   *  sql://host:port/path
   *  sql://host:port/path?
   *  sql://username:password@host:port/path?
   *  sql://ipv4
   *  sql://[ipv6]
   *
   * THEN there'll be the params parsing:
   *  bad formats
   *  unknown/unsupported key names
   *  case-sensitivity?
   */
}
END_TEST

Suite *tests_get_uri_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("uri");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, uri_parse_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
