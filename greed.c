/* 
 * greed.c v4.0 - Written by Matthew T. Day (mday@iconsys.uu.net), 09/06/89
 *
 * Now maintained by Eric S. Raymond <esr@snark.thyrsus.com>.  Matt Day dropped
 * out of sight and hasn't posted a new version or patch in four years.
 */

/* vi:set ts=8: Set tab break=8 in the "vi" editor */

/* Port to MSDOS (using Microsoft C v5.1 and PCCurses) provided by Fred C. *
 * Smith (..!ulowell!cg-alta!fredex), 07/29/89.                            */

/*
 * On SVr3.2 and later UNIX versions, Greed will detect color curses(3)
 * if you have it and generate the board in color, one color to each of the
 * digit values. This will also enable checking of an environment variable
 * GREEDOPTS to override the default color set, which will be parsed as a
 * string of the form:
 *
 *	<c1><c2><c3><c4><c5><c6><c7><c8><c9>[:[p]]
 *
 * where <cn> is a character decribing the color for digit n.
 * The color letters are read as follows:
 *	b = blue,
 *	g = green,
 *	c = cyan,
 *	r = red,
 *	m = magenta,
 *	y = yellow,
 *	w = white.
 * In addition, capitalizing a letter turns on the A_BOLD attribute for that
 * letter.
 *
 * If the string ends with a trailing :, letters following are taken as game
 * options. At present, only 'p' (equivalent to an initial 'p' command) is
 * defined.
 */

static char *version = "Greed v3.1";

#ifdef MSDOS
#define NOTBSD
#endif
#include <ctype.h>

#ifdef NCURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif NCURSES
#include <signal.h>
#ifndef MSDOS
#include <pwd.h>
#endif
#ifdef NOTBSD
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#ifdef A_COLOR
#include <ctype.h>
#endif
#ifdef MSDOS
#include <sys/types.h>

#include <io.h>
#include <stdlib.h>
#endif

#ifdef NOTBSD
#define crmode cbreak
#ifndef MSDOS
#define random lrand48			/* use high quality random routines */
#define srandom srand48
#else
#define random rand
#define srandom srand
#define ESC 27
#endif
#endif

#define MAXSCORE 10			/* max number of high score entries */
#define FILESIZE (MAXSCORE * sizeof(struct score))	/* total byte size of *
							 * high score file    */
#define rnd(x) (int) ((random() % (x))+1)	/* rnd() returns random num *
						 * between 1 and x          */
#define ME '@'				/* marker of current screen location */

#ifdef MSDOS
#define DOS_SCOREFILE "greed.hs"
#else
#define LOCKPATH "/tmp/Greed.lock"	/* lock path for high score file */
#endif

struct score {				/* changing stuff in this struct */
	char user[9];			/* makes old score files incompatible */
	int score;
};
int allmoves = 0, score = 1, grid[22][79], y, x, havebotmsg = 0;
char *cmdname;
WINDOW *helpwin = NULL;
#ifndef MSDOS
extern long random();
#endif
void topscores();

#ifdef MSDOS
char scorepath [_MAX_PATH];
#endif

/* botmsg() writes "msg" at the middle of the bottom line of the screen. *
 * Boolean "backcur" specifies whether to put cursor back on the grid or *
 * leave it on the bottom line (e.g. for questions).                     */

void botmsg(msg, backcur)
register char *msg;
register backcur;
{
	mvaddstr(23, 40, msg);
	clrtoeol();
	if (backcur) move(y, x);
	refresh();
	havebotmsg = 1;
}

/* quit() is run when the user hits ^C or ^\, it queries the user if he     *
 * really wanted to quit, and if so, checks the high score stuff (with the  *
 * current score) and quits; otherwise, simply returns to the game.         */

quit() {
	register ch;
#ifdef NOTBSD
	void (*osig)() = signal(SIGINT, SIG_IGN);	/* save old signal */
#else
	int (*osig)() = signal(SIGINT, SIG_IGN);
#endif
#ifndef MSDOS
	(void) signal(SIGQUIT, SIG_IGN);
#else
	(void) signal(SIGBREAK, SIG_IGN);
#endif

	if (stdscr) {
		botmsg("Really quit? ", 0);
		if ((ch = getch()) != 'y' && ch != 'Y') {
			move(y, x);
			(void) signal(SIGINT, osig);	/* reset old signal */
#ifndef MSDOS
			(void) signal(SIGQUIT, osig);
#else
			(void) signal(SIGBREAK, osig);
#endif
			refresh();
			return 1;
		}
		move(23, 0);
		refresh();
		endwin();
		puts("\n");
		topscores(score);
	}
	exit(0);
}

/* out() is run when the signal SIGTERM is sent, it corrects the terminal *
 * state (if necessary) and exits.                                        */

void out() {
	if (stdscr) endwin();
	exit(0);
}

/* usage() prints out the proper command line usage for Greed and exits. */

void usage() {
	fprintf(stderr, "Usage: %s [-p] [-s]\n", cmdname);
	exit(1);
}

/* showscore() prints the score and the percentage of the screen eaten at the *
 * beginning of the bottom line of the screen, moves the cursor back on the   *
 * grid, and refreshes the screen.                                            */

void showscore() {
	mvprintw(23, 7, "%d  %.2f%%", score, (float) score / 17.38);
	move(y, x);
	refresh();
}

void showmoves();

main(argc, argv)
int argc;
char *argv[];
{
	register val = 1;
	extern long time();
#ifdef A_COLOR
	int attribs[9];
	char *colors;
	extern char *getenv(), *strchr();
#endif

	cmdname = argv[0];			/* save the command name */
	if (argc == 2) {			/* process the command line */
		if (strlen(argv[1]) != 2 || argv[1][0] != '-') usage();
		if (argv[1][1] == 's') {
			topscores(0);
			exit(0);
		}
	} else if (argc > 2) usage();		/* can't have > 2 arguments */

	(void) signal(SIGINT, quit);		/* catch off the signals */
#ifndef MSDOS
	(void) signal(SIGQUIT, quit);
#else
	(void) signal(SIGBREAK, quit);
#endif
	(void) signal(SIGTERM, out);

#ifdef MSDOS
{
	char *envptr = getenv("GREEDPATH");

	if (!envptr) strcpy(scorepath, SCOREFILE);
	else {
		strcpy(scorepath, envptr);
		strcat(scorepath, "/");
		strcat(scorepath, DOS_SCOREFILE);
	}
}
#endif

	initscr();				/* set up the terminal modes */
	crmode();
	noecho();

	srandom(time(0) ^ getpid() << 16);	/* initialize the random seed *
						 * with a unique number       */

#ifdef A_COLOR
	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_YELLOW, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		init_pair(3, COLOR_GREEN, COLOR_BLACK);
		init_pair(4, COLOR_CYAN, COLOR_BLACK);	
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);

		attribs[0] = COLOR_PAIR(1);
		attribs[1] = COLOR_PAIR(2);
		attribs[2] = COLOR_PAIR(3);
		attribs[3] = COLOR_PAIR(4);
		attribs[4] = COLOR_PAIR(5);
		attribs[5] = COLOR_PAIR(1) | A_BOLD;
		attribs[6] = COLOR_PAIR(2) | A_BOLD;
		attribs[7] = COLOR_PAIR(3) | A_BOLD;
		attribs[8] = COLOR_PAIR(4) | A_BOLD;

		if ((colors = getenv("GREEDOPTS")) != (char *) NULL) {
			static char *cnames = " bgcrmywBGCRMYW";
			register char *cp;

			for (cp = colors; *cp && *cp != ':'; cp++)
				if (strchr(cnames, *cp) != (char *) NULL)
					if (*cp != ' ') {
						init_pair(cp-colors+1,
							strchr(cnames, tolower(*cp))-cnames,
							COLOR_BLACK);
						attribs[cp-colors]=COLOR_PAIR(cp-colors+1);
						if (isupper(*cp))
							attribs[cp-colors] |= A_BOLD;
					}
			if (*cp == ':')
			    while (*++cp)
				if (*cp == 'p')
				    allmoves = TRUE;
		}
	}
#endif

	for (y=0; y < 22; y++)			/* fill the grid array and */
		for (x=0; x < 79; x++)		/* print numbers out */
#ifdef A_COLOR
			if (has_colors()) {
				register newval = rnd(9);

				attron(attribs[newval - 1]);
				mvaddch(y, x, (grid[y][x] = newval) + '0');
				attroff(attribs[newval - 1]);
			} else
#endif
			mvaddch(y, x, (grid[y][x] = rnd(9)) + '0');

	mvaddstr(23, 0, "Score: ");		/* initialize bottom line */
	mvprintw(23, 40, "%s - Hit '?' for help.", version);
	y = rnd(22)-1; x = rnd(79)-1;		/* random initial location */
	standout();
	mvaddch(y, x, ME);
	standend();
	grid[y][x] = 0;				/* eat initial square */

	if (allmoves) showmoves(1);
	showscore();

	while ((val = tunnel(getch())) > 0)	/* main loop, gives tunnel() *
		continue;			 * user command              */

	if (!val) {				/* if didn't quit by 'q' cmd */
		botmsg("Hit any key..", 0);	/* then let user examine     */
		getch();			/* final screen              */
	}

	move(23, 0);
	refresh();
	endwin();
	puts("\n");				/* writes two newlines */
	topscores(score);
	exit(0);
}

/* tunnel() does the main game work.  Returns 1 if everything's okay, 0 if *
 * user "died", and -1 if user specified and confirmed 'q' (fast quit).    */

tunnel(cmd)
register cmd;
{
	register dy, dx, distance;
	void help();

	switch (cmd) {				/* process user command */
	case 'h': case 'H': case '4':
		dy = 0; dx = -1;
		break;
	case 'j': case 'J': case '2':
		dy = 1; dx = 0;
		break;
	case 'k': case 'K': case '8':
		dy = -1; dx = 0;
		break;
	case 'l': case 'L': case '6':
		dy = 0; dx = 1;
		break;
	case 'b': case 'B': case '1':
		dy = 1; dx = -1;
		break;
	case 'n': case 'N': case '3':
		dy = dx = 1;
		break;
	case 'y': case 'Y': case '7':
		dy = dx = -1;
		break;
	case 'u': case 'U': case '9':
		dy = -1; dx = 1;
		break;
	case 'p': case 'P':
		allmoves = !allmoves;
		showmoves(allmoves);
		move(y, x);
		refresh();
		return 1;
	case 'q': case 'Q':
		return quit();
	case '?':
		help();
		return 1;
	case '\14': case '\22':			/* ^L or ^R (redraw) */
		wrefresh(curscr);		/* falls through to return */
	default:
		return 1;
	}
	distance = (y+dy >= 0 && x+dx >= 0 && y+dy < 22 && x+dx < 79) ?
		grid[y+dy][x+dx] : 0;

	{
		register j = y, i = x, d = distance;

		do {				/* process move for validity */
			j += dy;
			i += dx;
			if (j >= 0 && i >= 0 && j < 22 && i < 79 && grid[j][i])
				continue;	/* if off the screen */
			else if (!othermove(dy, dx)) {	/* no other good move */
				j -= dy;
				i -= dx;
				mvaddch(y, x, ' ');
				while (y != j || x != i) {
					y += dy;	/* so we manually */
					x += dx;	/* print chosen path */
					score++;
					mvaddch(y, x, ' ');
				}
				mvaddch(y, x, '*');	/* with a '*' */
				showscore();		/* print final score */
				return 0;
			} else {		/* otherwise prevent bad move */
				botmsg("Bad move.", 1);
				return 1;
			}
		} while (--d);
	}

	if (allmoves) showmoves(0);		/* remove possible moves */

	if (havebotmsg) {			/* if old bottom msg exists */
		mvprintw(23, 40, "%s - Hit '?' for help.", version);
		havebotmsg = 0;
	}

	mvaddch(y, x, ' ');			/* erase old ME */
	do {					/* print good path */
		y += dy;
		x += dx;
		score++;
		grid[y][x] = 0;
		mvaddch(y, x, ' ');
	} while (--distance);
	standout();
	mvaddch(y, x, ME);			/* put new ME */
	standend();
	if (allmoves) showmoves(1);		/* put new possible moves */
	showscore();				/* does refresh() finally */
	return 1;
}

/* othermove() checks area for an existing possible move.  bady and badx are  *
 * direction variables that tell othermove() they are already no good, and to *
 * not process them.  I don't know if this is efficient, but it works!        */

othermove(bady, badx)
register bady, badx;
{
	register dy = -1, dx;

	for (; dy <= 1; dy++)
		for (dx = -1; dx <= 1; dx++)
			if ((!dy && !dx) || (dy == bady && dx == badx) ||
			    y+dy < 0 && x+dx < 0 && y+dy >= 22 && x+dx >= 79)
					/* don't do 0,0 or bad coordinates */
				continue;
			else {
				register j=y, i=x, d=grid[y+dy][x+dx];

				if (!d) continue;
				do {		/* "walk" the path, checking */
					j += dy;
					i += dx;
					if (j < 0 || i < 0 || j >= 22 ||
					    i >= 79 || !grid[j][i]) break;
				} while (--d);
				if (!d) return 1;	/* if "d" got to 0, *
							 * move was okay.   */
			}
	return 0;			/* no good moves were found */
}

/* showmoves() is nearly identical to othermove(), but it highlights possible */
/* moves instead.  "on" tells showmoves() whether to add or remove moves.     */

void showmoves(on)
register on;
{
	register dy = -1, dx;

	for (; dy <= 1; dy++) {
		if (y+dy < 0 || y+dy >= 22) continue;
		for (dx = -1; dx <= 1; dx++) {
			register j=y, i=x, d=grid[y+dy][x+dx];

			if (!d) continue;
			do {
				j += dy;
				i += dx;
				if (j < 0 || i < 0 || j >= 22
				    || i >= 79 || !grid[j][i]) break;
			} while (--d);
			if (!d) {
				register j=y, i=x, d=grid[y+dy][x+dx];

				/* The next section chooses inverse-video    *
				 * or not, and then "walks" chosen valid     *
				 * move, reprinting characters with new mode */

				if (on) standout();
				do {
					j += dy;
					i += dx;
					mvaddch(j, i, grid[j][i] + '0');
				} while (--d);
				if (on) standend();
			}
		}
	}
}

/* doputc() simply prints out a character to stdout, used by tputs() */

char doputc(c)
register char c;
{
	return(fputc(c, stdout));
}

/* topscores() processes it's argument with the high score file, makes any *
 * updates to the file, and outputs the list to the screen.  If "newscore" *
 * is 0, the score file is printed to the screen (i.e. "greed -s")         */

void topscores(newscore)
register int newscore;
{
	register fd, count = 1;
	static char termbuf[BUFSIZ];
	char *tptr = (char *) malloc(16), *boldon, *boldoff;
	struct score *toplist = (struct score *) malloc(FILESIZE);
	register struct score *ptrtmp, *eof = &toplist[MAXSCORE], *new = NULL;
	extern char *getenv(), *tgetstr();
#ifndef MSDOS
	extern struct passwd *getpwuid();
	void lockit();
#else
	char user_name[100];
#endif

	(void) signal(SIGINT, SIG_IGN);		/* Catch all signals, so high */
#ifndef MSDOS
	(void) signal(SIGQUIT, SIG_IGN);	/* score file doesn't get     */
#else
	(void) signal(SIGBREAK, SIG_IGN);	/* score file doesn't get     */
#endif
	(void) signal(SIGTERM, SIG_IGN);	/* messed up with a kill.     */
	(void) signal(SIGHUP, SIG_IGN);

		/* following open() creates the file if it doesn't exist */
		/* already, using secure mode */
#ifndef MSDOS
	if ((fd = open(SCOREFILE, O_RDWR|O_CREAT, 0600)) == -1) {
#else
	if ((fd = open(scorepath, O_RDWR|O_CREAT, S_IREAD|S_IWRITE)) == -1) {
#endif
		fprintf(stderr, "%s: %s: Cannot open.\n", cmdname,
#ifndef MSDOS
			SCOREFILE);
#else
			scorepath);
#endif
		exit(1);
	}

#ifndef MSDOS
	lockit(1);			/* lock score file */
#endif
	for (ptrtmp=toplist; ptrtmp < eof; ptrtmp++) ptrtmp->score = 0;
					/* initialize scores to 0 */
#ifndef MSDOS
	read(fd, toplist, FILESIZE);	/* read whole score file in at once */
#else
	read(fd, (char *) toplist, FILESIZE);
#endif

	if (newscore) {			/* if possible high score */
		for (ptrtmp=toplist; ptrtmp < eof; ptrtmp++)
					/* find new location for score */
			if (newscore > ptrtmp->score) break;
		if (ptrtmp < eof) {	/* if it's a new high score */
			new = ptrtmp;	/* put "new" at new location */
			ptrtmp = eof-1;	/* start at end of list */
			while (ptrtmp > new) {	/* shift list one down */
				*ptrtmp = *(ptrtmp-1);
				ptrtmp--;
			}

			new->score = newscore;	/* fill "new" with the info */
#ifndef MSDOS
			strncpy(new->user, getpwuid(getuid())->pw_name, 8);
			(void) lseek(fd, 0, 0);	/* seek back to top of file */
			write(fd, toplist, FILESIZE);	/* write it all out */
#else
			printf("Enter your name: ");
			gets(user_name);
			strncpy(new->user, user_name, 8);
			new->user[8] = '\0';
			(void) lseek(fd, 0L, 0);
			write(fd, (char *) toplist, FILESIZE);
#endif
		}
	}

	close(fd);
#ifndef MSDOS
	lockit(0);			/* unlock score file */
#endif

	if (toplist->score) puts("Rank  Score  Name     Percentage");
	else puts("No high scores.");	/* perhaps "greed -s" was run before *
					 * any greed had been played? */
#ifndef MSDOS
	if (new && tgetent(termbuf, getenv("TERM")) > 0) {
		boldon = tgetstr("so", &tptr);		/* grab off inverse */
		boldoff = tgetstr("se", &tptr);		/* video codes */
		if (!boldon || !boldoff) boldon = boldoff = 0;
						/* if only got one of the *
						 * codes, use neither     */
	}
#else
	boldon = "[1m";
	boldon = "[m";
#endif

	/* print out list to screen, highlighting new score, if any */
	for (ptrtmp=toplist; ptrtmp < eof && ptrtmp->score; ptrtmp++, count++) {
		if (ptrtmp == new && boldon)
#ifndef MSDOS
			tputs(boldon, 1, doputc);
		printf("%-5d %-6d %-8s %.2f%%\n", count, ptrtmp->score,
			ptrtmp->user, (float) ptrtmp->score / 17.38);
		if (ptrtmp == new && boldoff) tputs(boldoff, 1, doputc);
#else
			printf("%c%s", ESC, boldon);
		printf("%-5d %-6d %-8s %.2f%%", count, ptrtmp->score,
			ptrtmp->user, (float) ptrtmp->score / 17.38);
		if (ptrtmp == new && boldoff) printf("%c%s", ESC, boldoff);
		putchar('\n');
#endif
	}
}

/* lockit() creates a file with mode 0 to serve as a lock file.  The creat() *
 * call will fail if the file exists already, since it was made with mode 0. *
 * lockit() will wait approx. 15 seconds for the lock file, and then         *
 * override it (shouldn't happen, but might).  "on" says whether to turn     *
 * locking on or not.                                                        */

#ifndef MSDOS
void lockit(on)
register on;
{
	register fd, x = 1;

	if (on) {
		while ((fd = creat(LOCKPATH, 0)) == -1) {
			printf("Waiting for scorefile access... %d/15\n", x);
			if (x++ >= 15) {
				puts("Overriding stale lock...");
				if (unlink(LOCKPATH) == -1) {
					fprintf(stderr,
						"%s: %s: Can't unlink lock.\n",
						cmdname, LOCKPATH);
					exit(1);
				}
			}
			sleep(1);
		}
		close(fd);
	} else unlink(LOCKPATH);
}
#endif

#define msg(row, msg) mvwaddstr(helpwin, row, 2, msg);

/* help() simply creates a new window over stdscr, and writes the help info *
 * inside it.  Uses macro msg() to save space.                              */

void help() {
	if (!helpwin) {
	  helpwin = newwin(18, 65, 1, 7);
#ifndef MSDOS
	  box(helpwin, '|', '-');	/* print box around info */
					/* put '+' at corners, looks better */
	  (void) waddch(helpwin, '+'); mvwaddch(helpwin, 0, 64, '+');
	  mvwaddch(helpwin, 17, 0, '+'); mvwaddch(helpwin, 17, 64, '+');
#else
	  box(helpwin, (char) 0xba, (char) 0xcd);	/* special DOS chars */
	  mvwaddch(helpwin, 0, 0, (char) 0xc9);
	  mvwaddch(helpwin, 0, 64, (char) 0xbb);
	  mvwaddch(helpwin, 17, 0, (char) 0xc8);
	  mvwaddch(helpwin, 17, 64, (char) 0xbc);
#endif

	  mvwprintw(helpwin, 1, 2,
		"Welcome to %s, by Matthew Day (mday@iconsys.uu.net).",version);
#ifdef MSDOS
	  msg(2,"           (MSDOS adaptation by Fred C. Smith)");
#endif
	  msg(3," The object of Greed is to erase as much of the screen as");
	  msg(4," possible by moving around in a grid of numbers.  To move");
	  msg(5," your cursor, simply use the 'hjklyubn' keys or your number");
	  mvwprintw(helpwin, 6, 2,
		" keypad.  Your location is signified by the '%c' symbol.", ME);
	  msg(7," When you move in a direction, you erase N number of grid");
	  msg(8," squares in that direction, N being the first number in that");
	  msg(9," direction.  Your score reflects the total number of squares");
	  msg(10," eaten.  Greed will not let you make a move that would have");
	  msg(11," placed you off the grid or over a previously eaten square");
	  msg(12," unless no valid moves exist, in which case your game ends.");
	  msg(13," Other Greed commands are 'Ctrl-L' to redraw the screen,");
	  msg(14," 'p' to toggle the highlighting of the possible moves, and");
	  msg(15," 'q' to quit.  Command line options to Greed are '-s' to");
	  msg(16," output the high score file.");

	  (void) wmove(helpwin, 17, 64);
	  wrefresh(helpwin);
	} else {
	  touchwin(helpwin);
	  wrefresh(helpwin);
	}
	(void) wgetch(helpwin);
	touchwin(stdscr);
	refresh();
}