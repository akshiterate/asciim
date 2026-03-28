#include <ctype.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <termios.h>
#include <cstdlib>
#include <ctime>
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
    std::srand(static_cast<unsigned int>(std::time(nullptr))); 
    char grid[100][200];
    char stars[100][200];
    char entities[5] = {'.','*','o','\'','+'};
    for(int i=0;i<rows*cols*0.1;i++){
        int a = rand()%rows;
        int b = rand()%cols;
        stars[a][b]=entities[rand()%5];
    }
    while (1){
        processKey();
        editorRefreshScreen();
        std::cout<<"\033[90m";
        //write(STDOUT_FILENO,"\033[90m",4);
        for(int i=0;i<rows;i++){
            for(int j=0;j<cols;j++){
                if(stars[i][j] != '\0'){
                    std::cout<<"\033[1m"<<stars[i][j]<<"\033[0m";
                }
                else std::cout<<" ";
            }
            std::cout<<"\r\n";
        }


        std::cout.flush();
        usleep(500000);
    }
    return 0;
}