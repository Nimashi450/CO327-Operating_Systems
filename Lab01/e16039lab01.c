/*  
    CO327: Operating Systems 
    Simple Shell(Lab01)
    E/16/037 - Balasuriya B.M.N.U
    compile: gcc -o e16039lab01 e16039lab01.c
    run: ./e16039lab01
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

void getcmd(char userInput[], char *argv[] );

int main() {

    
    char userInput[100];
    char *argv[20];
    char path[100];
    char *envp[] = {(char*) "PATH=/bin", 0 }; // set an environmental variable

    while(1) {
        printf ("--> ");   //shell starts
        
        getcmd(userInput, argv);  //get commands from the shell
        
        // to exit from the shell enter 'quit' or 'q'
        if (strcmp (userInput, "quit") == 0 || strcmp (userInput, "q") == 0 )
            break;
        
        strcpy (path, "/bin/");
        strcat (path, userInput);
        int fd = access(path, F_OK); //access to the path
        pid_t pid = fork();   //process id
        
        if (fd == -1)
        {
            printf("Command not found\n");
        }
        
        else{

            if (pid < 0){
                printf("Fork did not work"); // pid<0 process not work

            } 
            //parent process
            else if (pid > 0){    
                wait(NULL);//wait untill chiled process

            }
            //chile process
            else{  
                 
                execve (path, argv, envp); //execute command

            }
        }

    }
    
    return 0;

}

void getcmd(char userInput[], char *argv[] ){
    
    char input [1024];
    int c = 0;
    int i = 0;
    char *arr[100], *token;

    // read the line
    for (;;){
        int line = fgetc (stdin);
        input[c++] = (char) line;
        
        if ( line == '\n') 
            break;
    }
    
    // if there is one argument return to main 
    if (c == 1) 
        return;

    token = strtok (input, " \n"); //setting string token to null

    
    while (token != NULL){
    
        arr[i++] = strdup (token);
        token = strtok (NULL, " \n");
        
    }  

    strcpy (userInput, arr[0]); //take first argument as command

    for (int k=0; k<i; k++) // other arguments are parameters
        argv[k] = arr[k];

    argv[i] = NULL; // terminate the parameter 
}
