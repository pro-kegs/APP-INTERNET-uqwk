/*
 * $Id: options.c,v 1.6 1999/10/19 08:48:42 js Exp $
 */

#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include "uqwk.h"

/*
 *  Determine runtime options
 */

void DefaultOptions()
/*
 *  Set up default options
 */
{
	struct passwd *pw;

	/* Do user-specific stuff*/
	if (NULL == (pw = getpwuid(getuid()))) {
		fprintf (stderr, "%s: warning: you don't exist\n", progname);
		strcpy (user_name, DEF_USER_NAME);
		strcpy (home_dir, DEF_HOME_DIR);
	} else {
		strcpy (user_name, pw->pw_name);
		strcpy (home_dir, pw->pw_dir);
	}

	/* Dinky misc options */
	do_debug = DEF_DO_DEBUG;
	do_mail = DEF_DO_MAIL;
	do_news = DEF_DO_NEWS;
#ifdef SCORES
        do_kills = DEF_DO_KILLS;
	kill_thresh = 0;
#endif
	ignore_0fd = DEF_IGNORE0FD;
	inc_hdrs = DEF_INC_HDRS;
	prt_opts = DEF_PRT_OPTS;
	read_only = DEF_READ_ONLY;
	max_blks = DEF_MAX_BLKS;
	grp_len = DEF_GRP_LEN;
	waf_mode = DEF_WAF_MODE;
	slnp_mode = DEF_SLNP_MODE;
	zip_mode = DEF_ZIP_MODE;
	xrf_mode = DEF_XRF_MODE;
	bw_kludge = DEF_BW_KLUDGE;
	xprt_mode = DEF_XPRT_MODE;
	sum_mode = DEF_SUM_MODE;
	sel_mode = DEF_SEL_MODE;
	every_mode = DEF_EVERY_MODE;

#ifdef NNTPAUTH
	strcpy (auth_user, DEF_AUTH_USER);
	strcpy (auth_pass, DEF_AUTH_PASS);
#endif
	strcpy (mail_dir, DEF_MAIL_DIR);
	strcpy (mail_file, DEF_MAIL_FILE);
	strcpy (nrc_file, DEF_NRC_FILE);
#ifndef NNTP
	strcpy (act_file, DEF_ACT_FILE);
	strcpy (news_dir, DEF_NEWS_DIR);
#endif

	strcpy (bbs_name, DEF_BBS_NAME);
	strcpy (bbs_city, DEF_BBS_CITY);
	strcpy (bbs_phone, DEF_BBS_PHONE);
	strcpy (bbs_sysop, DEF_BBS_SYSOP);
	strcpy (bbs_id, DEF_BBS_ID);
	strcpy (rep_file, DEF_REP_FILE);
	strcpy (trn_file, DEF_TRN_FILE);
	strcpy (host_name, DEF_HOST_NAME);
	strcpy (sum_file, DEF_SUM_FILE);
	strcpy (sel_file, DEF_SEL_FILE);
	strcpy (ng_file, DEF_NG_FILE);

	strcpy (inews_path, INEWS_PATH);
#ifdef SCORES
	strcpy (kill_dir, home_dir);
	strcat (kill_dir, "/");
	strcat (kill_dir, DEF_KILL_DIR);
/*	strcpy (kill_dir, DEF_KILL_DIR);	*/
#endif
#ifdef	GNKSA
	gnksa_level = GNKSA_VERBOSE|GNKSA_HONOUR;
#endif
}

void EnvOptions()
/*
 *  Override options from environment variables
 */
{
	char *c;

	if (NULL != (c = getenv ("UQ_DO_DEBUG"))) do_debug = atoi (c);
	if (NULL != (c = getenv ("UQ_DO_MAIL"))) do_mail = atoi (c);
	if (NULL != (c = getenv ("UQ_DO_NEWS"))) do_news = atoi (c);
#ifdef SCORES
	if (NULL != (c = getenv ("UQ_DO_KILLS"))) do_kills = atoi (c);
	if (NULL != (c = getenv ("UQ_KILL_THRESHOLD"))) kill_thresh = atoi (c);
#endif
	if (NULL != (c = getenv ("UQ_IGNORE0FD"))) ignore_0fd = atoi (c);
	if (NULL != (c = getenv ("UQ_INC_HDRS"))) inc_hdrs = atoi (c);
	if (NULL != (c = getenv ("UQ_PRT_OPTS"))) prt_opts = atoi (c);
	if (NULL != (c = getenv ("UQ_READ_ONLY"))) read_only = atoi (c);
	if (NULL != (c = getenv ("UQ_MAX_BLKS"))) max_blks = atoi (c);
	if (NULL != (c = getenv ("UQ_GRP_LEN"))) grp_len = atoi (c);
	if (NULL != (c = getenv ("UQ_WAF_MODE"))) waf_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_ZIP_MODE"))) zip_mode = atoi (c);
	if(zip_mode) slnp_mode = 0;
	if (NULL != (c = getenv ("UQ_SOUP_MODE"))) slnp_mode = atoi (c);
	if(slnp_mode) zip_mode = 0;
	if (NULL != (c = getenv ("UQ_XRF_MODE"))) xrf_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_BW_KLUDGE"))) bw_kludge = atoi (c);
	if (NULL != (c = getenv ("UQ_XPRT_MODE"))) xprt_mode = atoi (c);
	if (NULL != (c = getenv ("UQ_EVERY_MODE"))) every_mode = atoi (c);

	if (NULL != (c = getenv ("UQ_HOME_DIR"))) strcpy (home_dir, c);
#ifdef NNTPAUTH
	if (NULL != (c = getenv ("UQ_AUTH_USER"))) strcpy (auth_user, c);
	if (NULL != (c = getenv ("UQ_AUTH_PASS"))) strcpy (auth_pass, c);
#endif
	if (NULL != (c = getenv ("UQ_MAIL_DIR")))
		strcpy (mail_dir, c);
	else
		if (NULL != (c = getenv ("MAIL"))) strcpy (mail_file, c);
	if (NULL != (c = getenv ("UQ_MAIL_FILE"))) strcpy (mail_file, c);
	if (NULL != (c = getenv ("UQ_USER_NAME"))) strcpy (user_name, c);

        if (NULL != (c = getenv ("UQ_INEWS_PATH"))) strcpy (inews_path, c);
#ifdef SCORES
        if (NULL != (c = getenv ("UQ_KILL_DIR"))) strcpy (kill_dir, c);
#endif
#ifdef	GNKSA
	if (NULL != (c = getenv ("UQ_VALIDATE"))) gnksa_level = atoi (c);
#endif

	if (NULL != (c = getenv ("UQ_BBS_NAME"))) strcpy (bbs_name, c);
	if (NULL != (c = getenv ("UQ_BBS_CITY"))) strcpy (bbs_city, c);
	if (NULL != (c = getenv ("UQ_BBS_PHONE"))) strcpy (bbs_phone, c);
	if (NULL != (c = getenv ("UQ_BBS_SYSOP"))) strcpy (bbs_sysop, c);
	if (NULL != (c = getenv ("UQ_BBS_ID"))) strcpy (bbs_id, c);

#ifndef NNTP
	if (NULL != (c = getenv ("UQ_ACT_FILE"))) strcpy (act_file, c);
	if (NULL != (c = getenv ("UQ_NEWS_DIR"))) strcpy (news_dir, c);
#endif
	if (NULL != (c = getenv ("UQ_NRC_FILE"))) strcpy (nrc_file, c);

	if (NULL != (c = getenv ("UQ_REP_FILE"))) strcpy (rep_file, c);
	if (NULL != (c = getenv ("UQ_TRN_FILE"))) strcpy (trn_file, c);
	if (NULL != (c = getenv ("UQ_HOST_NAME"))) strcpy (host_name, c);
	if (NULL != (c = getenv ("UQ_NG_FILE"))) strcpy (ng_file, c);

	if (NULL != (c = getenv ("UQ_SUM_FILE")))
	{
		strcpy (sum_file, c);
		sum_mode = 1;
	}
	if (NULL != (c = getenv ("UQ_SEL_FILE")))
	{
		strcpy (sel_file, c);
		sel_mode = 1;
	}
}

void CommandOptions (argc, argv)
int argc;
char *argv[];
/*
 *  Override options from command line
 */
{
	int i;

	for (i=1; i<argc; i++) {
		switch (argv[i][0]) {
		case '+':
			switch (argv[i][1]) {
			case '%':	do_debug = 1;
					break;

#ifdef NNTPAUTH
			case 'A':	/* auth user:pass follows */
					if(!argv[i][2]) {
						fprintf(stderr,"%s: empty AUTH info, trying without NNTP authentication\n", progname);
					} else {
						char *c;
						if((c=strchr(&argv[i][2],':')) != NULL) {
							/* user:pass */
					                *c++ = '\0';
					                strcpy(auth_user,&argv[i][2]);
					                strcpy(auth_pass,c);
							do_auth = 1;
						}
					}
					break;
#endif
			case 'F':	ignore_0fd = 1;
					break;

			case 'm':	do_mail = 1;
					break;

			case 'n':	do_news = 1;
					break;

#ifdef SCORES
			case 'k':       do_kills = 1;
			                break;
#endif

			case 'h':	inc_hdrs = 1;
					break;

			case 'r':	read_only = 1;
					break;

			case 'w':	waf_mode = 1;
					break;

			case 'L':	slnp_mode = 1;
					zip_mode = 0;
					break;

			case 'z':	zip_mode = 1;
					slnp_mode = 0;
					break;
#ifdef	GNKSA
			case 'V':	if(!argv[i][2])
						gnksa_level = GNKSA_VERBOSE | GNKSA_HONOUR;
					else
						gnksa_level = atoi (&argv[i][2]);
					break;
#endif
			case 'x':	xrf_mode = 1;
					break;

			case 'W':	bw_kludge = 1;
					break;

			case 'X':	xprt_mode = 1;
					break;

			case 'e':	every_mode = 1;
					break;

			default:	BadFlag (argv[i]);
					break;
			}
			break;

		case '-':
			switch (argv[i][1]) {
			case '%':	do_debug = 0;
					break;

#ifdef NNTPAUTH
			case 'A':	do_auth = 0;
					break;
#endif
			case 'F':	ignore_0fd = 0;
					break;

			case 'm':	do_mail = 0;
					break;

			case 'n':	do_news = 0;
					break;

#ifdef SCORES
			case 'k':       do_kills = 0;
			                break;

			case 'T':	kill_thresh = atoi (&argv[i][2]);
					break;
#endif
			case 'h':	inc_hdrs = 0;
					break;

			case 'r':	read_only = 0;
					break;

			case 'w':	waf_mode = 0;
					break;

			case 'v':
			case '?':
			case 'p':	prt_opts = 1;
					break;

			case 'I':	strcpy(inews_path,&argv[i][2]);
					break;

			case 'L':	slnp_mode = 0;
					break;

			case 'z':	zip_mode = 0;
					break;
#ifdef	GNKSA
			case 'V':	gnksa_level = 0;
					break;
#endif
			case 'x':	xrf_mode = 0;
					break;

			case 'W':	bw_kludge = 0;
					break;

			case 'X':	xprt_mode = 0;
					break;

			case 'e':	every_mode = 0;
					break;

			case 'M':	strcpy (mail_dir, &argv[i][2]);
					break;

			case 'f':	strcpy (mail_file, &argv[i][2]);
					break;

			case 'u':	strcpy (user_name, &argv[i][2]);
					break;

			case 'H':	strcpy (home_dir, &argv[i][2]);
					break;

			case 'b':	strcpy (bbs_name, &argv[i][2]);
					break;

			case 'c':	strcpy (bbs_city, &argv[i][2]);
					break;

			case 'P':	strcpy (bbs_phone, &argv[i][2]);
					break;

			case 's':	strcpy (bbs_sysop, &argv[i][2]);
					break;

			case 'i':	strcpy (bbs_id, &argv[i][2]);
					break;

#ifndef NNTP
			case 'a':	strcpy (act_file, &argv[i][2]);
					break;

			case 'S':	strcpy (news_dir, &argv[i][2]);
					break;
#endif

			case 'N':	strcpy (nrc_file, &argv[i][2]);
					break;

			case 'B':	max_blks = atoi (&argv[i][2]);
					break;

			case 'R':	strcpy (rep_file, &argv[i][2]);
					break;

			case 'l':	grp_len = atoi (&argv[i][2]);
					break;

			case 't':	strcpy (trn_file, &argv[i][2]);
					break;

			case 'd':	strcpy (host_name, &argv[i][2]);
					break;

			case 'U':	strcpy (sum_file, &argv[i][2]);
					sum_mode = 1;
					break;

			case 'E':	strcpy (sel_file, &argv[i][2]);
					sel_mode = 1;
					break;

			case 'D':	strcpy (ng_file, &argv[i][2]);
					break;

#ifdef SCORES
			case 'K':	strcpy (kill_dir, &argv[i][2]);
			                break;
#endif

			default:	BadFlag (argv[i]);
					break;
			}
			break;

		default:
			BadFlag (argv[i]);
			break;
		}
	}

	/* If mail file has not been overridden, set it */
	if (!strcmp (mail_file, DEF_MAIL_FILE)) {
		strcpy (mail_file, mail_dir);
		strcat (mail_file, "/");
		strcat (mail_file, user_name);
	}

	/* If .newsrc file has not been overridden, set it */
	if (!strcmp (nrc_file, DEF_NRC_FILE)) {
		strcpy (nrc_file, home_dir);
		strcat (nrc_file, "/.newsrc");
	}

	/* Set up posting agent (inews_path + '-h') */
	sprintf(posting_agent, "%s -h", inews_path);

	/* Some sanity checks */

	if(do_news && !slnp_mode && !zip_mode) {
		fprintf(stderr, "%s: The QWK format is incompatible with Usenet beyond repair;\n", progname);
		fprintf(stderr, "%s: please use another format to participate in Usenet!\n", progname);
	}

	/* Summary or selection mode implies do news */
	if (sum_mode || sel_mode) do_news = 1;

	/* Summary mode implies no mail */
	if (sum_mode) do_mail = 0;

	/* Summary mode implies no special packet format */
	if (sum_mode) zip_mode = slnp_mode = 0;

	/* Do auth iff both username and password are there */

#ifdef NNTPAUTH
	if(auth_user[0] && auth_pass[0]) {
		do_auth = 1;
	}
	if(do_auth && (!auth_user[0] || !auth_pass[0])) {
		fprintf(stderr,"%s: no username or password: trying without NNTP authentication\n",progname);
		do_auth = 0;
	}
#endif

	if (prt_opts) {
		PrintOptions();
		exit (0);
	}
}

int BadFlag (c)
char *c;
{
	fprintf (stderr, "%s: bad flag: %s\n", progname, c);
	exit (-1);
}

void PrintOptions ()
{
	printf ("Version: %s\n", UQWK_VERSION);
	printf ("Do mail: %s\n", do_mail?"yes":"no");
	printf ("Do news: %s\n", do_news?"yes":"no");
#ifdef NNTPAUTH
	printf ("Use NNTP authentication: %s\n", do_auth?"yes":"no");
	if(do_auth) {
		printf ("AUTH user: %s\n", auth_user);
		printf ("AUTH pass: %s\n", auth_pass);
	}
#endif
#ifdef SCORES
	printf ("Score articles: %s\n", do_kills?"yes":"no");
	if(do_kills) {
		printf ("Kill threshold: %d\n", kill_thresh);
		printf ("Score directory: %s\n", kill_dir);
		printf ("Global score file: %s/%s\n", kill_dir, GLOBAL_SCORE_FILE);
	}
#endif
#ifdef	GNKSA
	printf ("Validate articles before posting: %s, invalid articles are %s%s\n",
		gnksa_level?"yes":"no",
		(gnksa_level&GNKSA_HONOUR)?"not posted":"posted anyway",
		(gnksa_level&GNKSA_VERBOSE)?"\n\t(errors or warnings are reported)":""
		);
#endif
	printf ("SOUP mode: %s\n", slnp_mode?"yes":"no");
	printf ("ZipNews mode: %s\n", zip_mode?"yes":"no");
	printf ("QWK mode: %s\n", (slnp_mode||zip_mode)?"no":"yes");
	printf ("Xref mode: %s\n", xrf_mode?"yes":"no");
	printf ("Read only: %s\n", read_only?"yes":"no");
	printf ("Maximum blocks: %d\n", max_blks);
	printf ("Mail directory: %s\n", mail_dir);
#ifndef NNTP
	printf ("Active file: %s\n", act_file);
	printf ("News directory: %s\n", news_dir);
#endif
	printf ("Mail file: %s\n", mail_file);
	printf ("User name: %s\n", user_name);
	printf ("Home directory: %s\n", home_dir);
	printf (".newsrc file: %s\n", nrc_file);
	printf ("Reply file: %s\n", rep_file);
	printf ("Posting agent: %s, called as: %s\n", inews_path, posting_agent);
	printf ("Newsgroup name translation table: %s\n", trn_file);
	printf ("Host name: %s\n", host_name);
	printf ("Summary file: %s\n", sum_file);
	printf ("Selection file: %s\n", sel_file);
	printf ("Newsgroup handling order file: %s\n", ng_file);
	printf ("Interpret newsrc file as a Waffle \"join\" file: %s\n", waf_mode?"yes":"no");
	printf ("Expert mode (QWK): %s\n", xprt_mode?"yes":"no");
	printf ("Include headers (QWK): %s\n", inc_hdrs?"yes":"no");
	printf ("Every group in CONTROL.DAT (QWK): %s\n", every_mode?"yes":"no");
	printf ("Group name limit (QWK): %d\n", grp_len);
	printf ("Blue Wave kludge (QWK): %s\n", bw_kludge?"yes":"no");
	printf ("BBS name (QWK): %s\n", bbs_name);
	printf ("BBS city (QWK): %s\n", bbs_city);
	printf ("BBS phone (QWK): %s\n", bbs_phone);
	printf ("BBS sysop (QWK): %s\n", bbs_sysop);
	printf ("BBS id (QWK): %s\n", bbs_id);
}
