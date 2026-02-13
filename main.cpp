#include <ctype.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <termios.h>
#include <cstdlib>


struct termios og_termios;
void die(const char *s){
    write(STDIN_FILENO,"\x1b[2J",4);
    write(STDIN_FILENO,"\x1B[?25h",7);
    write(STDIN_FILENO,"\x1B[H",3);

    perror(s);
    exit(1);
}
void disableRawMode(){
    if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&og_termios)){
        die("tcsetattr");
    }
}
void enableRawMode(){
    if(tcgetattr(STDIN_FILENO,&og_termios) == -1) die("tcgetattr");//get attributes
    atexit(disableRawMode);//either returning main or calling exit()

    struct termios raw = og_termios;

    //flags for raw mode
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);//c_lflag is the field for local flags
    raw.c_cc[VMIN] = 0;//control chars
    raw.c_cc[VTIME] = 1;

    if(tcsetattr(STDIN_FILENO ,TCSAFLUSH/*specifies when to apply change*/, &raw) == -1) die("tcsetattr");//TCSAFLUSH discards unread input before applying the changes.
}

char readKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO,&c,1)) !=1){
        if(nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}
void processKey(){
    char c = readKey();
    switch(c){
        case 'q':
            write(STDIN_FILENO,"\x1b[2J",4);
            write(STDIN_FILENO,"\x1B[?25h",7);
            write(STDIN_FILENO,"\x1B[H",3);
            exit(0);
            break;
    }
}
void editorRefreshScreen(){
    write(STDIN_FILENO,"\x1b[2J",4);// writes 4 bytes to the terminal \x1b is 1 byte [2J is the other 3 (J is used to erase in display), these are escape(\x1b) sequences.
    write(STDIN_FILENO,"\x1B[?25l",7);

}
int main(){
    editorRefreshScreen();
    enableRawMode();
    while (1){//read 1 byte into c and do it till no bytes r left
        processKey();
    }
    return 0;
}