#include <ctype.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <termios.h>
#include <cstdlib>
#include <sys/ioctl.h>
#include <iostream>


struct termios og_termios;
void die(const char *s){
    write(STDOUT_FILENO,"\x1b[2J",4);//clear screen
    write(STDOUT_FILENO,"\x1B[?25h",7);//remove cursor
    write(STDOUT_FILENO,"\x1B[H",3);//reset to top

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
    if (read(STDIN_FILENO,&c,1)==1) return c;
    else return '\0';
}
void processKey(){
    char c = readKey();
    switch(c){
        case 'q':
            write(STDOUT_FILENO,"\x1b[2J",4);
            write(STDOUT_FILENO,"\x1B[?25h",7);
            write(STDOUT_FILENO,"\x1B[H",3);
            exit(0);
            break;
    }
}
void editorRefreshScreen(){
    write(STDOUT_FILENO,"\x1b[2J",4);// writes 4 bytes to the terminal \x1b is 1 byte [2J is the other 3 (J is used to erase in display), these are escape(\x1b) sequences.
    write(STDOUT_FILENO,"\x1B[?25l",7);
    write(STDOUT_FILENO, "\x1B[H", 3);

}


int main(){
    enableRawMode();

    struct winsize w;
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
    int rows = w.ws_row,cols =w.ws_col;
    char grid[100][200];
    for(int i=0;i<rows;i++){
        for(int j=0;j<cols;j++){
            grid[i][j] = 'a'+(rand()%26);
        }
    }
    while (1){//read 1 byte into c and do it till no bytes r left
        processKey();
        editorRefreshScreen();
        for(int i=0;i<rows;i++){
            for(int j=0;j<cols;j++){
                std::cout<<grid[i][j];
            }
            std::cout<<"\r\n";
        }


        std::cout.flush();
        usleep(500000);
    }
    return 0;
}