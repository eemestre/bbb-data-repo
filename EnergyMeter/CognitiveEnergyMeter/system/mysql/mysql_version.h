/* Copyright Abandoned 1996,1999 TCX DataKonsult AB & Monty Program KB
   & Detron HB, 1996, 1999-2004, 2007 MySQL AB.
   This file is public domain and comes with NO WARRANTY of any kind
*/

/* Version numbers for protocol & mysqld */

#ifndef _mysql_version_h
#define _mysql_version_h
#ifdef _CUSTOMCONFIG_
#include <custom_conf.h>
#else
#define PROTOCOL_VERSION		10
#define MYSQL_SERVER_VERSION		"5.5.55"
#define MYSQL_BASE_VERSION		"mysqld-5.5"
#define MYSQL_SERVER_SUFFIX_DEF		"-0+deb7u1"
#define FRM_VER				6
#define MYSQL_VERSION_ID		50555
#define MYSQL_PORT			3306
#define MYSQL_PORT_DEFAULT		0
#define MYSQL_UNIX_ADDR			"/var/run/mysqld/mysqld.sock"
#define MYSQL_CONFIG_NAME		"my"
#define MYSQL_COMPILATION_COMMENT	"(Debian)"

/* mysqld compile time options */
#endif /* _CUSTOMCONFIG_ */

#ifndef LICENSE
#define LICENSE				GPL
#endif /* LICENSE */

#endif /* _mysql_version_h */
