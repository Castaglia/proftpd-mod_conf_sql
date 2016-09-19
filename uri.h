/*
 * ProFTPD - mod_conf_sql URI API
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

#ifndef MOD_CONF_SQL_URI_H
#define MOD_CONF_SQL_URI_H

/* Expected format of the URI:
 *
 * sql://dbuser:dbpass@dbserver[:dbport][/dbname]\
 *   &ctx=<table>[:id,parent_id,key,value][:where=<clause>]\
 *   &conf=<table>[:id,key,value][:where=<clause>]\
 *   &map=<table>[:conf_id,ctx_id][:where=<clause>]\
 *   [&base_id=<name>]
 */
int sqlconf_uri_parse(pool *p, const char *uri, char **host, unsigned int *port,
  char **path, char **username, char **password, pr_table_t *params);

/* Returns a URL-decoded version of the given string. */
int sqlconf_uri_urldecode(pool *p, const char *src, size_t srcsz, char **dst,
  size_t *dstsz);

#endif /* MOD_CONF_SQL_URI_H */
