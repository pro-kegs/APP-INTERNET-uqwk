/*
 *  $Id: uqwk.h,v 1.9 1999/10/19 08:48:42 js Exp $
 */

#ifdef	HAVE_CONFIG
  #include HAVE_CONFIG
#else
  #include "config-generic.h"
#endif

#include "uqwk-version.h"

#ifdef NNTP_USE_ACTIVE
	#undef	NNTP
	#define	NNTP
	#define NNTP_ACTIVESTR	"+full-active"
#else
	#define	NNTP_ACTIVESTR	""
#endif

#ifdef NNTPAUTH
	#undef	NNTP
	#define	NNTP
	#define	NNTPAUTHSTR	"auth"
#else
	#define	NNTPAUTHSTR	""
#endif

#ifdef NNTP
	#define NNTPSTR	"+nntp"
#else
	#define NNTPSTR	""
#endif

#ifdef SCORES
	#define SCORESTR	"+scoring"
#else
	#define SCORESTR	""
#endif

#ifdef GNKSA
	#define	NO_SIZE_T	/* don't ask... */
	#define GNKSASTR	"+gnksa"
	#define	GNKSA_VERBOSE	1
	#define GNKSA_HONOUR	2
#else
	#define GNKSASTR	""
#endif

#define	UQWK_VERSION	VERSION"/"CONFIGNAME""NNTPSTR""NNTPAUTHSTR""GNKSASTR""SCORESTR


#ifdef ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

#define	PATH_LEN	(256)	/* Length for file names, etc. */
#define BUF_LEN		(16384)	/* Length of general purpose buffer */

#define QWK_EOL		(227)	/* QWK end-of-line */
#define QWK_ACT_FLAG	(225)	/* Active message flag */
#define QWK_PUBLIC	' '	/* Public message flags */
#define QWK_PUBLIC2	'-'
#define QWK_PRIVATE	'+'	/* Private message flags */
#define QWK_PRIVATE2	'*'

#define SLNP_MAILER	SENDMAIL" -t"
#define ZIP_MAILER	SENDMAIL
#define XPRT_MAILER	SENDMAIL" -t"

#define MAIL_CONF_NAME  "Email"

#ifdef SCORES
  #define GLOBAL_SCORE_FILE "global"
#endif

/* Defaults */
#define	DEF_DO_DEBUG	(0)
#ifdef NNTPAUTH
  #define DEF_DO_AUTH	(0)
  #define DEF_AUTH_USER	""
  #define DEF_AUTH_PASS	""
#endif
#define DEF_DO_MAIL	(0)
#define DEF_DO_NEWS	(0)
#ifdef SCORES
  #define DEF_DO_KILLS	(1)
#endif
#define DEF_IGNORE0FD	(1)
#define DEF_INC_HDRS	(1)
#define DEF_PRT_OPTS	(0)
#define DEF_READ_ONLY	(0)
#define DEF_WAF_MODE	(0)
#define DEF_SLNP_MODE	(1)
#define DEF_ZIP_MODE	(0)
#define DEF_XRF_MODE	(1)
#define DEF_BW_KLUDGE	(0)
#define DEF_XPRT_MODE	(0)
#define DEF_SUM_MODE	(0)
#define DEF_SEL_MODE	(0)
#define DEF_EVERY_MODE	(0)
#define DEF_MAX_BLKS	(4096)	/* Exactly half a megabyte */
#define DEF_GRP_LEN	(15)
#define DEF_HOME_DIR	"."
#define DEF_USER_NAME	"nobody"
#define DEF_MAIL_FILE	"unknown"
#define DEF_NRC_FILE	"unknown"
#define DEF_REP_FILE	"none"
#define DEF_TRN_FILE	"none"
#define DEF_HOST_NAME	"nowhere"
#define DEF_SUM_FILE	"none"
#define DEF_SEL_FILE	"none"
#define DEF_NG_FILE	"none"
#ifdef SCORES
  #define DEF_KILL_DIR	".uqwk"
#endif

/* Runtime options */
EXTERN	int do_debug;
#ifdef NNTPAUTH
  EXTERN	int do_auth;		/* Do NNTP auth? */
#endif
EXTERN	int do_mail;			/* Process mail? */
EXTERN	int do_news;			/* Process news? */
#ifdef SCORES
  EXTERN	int do_kills;		/* Process kill/score files? */
  EXTERN	int kill_thresh;	/* global kill threshold */
  EXTERN	int group_kill_thresh;	/* kill threshold within this group */
#endif
EXTERN int ignore_0fd;

EXTERN	int inc_hdrs;			/* Include headers in messages? */
EXTERN	int prt_opts;			/* Display options; no processing */
EXTERN	int read_only;			/* Don't rewrite mail and .newsrc */
EXTERN	int max_blks;			/* Maximum blocks per QWK packet */
EXTERN	int grp_len;			/* Maximum newsgroup name length */
EXTERN	int waf_mode;			/* Waffle type .newsrc */
EXTERN	int slnp_mode;			/* Write SLNP packet */
EXTERN	int zip_mode;			/* Write ZipNews packet */
EXTERN	int xrf_mode;			/* Cancel crosspostings */
EXTERN	int bw_kludge;			/* Blue Wave kludge */
EXTERN	int xprt_mode;			/* Expert mode */
EXTERN	int sum_mode;			/* Summary mode */
EXTERN	int sel_mode;			/* Selection mode */
EXTERN	int every_mode;			/* Write every grp to CONTROL.DAT */

#ifdef NNTPAUTH
	EXTERN	char auth_user[PATH_LEN];/* NNTP auth user */
	EXTERN	char auth_pass[PATH_LEN];/* NNTP auth pass */
#else
	EXTERN	char act_file[PATH_LEN];	/* Active file */
	EXTERN	char news_dir[PATH_LEN];	/* News spool dir */
#endif
EXTERN	char mail_file[PATH_LEN];	/* mail spool */
EXTERN	char mail_dir[PATH_LEN];	/* dir for mail spool */
EXTERN	char home_dir[PATH_LEN];	/* home directory */
EXTERN	char user_name[PATH_LEN];	/* user's login name */
EXTERN	char bbs_name[PATH_LEN];	/* BBS name */
EXTERN	char bbs_city[PATH_LEN];	/* BBS city */
EXTERN	char bbs_phone[PATH_LEN];	/* BBS phone number */
EXTERN	char bbs_sysop[PATH_LEN];	/* BBS sysop name */
EXTERN	char bbs_id[PATH_LEN];		/* BBS ID */
EXTERN	char nrc_file[PATH_LEN];	/* .newsrc file */
EXTERN	char rep_file[PATH_LEN];	/* Reply packet file name */
EXTERN	char trn_file[PATH_LEN];	/* Newsgroup name translation table */
EXTERN	char host_name[PATH_LEN];	/* Host machine name */
EXTERN	char sum_file[PATH_LEN];	/* Summary file */
EXTERN	char sel_file[PATH_LEN];	/* Selection file */
EXTERN	char ng_file[PATH_LEN];		/* Desired ng file */

EXTERN	char inews_path[PATH_LEN];		/* path to inews */
EXTERN	char posting_agent[PATH_LEN];		/* posting agent (inews + '-h') */

#ifdef SCORES
	EXTERN  char kill_dir[PATH_LEN];/* directory for score/kill files */
#endif

#ifdef	GNKSA
EXTERN	int gnksa_level;			/* Validate messages before posting? */
#endif

char *getenv();
char *Fgets();
char *ParseFrom();
struct act_ent *FindActive();
struct sub_ent *SubList();
struct sub_ent *MarkRead();
struct sub_ent *FixSub();
struct conf_ent *NewConference();

/* Various globals */
EXTERN	char *progname;			/* Program name */
EXTERN	int msg_cnt;			/* Total number of messages */
EXTERN	int conf_cnt;			/* Total number of conferences */
EXTERN	FILE *msg_fd;			/* MESSAGES.DAT file desc */
EXTERN	char msg_fn[PATH_LEN];		/* SLNP *.MSG file name */
EXTERN	FILE *ctl_fd;			/* CONTROL.DAT file desc */
EXTERN	FILE *ndx_fd;			/* xxx.NDX file desc */
EXTERN	char ndx_fn[PATH_LEN];		/* xxx.NDX file name */
EXTERN	FILE *act_fd;			/* Active file file desc */
EXTERN	FILE *nrc_fd;			/* .newsrc file desc */
EXTERN	FILE *rep_fd;			/* Reply packet file desc */
EXTERN	FILE *nws_fd;			/* ZipNews .nws file desc */
EXTERN	FILE *idx_fd;			/* ZipNews .idx file desc */
EXTERN	FILE *mai_fd;			/* ZipNews .mai file desc */
EXTERN	int zip_flag;			/* idx entry written for this ng? */
EXTERN  char buf[BUF_LEN];	        /* General purpose buffer */
EXTERN	int blk_cnt;			/* Blocks written to messages.dat */
EXTERN	int sum_flag;			/* Summary hdr for this ng yet? */
EXTERN	FILE *sum_fd;			/* Summary file desc */
EXTERN	FILE *sel_fd;			/* Selection file desc */

/* This is the stuff we remember about each spooled mail message */
EXTERN	struct mail_ent
{
	long begin;		/* Offset of start of header */
	long text;		/* Offset to end of header, start of text */
	long end;		/* Offset to start of next message */
	struct mail_ent *next;	/* Pointer to next */
} *mail_list;

/* This is stuff we remember about each "conference" */
EXTERN	struct conf_ent
{
	char *name;		/* Conference name */
	int number;		/* Conference number */
	int count;		/* Number of articles in this conference */
	struct conf_ent *next;	/* Pointer to next */
} *conf_list, *last_conf;

/* This is the QWK message header format */
struct qwk_hdr
{
        char status;
        char number[7];
        char date[8];
        char time[5];
        char to[25];
        char from[25];
        char subject[25];
        char password[12];
        char refer[8];
        char blocks[6];
        char flag;
        char conference[2];
        char msg_num[2];
        char tag;
};

EXTERN	struct qwk_hdr rep_hdr;		/* Header for replies */

/* Stuff we remember about each active newsgroup */
EXTERN	struct act_ent
{
	char *name;		/* Newsgroup name */
	int hi;			/* High article number */
	int lo;			/* Low article number */
	struct act_ent *next;	/* Pointer to next */
} *act_list;

/* Stuff we remember about the .newsrc file */
EXTERN	struct nrc_ent
{
	char *name;		/* Newsgroup name */
	int subscribed;		/* Subscribed flag */
	struct sub_ent *sub;	/* Subscription list */
	int conf;		/* Corresponding conference number */
	struct nrc_ent *next;	/* Pointer to next */
} *nrc_list;

/* Stuff we remember about the newsgroup name translation table */
EXTERN	struct trn_ent
{
	char *old;		/* Old name */
	char *new;		/* New name */
	struct trn_ent *next;	/* Pointer to next */
} *trn_list;

/* Stuff we remember about a subscription list (in the .newsrc) */
struct sub_ent
{
	int lo, hi;		/* Range of articles */
	struct sub_ent *next;	/* Pointer to next */
};

/* Stuff we remember about the desired newsgroup list */
EXTERN	struct ng_ent
{
	char *name;		/* Newsgroup name */
	struct ng_ent *next;	/* Pointer to next */
} *ng_list;

/* Function prototypes */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<unistd.h>

void DefaultOptions();
void EnvOptions();
void CommandOptions(int, char **);
void InitStuff();
int DoReply();
int DoNews();
int DoMail();
int DoClassicMail();
int DoMaildirMail();
void AddMessage(FILE *fd, struct mail_ent *me, char *xhdrs);
void SLNPMessage(FILE *fd, struct mail_ent *me, char *xhdrs);
void ZipMessage(FILE *fd, struct mail_ent *me, char *xhdrs);
void QwkMessage(FILE *fd, struct mail_ent *me, char *xhdrs);
void DoFromLine(long off);
int CloseStuff();
int BadFlag (char *);
void PrintOptions();
int ReadTrans();
int ReadNG();
void OutOfMemory();
void MakeMailList();
void OpenZipMail();
void inttoms(int, unsigned char[4]);
int  buftoint(unsigned char c[4]);
void Spaces(char *, int);
void PadNum(int, char *, int);
void IntNum(int, char[2]);
void PadString( char *, char *, int);
void ParseDate(char *, struct qwk_hdr *);
char *ParseFrom(char *);
int WriteControl();
int WriteNewsrc();
int WriteAreas();
void TransName (char *);
void WriteSub (FILE *fd, struct sub_ent *sp);
int OpenZipFiles();
int ReadActive();
int ReadNewsrc();
int DoSelect();
int WriteJoin();
int DoGroup (struct nrc_ent *);
void NdxClose (FILE *);
void MsgClose (FILE *);
int IsRead (int, struct sub_ent *);
int DoArticle (char *, int, struct nrc_ent *, struct conf_ent *);
int DoXref (FILE *, long);
int SLNPArticle (FILE *, int);
int ZipArticle (FILE *, int, struct nrc_ent *);
int SumArticle (FILE *, int, long, struct nrc_ent *);
void ws(FILE *fd, struct sub_ent *, struct sub_ent *);
int Xref(char *);
int GetHdr (FILE *, char *, int, char *);
int DoZipReplies();
int SLNPReply();
int SendReply();
int SendWarning(char *);
int CheckTo(char *, int);
int SLuMail(char *);
int SLuNews(char *);
int SLbMail(char *);
int SLbNews(char *);
int SLPipe(FILE *, int, char *);
void AppendBadArticle(char *);
int ZipId();
int ZipMail(char *);
int ZipNews(char *);


int OffLine(FILE *, int);
int QWKOffLine (int, FILE *);
int GetLine(int *, FILE *);
void Help(FILE *);
int Subscribe(FILE *);
int Unsubscribe(FILE *);
int Groups(FILE *);
int Allgroups(FILE *);
int Shell(FILE *);
int Catchup(FILE *);

FILE *getactive_nntp();
void put_server ();
