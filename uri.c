/*
 * ProFTPD - mod_conf_sql URI implementation
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
#include "uri.h"

static char *uri_parse_host(pool *p, const char *orig_uri, const char *uri,
    char **remaining) {
  char *host = NULL, *ptr = NULL;

  /* We have either of:
   *
   *  host<:...>
   *  [host]<:...>
   *
   * Look for an opening square bracket, to see if we have an IPv6 address
   * in the URI.
   */
  if (uri[0] == '[') {
    ptr = strchr(uri + 1, ']');
    if (ptr == NULL) {
      /* If there is no ']', then it's a badly-formatted URI. */
      pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
        ": badly formatted IPv6 address in host info '%.100s'", orig_uri);
      errno = EINVAL;
      return NULL;
    }

    host = pstrndup(p, uri + 1, ptr - uri - 1);

    if (remaining != NULL) {
      size_t urilen;
      urilen = strlen(ptr);

      if (urilen > 0) {
        *remaining = ptr + 1;

      } else {
        *remaining = NULL;
      }
    }

    return host;
  }

  ptr = strchr(uri + 1, ':');
  if (ptr == NULL) {
    if (remaining != NULL) {
      *remaining = NULL;
    }

    host = pstrdup(p, uri);
    return host;
  }

  if (remaining != NULL) {
    *remaining = ptr;
  }

  host = pstrndup(p, uri, ptr - uri);
  return host;
}

static int uri_parse_port(pool *p, const char *orig_uri, const char *uri,
    unsigned int *port) {
  register unsigned int i;
  char *ptr, *ptr2, *portspec;
  size_t portspeclen;

  /* Look for any possible trailing '/'. */
  ptr = strchr(uri, '/');
  if (ptr == NULL) {
    portspec = ptr + 1;
    portspeclen = strlen(portspec);

  } else {
    portspeclen = uri - (ptr + 1);
    portspec = pstrndup(p, ptr + 1, portspeclen);
  }

  /* Ensure that only numeric characters appear in the portspec. */
  for (i = 0; i < portspeclen; i++) {
    if (isdigit((int) portspec[i]) == 0) {
      pr_log_debug(DEBUG2, MOD_CONF_SQL_VERSION
        ": invalid character (%c) at index %d in port specification '%.100s'",
        portspec[i], i, portspec);
      errno = EINVAL;
      return -1;
    }
  }

  /* The above check will rule out any negative numbers, since it will reject
   * the minus character.  Thus we only need to check for a zero port, or a
   * number that's outside the 1-65535 range.
   */
  *port = atoi(portspec);
  if (*port == 0 ||
      *port >= 65536) {
    pr_log_debug(DEBUG2, MOD_CONF_SQL_VERSION
      ": port specification '%.100s' yields invalid port number %d",
      portspec, *port);
    errno = EINVAL;
    return -1;
  }

  return 0;
}

/* Determine whether "username:password@" are present.  If so, then parse it
 * out, and return a pointer to the portion of the URI after the parsed-out
 * userinfo.
 */
static char *uri_parse_userinfo(pool *p, const char *orig_uri,
    const char *uri, char **username, char **password) {
  char *ptr, *ptr2, *rem_uri = NULL, *userinfo, *user = NULL, *passwd = NULL;

  /* We have either:
   *
   *  host<:...>
   *  [host]<:...>
   *
   * thus no user info, OR:
   *
   *  username:password@host...
   *  username:password@[host]...
   *  username:@host...
   *  username:pass@word@host...
   *  user@domain.com:pass@word@host...
   *
   * all of which have at least one occurrence of the '@' character.
   */

  ptr = strchr(uri, '@');
  if (ptr == NULL) {
    /* No '@' character at all?  No user info, then. */

    if (username != NULL) {
      *username = NULL;
    }

    if (password != NULL) {
      *password = NULL;
    }

    return pstrdup(p, uri);
  }

  /* To handle the case where the password field might itself contain an
   * '@' character, we first search from the end for '@'.  If found, then we
   * search for '@' from the beginning.  If also found, AND if both ocurrences
   * are the same, then we have a plain "username:password@" string.
   *
   * Note that we can handle '@' characters within passwords (or usernames),
   * but we currently cannot handle ':' characters within usernames.
   */

  ptr2 = strrchr(uri, '@');
  if (ptr2 != NULL) {
    if (ptr != ptr2) {
      /* Use the last found '@' as the delimiter. */
      ptr = ptr2;
    }
  }

  userinfo = pstrndup(p, uri, ptr - uri);
  rem_uri = ptr + 1;

  ptr = strchr(userinfo, ':');
  if (ptr == NULL) {
    if (username != NULL) {
      *username = NULL;
    }

    if (password != NULL) {
      *password = NULL;
    }

    return rem_uri;
  }

  user = pstrndup(p, userinfo, ptr - userinfo);
  if (username != NULL) {
    *username = user;
  }

  /* Watch for empty passwords. */
  if (*(ptr+1) == '\0') {
    passwd = pstrdup(p, "");

  } else {
    passwd = pstrdup(p, ptr + 1);
  }

  if (password != NULL) {
    *password = passwd;
  }

  return rem_uri;
}

static int uri_parse_params(pool *p, const char *orig_uri, const char *uri,
    pr_table_t *params) {
  errno = ENOSYS;
  return -1;
}

int sqlconf_uri_parse(pool *p, const char *orig_uri, char **host,
    unsigned int *port, char **username, char **password, pr_table_t *params) {
  pool *sub_pool;
  char *ptr, *ptr2, *uri;
  size_t len;

  if (p == NULL ||
      uri == NULL ||
      params == NULL) {
    errno = EINVAL;
    return -1;
  }

  len = strlen(uri);
  if (len < 7) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": unknown/unsupported scheme in URI '%.100s'", uri);
    errno = EINVAL;
    return -1;
  }

  if (strncmp(uri, "sql://", 6) != 0) {
    pr_log_debug(DEBUG0, MOD_CONF_SQL_VERSION
      ": unknown/unsupported scheme in URI '%.100s'", uri);
    errno = EINVAL;
    return -1;
  }

  sub_pool = make_sub_pool(p);

  uri = pstrdup(sub_pool, orig_uri + 6);
  ptr = uri;

  /* Possible URIs at this point:
   *
   *  host:port/dbname?...
   *  host:port?...
   *  host:port
   *  host?...
   *  host
   *  username:password@host...
   *
   * And, in the case where 'host' is an IPv6 address:
   *
   *  [host]:port/dbname?...
   *  [host]:port?...
   *  [host]:port
   *  [host]?...
   *  [host]
   *  username:password@[host]...
   */

  ptr = strchr(uri, '?');
  if (ptr != NULL) {
    if (uri_parse_params(sub_pool, uri, ptr + 1, params) < 0) {
      int xerrno = errno;

      destroy_pool(sub_pool);
      errno = xerrno;
      return -1;
    }

    *ptr = '\0';
  }

  /* Note: Will we want/need to support URL-encoded characters in the future? */

  ptr = uri_parse_userinfo(sub_pool, uri, ptr, username, password);

  ptr2 = strchr(ptr, ':');
  if (ptr2 == NULL) {
    *host = uri_parse_host(sub_pool, uri, ptr, NULL);

  } else {
    *host = uri_parse_host(sub_pool, uri, ptr, &ptr2);
  }

  /* Optional port field present? */
  if (ptr2 != NULL) {
    ptr2 = strchr(ptr2, ':');
    if (ptr2 != NULL) {
      if (uri_parse_port(sub_pool, uri, ptr2, port) < 0) {
        int xerrno = errno;

        destroy_pool(sub_pool);
        errno = xerrno;
        return -1;
      }
    }
  }

  destroy_pool(sub_pool);
  return 0;
}
