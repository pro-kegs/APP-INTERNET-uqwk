/*
 *  $Id: uqwk.c,v 1.6 2001/10/25 20:22:20 js Exp $
 */

#include <stdio.h>
#include <string.h>

#define ALLOCATE
#include "uqwk.h"

void install_signals(void);

/*
 *  Make really sure we close up properly...
 */


void do_exit(void)
{
  CloseStuff();
}


/*
 *  Try to make QWK packets from a mail and/or news spool
 */

int main (argc, argv)
int argc;
char *argv[];
{
	progname = argv[0];

	/* Set up defaults */
	DefaultOptions();

	/* Look for environment variable overrides */
	EnvOptions();

	/* Look for command line overrides */
	CommandOptions (argc, argv);

	/* Initialize files, etc. */
	InitStuff();
	atexit(do_exit);

	/* Do reply packet? */
	if (strcmp (rep_file, DEF_REP_FILE) && DoReply() < 0) {
		fprintf (stderr, "%s: error sending replies\n", progname);
		return(-1);
	}

	install_signals();

	/* Do news? */
	if (do_news && DoNews() < 0) {
		fprintf (stderr, "%s: error getting news\n", progname);
		return(-1);
	}

	/* Mail? */
	if (do_mail && DoMail() < 0) {
		fprintf (stderr, "%s: error getting mail\n", progname);
		return(-1);
	}

	return CloseStuff();
}
