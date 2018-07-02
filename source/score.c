/*
 * $Id: score.c,v 1.4 1999/04/14 15:15:11 js Exp $
 */

#ifdef	SCORES

#include <stdio.h>
#include <string.h>
#include <regex.h>			/* POSIX regular expressions */
#include "uqwk.h"

/*
 * Score file processing
 *   function for scanning article header for matches
 *   utilities for reading/freeing score files
 *
 * 960413 Jim Tittsler <7j1ajh@amsat.org>
 */


struct scorest {
	struct scorest *next;
	regex_t rexp;
#ifdef DEBUG
	char re_text[BUF_LEN];
#endif
	int score;
};

struct scorest *kill_list = NULL;
struct scorest *group_kill_list = NULL;

char *sws ( char *p ) {
char c;
	while((c = *p)) {
		if ((c == ' ') || (c == '\t')) ++p;
		else break;
	}
	return p;
}

char *snf ( char *p ) {
char c;
	while((c = *p)) {
		if ((c != ' ') && (c != '\t')) ++p;
		else break;
	}
	return sws(p);
}

int read_score_file ( const char *score_filename )
{
	FILE *score_fd;
	char score_pathname[PATH_LEN];
	char score_regex[BUF_LEN];
	char *p,*s;
	struct scorest *score_entry;
	struct scorest **score_link;
	int	global = 0;

	strcpy (score_pathname, kill_dir);
	strcat (score_pathname, "/");
	strcat (score_pathname, score_filename);


	global = !strcasecmp (score_filename, GLOBAL_SCORE_FILE);
	if(global) {
		score_link = &kill_list;
	} else {
		score_link = &group_kill_list;
	}

#ifdef DEBUG
	fprintf(stderr,"Looking for score file: %s\n", score_pathname);
#endif
	if ((score_fd = fopen(score_pathname, "r")) == NULL) {
		printf(" ");
		return 0;			/* no score file */
	}
	printf("+");

#ifdef DEBUG
	fprintf(stderr,"Loading %d score file \"%s\"\n",
		global?"global":"group", score_filename);
#endif

	while (Fgets (buf, BUF_LEN, score_fd) != NULL) {
		p = sws (buf);
		if ( (*p == '#') || (*p == '\0') || (*p == ';') ) continue;
		if (!strncasecmp(p, "killthr", 7)) {
			p = snf(p);
			if (global) {
				kill_thresh = atoi (p);
#ifdef DEBUG
	fprintf(stderr,"Global kill threshold: %d\n", kill_thresh);
#endif
			} else {
				group_kill_thresh = atoi (p);
#ifdef DEBUG
	fprintf(stderr,"Group kill threshold: %d\n", group_kill_thresh);
#endif
			}
			continue;
		}
		if ( (score_entry = (struct scorest *)malloc( sizeof (struct scorest) ) ) == NULL) {
			fprintf (stderr, "unable to allocate space for building score file\n");
			return (0);
		}
		score_entry->next = NULL;

		score_entry->score = atoi(p);
		p = snf (p);
		if (strncasecmp(p, "pattern", 7) == 0) p = snf (p);
		s = score_regex;
		while (*p) {
			if (*p == ' ') break;
			*s++ = *p++;
		}
		*s = '\0';
		p = sws (p);

		if (strncasecmp(score_regex, "Header", 6) == 0)
			*score_regex = '\0';
		else if (strncasecmp(score_regex, "Body", 4) == 0) {
			fprintf (stderr, "Body scoring not supported\n");
			free (score_entry);
			return 0;
		}
		strcat(score_regex, ".*");
		strcat(score_regex, p);
		strcat(score_regex, ".*");

#ifdef	DEBUG
	strcpy(score_entry->re_text, score_regex);
	fprintf(stderr,"score: %d where:%s: expression:%s:\n", score_entry->score, score_regex, p);

	fprintf(stderr,"compiling regular expresion\n");
#endif

		if ( regcomp(&score_entry->rexp,
		     score_regex,
		     REG_ICASE | REG_NOSUB) ) {
			fprintf(stderr, "bad regular expression\n");
				free(score_entry);
				return 0;
		} else {
#ifdef	DEBUG
	fprintf(stderr,"compiled okay, linking into score list\n");
#endif
			*score_link = score_entry;
			 score_link = &score_entry->next;
		}
#ifdef	DEBUG
	fprintf(stderr,"next score entry\n");
#endif
	}

	fclose (score_fd);

#ifdef DEBUG
	for (sp = kill_list; sp ; sp = sp->next)
		fprintf(stderr, "(%d) %d\t%s [global]\n", (unsigned long)sp, sp->score, sp->re_text);
	for (sp = group_kill_list; sp ; sp = sp->next)
		fprintf(stderr, "(%d) %d\t%s [%s]\n",
			(unsigned long)sp, sp->score, sp->re_text, score_filename);
#endif

	return 1;
}

int free_group_score(void)
{
	struct scorest *sp, *np;

#ifdef	DEBUG
	fprintf(stderr,"freeing group score...\n");
#endif
	for (sp = group_kill_list ; sp ; sp = np)
	{
#ifdef DEBUG
	fprintf(stderr, "\t(%ld) %d\t%s\n", (unsigned long)sp, sp->score, sp->re_text);
#endif
		np = sp->next;
		regfree(&sp->rexp);
		free(sp);
	}
	group_kill_list = NULL;

	return (0);
}

int Kill (FILE *art_fd)
{
	struct scorest *sp;
	int score = 0;

	/* scan through header lines, looking for regex matches */
	while (Fgets (buf, BUF_LEN, art_fd) != NULL) {
		if (strlen(buf) == 0) break;
#ifdef	DEBUG
	fprintf(stderr,"header line: %s\n", buf);
#endif
		for (sp = kill_list; sp ; sp = sp->next) {
			if (regexec(&(sp->rexp), buf, 0, NULL, 0) == 0) {
				score += sp->score;
			}
		}
		for (sp = group_kill_list; sp ; sp = sp->next) {
			if (regexec(&(sp->rexp), buf, 0, NULL, 0) == 0) {
				score += sp->score;
			}
		}
	}
#ifdef	DEBUG
	fprintf(stderr,"article score: %d\n", score);
#endif
	return score;
}

#endif	/* SCORES */
