/*************************************************
*	Dear stranger welcome to source code
*	of jusep Power Shell 1.0
*
*	author : Jurica Šeparović
*	datum : 20.1.2016
*	Pogramiranje za UNIX -> seminarski rad
************************************************/
/*my utility functions
* writing message to shell and
* checking special signs
*/
#include "utilityFunctions.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>

#define BUFFSIZE 50
#define BUFFSIZE_CWD 128
#define EXIT_FLAG -5

//reading command line and storing
char* ReadLine();
char** CmdSplit(char*,int*);

// shell functionality
int ExecuteMyCommand(char**,int);
int ExecuteOtherCommand(char**);
int ExecuteBackgroundCmd(char**,int);
int ChangeDir(char** , int);
int KillProcess(char**,int);
int RedirectOut(char**,int,int);
int RedirectIn(char**,int,int);
int PipeIt(char**,int,int);



int main(int argc,char *argv[])
{
	char** myArguments = NULL;		//entered arguments goes in array here
	int argCount = 0;               // argument counter
	char *userInput = NULL;		    //user input line
	int place = 0;                  // place where is special sign > < | or &
	int status = 1;

	PrintWelcomeMessage();
	do
	{
		PrintShellMessage();
		userInput = ReadLine();
		if(userInput != NULL)
		{
			myArguments = CmdSplit(userInput,&argCount);
                if(isBuiltInCommand(myArguments))
                {
                       status = ExecuteMyCommand(myArguments,argCount);
                }
                else if((place = isOutputRedirect(myArguments,argCount)))
                {
                        RedirectOut(myArguments,place,argCount);
                }
                else if((place = isInputRedirect(myArguments,argCount)))
                {
                        RedirectIn(myArguments,place,argCount);
                }
                else if((place = isPipe(myArguments,argCount)))
                {
                        PipeIt(myArguments,place,argc);
                }
                else if((place = isBackground(myArguments,argCount)))
                {
                        ExecuteBackgroundCmd(myArguments,place);
                }
                else
                {
                    ExecuteOtherCommand(myArguments);
                }
			free(myArguments);
			free(userInput);
		}
		else
		{
            continue;
		}
	} while (status != EXIT_FLAG);

	ExitMessage();

	return(1);
}

int ExecuteBackgroundCmd(char** args,int place)
{
    pid_t pid;
    args[place] = '\0';

    pid = fork();
    if(pid < 0)
    {
        printf("fork error \n");
        return -1;
    }
    else if(pid > 0)    // parent
    {
        printf("executing command in background\n");
    }
    else if(pid == 0) // child proces
    {
        daemon(0,0);
        if(execvp(args[0],&args[0]) == -1)
        {
            printf("cant execute command :( \n");
            return -1;
        }
        return 1;
     }
    return 0;
}
int PipeIt(char** args, int place , int argc)
{
    int pfds[2]; //pipe file desc
   int origSTDIN,origSTDout;
    int status;
    pid_t pidLeft,pidRight;

    origSTDIN = dup(0);
    origSTDout = dup(1);

    args[place] = '\0';
    if (pipe(pfds) == -1)
     {
        perror("pipe");
        return -1;
     }

    pidLeft = fork();
    if(pidLeft < 0)
    {
        printf("fork error\n");
        return -2;
    }
    else if(pidLeft== 0) //child pidleft
    {
        dup2(pfds[1],STDOUT_FILENO);
        close(pfds[0]);
        execvp(args[0],&args[0]);
    }
        close(pfds[1]); // parent

    pidRight = fork();
    if(pidRight < 0)
    {
        printf("fork error\n");
        return -3;
    }
    if(pidRight == 0) // Child
    {
        dup2(pfds[0],STDIN_FILENO);
        close(pfds[0]); // dupliciran je na STDIN_FILENO
        execvp(args[place+1],&args[place+1]);
    }
    close(pfds[0]);
    waitpid(pidLeft,NULL,0);
    waitpid(pidRight,&status,0);

    dup2(origSTDIN,0);
    dup2(origSTDout,1);
    if (WIFEXITED(status))
    {
        switch WEXITSTATUS(status)
         {
            case 0:
            printf("Success ! \n");
            break;
            case 1:
            printf("Datoteka ne postoji\n");
            break;
            default:
            fprintf(stderr, "Greska!\n");
        }
    }
    return status;
}
int RedirectIn(char** args, int place,int argc)
{
    int in,tmp;                // input file descriptor
    char *destinationFile;    // name of destination file
    args[place] = '\0';       // changing redirection sign with '\0' so execvp can work
    if(place == argc-1)      // if there is nothing after >
    {
        printf("use proces > output \n");
        return 1;
    }
    destinationFile = args[place+1];
    in = open(destinationFile,O_RDONLY | O_CREAT ,S_IRUSR | S_IWUSR);
    if(in < 0)
    {
        printf("error opening %s\n",destinationFile);
        return -1;
    }
     //makes newfd be the copy of oldfd, closing newfd first if necessary
    // dup2(int oldfd, int newfd);
    tmp = dup(0); // STDIN_FILENO deleted and saved to tmp
    dup2(in,0);  // out is on 1 place

    ExecuteOtherCommand(args);
    dup2(tmp,0); // restoring STDOUT_FILENO
    close(in);
    return -1;
}
int RedirectOut(char** args, int place,int argc)
{
    int out,tmp;                 // output file descriptor
    char *destinationFile;   // name of destination file
    args[place] = '\0';       // changing redirection sign with '\0' so execvp can work
    if(place == argc-1)   // if there is nothing after >
    {
        printf("use proces > output \n");
        return 1;
    }
    destinationFile = args[place+1];
    out = open(destinationFile,O_WRONLY | O_CREAT ,S_IRUSR | S_IWUSR);
    if(out < 0)
    {
        printf("error opening %s\n",destinationFile);
        return -1;
    }
     //makes newfd be the copy of oldfd, closing newfd first if necessary
    // dup2(int oldfd, int newfd);
    tmp = dup(1); // STDOUT_FILENO deleted and saved to tmp
    dup2(out,1);  // out is on 1 place

    ExecuteOtherCommand(args);
    dup2(tmp,1); // restoring STDOUT_FILENO
    close(out);
    return -1;
}
int ExecuteOtherCommand(char** args)
{
    //this function executes any unix command written in args
    int status = 0;
    pid_t pid;
    pid = fork();
    if(pid > 0) //its parent process
    {
        waitpid(-1,&status,0);
        if(WIFEXITED(status)) //returns true if the child terminated normally
        {
            printf("command finished by PowerShell 1.0 EXIT STATUS[%d]\n",WEXITSTATUS(status));
            return 1;
        }
        else
        {
            printf("ABNORMAL TERMINATION \n");
            return -1;
        }
    }
    else if(pid == 0) // its child proces
    {
        if(execvp(args[0],&args[0]) == -1)
        {
            printf("cant execute command :( \n");
            return -1;
        }
    }
    else if(pid == -1)
    {
        printf("fork error\n");
        return -1;
    }
    return 1;
}
int ExecuteMyCommand(char** args, int argCnt)
{
    int status = 1;
    if(strcmp(args[0],"cd") ==0 )
    {
        status = ChangeDir(args,argCnt);
    }
    else if(strcmp(args[0],"kill") == 0)
    {
        status = KillProcess(args,argCnt);
    }
    else if(strcmp(args[0],"help") == 0)
    {
        PrintHelp();
    }
    else if(strcmp(args[0],"clear") == 0)
    {
        status = ClearScreen();
    }
    else if(strcmp(args[0],"exit") == 0)
    {
        return EXIT_FLAG;
    }
    return status;
}
int KillProcess(char** arguments , int argCnt)
{
    int signalNumber = 0;
    int i = 2;
    if(argCnt < 3)
    {
        printf("pls use like this kill <signal number> <process id 1> <....> <proces id n>\n");
        return 1;
    }
    else
    {
        if(isValidSignal(arguments[1]))
        {
            signalNumber = atoi(arguments[1]);
            for(i = 2; i < argCnt; i++)
            {
               pid_t pid = atoi(arguments[i]);
               if(kill(pid,signalNumber) == -1)
               {
                    printf("killing error , plese contact ISIL [pid does not exist]\n");
                    return -1;
               }
               else
               {
                    printf("proces [%d] killed successfully\n",pid);
                    return 1;
               }
            }
        }
        else
        {
            printf("signal must be 1-64\n");
            return -1;
        }
        return 1;
    }
}

int ChangeDir(char** arguments, int argCnt)
{
    if(argCnt < 2 || argCnt > 2)
    {
        printf("use cd <destination>\n");
        return -1;
    }
    else
    {
        if(chdir(arguments[1]) == 0)
        {
            return 1;
        }
        else
        {
            printf("unable to change to %s",arguments[1]);
            return -2;
        }
    }
}
char* ReadLine()
{
		// this function reads whole line that user enter
		// it also allocates var cmdLineBuffer
		char *cmdLineBuffer = NULL;    //user input goes here
		size_t buffSize = 0;

        buffSize = getline(&cmdLineBuffer,&buffSize,stdin);
        if(cmdLineBuffer == NULL)
        {
            return NULL;
        }
		if(buffSize < 2)
		{
            //there is nothing in line buffer (just new line character)
			return NULL;
		}
		cmdLineBuffer[buffSize-1] = '\0';  //so strcmp can work
		return cmdLineBuffer;
}
char** CmdSplit(char* inputLine,int* argCnt)
{
        //splits input line into **my_argv using strtok and " " delimiter
        char **tokens = malloc(BUFFSIZE *sizeof(char*));
		char *token;
		*argCnt = 0;

		if (tokens == NULL)
		{
			perror("tokens allocation error\n");
			return NULL;
		}

		token = strtok(inputLine," ");
		while (token != NULL)
		{
			tokens[*argCnt] = token;
			(*argCnt)++;
			token = strtok(NULL," ");
		}
		tokens[*argCnt] = NULL;
	return tokens;
}














