#
/*	This is the second and final segment of the QMC Unix Editor - em */
#define LBSIZE	512
#define UNIXBUFL 100
#define	error  errfunc() /*goto errlab pgas: don't think you can goto a pointer label in modern C*/
#define TABSET	7	/* this should be determined dynamically */

/* #define RAW	040 */
#define ECHO	010
#define OPEN	'/'
#define BELL	07
#define ESCAPE	033
#define SPACE	040
#define BACKSL	0134
#define RUBOUT	0177
#define CTRLA	01
#define CTRLB	02
#define CTRLC	03
#define CTRLD	04
#define CTRLE	05
#define CTRLF	06
#define CTRLH	010
#define CTRLI	011
#define CTRLQ	021
#define CTRLR	022
#define CTRLS	023
#define	CTRLV	026
#define CTRLW	027
#define CTRLX	030
#define CTRLZ	032

#define ITT	0100000

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>


extern int	peekc;
extern char	*linebp, *loc2, linebuf[LBSIZE], genbuf[LBSIZE],
			unixbuffer[UNIXBUFL];
extern struct sigaction	onhup;
extern struct sigaction	onquit;
extern int	*zero, *addr1, *addr2;

/*extern */
int append(int (*f)(void), int *a);
void compile(int aeof);
void errfunc(void);
int execute(int gf,int* addr);
void delete(void);
char *em_getline(int tl);
void setdot(void);
void nonzero(void);
int putline(void);
void putchr(int ac);
void putstr(char *as);
int getchr(void);


/* forward declaration */
void setraw(void);
void setcook(void);
int getnil(void);
int getopen(void);
int gopen(void);
void help(void);
void putb(char *ptr);
int rescan(void);
int inword(char c) ;
void putch(char ch);

int	margin	= LBSIZE - 40;
int	oflag;
char	*threshold, *savethresh;
char	*lnp,*gnp,*brp;


/* terminal code from:
Advanced Programming in the UNIX Environment 2nd edition*/
/*int	savetty, tty[3];*/
static struct termios       save_termios;
static int     ttysavefd = -1; /*needed when/if tty_atexit is installed*/
static enum { RESET, RAW, CBREAK } ttystate = RESET;



void op(size_t inglob)
{	register int *a1;
	register char *lp, *sp;
	int seof, ch;
	int t, nl;

	threshold = genbuf + margin;
	savethresh = 0;
	
	switch (ch = peekc = getchr()) {

		case BACKSL:
			t = 1;
			delete();
			addr2 = addr1;
			break;

		case ';':
		case '+':
			t = 0;
			break;

		case '-':
			t =1;
			break;

		default:
			goto normal;

	}

	peekc = 0;
	if(addr1 != addr2) error;
	oflag = 0;
	append(getnil, addr2 - t);
	addr1 = addr2 -= (t-1);
	setdot();
	nonzero();

normal:
	if(addr1 == zero) error;
	if ((seof = getchr()) == '\n') { loc2 = linebuf-1; seof = 0;}
		else compile(seof);
	setraw(); /* terminal into raw mode*/
	for ( a1 = addr1; a1 <= addr2; a1++) {
		if (seof) {if (execute(0,a1) == 0) continue;}
			else em_getline(*a1);
		putstr("\\\r");
		lp = linebuf;
		sp = genbuf;
		inglob |= 01;
		while (lp < loc2){ putch(*lp); *sp++ = *lp++; }
		lnp = lp;
		gnp = sp;

		oflag = gopen(); /* open the current line */
		*a1 = putline(); /* write revised line */
		nl = append( getopen,a1);
		a1 += nl;
		addr2 += nl;
	}
	setcook(); /* terminal in cook mode */
	putchr('\n');
	if (inglob == 0) { putchr('?'); error;}
}

int getnil(void)
{
	if(oflag == EOF) return EOF;
	linebuf[0] = '\0';
	oflag = EOF;
	return 0;
}

void setraw(void)
{
  /* terminal code from:
     Advanced Programming in the UNIX Environment 2nd edition*/
  struct termios  buf;

  if (ttystate != RESET) {
    error;
  }
    if (tcgetattr(0, &buf) < 0)
      error;
    save_termios = buf; 
    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    buf.c_cflag &= ~(CSIZE | PARENB);
    buf.c_cflag |= CS8;
    buf.c_cc[VMIN] = 1;
    buf.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSAFLUSH, &buf) < 0)
      error;
    if (tcgetattr(0, &buf) < 0) {
        tcsetattr(0, TCSAFLUSH, &save_termios);
        error;
    }
    ttystate = RAW;
    ttysavefd = 0;
}

void setcook(void)
{
   if (ttystate == RESET)
        return;
    if (tcsetattr(0, TCSAFLUSH, &save_termios) < 0)
        error;
    ttystate = RESET;
    /* tty[2] = savetty; */
	/* stty(0, tty); */
}

/* for the record....probably a good thing to setup in main */
/* void */
/* tty_atexit(void)        /\* can be set up by atexit(tty_atexit) *\/ */
/* { */
/*     if (ttysavefd >= 0) */
/*         tty_reset(ttysavefd); */
/* } */


int inword(char c) 
{	if(c -'0' < 0 ) return 0;
	if(c - '9' <= 0) return 1;
	c &= 0137;
	if(c -'A' < 0) return 0;
	if(c - 'Z' <= 0) return 1;
	return 0;
}

int rescan(void)
{	register char *lp, *sp;

	if(savethresh) { threshold = savethresh; savethresh = 0; }
	lp = linebuf;
	sp = genbuf;
	while(*lp++ = *sp++) if(lp>linebuf+LBSIZE) {
				*(--lp) = 0;
				return 0;
          }
}


int gopen(void)
	/*leaves revised line in linebuf,
	returns 0 if more to follow, EOF if last line */
{	register char *lp, *sp, *rp;
	int ch;
	char *br, *pr;
	int tabs;
	int retcode, savint, pid, rpid;
	int render_width;

	lp = lnp;
	sp = gnp;
	tabs = 0;
	render_width = 0;
	for (rp = genbuf; rp < sp; rp++) if (*rp == CTRLI) tabs =+ TABSET;

	for(;;){
		switch (ch = getchr()) {

			case CTRLD:
			case ESCAPE:	/* close the line (see case '\n' also) */
			close:
				putb(lp);
				while(*sp++ = *lp++);
				rescan();
				return(EOF);

			case CTRLA:	/* verify line */
			verify: {
				int prefix_width = 0;
				int tail_width = 0;
				int full_width = 0;
				int clear_extra = 0;
				int backspaces = 0;
				char *tp;

				putstr("\\\r");
				*sp = '\0';
				putb(genbuf);
				putb(lp);

				for (tp = genbuf; tp < sp; tp++)
					prefix_width += (*tp == CTRLI) ? TABSET : 1;
				for (tp = lp; *tp; tp++)
					tail_width += (*tp == CTRLI) ? TABSET : 1;

				full_width = prefix_width + tail_width;
				if (render_width > full_width) {
					clear_extra = render_width - full_width;
					while (clear_extra--)
						putch(' ');
					clear_extra = render_width - full_width;
				}

				backspaces = tail_width + clear_extra;
				while (backspaces--)
					putch('\b');
				render_width = full_width;
				continue;
			}

			case CTRLB:	/* back a word */
				if(sp == genbuf) goto backquery;

				while((*(--lp) = *(--sp)) == SPACE)
					if(sp < genbuf) goto out;
				if(inword(*sp)) {
					while(inword((*(--lp) = *(--sp))))
						 if(sp < genbuf) goto out;
					if(*sp == SPACE)
						while((*(--lp) = *(--sp)) == SPACE)
							if(sp < genbuf) goto out;
				}
				else while(sp >= genbuf && !inword(*sp))
					if((*lp-- = *sp--) == CTRLI) tabs =- TABSET;
			out:	sp++;
				lp++;
				goto verify;

			case CTRLC:
			case CTRLQ: /* forward one char */
				if(*lp == 0) goto backquery;
				goto forward_echo;
			forward_echo:
				putch(*lp);
			forward:
				if(*lp == SPACE && sp + tabs > threshold) {
					putch('\r');
					ch = '\n'; putch(ch);
					lp++;
					*sp++ = ch;
					br = sp;
					break;
				}
				if(*lp == CTRLI) tabs =+ TABSET;
				*sp++ = *lp++;	/* one character */
				if(sp + tabs == threshold) putch(BELL);
				continue;


			case CTRLF:
			/* delete forward */
				while(*lp++);
				lp--;
				goto verify;

			case CTRLE:
				putb(lp);
				goto verify;

			case CTRLH:  help(); goto verify;

			case CTRLR:	/* margin release */
				if(threshold-genbuf<LBSIZE-40) {
					savethresh = threshold;
					threshold = genbuf+LBSIZE-40;
				}
				else goto backquery;
				continue;

			case CTRLS:	/* re-set to start of line */
				while(*sp++ = *lp++);
				rescan();
				lp = linebuf; sp = genbuf;
				tabs = 0;
				goto verify;

/* 			case CTRLV:	/\* verify spelling *\/ */
/* 				rp = sp; */
/* 				pr = unixbuffer+UNIXBUFL-2; */
/* 				*pr = 0; */
/* 				while(*(--rp) == SPACE); */
/* 				while(inword(*rp) && rp >= genbuf) */
/* 					*(--pr) = *rp--; */
/* 				if(*pr == 0) goto backquery; */
/* 				putstr("!!"); */
/* 				setcook(); */
/* 				if((pid = fork()) == 0) */
/* 					{signal(SIGHUP, onhup); */
/* 					  signal(SIGQUIT, onquit); */
/* 					  execl("/bin/spell", "spell", pr, 0); */
/* 					  putstr("sorry, can't spell today"); */
/* 					  exit(); */
/* 				} */
/* 				savint = signal(SIGINTR,1); */
/* 				while((rpid = wait(&retcode)) != pid */
/* 					 && rpid != -1); */
/* 				signal(SIGINTR, savint); */
/* 				setraw(); */
/* 				putstr("!!"); */
/* 				goto verify; */


			case CTRLW:	/* forward one word */
				if(*lp == '\0') goto backquery;
				while(*lp == SPACE)
					putch(*sp++ = *lp++);
				if(inword(*lp)) {
					while(inword(*lp)) {
						putch(*sp++ = *lp++);
						if(sp+tabs==threshold) putch(BELL);
					}
					if(*lp == SPACE) {
						if(sp+tabs>threshold) {
							ch = '\n';
							lp++;
							*sp++ = ch;
							br = sp;
							putch('\r');
							putch('\n');
						}
						if(*lp == SPACE)
							while(*(lp+1) == SPACE)
							   putch(*sp++ = *lp++);
					}
				}
				else while(*lp && !inword(*lp)) {
					if(*lp == CTRLI) tabs =+ TABSET;
					putch(*sp++ = *lp++);
					if(sp+tabs==threshold) putch(BELL);
				}
				break;


			case CTRLZ:	/* delete a word */
				if(sp == genbuf) goto backquery;

				while(*(--sp) == SPACE) if(sp < genbuf) goto zout;
				if(inword(*sp)) {
					while(inword(*(--sp)))
						 if(sp < genbuf) goto zout;
					if(*sp == SPACE)
						while(*(--sp) == SPACE)
							if(sp < genbuf) goto zout;
				}
				else while(sp >= genbuf && !inword(*sp))
					if(*sp-- == CTRLI) tabs =- TABSET;
			zout:	sp++;
				goto verify;

			case '@':	/*delete displayed line */
			/* delete backward */
				sp = genbuf;
				tabs = 0;
				goto verify;

			case RUBOUT:
				putstr("\\\r");
				setcook();
				error;

			case CTRLX:
				putch('#');
			case '#':
				if( sp == genbuf) goto backquery;
				if(*(--sp) == CTRLI) tabs =- TABSET;
				goto verify;

			case '\n':
			case '\r': /* split line, actually handled at
					end of switch block */
				ch = '\n';
				*sp++ = ch;
				br = sp;
				break;

			case BACKSL: /* special symbols */
				switch (ch = peekc = getchr()) {
				case '(':  ch = '{'; peekc = 0; break;
				case ')':  ch = '}'; peekc = 0; break;
				case '!':  ch = '|'; peekc = 0; break;
				case '^':  ch = '~'; peekc = 0; break;
				case '\'':  ch = '`'; peekc = 0; break;
				case BACKSL:
				case '#':
				case '@':  peekc = 0; break;

				default:  if(ch >= 'a' && ch <= 'z') {
						peekc = 0; ch =- 040;}
					else {
						*(--lp) = BACKSL;
						goto forward;
						}
				}

			default:
				*(--lp) = ch;
				goto forward_echo;
			}

		if (ch == '\n') { /* split line */
			if(*(br-1) != '\n') putstr("!!");	/*debugging only */
			lnp = sp;
			while(*sp++ = *lp++); /*move the rest over */
			brp  = linebuf +(br - genbuf);
			lnp = linebuf + (lnp - br);
			rescan();
			*(brp-1) ='\0';
			return(0);
			}
		else continue;
       backquery: putch(CTRLZ);
        } /* end of forloop block */
} /* end of gopen */



int getopen(void)  /* calls gopen, deals with multiple lines etc. */
{	register char *lp, *sp;
	if (oflag == EOF) return EOF;

/* otherwise, multiple lines */

	lp = linebuf;
	sp = brp;
	while(*lp++ = *sp++); /*move it down */
	sp = genbuf;
	lp = linebuf;
	while (lp < lnp) *sp++ = *lp++;
	gnp = sp;
	/* should check whether empty line returned */
	oflag = gopen();
	return 0;
}

void putch(char ch)
{ write(1, &ch, 1); }

void putb(char *ptr)	/*display string */
{	register char *p;

	p = ptr;
	if(*p == '\0') return;
	while(*(++p));
	write(1,ptr,p-ptr);
}

void help(void)
{	putstr("\n");
	putstr("	^A	display Again		^Q	next character");
	putstr("	^B	backup word		^R	Release margin");
	putstr("	ESCAPE				^S	re-scan from Start");
	putstr("	or ^D	close line and exit	^V	verify spelling");
	putstr("	^E	display to End		^W	next Word");
	putstr("	^F	delete line Forward	^Z	delete word");
	putstr("	^H	Help			# or ^X delete character");
	putstr("	RUBOUT	exit unchanged		@	delete line backward\n");
	putstr("	Other characters (including RETURN) inserted as typed");
}
