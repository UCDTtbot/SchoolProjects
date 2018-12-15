#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <limits.h>


//GLOBALS
const int BUFFER_SIZE = 128;
const int HISTORY_SIZE = 10;
const int ARG_LENGTH = 20;
const int NUM_ARGS = 50;

//DELETE THIS BEFORE SUBMITTING, INTERNET CODE TO HELP CONVERT INT TO STRING BECAUSE STD::TO_STRING WAS NOT WORKING
std::string to_string(int number){
    std::string number_string = "";
    char ones_char;
    int ones = 0;
    while(true){
        ones = number % 10;
        switch(ones){
            case 0: ones_char = '0'; break;
            case 1: ones_char = '1'; break;
            case 2: ones_char = '2'; break;
            case 3: ones_char = '3'; break;
            case 4: ones_char = '4'; break;
            case 5: ones_char = '5'; break;
            case 6: ones_char = '6'; break;
            case 7: ones_char = '7'; break;
            case 8: ones_char = '8'; break;
            case 9: ones_char = '9'; break;
            default : ;
        }
        number -= ones;
        number_string = ones_char + number_string;
        if(number == 0){
            break;
        }
        number = number/10;
    }
    return number_string;
}

class history								//CLASS FOR HISTORY TO MAKE WORKING WITH EASIER
{
	std::string hist[HISTORY_SIZE]; 
    int curHisPos;	//CURRENT POSITION OF HISTORY "POINTER"
	int hisSize;	//CURRENT HISTORY SIZE
    void init(){curHisPos = 0; hisSize = 0;};
    
    public:
    	history(){init();};
    	int getPos(){return curHisPos;}
    	void setPos(int in){curHisPos = in;	};
    	
		int getSize(){return hisSize;};
    	void incSize(){hisSize++;};
    	
    	std::string getHist(int spot){return hist[spot];};
    	
    	void insert(std::string in){hist[curHisPos] = in; curHisPos++; if(hisSize != HISTORY_SIZE) hisSize++;};
    	
    	bool isFull(){if(hisSize == HISTORY_SIZE) return true; else return false;};
    	
    	void printHistory()
    	{
    		for(int x = 0; x < getSize(); x++)
    		{
    			std::string num = to_string(x);
    			std::string toPrint = getHist(x);
    			write(STDOUT_FILENO, "\n", 1);
    			write(STDOUT_FILENO, num.c_str(), 1); write(STDOUT_FILENO, " ", 1);
    			write(STDOUT_FILENO, toPrint.c_str(), toPrint.length());
			}
			write(STDOUT_FILENO, "\n", 1);
		}
	
};

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

void printCurDir()										//PRINTS CURRENT WORKING DIRECTORY
{
	std::string cwd;
    char cwdBuffer[PATH_MAX];
	cwd = getcwd(cwdBuffer, PATH_MAX);
    write(STDOUT_FILENO, cwd.c_str(), cwd.length());
    write(STDOUT_FILENO, "\n", 1);

}

void copyToHistory(history &hist, std::string buf)		//COPIES BUFFER TO HISTORY
{
	if(hist.getPos() == HISTORY_SIZE - 1)
	{
		hist.setPos(0);
	}
	
	hist.insert(buf);
}

void getStats(std::string directory)
{
	struct stat stats;
	
	stat(directory.c_str(), &stats);
	if(S_ISDIR(stats.st_mode)){write(STDOUT_FILENO, "d", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IRUSR){write(STDOUT_FILENO, "r", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IWUSR){write(STDOUT_FILENO, "w", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IXUSR){write(STDOUT_FILENO, "x", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IRGRP){write(STDOUT_FILENO, "r", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IWGRP){write(STDOUT_FILENO, "w", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IXGRP){write(STDOUT_FILENO, "x", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IROTH){write(STDOUT_FILENO, "r", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IWOTH){write(STDOUT_FILENO, "w", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	if(stats.st_mode & S_IXOTH){write(STDOUT_FILENO, "x", 1);} else{write(STDOUT_FILENO, "-", 1);	}
	write(STDOUT_FILENO, " ", 1);
}

void printLS()
{
	pid_t pid;
	int status;
	std::string cwd;
    char cwdBuffer[PATH_MAX];
	cwd = getcwd(cwdBuffer, PATH_MAX);
	struct dirent *files;
	DIR *directory;
	
	pid = fork();
	
	if(pid < 0){
		write(STDOUT_FILENO, "\nFork Failed\n", 13);
	}
	else if(pid == 0){									//CHILD
		directory = opendir(cwd.c_str());
		while((files = readdir(directory))){
			std::string dir_file_append = cwd;
			dir_file_append.append("/");
			dir_file_append.append(files->d_name);
			getStats(dir_file_append);//stat(dir_file_append.c_str(), &stats);
		//	write(STDOUT_FILENO, stats.st_mode, strlen(stats.st_mode) );
			write(STDOUT_FILENO, files->d_name, strlen(files->d_name));
			write(STDOUT_FILENO, "\n", 1);
		}
		closedir(directory);
		exit(0);
	}
	else{												//PARENT
		wait(NULL);
	}
}

void checkArgs(std::string args[], int numArgs, int &status, history &hisList)
{
	if(args[0].compare("exit") == 0 || args[0].compare("Exit") == 0)			//EXIT
		status = 1;
	else if(args[0].compare("cd") == 0)											//CD
		write(STDOUT_FILENO, "Got CD\n", 8);//todo
	else if(args[0].compare("ls") == 0)											//LS
		printLS();//write(STDOUT_FILENO, "Got LS\n", 8);//todo
	else if(args[0].compare("pwd") == 0)										//PWD
		printCurDir();//write(STDOUT_FILENO, "Got PWD\n", 9);//todo
	else if(args[0].compare("history") == 0)									//HISTORY
		hisList.printHistory();
	else{
		write(STDOUT_FILENO, "Failed to execute ", 19);							//FAILURE
		write(STDOUT_FILENO, args[0].c_str(), args[0].length());
		write(STDOUT_FILENO, "\n", 1);
	}
}

//CD, LS, PWD, HISTORY, EXIT . . . 
void parse(std::string buf, int bufSize, int &status, history &hisList)
{
	std::string token;
	std::string args[NUM_ARGS];
	int tokPos = 0;					//TOKEN POSITION
	int curArg = 0;					//CURRENT ARGUMENT
	std::string validChars("abcdefghijklmnopqrstuvwxyz<>|&123456789");			//VALID CHARACTERS FOR PARSING
	
	while(buf.length() != 0){

		tokPos = buf.find_first_not_of(validChars);	//FIND FIRST NONVALID CHAR
		
		if(tokPos == 0)				//IF WERE ON LAST TOKEN
		{
			token = buf.substr(0, buf.length());
			buf.clear();
		}
		else						//ERASE PART OF BUFFER AND MAKE TOKEN
		{
			token = buf.substr(0, tokPos);
			buf.erase(0, token.length());
		}

		args[curArg] = token;		//ARGUMENTS ARRAY HOLDING TOKENS
		token.clear();
		curArg++;
	}

	write(STDOUT_FILENO, "\n", 1);
	
	checkArgs(args, curArg, status, hisList);
}

void clear_screen()					//CLEARS OFF SCREEN
{
	for(int x = 0; x < BUFFER_SIZE; x++)
	{
		write(STDOUT_FILENO, "\b \b", 3);
	}
}

void printCWD()
{
    std::string cwd;
    char cwdBuffer[PATH_MAX];
   
    cwd = getcwd(cwdBuffer, PATH_MAX);
    cwd = getcwd(cwdBuffer, PATH_MAX);
    if (cwd.length() > 16){
        std::string mystring(cwd);   
        unsigned SlashPos = mystring.find_last_of('/');
        mystring.erase(0, SlashPos);
        mystring.insert(0, "/...");
        const char *cwdsmall = mystring.c_str();
        write(STDOUT_FILENO, cwdsmall, strlen(cwdsmall));
        write(STDOUT_FILENO, "> ", 2);
    }
    else{
    write(STDOUT_FILENO, cwd.c_str(), cwd.length());
    write(STDOUT_FILENO, "> ", 2);
    }
}

int main(int argc, char *argv[]){
    struct termios SavedTermAttributes;
    char RXChar;
    std::string buf;
    int bufPos = 0;
    history hisList;
    int arrows = 0;
    int status = 0; // 0 is good, 1 is exit
    int charsWritten = 0;
    
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    

	printCWD();
    
    while(1){
		if(status != 1)
			read(STDIN_FILENO, &RXChar, 1);
	
       	if(0x04 == RXChar){ // C-d		//CTRL-D
			break;
        }
        if(status == 1){
			break;
		}	
        else if(0x7F == RXChar)															//BACKSPACE
        {
        	if(charsWritten > 0){
				write(STDOUT_FILENO, "\b \b", 3);
				buf.erase(buf.end()-1);
				charsWritten--;
			}
		}
		else if(0x0A == RXChar)															//THIS IS ENTER
		{
   			copyToHistory(hisList, buf);
   			parse(buf, buf.length(), status, hisList);
			buf.clear();
		   	bufPos = 0;
			arrows = 0;
			if(status == 1){
				break;
			}
		    printCWD();
		}
		else if(0x1B == RXChar){														//UP ARROW
			read(STDIN_FILENO, &RXChar, 1);
			if(0x5B == RXChar){
				read(STDIN_FILENO, &RXChar, 1);
				if(0x41 == RXChar){														
//					if(arrows == hisSize){
//						write(STDOUT_FILENO, "\a", 1);
//					}
//					else{
//						clear_screen();
//					    cwd = getcwd(cwdBuffer, PATH_MAX);
//					    write(STDOUT_FILENO, cwd.c_str(), cwd.length());
//					    write(STDOUT_FILENO, "> ", 2);
//						write(STDOUT_FILENO, history[curHisPos-1].c_str(), history[curHisPos-1].length());
//						curHisPos--;
//						if(curHisPos == 0 && hisSize == HISTORY_SIZE)
//							curHisPos = 9;
//						arrows++;
//					}
				}
				if(0x42 == RXChar){														//DOWN ARROW	
//					if(arrows == 0){
//						clear_screen();
//						cwd = getcwd(cwdBuffer, PATH_MAX);
//					    write(STDOUT_FILENO, cwd.c_str(), cwd.length());
//					    write(STDOUT_FILENO, "> ", 2);
//						write(STDOUT_FILENO, "\a", 1);
//					}
//					else{
//						clear_screen();
//						cwd = getcwd(cwdBuffer, PATH_MAX);
//					    write(STDOUT_FILENO, cwd.c_str(), cwd.length());
//					    write(STDOUT_FILENO, "> ", 2);
//						write(STDOUT_FILENO, history[curHisPos].c_str(), history[curHisPos].length());
//						curHisPos++;
//						if(curHisPos == 9 && hisSize == HISTORY_SIZE)
//							curHisPos = 0;
//						arrows--;
//					}
					
				}	
			}	

			}
		else{
			write(STDOUT_FILENO, &RXChar, 1);
			buf.push_back(RXChar);
			bufPos++;
			charsWritten++;
		}
		
        
	}
    
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
	
	
}
