#include <u.h>
#include <libc.h>
#include <bio.h>
#include <thread.h>

/* thread example: /sys/src/libthread/example.c */

void
usage(void)
{
	fprint(2, "usage: %s [ -b chatdir ] [ -s initial channel ] [ -c channel file ]\n", argv0);
	exits("usage");
}

#define LOG "chatnofs"

char *chatfile = "lobby";
char *basedir  = "/mnt/chat/";
char *motd = nil;
char *chanfile = nil;

int   fd = 0; /* for writing */
char *user;

int rprocid;
int readfd;

void
readproc(void*)
{
	int n;
	char readbuf[4096];
	
	readfd = open(chatfile, OREAD);
	if (readfd < 0) {
		syslog(0, LOG, "error opening %s: %r\n", chatfile);
		return;
	}
	seek(readfd, -200, 2);
	while (readfd > 0){
		/* blocking instead of looping would be nicer */
		/* channel would be even nicer */
		n = read(readfd, readbuf, 4096);
		if (n <= 0){
			sleep(100);
			continue;
		}
		write(1, readbuf, n);
	}
}

enum {
	Cjoin,
	Cquit,
	Cstatus,
	Cend,
};

enum {
	Soffline,
	Sonline,
	Saway,
	Send,
};

void
printfile(char *f)
{
	int fd, n;
	char buf[1024];

	if (!f) return;

	fd = open(f, OREAD);
	if (fd < 0) {
		syslog(0, LOG, "error opening file %s: %r\n", f);
		print("error: %r\n");
		return;
	}
	while ((n = read(fd, buf, 1024)) > 0)
		write(1, buf, n);

	close(fd);
}

void
leave(void)
{
	seek(fd, 0, 2);
	fprint(fd, "%s left the channel\n", user);
}

int
joinchat(char *chat)
{
	char *s = 0;
	int n;
	
	if (s)
		free(s);
	n = (strlen(basedir)+strlen(chat)+1)*sizeof(char);
	s = malloc(n);
	if (s == nil)
		return -1;
	
	snprint(s, n, "%s%s", basedir, chat);
	chatfile = s;
	
	if (readfd)
		close(readfd);
	readfd = open(chatfile, OREAD);
	if (readfd < 0) {
		syslog(0, LOG, "error opening %s: %r\n", chatfile);
		return 1;
	}
	if (rprocid)
		threadint(rprocid);
	if (!rprocid)
		rprocid = proccreate(readproc, nil, 2048);
	print("joined channel %s\n", chat);
	
	leave();
	if (fd > 0)
		close(fd);
	fd = open(chatfile, OWRITE);
	if (fd <= 0){
		syslog(0, LOG, "cannot open file: %r\n");
		print("error: %r\n");
		sysfatal("error: %r\n");
		return 1;
	}
	
	seek(fd, 0, 2);
	fprint(fd, "%s joined the channel\n", user);
	
	return 0;
}

int
interpret(char *buf)
{
	char *arg[3];
	int f;
	
	f = getfields(buf, arg, 3, 1, " \t");
	
	switch(buf[1]){
	case 'j':
		if (f < 2){
			werrstr("usage: j channel\n");
			return 1;
		}
		if (joinchat(arg[1])){
			werrstr("cannot join channel: %r\n");
			return 1;
		}
		break;
	case 'q':
		leave();
		threadexitsall(nil);
	case 'm':
		printfile(motd);
		break;
	case 'c':
		printfile(chanfile);
		break;
	default:
		werrstr("bad command");
		return 1;
	}
	return 0;
}

int
initchatdir(void)
{
	long n;
	char *s;
	
	n = strlen(basedir);
	
	if (basedir[n-1] != '/'){
		s = malloc((n+2)*sizeof(char));
		if (s == nil){
			syslog(0, LOG, "cannot allocate memory: %r\n");
			print("cannot allocate memory: %r\n");
			sysfatal("cannot allocate memory: %r");
			return -1;
		}
		snprint(s, n+2, "%s/", basedir);
		basedir = s;
	}
	
	if (joinchat(chatfile)){
		syslog(0, LOG, "error: %r\n");
		print("error: %r\n");
		sysfatal("error: %r\n");
		return -1;
	}
	return 0;
}

void
threadmain(int argc, char **argv)
{
	long n;
	char *buf;
	Biobuf *b;
	
	ARGBEGIN{
	case 'b':
		basedir = EARGF(usage());
		break;
	case 's':
		chatfile = EARGF(usage());
		break;
	case 'c':
		chanfile = EARGF(usage());
		break;
	case 'm':
		motd = EARGF(usage());
		break;
	}ARGEND;
	
	user = getenv("user");
	if (user == nil)
		user = "nope";
	
	if (motd)
		printfile(motd);
	
	if (initchatdir()){
		print("error: %r\n");
		syslog(0, LOG, "error: %r\n");
		return;
	}
	
	b = Bfdopen(0, OREAD);
	if (b == nil){
		syslog(0, LOG, "cannot open stdin: %r\n");
		print("error: %r\n");
		return;
	}
	
	while ((buf = Brdline(b, '\n')) != 0){
		n = Blinelen(b);
		if (n == 1)
			continue;
		buf[n-1] = 0;
		if (buf[0] == ''){
			if (interpret(buf))
				print("error: %r\n");
			continue;
		}
		seek(fd, 0, 2);
		fprint(fd, "%s → %s\n", user, buf);
	}
	
	threadexitsall(nil);
}
