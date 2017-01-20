#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFFSIZE_CWD 128

//utility functions
int isBuiltInCommand(char**);
int isValidSignal(char*);
int isOutputRedirect(char**,int);
int isInputRedirect(char**,int);
int isPipe(char**,int);
int isBackground(char**,int);
int ClearScreen();

void PrintShellMessage();
void ExitMessage();
void PrintHelp();

void PrintWelcomeMessage()
{
        ClearScreen();
        printf("****************************************************************\n");
        printf("*\t Welcome to PowerShell 1.0\n");
        printf("*\t author : Jurica Separovic\n");
        printf("*\t course : programiranje za unix\n\n");
        printf("*\t you can use:\n");
        printf("*\t\t ->shell built in commands like [cd ,kill,clear, exit]  \n");
        printf("*\t\t ->redirection,pipes and executing in background\n");
        printf("*\t\t ->any unix built in command\n");
        printf("*\t if you are lost type help, good luck !\n");
        printf("****************************************************************\n");
}
void PrintShellMessage()
{
    char currentDir[128];
    getcwd(currentDir,BUFFSIZE_CWD);
	printf("[jusep]PowerShell 1.0 ~%s >  ",currentDir);
}
void ExitMessage()
{
    ClearScreen();
    printf("****************************************************************\n");
    printf("\tthx for using jusep PowerShell\n");
    printf("\tSeeYouuAgain\n");
    printf("****************************************************************\n");
}
void PrintHelp()
{
    ClearScreen();
    printf("*********************  HELP   ************************************\n");
    printf("\t-> cd <destination>\n");
    printf("\t-> kill <signal number> <process id 1> <....> <proces id n>\n");
    printf("\t-> program > output \n");
    printf("\t-> program < input  \n");
    printf("\t-> program | program \n");
    printf("\t-> program & \n");
    printf("\t->type exit or clear\n");
    printf("\nyou can also execute any other unix bult in command\n");
    printf("****************************************************************\n");
}
int ClearScreen()
{
    /*just clears screen  NOT PORTABLE*/
    system("clear");
    return 1;
}
int isBuiltInCommand(char** arguments)
{
    //funkcija provjerava dali je uneseni argument funkija moje ljuske
    int stat = 0;
    int i = 0;
    char* commands[] = {"cd","kill","help","exit","clear"};
    for(i = 0; i < 5 ; i++)
    {
        if(strcmp(arguments[0],commands[i])==0)
        {
            stat++;
        }
    }
    return stat;
}
int isValidSignal(char* sigChar)
{
    //just check if signal is valid number
    int sig = atoi(sigChar);
    if(sig > 0 && sig < 64)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
int isOutputRedirect(char** args , int argCnt)
{
    // i consider that > cant be first argument
       // if found returns his index
    int i;
    for(i = 1; i < argCnt ; i++)
    {
        if(strcmp(args[i],">") == 0)
         return i;
    }
    return 0;
}
int isInputRedirect(char** args , int argCnt)
{
    // i consider that < cant be first argument
       // if found returns his index
    int i;
    for(i = 1; i < argCnt ; i++)
    {
        if(strcmp(args[i],"<") == 0)
         return i;
    }
    return 0;
}
int isPipe(char** args,int argCnt)
{
    // i consider that | cant be first argument
       // if found returns his index
    /*it returns the index of | */
    int i;
    for(i = 1; i < argCnt ; i++)
    {
        if( (strcmp(args[i],"|")) == 0)
         {
            return i;
         }
    }
    return 0;
}
int isBackground(char** args,int argCnt)
{
     // i consider that & cant be first argument
     // if found returns his index
    int i;
    for(i = 1; i < argCnt ; i++)
    {
        if(strcmp(args[i],"&") == 0)
         return i;
    }
    return 0;
}
