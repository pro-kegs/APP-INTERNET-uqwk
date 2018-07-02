/* $Id: */

#include <stdio.h> 
#include <signal.h>
#include "uqwk.h"

char *strsignal(int sig)
{
  struct
  {
    int s;
    char *str;
  }
  sigs[]= {
    {SIGSEGV,"segmentation error"},
    {SIGILL,"illegal instruction"},
    {SIGABRT,"abort signal"},
    {SIGTERM,"termination signal"},
    {SIGINT,"interrupt"},
    {SIGKILL,"kill process"},
    {SIGHUP,"hangup signal"},
    {SIGQUIT,"quit signal"},
    {SIGBUS,"bus error"},
    {-99,"call for undefined signal message"}},*p;
  
  for(p=sigs;p->s!=-99;p++)
    if (p->s==sig)
      break;
  return p->str;
}

static void catchsig(int sig)
{
  fprintf(stderr,"%s: caught signal # %d: %s\n",progname,sig,strsignal(sig));
  exit(sig);   
}

void install_signals(void)
{
  int sigs[]={SIGINT,SIGKILL,SIGQUIT,SIGTERM,SIGSEGV,SIGILL,SIGHUP,SIGBUS,-1};
  int *i;
  
  for(i=sigs;*i!=-1;i++)
    signal(*i,catchsig);
}
