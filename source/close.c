/*
 * $Id: close.c,v 1.7 2001/10/25 20:22:20 js Exp $
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "uqwk.h"

#ifdef NNTP
#include "nntp.h"
extern void close_server();
#endif

/*
 *  Wrap things up
 */

int _CloseStuff()
{
#ifdef NNTP
	/*  JS -- server might have been connected to implicitly! */
	close_server();
#endif

	if (msg_fd != NULL) fclose (msg_fd);

	/* Write QWK CONTROL.DAT file (or SLNP AREAS file) */
	if (!zip_mode && !sum_mode && (do_mail || do_news)
	&&  WriteControl() < 0)
		return -1;

	/* Close summary file */
	if (sum_mode) fclose (sum_fd);

	/* Update .newsrc */
	if (do_news && (!read_only) && WriteNewsrc() < 0)
		return(-2);

	if ( (blk_cnt >= max_blks) && (max_blks > 0) ) {
		fprintf (stderr,
			"%s: block count exceeded; some articles not packed\n",
			progname);
	}

	/* Remove reply packet */
	if ( (!read_only) && (strcmp (rep_file, DEF_REP_FILE)))
		unlink (rep_file);

	return 0;
}

int CloseStuff()
{
  static int closed = 0;
  static int ret = 0;
 
  if (!closed++) {
    ret = _CloseStuff();
    if(ret < 0)
      fprintf (stderr, "%s: error closing up\n", progname);
  }
  return ret;
}

int WriteControl()
/*
 *  Create the CONTROL.DAT file (or AREAS if SLNP)
 */
{
	struct conf_ent *cp;
	struct tm *t;
	char ctl_fname[PATH_LEN];
	time_t clock;
	int n;

	if(!conf_list)
		return 0;

	strcpy (ctl_fname, home_dir);
	strcat (ctl_fname, "/");

	if (slnp_mode)
		strcat (ctl_fname, "AREAS");
	else
		strcat (ctl_fname, "control.dat");

	if (NULL == (ctl_fd = fopen (ctl_fname, "w"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, ctl_fname);
		return -1;	/* Return failure error code to caller */
	}

	/* SLNP AREAS file is different */
	if (slnp_mode)
		return WriteAreas();

	if(fprintf(ctl_fd, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",
		bbs_name, bbs_city, bbs_phone, bbs_sysop, bbs_id) < 0
	&& ferror(ctl_fd)) {
		fprintf(stderr,"%s: can't write to %s\n",
			progname, ctl_fname);
		return -1;
	}

	/* Date */
	clock = time (NULL);
	t = gmtime (&clock);
	if(fprintf(ctl_fd, "%02d-%02d-%04d,%02d:%02d:%02d\r\n",
		t->tm_mon+1, t->tm_mday, t->tm_year+1900,
		t->tm_hour, t->tm_min, t->tm_sec) < 0
	&& ferror(ctl_fd)) {
		fprintf(stderr,"%s: can't write to %s\n",
			progname, ctl_fname);
		return -1;
	}

	if(fprintf (ctl_fd, "%s\r\n \r\n0\r\n", user_name) < 0
	&& ferror(ctl_fd)) {
		fprintf(stderr,"%s: can't write to %s\n",
			progname, ctl_fname);
		return -1;
	}

	/* Count conferences with articles */
	n = 0;
	cp = conf_list;
	while (cp != NULL) {
		if (every_mode || (cp->count > 0)) n++;
		cp = cp->next;
	}
	if(fprintf (ctl_fd, "%d\r\n%d\r\n", msg_cnt, n-1) < 0
	&& ferror(ctl_fd)) {
		fprintf(stderr,"%s: can't write to %s\n",
			progname, ctl_fname);
		return -1;
	}

	/* List of conferences */
	cp = conf_list;
	while (cp != NULL) {
		if (every_mode || (cp->count > 0)) {
			strcpy (buf, cp->name);

			/* Translate the name if desired */
			if (trn_list != NULL) TransName (buf);

			/* Truncate the group name if desired */
			if ( (grp_len > 0) && (grp_len < BUF_LEN) )
				buf[grp_len] = 0;

			if(fprintf(ctl_fd, "%d\r\n%s\r\n", cp->number, buf) < 0
			&& ferror(ctl_fd)) {
				fprintf(stderr,"%s: can't write to %s\n",
					progname, ctl_fname);
				return -1;
			}
		}
		cp = cp->next;
	}

	if(fprintf (ctl_fd, "WELCOME.DAT\r\nNEWS.DAT\r\nLOGOFF.DAT\r\n\032") < 0
	&& ferror(ctl_fd)) {
		fprintf(stderr,"%s: can't write to %s\n",
			progname, ctl_fname);
		return -1;
	}
	fclose (ctl_fd);
	return 0;
}

int WriteAreas()
/*
 *  Write the SLNP AREAS file
 */
{
	struct conf_ent *cp;
	int	ret = 0;

	/* Loop through conference list */
	cp = conf_list;
	while (cp != NULL) {
		/*  Don't add empty conferences  */
		if(cp->count) {
		        if (strcmp (cp->name, MAIL_CONF_NAME)) {
			        if (cp->count > 0)
				        ret = fprintf (ctl_fd, "%07d\011%s\011un\n",
					        cp->number, cp->name);
		        } else {
			        ret = fprintf (ctl_fd, "%07d\011%s\011bn\n",
				        cp->number, cp->name);
		        }
		}
		cp = cp->next;
	}
	fclose (ctl_fd);
	return ret==EOF?ret:0;
}

int WriteNewsrc()
/*
 *  Rewrite the updated .newsrc file
 */
{
	struct nrc_ent *np;
	int err = 0;

	if (read_only) return (0);

	if (NULL == (nrc_fd = fopen (nrc_file, "w"))) {
		fprintf (stderr, "%s: can't write %s\n",
			progname, nrc_file);
		exit(-1);	/* Return failure error code to caller */
	}

	for (np=nrc_list; np!=NULL; np=np->next) {
		/* Write this one */
		if (waf_mode) {
			if(np->subscribed) {
				if(fprintf(nrc_fd, "%s %d\r\n", np->name, np->sub->hi) < 0
				&& ferror(nrc_fd))
					err++;
			}
		} else {	/* Not waffle mode */
			if(np->subscribed) {
				if(fprintf(nrc_fd, "%s: ", np->name) < 0
				&& ferror(nrc_fd))
					err++;
			} else {
				if(fprintf(nrc_fd, "%s! ", np->name) < 0
				&& ferror(nrc_fd))
					err++;
			}
			if(!err)
				WriteSub (nrc_fd, np->sub);
		}
		if(err) {
			fprintf(stderr, "%s: can't write to %s\n",
				progname, nrc_file);
			exit(-1);
		}
	}
	fclose (nrc_fd);
	return 1;
}

void TransName (n)
char *n;
/*
 *  Translate newsgroup name
 */
{
	struct trn_ent *tp;

	tp = trn_list;

	while (tp != NULL) {
		if (!strcmp (n, tp->old)) {
			/* Found a match */
			strcpy (n, tp->new);
			return;
		}
		tp = tp->next;
	}
}

