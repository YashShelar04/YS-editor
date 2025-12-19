#include <stdlib.h>
#include<errno.h>
#include <termios.h>
#include <unistd.h>
#include<ctype.h>
#include<stdio.h>

#define CTRL_KEY(k) ((k)&0x1f)

struct editorConfig{
	struct termios orig_termios;
}


struct editorConfig E;

void die(const char *s){
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}

void disable_raw_mode() {
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios)==-1) die("tcsetattr");
}

void enable_raw_mode() {
  if(tcgetattr(STDIN_FILENO, &E.orig_termios)==-1) die("tcgetattr");
  atexit(disable_raw_mode);
  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(IXON|ICRNL|ISTRIP|INPCK|BRKINT);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO|ICANON|IEXTEN|ISIG);
  raw.c_cc[VMIN] =0;
  raw.c_cc[VTIME] =1;

  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)==-1) die("tcsetattr");
}

void editorDrawRows(){
	int i;
	for(i=0;i<24;i++){
		write(STDOUT_FILENO,"~\r\n",3);
	}
}

void editorScreenRefresh(){
	write(STDOUT_FILENO,"\x1b[1J",4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();

	write(STDOUT_FILENO,"\x1b[H",3);
}

char editorKeyRead(){
	int nread;
	char c;
	while((nread=read(STDIN_FILENO,&c,1))!=1){
		if(nread==-1 && errno!=EAGAIN) die("read");

	}

	return c;
}


void editorKeyPressProcessor(){
	char c = editorKeyRead();

	switch(c){
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}


int main(){
	enable_raw_mode();

	while(1){
		editorScreenRefresh();
		editorKeyPressProcessor();
	}
	return 0;
}
