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
#include <algorithm>


//GLOBALS
const int BUFFER_SIZE = 128;
const int HISTORY_SIZE = 10;
const int ARG_LENGTH = 20;
const int NUM_ARGS = 50;

std::string to_string(int number){	//Code adapted from the internet. Don't kill me MOSS I linked to it in the readme
    std::string number_string = "";
    char num;
    int digit = 0;
    while(true){
        digit = number % 10;
        switch(digit){
            case 0: num = '0'; break;
            case 1: num = '1'; break;
            case 2: num = '2'; break;
            case 3: num = '3'; break;
            case 4: num = '4'; break;
            case 5: num = '5'; break;
            case 6: num = '6'; break;
            case 7: num = '7'; break;
            case 8: num = '8'; break;
            case 9: num = '9'; break;
            default : ;
        }
        number -= digit;
        number_string = num + number_string;
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
    	
    	void insert(std::string in){hist[curHisPos] = in; if(curHisPos == 9) curHisPos = 0; else curHisPos++; if(hisSize != HISTORY_SIZE) hisSize++;};
    	
    	bool isFull(){if(hisSize == HISTORY_SIZE) return true; else return false;};
    	
    	void printHistory()
    	{
			for(int x = 0; x < getSize(); x++)
    		{
	    		std::string num = to_string(x);
	    		std::string toPrint = getHist(x);
	    		write(STDOUT_FILENO, num.c_str(), 1); write(STDOUT_FILENO, " ", 1);
	    		write(STDOUT_FILENO, toPrint.c_str(), toPrint.length());
	    		write(STDOUT_FILENO, "\n", 1);
			}
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

void getStats(std::string directory)	//Adapted from codewiki to help get stats. I could not figure this out easily. See README
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

void printLS()			//PRINTS LS TO SCREEN
{
	std::string cwd;		//Got info from a website to help, check readme
    char cwdBuffer[PATH_MAX];
	cwd = getcwd(cwdBuffer, PATH_MAX);
	struct dirent *files;
	DIR *directory;

	directory = opendir(cwd.c_str());
	while((files = readdir(directory))){
		std::string dir_file_append = cwd;
		dir_file_append.append("/");	//Appends directory to files needed
		dir_file_append.append(files->d_name);
		getStats(dir_file_append);		//Gets stats
		write(STDOUT_FILENO, files->d_name, strlen(files->d_name));
		write(STDOUT_FILENO, "\n", 1);
	}
	closedir(directory);	//Make sure to close dir when done
}

void printCD(const std::string& DestinationStr)		//changes directory
{	
	std::string cwd;
    char cwdBuffer[PATH_MAX];
	cwd = getcwd(cwdBuffer, PATH_MAX);
	
    if(DestinationStr == " .."){
         std::string mystring2(cwd);
         unsigned SlashPos2 = mystring2.find_last_of('/');
         mystring2.erase(SlashPos2+1);
         const char *destin2 = mystring2.c_str();
         if (chdir(destin2) == -1){
              write(STDOUT_FILENO, "Error Changing Directory\n", 25);
         }
    }
    else if(DestinationStr.empty()){
         char *HomeDir = getenv("HOME");
         chdir(HomeDir);
    }
    else if((DestinationStr == " ") || (DestinationStr == "  ") || (DestinationStr == "   ")){
         char *HomeDir = getenv("HOME");
         chdir(HomeDir);
    }
	else{
         cwd.append("/");
         cwd.append(DestinationStr);
         const char *destin = cwd.c_str();
         if(chdir(destin) == -1){
               write(STDOUT_FILENO, "Error Changing Directory\n", 25);
         }
    }   
}

void execute(std::string args[], int numArgs)	//Executes a program
{
	char *charArgs[numArgs+1];
	int argSize = 0;
	int x = 0;
	
	while(x < numArgs)	//Checks the string for any < > | &, these must be taken out to pass to execvp
	{
		char *toPut = const_cast<char*>(args[x].c_str());
		if(args[x].compare("&") != 0 && args[x].compare("|") != 0 && args[x].compare(">") != 0 && args[x].compare("<") != 0){
			charArgs[argSize] = toPut;
			argSize++;
		}
		charArgs[argSize] = NULL;
		x++;
	}
	
	execvp(charArgs[0], charArgs); //It works with make atleast
	
	exit(1); // Exit 1 if execvp fails
	
}

void tryFork(std::string args[], int numArgs, int sChars[], int numSChar, history &hisList, std::string cmd)
{						//INITIAL FORK FOR COMMANDS LS, PWD, HISTORY, EXEC
	pid_t pid;
	int fd;
	int status;
	int pip[2];
	
	pid = fork();
	pipe(pip);

	if(pid < 0){
		write(STDOUT_FILENO, "\nFork Failed\n", 13); 	//iF FORK FAILS
	}
	else if(pid == 0){									//CHILD
		if(numSChar == 1){
			if(args[1].compare("<") ==0){
				fd = open(args[2].c_str(), O_RDONLY);
				dup2(fd, STDIN_FILENO);
			}
			else if(args[1].compare(">") == 0){
				fd = open(args[2].c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);//Someone from piazza helped me figure this out, for the love of god MOSS don't flag me
				dup2(fd, STDOUT_FILENO);															
			}
			else{
				for(int x = 0; x < numArgs; x++){		//ATTEMPTING TO CREATE A PIPE, WILL NOT BE FULLY FINISHED
					if(args[x].compare("|") == 0){	//***
						close(pip[0]);				//Not exactly right. Currently just closing pipe around child
					}							//To get it fully working, we'll want to close this end to the left program, or args[x - 1]
				}							//The other end needs to pipe to the thing on the right, which would be closed around that program
										//I'm pretty sure this requires a second fork, but at this time I do not have enough time to implement a full piping system
			}
		}
		else if(numSChar > 1){			//CHECK FOR MULTIPLE SPECIAL CHARACTERS
			for(int x = 0; x < numArgs; x++){
				if(args[x].compare("<") == 0){
					fd = open(args[x+1].c_str(), O_RDONLY);
					dup2(fd, STDIN_FILENO);
				}
				else if(args[x].compare(">") == 0){
					fd = open(args[x+1].c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
					dup2(fd, STDOUT_FILENO);
				}
		}
			
		}
		if(cmd.compare("ls") == 0)									//LS
			{printLS();}
		else if(cmd.compare("history") == 0)						//HISTORY
			{hisList.printHistory();}
		else if(cmd.compare("pwd") == 0)							//PWD
			{printCurDir();}	
		else if(cmd.compare("exec") == 0)							//EXECUTE COMMAND
			{execute(args, numArgs);}
			
		
		exit(0);
	}
	else{															//PARENT
		
		bool ampWait = true;
		close(pip[1]);
		for(int x = 0; x < numArgs; x++)
		{
			if(args[x].compare("&") == 0)
				ampWait = false;
		}
		if(ampWait)
			wait(NULL);
	}	
}

void checkSpecial(std::string args[], int numArgs, int sChars[], int &numSChar)
{
	for(int x = 0; x < numArgs; x++)			//Well all these ifs are redundant. Oops
	{											//Checks for special characters < > | &
		if(args[x].compare("<") == 0){
			sChars[numSChar] = x;
			numSChar++;
		}		
		else if(args[x].compare(">") == 0){
			sChars[numSChar] = x;
			numSChar++;		
		}		
		else if(args[x].compare("|") == 0){
			sChars[numSChar] = x;
			numSChar++;			
		}
		else if(args[x].compare("&") ==0){
			sChars[numSChar] = x;
			numSChar++;			
		}
	}
}

void checkArgs(std::string args[], int numArgs, int &status, history &hisList)	//Check the parse, check for specials, and fork or do command.
{
	int sChars[numArgs];		//Special Chars handles < > | & and gets their index
	int numSChar = 0;	
	std::string cmd;
	
	if(args[0].compare("exit") == 0 || args[0].compare("Exit") == 0)			//EXIT
		status = 1;
	else if(args[0].compare("cd") == 0)											//CD
		printCD(args[1]);
	else if(args[0].compare("ls") == 0)											//LS
		{cmd = "ls"; checkSpecial(args, numArgs, sChars, numSChar);	tryFork(args, numArgs, sChars, numSChar, hisList, cmd);}
	else if(args[0].compare("history") == 0)									//HISTORY
		{cmd = "history"; checkSpecial(args, numArgs, sChars, numSChar); tryFork(args, numArgs, sChars, numSChar, hisList, cmd);}
	else if(args[0].compare("pwd") == 0)										//PWD
		{cmd = "pwd"; checkSpecial(args, numArgs, sChars, numSChar); tryFork(args, numArgs, sChars, numSChar, hisList, cmd);}	
	else if(args[0].length() != 0)												//EXECUTE COMMAND
		{cmd = "exec"; checkSpecial(args, numArgs, sChars, numSChar); tryFork(args, numArgs, sChars, numSChar, hisList, cmd);}																	
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
	std::string validChars("abcdefghijklmnopqrstuvwxyz<>|&123456789.");			//VALID CHARACTERS FOR PARSING
	
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
			buf.erase(0, token.length()+1);
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

void printCWD()	//Prints the current working directory
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
    int arrows = 0;	//used for keeping track of arrow presses
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
        else if(0x7F == RXChar)									//BACKSPACE
        {
        	if(charsWritten > 0){
				write(STDOUT_FILENO, "\b \b", 3);
				buf.erase(buf.end()-1);
				charsWritten--;
			}
		}
		else if(0x0A == RXChar)									//THIS IS ENTER
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
		else if(0x1B == RXChar){								//UP ARROW
			read(STDIN_FILENO, &RXChar, 1);
			if(0x5B == RXChar){
				read(STDIN_FILENO, &RXChar, 1);
				int bottom = hisList.getPos();
				if(0x41 == RXChar){														
					if(arrows == hisList.getSize()){
						write(STDOUT_FILENO, "\a", 1);
					}
					else{
						clear_screen();
		   				printCWD();
		   				int hisPos = hisList.getPos(); 
						if(hisPos == 0) hisPos = hisPos;
						else hisPos--;
						hisList.setPos(hisPos);
						std::string toPrint = hisList.getHist(hisPos);
						write(STDOUT_FILENO, toPrint.c_str(), toPrint.length());
						buf = toPrint;
						arrows++;
					}
				}
				if(0x42 == RXChar){														//DOWN ARROW	
					if(arrows == 0){
						clear_screen();
						printCWD();
						write(STDOUT_FILENO, "\a", 1);
					}
					else{
						clear_screen();
						printCWD();
						int hisPos = hisList.getPos();
						if(hisPos == bottom) hisPos = hisPos;
						else hisPos++;
						hisList.setPos(hisPos);
						std::string toPrint = hisList.getHist(hisPos);
						write(STDOUT_FILENO, toPrint.c_str(), toPrint.length());
						buf = toPrint;
						arrows--;
					}
					
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
