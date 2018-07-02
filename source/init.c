/*
 *  $Id: init.c,v 1.3 1999/04/14 13:44:49 js Exp $
 */

#include <stdio.h>
#include <string.h>
#include "uqwk.h"

#define QWK_MAGIC "Produced by Qmail...Copyright (c) 1987 by Sparkware.  All rights Reserved"

#ifdef NNTP
#include "nntp.h"
extern void connect_nntp();
#endif

/*
 *  Initialize stuff
 */
void InitStuff()
{
	char msg_fname[PATH_LEN];

	/* Mail, conference, etc. lists */
	mail_list = NULL;
	conf_list = NULL;
	last_conf = NULL;
	act_list = NULL;
	nrc_list = NULL;
	trn_list = NULL;
	ng_list = NULL;

	/* Message and conference counts */
	msg_cnt = 0;
	conf_cnt = 0;

	/* Kludge around fclose() bug in close.c */
	msg_fd = NULL;

	/* Open MESSAGES.DAT */
	if (!slnp_mode && !zip_mode && !sum_mode) {
		strcpy (msg_fname, home_dir);
		strcat (msg_fname, "/");
		strcat (msg_fname, "messages.dat");

		if (NULL == (msg_fd = fopen (msg_fname, "w"))) {
			fprintf (stderr, "%s: can't open %s\n",
				progname, msg_fname);
			exit(-1);	/* Return failure errorcode to caller */
		}

		/* Write magic string to MESSAGES.DAT */
		if(	(fprintf(msg_fd, "%-128s", QWK_MAGIC) < 0) /* != 128? */
			&& ferror(msg_fd) ) {
			fprintf(stderr,"%s: write error!\n",progname);
			exit(-1);
		}
		blk_cnt = 2;
	}

	/* Open summary file */
	if (sum_mode) {
		if (NULL == (sum_fd = fopen (sum_file, "w"))) {
			fprintf (stderr, "%s: can't open %s\n",
					progname, sum_file);
			exit(-1);	/* Return failure errorcode to caller */
		}
	}

	/* Read the ng trans table if specified */
	if (strcmp (trn_file, DEF_TRN_FILE)) ReadTrans();

	/* Read the desired newsgroups file if specified */
	if (strcmp (ng_file, DEF_NG_FILE)) ReadNG();

#ifdef NNTP
/* JS -- no need to open a connection if replying only!
	if (strcmp (rep_file, DEF_REP_FILE) || do_news) connect_nntp();
*/
	if (do_news) connect_nntp();

#endif
}

int ReadTrans()
/*
 *  Read the newsgroup name translation table
 */
{
	FILE *trn_fd;
	struct trn_ent *tp;
	char n1[PATH_LEN], n2[PATH_LEN];

	/* Open the file */
	if (NULL == (trn_fd = fopen (trn_file, "r"))) {
		fprintf (stderr, "%s: can't open %s, tran table not used\n",
			progname, trn_file);
		exit(-1);	/* Return failure errorcode to caller */
	}

	/* Read through the file */
	while (2 == fscanf (trn_fd, "%s %s", n1, n2)) {
		/* Get space for new list entry */

		if (NULL == (tp = (struct trn_ent *)
				malloc (sizeof (struct trn_ent)))) {
			OutOfMemory();
		}
		if (NULL == (tp->old = (char *) malloc (strlen(n1)+1))) {
			OutOfMemory();
		}
		if (NULL == (tp->new = (char *) malloc (strlen(n2)+1))) {
			OutOfMemory();
		}

		/* Fill in new entry */
		strcpy (tp->old, n1);
		strcpy (tp->new, n2);

		/* Add to list */
		tp->next = trn_list;
		trn_list = tp;
	}

	fclose (trn_fd);
	return (0);
}

int ReadNG()
/*
 *  Read the desired newsgroups file
 */
{
	FILE *ng_fd;
	struct ng_ent *np, *last_np = NULL;
	char n[PATH_LEN];

	/* Open the file */
	if (NULL == (ng_fd = fopen (ng_file, "r"))) {
		fprintf (stderr, "%s: can't open %s, ng file not used\n",
			progname, ng_file);
		exit(-1);	/* Return failure errorcode to caller */
	}

	/* Read through the file */
	while (1 == fscanf (ng_fd, "%s", n)) {
		/* Get space for new list entry */

		if (NULL == (np = (struct ng_ent *)
				malloc (sizeof (struct ng_ent)))) {
			OutOfMemory();
		}
		if (NULL == (np->name = (char *) malloc (strlen(n)+1))) {
			OutOfMemory();
		}

		/* Fill in new entry */
		strcpy (np->name, n);

		/* Add to list */
		if (ng_list == NULL) {
			/* First one */
			ng_list = np;
		} else {
			/* Add to end */
			last_np->next = np;
		}
		np->next = NULL;
		last_np = np;
	}

	fclose (ng_fd);
	return (0);
}
