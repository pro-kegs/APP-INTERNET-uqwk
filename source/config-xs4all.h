/*
 * $Id: config-xs4all.h,v 1.4 2000/01/03 08:56:56 js Exp $
 *
 * This is an uqwk configuration file for the XS4ALL ISP
 */

#define CONFIGNAME      "XS4ALL"

#define DEF_BBS_NAME	"XS4ALL"
#define DEF_BBS_CITY	"Amsterdam, NL"
#define DEF_BBS_PHONE	"+31 30 3987654"
#define DEF_BBS_SYSOP	"xs4all helpdesk <helpdesk@xs4all.nl>"
#define DEF_BBS_ID	"0,XS4ALL"

#ifndef NNTP
  #define DEF_ACT_FILE	"/usr/local/news/lib/active"
  #define DEF_NEWS_DIR	"/usr/local/news/spool"
#endif
#define	DEF_MAIL_DIR	"/var/spool/mail"

#define SENDMAIL	"/usr/sbin/sendmail -i"
#define INEWS_PATH      "/usr/contrib/lib/news/inews"
#define NNTP_HOST_FILE  "/usr/local/news/lib/nntp_server"	/* news server file */
