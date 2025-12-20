#include <stdlib.h>
#include<errno.h>
#include <termios.h>
#include <unistd.h>
#include<ctype.h>
#include<stdio.h>
#include<sys/ioctl.h>
#define CTRL_KEY(k) ((k)&0x1f)

struct editorConfig{
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

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
	for(i=0;i<E.screenrows;i++){
		write(STDOUT_FILENO,"~\r\n",3);
	}
}



void initEditor(){
	if(getScreenSize(&E.screenrows,&E.screencols)==-1) die("getScreenSize");
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


int getCursorPosition(int *rows, int *cols) {
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  printf("\r\n");
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
  }
  editorKeyRead();
  return -1;
}


int getScreenSize(int *rows, int *cols){
        struct winsize ws;

        if(1||ioctl(STDOUT_FILENO,TIOCGWINSZ,&ws)==-1 ||ws.ws_col==0){
                if(write(STDOUT_FILENO,"\x1b[999C\x1b[999B",12)!=12) return -1;
                return getCursorPosition(rows,cols);
        }else{
                *rows= ws.ws_row;
                *cols = ws.ws_col;
                return 0;
        }
}



int main(){
	enable_raw_mode();
	initEditor();
	while(1){
		editorScreenRefresh();
		editorKeyPressProcessor();
	}
	return 0;
}
