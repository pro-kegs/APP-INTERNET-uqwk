/*
 * $Id: config-generic.h,v 1.5 2000/01/03 08:56:56 js Exp $
 *
 * Generic uqwk config
 */

#define CONFIGNAME	"generic"

#define DEF_BBS_NAME	"The BBS That Shall Not Be Named"
#define DEF_BBS_CITY	"Nowhere, Utopia"
#define DEF_BBS_PHONE	"555-1212"
#define DEF_BBS_SYSOP	"Joe A. Sysop"
#define DEF_BBS_ID	"0,TBSNBN-BBS"

#ifndef NNTP
  #define DEF_ACT_FILE	"/usr/lib/news/active"
  #define DEF_NEWS_DIR	"/var/spool/news"
#endif
#define	DEF_MAIL_DIR	"/var/spool/mail"

#define SENDMAIL	"/usr/lib/sendmail -i"
#define INEWS_PATH      "/usr/local/bin/inews"
#define NNTP_HOST_FILE  "/usr/local/news/lib/nntp_server"	/* news server file */
