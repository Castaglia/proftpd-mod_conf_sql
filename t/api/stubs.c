/*
 * ProFTPD - mod_conf_sql API testsuite
 * Copyright (c) 2016-2020 TJ Saunders <tj@castaglia.org>
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

#include "tests.h"

/* Stubs */

session_t session;
int ServerUseReverseDNS = FALSE;
server_rec *main_server = NULL;
pid_t mpid = 1;
unsigned char is_master = TRUE;
unsigned int recvd_signal_flags = 0;
module *static_modules[] = { NULL };
module *loaded_modules = NULL;
xaset_t *server_list = NULL;

void pr_alarms_block(void) {
}

void pr_alarms_unblock(void) {
}

void pr_log_debug(int level, const char *fmt, ...) {
  if (getenv("TEST_VERBOSE") != NULL) {
    va_list msg;

    fprintf(stderr, "DEBUG%d: ", level);

    va_start(msg, fmt);
    vfprintf(stderr, fmt, msg);
    va_end(msg);

    fprintf(stderr, "\n");
  }
}

void pr_log_pri(int prio, const char *fmt, ...) {
  if (getenv("TEST_VERBOSE") != NULL) {
    va_list msg;

    fprintf(stderr, "PRI%d: ", prio);

    va_start(msg, fmt);
    vfprintf(stderr, fmt, msg);
    va_end(msg);

    fprintf(stderr, "\n");
  }
}

void pr_signals_handle(void) {
}

int pr_snprintf(char *buf, size_t bufsz, const char *fmt, ...) {
  int res;
  va_list msg;

  va_start(msg, fmt);
  res = vsnprintf(buf, fmt, msg);
  va_end(msg);

  return res;
}

int pr_trace_msg(const char *trace_channel, int level, const char *fmt, ...) {
  if (getenv("TEST_VERBOSE") != NULL) {
    va_list msg;

    fprintf(stderr, "<%s:%d>: ", trace_channel, level);

    va_start(msg, fmt);
    vfprintf(stderr, fmt, msg);
    va_end(msg);

    fprintf(stderr, "\n");
  }

  return 0;
}
