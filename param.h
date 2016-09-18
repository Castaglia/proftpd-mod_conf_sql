/*
 * ProFTPD - mod_conf_sql URL Param API
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

#ifndef MOD_CONF_SQL_PARAM_H
#define MOD_CONF_SQL_PARAM_H

/* Expected format of the conf parameter:
 *
 *   conf=<table>[:id,key,value][:where=<clause>]
 */
int sqlconf_param_parse_conf(pool *p, pr_table_t *params, char **table,
  char **id_col, char **key_col, char **value_col, char **where);
#define CONF_SQL_CONF_DEFAULT_TABLE_NAME		"ftpconf_conf"
#define CONF_SQL_CONF_DEFAULT_ID_COL_NAME		"id"
#define CONF_SQL_CONF_DEFAULT_KEY_COL_NAME		"key"
#define CONF_SQL_CONF_DEFAULT_VALUE_COL_NAME		"value"

/* Expected format of the ctx parameter:
 *
 *   ctx=<table>[:id,parent_id,key,value][:where=<clause>]
 */
int sqlconf_param_parse_ctx(pool *p, pr_table_t *params, char **table,
  char **id_col, char **parent_id_col, char **key_col, char **value_col,
  char **where);
#define CONF_SQL_CTX_DEFAULT_TABLE_NAME			"ftpconf_ctx"
#define CONF_SQL_CTX_DEFAULT_ID_COL_NAME		"id"
#define CONF_SQL_CTX_DEFAULT_PARENT_ID_COL_NAME		"parent_id"
#define CONF_SQL_CTX_DEFAULT_KEY_COL_NAME		"key"
#define CONF_SQL_CTX_DEFAULT_VALUE_COL_NAME		"value"

/* Expected format of the map parameter:
 *
 *   map=<table>[:conf_id,ctx_id][:where=<clause>]
 */
int sqlconf_param_parse_map(pool *p, pr_table_t *params, char **table,
  char **conf_id_col, char **ctx_id_col, char **where);
#define CONF_SQL_MAP_DEFAULT_TABLE_NAME			"ftpconf_map"
#define CONF_SQL_MAP_DEFAULT_CONF_ID_COL_NAME		"conf_id"
#define CONF_SQL_MAP_DEFAULT_CTX_ID_COL_NAME		"ctx_id"

#endif /* MOD_CONF_SQL_PARAM_H */
