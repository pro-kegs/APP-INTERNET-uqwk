/*
 *  $Id: mail.c,v 1.10 2001/10/24 11:37:10 js Exp $
 */

#include <stdio.h>
#include "uqwk.h"
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
/*
 *  All sorts of stuff to do mail processing
 */

FILE   *mail_fd;			/* For mail spool */
struct mail_ent *last_mp;	/* Points to last mail list entry */


/*
 *  A few useful routines
 */

int fileexists(const char *fname)
{
  struct stat fbuf;

  return (stat(fname,&fbuf)!=EOF && !(fbuf.st_mode&S_IFDIR));
}

time_t filetime(const char *fname)
{
  struct stat fbuf;

  return stat(fname,&fbuf)!=EOF ? fbuf.st_ctime : -1L;
}

long filesize(const char *s)
{
  struct stat st;

  return (stat((char*)s,&st)!=EOF) ? st.st_size : -1L;
}

long openfilesize(FILE *fp)
{
  struct stat st;

  return (fstat(fileno(fp),&st)!=EOF) ? st.st_size : -1L;
}

int  isdir(const char *s)
{
  struct stat     st;

  return (stat((char*)s,&st)!=EOF && !(st.st_mode & S_IFDIR));
}

#define strend(x) ((x) + strlen(x))

/*
 *  Some stuff for maildir-handling
 */


int ismaildir(char *dirname)
{
  char subdirname[255];
  char *subdir_names[] = {"/cur","/new","/tmp",NULL}, **p;
  char *endp;

  if(!isdir(dirname))
    return 0;

  strcpy(subdirname, dirname);
  endp = subdirname+strlen(subdirname);
  for (p=subdir_names; *p; p++) {
    strcpy(endp, *p);
    if(isdir(subdirname) <= 0)
      return 0;
  }
  return 1;
}


/*
 *  Process mail into QWK packet
 */

int DoMail()
{
	if(!fileexists(mail_file))
		return 1;

/*	if(ismaildir(mail_file)) */
	if(isdir(mail_file))
		return DoMaildirMail();
	else
		return DoClassicMail();
}


/*
 *  Maildir handling

     HOW A MESSAGE IS READ
          A mail reader operates as follows.

          It looks through the new directory for new messages.  Say
          there is a new message, new/unique.  The reader may freely
          display the contents of new/unique, delete new/unique, or
          rename new/unique as cur/unique:info.  See
          http://pobox.com/~djb/proto/maildir.html for the meaning of
          info.

          The reader is also expected to look through the tmp
          directory and to clean up any old files found there.  A file
          in tmp may be safely removed if it has not been accessed in
          36 hours.
          It is a good idea for readers to skip all filenames in new
          and cur starting with a dot.  Other than this, readers
          should not attempt to parse filenames.

 */

typedef struct flist {
  char         *fnam;
  struct flist *tail;
} filelist;

filelist *AddMaildirfile(l, s)
filelist *l;
char     *s;
{
	filelist *new;

	/* Get space for new entry */
	if (NULL== (new = (filelist *) malloc(sizeof(filelist)))) {
		fprintf (stderr, "%s: out of memory\n", progname);
		exit(-1);
	}

	new->fnam = (char *) strdup(s);
	new->tail = l;

	return new;
}

static void free_filelist(register filelist *f)
{
  register filelist *ptr;

  while(f) {
    ptr=f->tail;
    if (f->fnam)
      free(f->fnam);
    free(f);
    f=ptr;
  }
}

int DoMaildirMail()
{
	struct conf_ent *cp;
	struct mail_ent me;
	DIR             *dirp;
	struct dirent   *d;
	char            *endp;
	char            tmp[PATH_LEN];
	filelist	    *mfs = NULL, *l;

	/* get name of directory */
	strcpy(tmp, mail_file);
	endp = strend(tmp);
	if (endp > tmp && endp[-1] == '/')
		*--endp = '\0';

	/* Do new directory */
	strcpy(endp, "/new/");
	if ((dirp = opendir(tmp))) { 			/* open directory */
		while ((d = readdir (dirp))) {		/* empty the directory */
			if (d->d_name[0] != '.') {	/* skip ., .., and .files */
				strcpy(endp+5, d->d_name);
				mfs = AddMaildirfile(mfs, tmp);
			}
		}
		closedir (dirp);	/* flush directory */
	}

	/* Do cur directory */
	strcpy(endp, "/cur/");
	if ((dirp = opendir(tmp))) { 			/* open directory */
		while ((d = readdir (dirp))) {		/* empty the directory */
			if (d->d_name[0] != '.') {	/* skip ., .., and .files */
				strcpy(endp+5, d->d_name);
				mfs = AddMaildirfile(mfs, tmp);
			}
		}
		closedir (dirp);	/* flush directory */
	}

	if (!mfs) {
		fprintf (stdout, "no mail for %s\n", user_name);
		return(0);
	}

	/* Define the mail "conference" */
	cp = NewConference (MAIL_CONF_NAME, conf_cnt);


	/* Open ZipNews mail file */
	if (zip_mode && (mail_list != NULL)) OpenZipMail();

	printf ("maildir mailbox (%s)", mail_file);
	/* Walk through all the messages */


	/* handle list of messages */
	me.next = NULL;
	for(l = mfs; l; l = l->tail) {
		long ln;
		char *uidl, *s;
		char xheaders[128] = {0};
		char status[16]    = {0};
		char xstatus[16]   = {0};

		if (!l->fnam)
			continue;

		ln = filesize(l->fnam);
		uidl = (char *) strrchr(l->fnam, '/');
		if(uidl && !strncmp(uidl-3, "cur", 3))
			strcat(status, "O");
		uidl++;
		s = (char *) strchr(uidl, ':');
		if(s != NULL) {
			*s++ = 0;
			/* Add (X-)Status codes */
			if(strchr(s, 'S'))
				strcat(status,  "R");
			if(strchr(s, 'R')) {
				strcat(status,  "r");
				strcat(xstatus, "A");
			}
			if(strchr(s, 'F'))
				strcat(xstatus, "F");
			if(strchr(s, 'T'))
				strcat(xstatus, "D");
		}
		sprintf(xheaders, "X-UIDL: %s\n", uidl?uidl:"unknown");
		if(s != NULL)
			*(s-1) = ':';
		if(*status) {
			strcat(xheaders, "Status: ");
			strcat(xheaders, status);
			strcat(xheaders, "\n");
		}
		if(*xstatus) {
			strcat(xheaders, "X-Status: ");
			strcat(xheaders, xstatus);
			strcat(xheaders, "\n");
		}
		if(ln > 0) {
			FILE *fd;

			/* Open the message file */
			if (NULL == (fd = fopen(l->fnam, "r"))) {
				fprintf (stderr, "could not open %s\n", l->fnam);
				exit(1);
			}
			cp->count++;
			me.begin = 0;
			me.end   = ln;
			AddMessage(fd, &me, xheaders);
			if(fclose(fd)==EOF) {
				fprintf(stderr, "error closing file %s\n", l->fnam);
				exit(1);
			}
		}
	}

	printf(" %d message%s\n", cp->count, cp->count==1?"":"s");
	if (!slnp_mode && !zip_mode) fclose (ndx_fd);
	if (slnp_mode) {
		fclose (msg_fd);
		msg_fd = NULL;
	}
	if (zip_mode && (mail_list != NULL)) fclose (mai_fd);

	/* in read only mode we're done now */
	if (read_only) {
		free_filelist(mfs);
		return 1;
	}

	/* Now empty the mail box */
	for(l = mfs; l; l = l->tail) {
		if (l->fnam)
			unlink(l->fnam);
	}

	/* purge old junk from tmp */
	strcpy(endp, "/tmp/");
	if ((dirp = opendir(tmp))) { 			/* open directory */
		while ((d = readdir (dirp))) {
			if ((d->d_name[0] != '.')) {	/* skip ., .., and .files */
				strcpy(endp+5, d->d_name);
				if (filetime(tmp) < (time(NULL)-(36L*60*60))
				&&  access(tmp,W_OK)==0) {
					unlink(tmp);
				}
			}
		}
		closedir (dirp);	/* flush directory */
	}

	free_filelist(mfs);
	return 1;
}


int DoClassicMail ()
{
	struct mail_ent *mailp;
	struct conf_ent *cp;

	/* No mail in summary mode */
	if (sum_mode) return (0);

	/* Open the mail spool */
	if (NULL == (mail_fd = fopen(mail_file, "r"))) {
		fprintf (stdout, "no mail in %s for %s\n", mail_file, user_name);
		return(0);
	}

	/* Construct the mail linked list */
	MakeMailList ();

	if (!mail_list) {
		fprintf (stdout, "no mail in %s for %s\n", mail_file, user_name);
		fclose(mail_fd);
		return(0);
	}

	/* Define the mail "conference" */
	cp = NewConference (MAIL_CONF_NAME, conf_cnt);


	/* Open ZipNews mail file */
	if (zip_mode && (mail_list != NULL)) OpenZipMail();

	printf ("classic mailbox (%s)", mail_file);

	/* Walk through all the messages */
	mailp = mail_list;
	while (mailp != NULL) {
		cp->count++;

		AddMessage(mail_fd, mailp, NULL);

		mailp = mailp->next;
	}

	fclose (mail_fd);
	printf(" %d message%s\n", cp->count, cp->count==1?"":"s");
	if (!slnp_mode && !zip_mode) fclose (ndx_fd);
	if (slnp_mode) {
		fclose (msg_fd);
		msg_fd = NULL;
	}
	if (zip_mode && (mail_list != NULL)) fclose (mai_fd);

	/* Now empty the mail box */
	if (!read_only) {
		if (NULL == (mail_fd = fopen (mail_file, "w"))) {
			fprintf (stderr, "%s: can't write %s\n", progname,
								mail_file);
			return(-1);
		} else {
			fclose (mail_fd);
		}
	}
	return(1);
}


/*
 *  Construct linked list of pointers to individual messages in
 *  the mail spool.
 */

void MakeMailList()
{
	long offset;

	last_mp = NULL;

	/* Read through, looking for "From" lines */
	offset = ftell (mail_fd);
	while (NULL != Fgets (buf, BUF_LEN, mail_fd)) {
		if (!strncmp (buf, "From ",  5)) {
			DoFromLine (offset);
		}
		offset = ftell (mail_fd);
	}
	if (last_mp != NULL) last_mp->end = offset;
}

void DoFromLine (offset)
long offset;
{
	struct mail_ent *mp;

	/* Get space for new mail list entry */
	if (NULL==(mp=(struct mail_ent *) malloc(sizeof(struct mail_ent)))) {
		fprintf (stderr, "%s: out of memory\n", progname);
		exit(-1);
	}

	/* Fill in offset */
	mp->begin = offset;

	if (last_mp == NULL) {
		/* This is first message */
		mail_list = mp;
	} else {
		/* Add to end of list */
		last_mp->next = mp;
		last_mp->end = offset;
	}

	mp->next = NULL;
	last_mp = mp;
}


void AddMessage(fd, mp, xhdrs)
FILE            *fd;
struct mail_ent *mp;
char		*xhdrs;
{
        if (slnp_mode)
	        SLNPMessage(fd, mp, xhdrs);
        else if (zip_mode)
	        ZipMessage(fd, mp, xhdrs);
        else
	        QwkMessage(fd, mp, xhdrs);
}


/*
 *  Convert a message to SLNP format
 */

void SLNPMessage (fd, mp, xhdrs)
FILE            *fd;
struct mail_ent *mp;
char		*xhdrs;
{
	long bytes, c, b;

	/* Compute message size */
	b = bytes = mp->end - mp->begin;
	/* If there are xtra heades to be added, add those to the byte count */
	if(xhdrs)
		b += strlen(xhdrs);

	/* Write byte count line */
	for (c=3; c>=0; c--) {
		buf[c] = b % 256;
		b = b / 256;
	}
	if(fwrite(buf, 4, 1, msg_fd) != 1) {
		fprintf(stderr,"%s: write error!\n",progname);
		exit(-1);
	}

	/* Seek to start of message */
	fseek (fd, mp->begin, 0);

	if(xhdrs && *xhdrs) {
		char s[1024] = "";
		int  err=0;

		while(fgets(s, sizeof(s), fd) && ftell(fd) < mp->end) {
			bytes -= strlen(s);
			if(s[0]=='\n' || (s[0]=='\r' && s[1]=='\n')) {
				if(fputs(xhdrs,msg_fd)==EOF
				|| fputs(s,    msg_fd)==EOF)
					err++;
				break;
			}
			if(fputs(s, msg_fd)==EOF) {
				err++;
				break;
			}
		}
		if(err) {
			fprintf(stderr, "%s: write error!\n", progname);
			exit(-1);
		}
	}

	/* Copy rest of bytes */
	while (bytes--) {
		if((c = getc(fd)) == EOF) {
			fprintf(stderr,"%s: read error (%ld bytes unread)\n", progname, bytes+1);
			exit(-1);
		}
		if(putc((0xff & c), msg_fd) == EOF) {
			fprintf(stderr,"%s: write error!\n",progname);
			exit(-1);
		}
	}
}


/*
 *  Convert a message to QWK format
 *  We're ignoring any xhdrs -- QWK is too badly broken anyway
 */

void QwkMessage(fd, mp, xhdrs)
FILE *          fd;
struct mail_ent *mp;
char		*xhdrs;
{
	struct qwk_hdr hdr;
	char c[PATH_LEN], *eof, ndx[5];
	int out_bytes, n, i;

	/* Write the ndx file entry */
	inttoms (blk_cnt, (unsigned char *) ndx);
	ndx[4] = conf_cnt;
	if(fwrite (ndx, 5, 1, ndx_fd) != 1) {
		fprintf(stderr,"%s: write error!\n",progname);
		exit(-1);
	}

	Spaces ((char *)&hdr, 128);

	/* Fill in the header fields we can do now */
	hdr.status = QWK_PRIVATE;
	PadNum (msg_cnt, hdr.number, 7);
	Spaces (hdr.password, 12);
	Spaces (hdr.refer, 8);
	hdr.flag = (char) QWK_ACT_FLAG;
	IntNum (conf_cnt, hdr.conference);
	IntNum (msg_cnt+1, hdr.msg_num);
	hdr.tag = ' ';

	msg_cnt++;

	/* Seek to start of message */
	fseek (fd, mp->begin, 0);

	/* Read the From line */
	Fgets (buf, BUF_LEN, fd);

	/* The second field of the From line is used as a first
	   guess for who sent the message */
	sscanf (&buf[5], "%s", c);
	PadString (c, hdr.from, 25);

	/* Now read through header lines, looking for ones we need */
	eof = Fgets (buf, BUF_LEN, fd);
	while ( (0 != strlen(buf)) && (eof != NULL) ) {
		if (!strncmp (buf, "Date: ", 6)) {
			ParseDate (&buf[6], &hdr);
		} else if (!strncmp (buf, "To: ", 4)) {
			PadString (&buf[4], hdr.to, 25);
		} else if (!strncmp (buf, "Subject: ", 9)) {
			PadString (&buf[9], hdr.subject, 25);
		} else if (!strncmp (buf, "From: ", 6)) {
			PadString (ParseFrom(&buf[6]), hdr.from, 25);
		}
		eof = Fgets (buf, BUF_LEN, fd);
	}
	mp->text = ftell(fd);

	/* Fill in block count */
	if (inc_hdrs) {
		PadNum (2+(mp->end-mp->begin)/128, hdr.blocks, 6);
		blk_cnt += (1+(mp->end - mp->begin)/128);
	} else {
		PadNum (2+(mp->end-mp->text)/128, hdr.blocks, 6);
		blk_cnt += (1+(mp->end - mp->text)/128);
	}

	/* Write out the message header */
	if( fwrite(&hdr, 128, 1, msg_fd) != 1) {
		fprintf(stderr,"%s: write error!\n",progname);
		exit(-1);
	}
	blk_cnt++;

	/* Now write the message text */
	if (inc_hdrs) fseek(fd, mp->begin, 0);
	out_bytes = 0;

	eof = Fgets (buf, BUF_LEN, fd);
	do {
		n = strlen (buf);

		/* MMDF puts funny things in messages -- change to spaces */
		for (i=0; i<n; i++) {
			if (buf[i] == 1) buf[i] = ' ';
			if (buf[i] == 0) buf[i] = ' ';
		}

		if( fwrite(buf, 1, n, msg_fd) != n) {
			fprintf(stderr,"%s: write error!\n",progname);
			exit(-1);
		}
		out_bytes += n;
		if (n < BUF_LEN-1) {
			if( putc (QWK_EOL, msg_fd) == EOF) {
				fprintf(stderr,"%s: write error!\n",progname);
				exit(-1);
			}
			out_bytes++;
		}
		eof = Fgets (buf, BUF_LEN, fd);
	} while ( (strncmp(buf,"From ", 5)) && (NULL != eof) );

	/* Pad block as necessary */

	n = out_bytes % 128;
	for (;n<128;n++)
		if( putc(' ', msg_fd) == EOF) {
		fprintf(stderr,"%s: write error!\n",progname);
		exit(-1);
	}
}


/*
 *  Open ZipNews mail file
 */

void OpenZipMail ()
{
	char fn[PATH_LEN];

	/* Make name */
	sprintf (fn, "%s/%s.mai", home_dir, user_name);

	/* Open it */
	if (NULL == (mai_fd = fopen (fn, "w"))) {
		fprintf (stderr, "%s: can't open %s\n", progname, fn);
		exit(-1);
	}
}


/*
 *  Convert a message to Zip format
 */

void ZipMessage(fd, mp, xhdrs)
FILE *          fd;
struct mail_ent *mp;
char		*xhdrs;
{
	int bytes, c;

	/* Compute message size */
	bytes = mp->end - mp->begin;

	/* Write separator */
	for (c=0; c<20; c++)
		if( putc(1, mai_fd) == EOF) {
			fprintf(stderr,"%s: write error!\n",progname);
			exit(-1);
		}

	if(	(fprintf (mai_fd, "\r\n") < 0)
		&& ferror(mai_fd) ) {
		fprintf(stderr,"%s: write error!\n",progname);
		exit(-1);
	}

	/* Seek to start of message */
	fseek (fd, mp->begin, 0);

	if(xhdrs && *xhdrs) {
		char s[1024] = "";
		int  err=0;

		while(fgets(s, sizeof(s), fd) && ftell(fd) < mp->end) {
			bytes -= strlen(s);
			if(s[0]=='\n' || (s[0]=='\r' && s[1]=='\n')) {
				if(fputs(xhdrs,msg_fd)==EOF
				|| fputs(s,    msg_fd)==EOF)
					err++;
				break;
			}
			if(fputs(s, msg_fd)==EOF) {
				err++;
				break;
			}
		}
		if(err) {
			fprintf(stderr, "%s: write error!\n", progname);
			exit(-1);
		}
	}

	/* Copy bytes */
	while (bytes--) {
		if( (c = getc(fd)) == EOF) {
			fprintf(stderr,"%s: read error (%d bytes unread)\n", progname, bytes);
			exit(-1);
		}

		/* ZipNews doesn't like ^Z's */
		if (c == 26) c = 32;

		/* Map LF to CRLF */
		if ( c==10 && putc(13, mai_fd) == EOF) {
			fprintf(stderr,"%s: write error!\n",progname);
			exit(-1);
		}

		if( putc((0xff & c), mai_fd) == EOF) {
			fprintf(stderr,"%s: write error!\n",progname);
			exit(-1);
		}
	}
}
