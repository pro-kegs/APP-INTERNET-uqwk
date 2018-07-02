#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uqwk.h"
#ifdef	GNKSA
	#include <gnksa.h>
#endif

/*
 *  Process a reply packet
 */


int DoReply ()
{
	int n, rep_cnt, ret;
	char bbs[PATH_LEN];

	/* Check for ZipNews reply */
	if (zip_mode)
		return DoZipReplies();

	rep_cnt = 0;

	/* Open the packet */
	if (NULL == (rep_fd = fopen (rep_file, "r"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, rep_file);
		return(-1);
	}

	/* Handle SLNP reply */
	if (slnp_mode) {
		if((ret=SLNPReply()))
			return ret;

		fclose (rep_fd);
		return 0;
	}

	/* Get the first block, the BBS ID */
	if (1 != fread (buf, 128, 1, rep_fd)) {
		fprintf (stderr, "%s: reply packet read error\n", progname);
		fclose (rep_fd);
		return -1;
	}

	/* Extract bbs id and check */
	sscanf (bbs_id, "%*d,%s", bbs);
	n = strlen (bbs);
	buf[n] = 0;
	if (strcmp (bbs, buf)) {
		fprintf (stderr, "%s: reply BBS ID mismatch: %s != %s\n",
			progname, buf, bbs);
		fclose (rep_fd);
		return 0;
	}

	/* Read the .newsrc file; we will need the list of conferences */
	if((ret=ReadNewsrc()<=0))
		return(ret);

	/* Read the next message header and process it */
	while (1 == fread (&rep_hdr, 128, 1, rep_fd)) {
		if((ret=SendReply()) <= 0) return(ret);
		rep_cnt++;
	}

	fclose (rep_fd);
	printf ("%s: sent %d replies\n", progname, rep_cnt);

	return 1;
}

/*
 *  Pipe a reply to the mailer or inews
 */
int SendReply ()
{
	FILE    *pfd;
	unsigned char    c;
        char    to[PATH_LEN], subject[PATH_LEN], group[PATH_LEN];
	int     i, n, blocks, bytes, conf;
	struct  nrc_ent *np;

	/* Extract recipient */
	strncpy (buf, rep_hdr.to, 25);
	buf[25] = 0;
	sscanf (buf, "%s", to);

	/* Extract conference number */
	strncpy (buf, rep_hdr.number, 7);
	buf[7] = 0;
	sscanf (buf, "%d", &conf);

	/* Extract subject */
	strncpy (buf, rep_hdr.subject, 25);
	buf[25] = 0;
	strcpy (subject, buf);

	/* Get rid of single quotes in subject */
	n = strlen (subject);
	for (i=0; i<n; i++) if (subject[i] == '\'') subject[i] = '`';

	/* Find newsgroup with this conference number */
	np = nrc_list;
	while (np != NULL) {
		if (np->conf == conf) break;
		np = np->next;
	}

	/* Get newsgroup name */
	if (np == NULL) {
		/* Bet this generates lots of email for "ALL" */
		rep_hdr.status = QWK_PRIVATE;
	} else {
		strcpy (group, np->name);
	}

	/* Extract block count */
	strncpy (buf, rep_hdr.blocks, 6);
	buf[6] = 0;
	sscanf (buf, "%d", &blocks);
	blocks -= 1;
	bytes = 128 * blocks;

	/* Check for off-line command message */
	if ( (!strcmp (to, "uqwk")) || (!strcmp (to, "UQWK")) )
		return(QWKOffLine (bytes, rep_fd));

	/* Check for a configuration message intended for some
	   other QWK "door" */
	if (	(!strcasecmp (to, "MARKMAIL"))	||
		(!strcasecmp (to, "QMAIL"))	||
		(!strcasecmp (to, "ROSEMAIL"))	||
		(!strcasecmp (to, "KMAIL"))	||
		(!strcasecmp (to, "MAINMAIL"))	||
		(!strcasecmp (to, "CMPMAIL"))	||
		(!strcasecmp (to, "ULTRABBS"))	||
		(!strcasecmp (to, "BGQWK"))	||
		(!strcasecmp (to, "CAM-MAIL"))	||
		(!strcasecmp (to, "TRIMAIL"))	||
		(!strcasecmp (to, "QSO"))	) {
		/* Send warning to user */
		if((n=SendWarning(to)) <= 0) return(n);

		/* Skip the rest of the message */
		while (bytes--) fread (&c, 1, 1, rep_fd);

		return (0);
	}

	/* Check for a "To: " line in the body of the message */
	CheckTo (to, bytes);

	/* Open pipe to proper program */
	pfd = NULL;

/*
 *	QWK is beyond hope -- don't even bother basic GNKSA-checking :-(
 */

	if ( (rep_hdr.status == QWK_PUBLIC) ||
	     (rep_hdr.status == QWK_PUBLIC2) ) {
		/* Public message, open pipe to inews */
		if (xprt_mode) {
			sprintf (buf, "%s", posting_agent);
		} else {
#ifdef NNTP
			sprintf (buf, "%s", inews_path);
#else
			sprintf (buf, "%s -t '%s' -n '%s'",
				inews_path, subject, group);
#endif
		}
		printf ("%s: Posting to %s\n", progname, group);
		if (NULL == (pfd = popen (buf, "w"))) {
			fprintf (stderr, "%s: can't popen() %s\n",
					progname, inews_path);
			return(-1);
		}
#ifdef NNTP
		if (!xprt_mode && (pfd != NULL) ) {
			fprintf (pfd, "Newsgroups: %s\nSubject: %s\n\n",
				group, subject);
		}
#endif
	} else if ( (rep_hdr.status == QWK_PRIVATE) ||
	          (rep_hdr.status == QWK_PRIVATE2) ) {
		/* Open pipe to mail */
		if (xprt_mode) {
			sprintf (buf, "%s", XPRT_MAILER);
		} else {
			sprintf (buf, "%s '%s'",
				SENDMAIL, to);
		}
		printf ("%s: Mailing to %s\n", progname, to);
		if (NULL == (pfd = popen (buf, "w"))) {
			fprintf (stderr, "%s: can't popen() %s\n", progname, buf);
			return(-1);
		}
		fprintf(pfd,"Subject: %s\n", subject);
	}

	/* Read and send all bytes of message */
	for (i=0; i<bytes; i++) {
		fread (&c, 1, 1, rep_fd);
		if (c == QWK_EOL) c = 012;
		if( (pfd != NULL) && (fwrite(&c, 1, 1, pfd) != 1)) return(-1);
	}

	return(pclose (pfd));
}

int SendWarning (to)
char *to;
/*
 *  Mail a warning to the user if the reply packet
 *  contains a message apparently for some other QWK
 *  "door" program.
 */
{
	FILE *pfd;

	/* Open pipe to mailer */
/*
	sprintf (buf, "%s -s 'UQWK Error Message' %s",
			MAILER_PATH, user_name);
*/
	sprintf (buf, "%s %s",
			SENDMAIL, user_name);
	if (NULL == (pfd = popen (buf, "w"))) {
		fprintf (stderr, "%s: can't popen() mail\n", progname);
		return(-1);
	}

	/* Send the message */

	fprintf (pfd,
		"Subject: UQWK Error Message\n\n"

		"Hello. You sent a message to the username %s, presumably to\n"
		"perform some sort of offline configuration. This QWK processor,\n"
		"called UQWK, cannot process this message. To perform offline\n"
		"configuration using UQWK, you must send a message to the username\n"
		"UQWK. Commands are to be included in the body of the message.\n"
		"For a list of commands, send a message to UQWK with the word\n"
		"HELP in the body of the message (not the subject). Thanks!\n",
		to);

	return(pclose (pfd));
}

int CheckTo (to, bytes)
char *to;
int bytes;
/*
 *  Check the first line of the body of the message for a To: line.
 *  This is the only way to send to addresses over 25 characters.
 *
 *  Whether we find a To: line or not, we have to leave the file
 *  positioned right where it is.
 */
{
	long offset;
	unsigned char c;
	int i;

	/* Sanity check */
	if (bytes < 5) return (0);

	offset = ftell (rep_fd);

	/* Check first four bytes */
	fread (buf, 4, 1, rep_fd);
	bytes -= 4;
	if (strncmp (buf, "To: ", 4)) {
		/* Doesn't match */
		fseek (rep_fd, offset, 0);
		return (0);
	}

	/* Copy in the rest of the line until white space, EOL,
	   or run out of bytes */
	i = 0;
	fread (&c, 1, 1, rep_fd);
	bytes--;

	while ( (bytes >= 0) && (c != QWK_EOL) &&
	        (c != 9) && (c != ' ') ) {
		to[i++] = c;

		fread (&c, 1, 1, rep_fd);
		bytes--;
	}
	to[i] = 0;

	/* Done! */
	fseek (rep_fd, offset, 0);
	return (1);
}

/*
 *  Process an SLNP reply packet
 */
int SLNPReply ()
{
	char fname[PATH_LEN], kind[PATH_LEN], type[PATH_LEN];
	int rval=0;

	/* Look through lines in REPLIES file */
	while (Fgets (buf, BUF_LEN, rep_fd)) {
		if (3 != sscanf (buf, "%s %s %s", fname, kind, type)) {
			fprintf (stderr, "%s: malformed REPLIES line\n",
					progname);
			return (-1);
		}

		/* Look for mail or news */
		if (strcmp(kind,"mail") && strcmp(kind,"news")) {
			fprintf (stderr, "%s: bad reply kind: %s\n",
					progname, kind);
		}

		/* Check reply type */
		else if ( (type[0] != 'u') &&
			  (type[0] != 'b') &&
			  (type[0] != 'B') ) {
			fprintf (stderr, "%s: reply type %c not supported\n",
					progname, type[0]);
		} else {
			/* Make file name */
			strcat (fname, ".MSG");

			/* Process it */
			switch (type[0]) {
			case 'u':
				if( !strcmp(kind, "mail")) rval=SLuMail(fname);
				if( !strcmp(kind, "news")) rval|=SLuNews(fname);
				break;

			case 'b':
			case 'B':
				if( !strcmp(kind, "mail")) rval=SLbMail(fname);
				if( !strcmp(kind, "news")) rval|=SLbNews(fname);
				break;
			}

			/* Delete it */
			if (!read_only) unlink (fname);
		}
	}
	return(rval);
}

/*
 *  Process a SLNP mail reply file, usenet type
 */
int SLuMail(char *fn)
{
	FILE *fd;
	int bytes, ret = 0;
	char *to, cmd[PATH_LEN];
	long offset;

	/* Get space for To: */
	if (NULL == (to = (char *) malloc (BUF_LEN))) OutOfMemory();

	/* Open the reply file */
	if (NULL == (fd = fopen (fn, "r"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		free (to);
		return(-1);
	}

	/* Read through it */
	while (NULL != Fgets (buf, BUF_LEN, fd)) {
		if (strncmp (buf, "#! rnews ", 9)) {
			fprintf (stderr, "%s: malformed reply file\n",
					progname);
			fclose (fd);
			free (to);
			return (-1);
		}

		/* Get byte count */
		sscanf (&buf[8], "%d", &bytes);

		/* Remember file position */
		offset = ftell (fd);

		/* Look for To: */
		GetHdr (fd, to, bytes, "To: ");

		/* Check for offline command */
		if (!strcasecmp (to, "UQWK")) {
			if((ret = OffLine (fd, bytes)) == -1)
				break;
			continue;
		}

		/* Construct delivery line */
		sprintf (cmd, "%s", SLNP_MAILER);

		printf ("%s: Mailing to %s\n", progname, to);

		/* Pipe message to delivery agent */
		ret = SLPipe(fd, bytes, cmd);
	}
	free (to);

	fclose (fd);
	return (ret);
}

int SLuNews (fn)
char *fn;
/*
 *  Process a SLNP news reply file, usenet type
 */
{
	FILE *fd;
	int bytes, rval = 0;
	char *grp;

	/* Get space for newsgroup name */
	if (NULL == (grp = (char *) malloc (BUF_LEN))) {
		OutOfMemory();
		return (-1);
	}

#ifdef	GNKSA
	if(gnksa_level &&
	   GnksaErrors(GnksaCheckFile(fn, fn, gnksa_level&GNKSA_VERBOSE, FROMOPTIONAL))) {
		AppendBadArticle(fn);
		if(gnksa_level & GNKSA_HONOUR) {
			fprintf (stderr, "%s: cannot post invalid message\n", progname);
			return(-1);
		} else
			fprintf (stderr, "%s: warning: attempting to post invalid message\n", progname);
	}
#endif

	/* Open the reply file */
	if (NULL == (fd = fopen (fn, "r"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		return(-1);
	}

	/* Read through it */
	while (NULL != Fgets (buf, BUF_LEN, fd)) {
		if (strncmp (buf, "#! rnews ", 9)) {
			fprintf (stderr, "%s: mailformed reply file\n",
					progname);
			fclose (fd);
			return(-1);
		}

		/* Get byte count */
		sscanf (&buf[8], "%d", &bytes);

		if (GetHdr (fd, grp, bytes, "Newsgroups: ")) {
			printf ("%s: Posting article to %s\n", progname, grp);
		}

		/* Pipe message to delivery agent */
		rval = SLPipe(fd, bytes, posting_agent);
	}
	free (grp);
	fclose (fd);
	return (rval);
}

/*
 *  Process a SLNP mail reply file, binary type
 */
int SLbMail(char *fn)
{
	FILE *fd;
	int bytes, ret = 0;
	char *to, cmd[PATH_LEN];
	long offset;

	/* Get space for To: */
	if (NULL == (to = (char *) malloc (BUF_LEN))) return (-1);

	/* Open the reply file */
	if (NULL == (fd = fopen (fn, "r"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		free (to);
		return(-1);
	}

	/* Read through it */
	while (0 != fread (buf, 4, 1, fd)) {

		/* Get byte count */
		bytes = buftoint((unsigned char *) buf);

		/* Remember file position */
		offset = ftell (fd);

		/* Find the To: line */
		GetHdr (fd, to, bytes, "To: ");

		/* Check for offline command */
		if (!strcmp (to, "uqwk") || !strcmp (to, "UQWK")) {
			if((ret=OffLine(fd, bytes)) == -1)
				break;
			continue;
		}

		/* Construct delivery line */
		sprintf (cmd, "%s", SLNP_MAILER);

		printf ("%s: Mailing to %s\n", progname, to);

		/* Pipe message to delivery agent */
		ret = SLPipe(fd, bytes, cmd);
	}
	free (to);

	fclose (fd);
	return (ret);
}

/*
 *  Process a SLNP news reply file, binary type
 */
int SLbNews (char *fn)
{
	FILE *fd;
	int  bytes, rval=0;
	char *grp;

	/* Get space for newsgroup name */
	if (NULL == (grp = (char *) malloc (BUF_LEN))) {
		OutOfMemory();
		return (-1);
	}

#ifdef	GNKSA
	if(gnksa_level &&
	   GnksaErrors(GnksaCheckFile(fn, fn, gnksa_level&GNKSA_VERBOSE, FROMOPTIONAL))) {
		AppendBadArticle(fn);
		if(gnksa_level & GNKSA_HONOUR) {
			fprintf (stderr, "%s: cannot post invalid message\n", progname);
			return(-1);
		} else
			fprintf (stderr, "%s: warning: attempting to post invalid message\n", progname);
	}
#endif

	/* Open the reply file */
	if (NULL == (fd = fopen (fn, "r"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		return(-1);
	}

	/* Read through it */
	while (0 != fread (buf, 4, 1, fd)) {

		/* Get byte count */
		bytes = buftoint((unsigned char *) buf);

		if (GetHdr (fd, grp, bytes, "Newsgroups: ")) {
			printf ("%s: Posting article to %s\n", progname, grp);
		}

		/* Pipe message to delivery agent */
		rval = SLPipe(fd, bytes, posting_agent);
	}
	free (grp);
	fclose (fd);
	return (rval);
}

int SLPipe (fd, bytes, agent)
FILE *fd;
int bytes;
char *agent;
/*
 *  Pipe a SLNP reply to the specified delivery agent
 */
{
	FILE *pfd;
	int c;

	/* Open pipe to agent */
	if (NULL == (pfd = popen (agent, "w"))) {
		fprintf (stderr, "%s: can't open reply pipe (%s)\n", progname, agent);
		while (bytes--) getc (fd);
		return(-1);
	}

	/* Send message to pipe */
	while (bytes--) {
		if( (c = getc(fd) & 0xff) == EOF) {
			fprintf(stderr,"%s: unable to read %d bytes\n",
			        progname, bytes);
			return -1;
		}
		if( putc(c, pfd) == EOF) {
			fprintf(stderr, "%s: write error\n", progname);
			return -1;
		}
	}

	return(pclose(pfd));
}

int GetHdr (fd, cc, bytes, hdr)
FILE *fd;
char *cc, *hdr;
int bytes;
/*
 *  Find given header line
 */
{
	int offset, n, cnt;
	char *rc;

	cnt = strlen (hdr);

	/* Remember file position */
	offset = ftell (fd);

	/* Look through header */
	rc = Fgets (buf, BUF_LEN, fd);
	n = strlen (buf);
	while ( (rc != NULL) && (bytes > 0) && (n > 0) ) {
		/* Right line? */
		if (!strncmp (buf, hdr, cnt)) {
			strcpy (cc, &buf[cnt]);
			fseek (fd, offset, 0);
			return (1);
		}

		/* Get next line */
		bytes -= n;
		rc = Fgets (buf, BUF_LEN, fd);
		if (rc != NULL) n = strlen (buf);
	}

	/* Reposition file */
	fseek (fd, offset, 0);
	return (0);
}


int DoZipReplies ()
/*
 *  Process replies in a Zip packet
 */
{
	int n;
	char fn[PATH_LEN];

	if ((n=ZipId()) <=0) return(n);

	/* Loop through possible mail files.  This is a little kludgy,
	   but readdir() seems to have problems on some systems,
	   including Esix, which is the system I use. */
	for (n=0; n<100; n++) {
		/* Construct file name */
		sprintf (fn, "%s/%s.m%02d", rep_file, user_name, n);

		/* Process it */
		if((n=ZipMail(fn)) <= 0) return(n);
	}

	/* Loop through possible news files */
	for (n=0; n<100; n++) {
		/* Construct file name */
		sprintf (fn, "%s/%s.n%02d", rep_file, user_name, n);

		/* Process it */
		if((n=ZipNews(fn)) <= 0) return(n);
	}
	return (1);
}

char ZZZ[PATH_LEN];

int ZipId ()
{
	char fn[PATH_LEN],zzz[PATH_LEN],zzZ[256],*zZz,ZzZ[PATH_LEN],
	ZZz[PATH_LEN];FILE *zz;int zZ,zZZ=0,Zzz,z=0;

	sprintf (fn, "%s/%s.id", rep_file, user_name);
	if (NULL == (zz = fopen (fn, "r"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		return(-1);
	}

	strcpy(zzz,"Vrth#glfp#YjsMftp-#Kllqbz-");Zzz=strlen(zzz);
	for(zZz=(&zzz[0]);*zZz;zZz++)*zZz^=3;zZz=(&zzz[0]);zZ=getc(zz);
	while(zZ!=EOF){zzZ[z]=zZ^*(zZz+zZZ)^(*zZz*zZZ);*(zZz+zZZ)+=
	(zZZ<(Zzz+1))?*(zZz+zZZ+1):*zZz;if(!*(zZz+zZZ))(*(zZz+zZZ))++;
	if(++zZZ>=Zzz)zZZ=0;z++;zZ=getc(zz);}zzZ[z]=0;for(z=0;z<9;z++)
	zzZ[z]^=3;fclose(zz);if(strncmp(zzZ,"YMQ(VRTH#",9))return(0);
	zZz=(&zzZ[0]);z=0;zZz=strtok(zZz+9,"\n");zZz=strtok(NULL,"\n");
	strcpy(ZZz,zZz);zZz=strtok(NULL,"\n");strcpy(ZzZ,zZz);strcat
	(ZzZ,"\100");strcat(ZzZ,host_name);for(zZz=(&ZzZ[0]);*zZz;zZz++)
	*zZz^=3;for(zZz=(&ZZz[0]);*zZz;zZz++)if((*zZz>='A')&&(*zZz<='Z'))
	*zZz=tolower(*zZz);z=0;while(ZZz[z]){if(z==0)ZZz[z]=toupper(ZZz[z]);
	if((ZZz[z]==' ')||(ZZz[z]=='-')||(ZZz[z]==0x27))ZZz[z+1]=toupper
	(ZZz[z+1]);z++;}for(zZz=(&ZZz[0]);*zZz;zZz++)*zZz^=3;sprintf(ZZZ,
	"Eqln9#%s#+%s*",ZzZ,ZZz);for(zZz=(&ZZZ[0]);*zZz;zZz++)*zZz^=3;

	return (1);
}

int ZipMail (fn)
char *fn;
/*
 *  Process ZipNews mail reply
 */
{
	FILE *fd, *pfd;
	struct stat stat_buf;
	int c, have_cc, rval = 0;
	char *to, *cc, *addr;

	/* Get space for To: and Cc: */
	if (NULL == (to = (char *) malloc (BUF_LEN))) return (0);
	if (NULL == (cc = (char *) malloc (BUF_LEN))) return (0);

	/* Try to stat() it */
	if (0 != stat (fn, &stat_buf)) return (0);

	/* Try to open it */
	if (NULL == (fd = fopen (fn, "r"))) return(-1);

	/* Get To: and Cc: */
	GetHdr (fd, to, stat_buf.st_size, "To: ");
	have_cc = GetHdr (fd, cc, stat_buf.st_size, "Cc: ");

	/* Check for offline command */
	if (!strcasecmp (to, "UQWK")) {
		int ret = OffLine(fd, stat_buf.st_size);
		free (cc); free (to);
		fclose (fd);
		if (!read_only) unlink (fn);
		return (ret);
	}

	/* Make mailer command line */
	sprintf (buf, "%s '%s'", ZIP_MAILER, to);

	printf ("%s: Mailing to %s\n", progname, to);

	/* Open pipe to mailer */
	if (NULL == (pfd = popen (buf, "w"))) {
		fprintf (stderr, "%s: can't popen() mailer\n", progname);
		free (cc); free (to);
		fclose (fd);
		return (-1);
	}

	if( (fprintf (pfd, "%s\n", ZZZ) < 0) && ferror(pfd)) return(-1);

	/* Send bytes of message */
	while (EOF != (c = getc (fd))) {
		if (c != '\r') putc ((0xff & c), pfd);
	}

	/* Done */
	rval = pclose (pfd);

	/* Now do Cc: addresses */
	if (have_cc) {
		addr = strtok (cc, ", \t");

		while (addr != NULL) {
			/* Rewind file */
			fseek (fd, 0, 0);

			/* Make mailer command line */
			sprintf (buf, "%s '%s'", ZIP_MAILER, addr);

			printf ("%s:  Cc'ing to %s\n", progname, addr);

			/* Open pipe to mailer */
			if (NULL == (pfd = popen (buf, "w"))) {
				fprintf (stderr, "%s: can't popen() mailer\n",
						progname);
				fclose (fd);
				return(-1);
			}

			if( (fprintf (pfd, "%s\n", ZZZ) < 0) && ferror(pfd)) return(-1);

			/* Send bytes of message */
			while (EOF != (c = getc (fd))) {
				if (c != '\r') putc ((0xff & c), pfd);
			}

			/* Done */
			rval |= pclose (pfd);

			addr = strtok (NULL, ", \t");
		}
	}
	free (cc); free (to);

	fclose (fd);
	if (!read_only) unlink (fn);

	return (rval);
}

int ZipNews (fn)
char *fn;
/*
 *  Process ZipNews news reply
 */
{
	FILE *fd, *pfd;
	struct stat stat_buf;
	int c, rval = 0;

	/* Try to stat() it */
	if (0 != stat (fn, &stat_buf)) return (0);

#ifdef	GNKSA
	if(gnksa_level &&
	   GnksaErrors(GnksaCheckFile(fn, fn, gnksa_level&GNKSA_VERBOSE, FROMOPTIONAL))) {
		AppendBadArticle(fn);
		if(gnksa_level & GNKSA_HONOUR) {
			fprintf (stderr, "%s: cannot post invalid message\n", progname);
			return(-1);
		} else
			fprintf (stderr, "%s: warning: attempting to post invalid message\n", progname);
	}
#endif

	/* Try to open it */
	if (NULL == (fd = fopen (fn, "r"))) return(-1);


	printf ("%s: Posting article...\n", progname);

	/* Open pipe to inews */
	if (NULL == (pfd = popen (posting_agent, "w"))) {
		fprintf (stderr, "%s: can't popen() inews\n", progname);
		fclose (fd);
		return(-1);
	}

	if( (fprintf (pfd, "%s\n", ZZZ) < 0) && ferror(pfd)) return(-1);

	/* Send bytes of message */
	while (EOF != (c = getc (fd))) {
		if (c != '\r') putc ((0xff & c), pfd);
	}

	/* Done */
	rval = pclose (pfd);
	fclose (fd);
	if (!read_only) unlink (fn);

	return (rval);
}

#ifdef	GNKSA

void AppendBadArticle(char *fn)
{
	char   *buf, out_file[PATH_LEN];
	FILE   *fd;
	size_t size;

	sprintf(out_file, "%s/dead.articles", home_dir);

	if((fd = fopen(out_file, "a")) == NULL)
		fprintf(stderr, "could not open %s for appending\n", out_file);

	if((buf = SlurpFile(fn, &size)) == NULL) {
		fprintf(stderr, "could not get bad article %s\n", fn);
		fclose(fd);
		return;
	}

	if(fwrite(buf, 1, size, fd) != size)
		fprintf(stderr, "failed to write %i bytes to %s\n", size, fn);

	fclose(fd);
	free(buf);

	fprintf(stderr, "%s: bad article appended to %s\n", progname, out_file);
}
#endif
