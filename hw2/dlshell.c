#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define BUFFSIZE 254
#define ARGBUFF 64
#define HOME "/home"

void start();

char *getLineInput();

char **parseLine(char *line,int* argc);

void exec(char** args);

void purge(char *line, char **args);

void changeDIR(char* arg);

void clear(); // clear screen

char *DELIM = " \t\n\r\a";

void purge(char *line, char **args) {
    if(args!=NULL){
        for (int i = 0; i < ARGBUFF; i++) {
            if (args[i] != NULL)
                free(args[i]);
        }
        free(args);
    }

    free(line);

}
void clear ()
{
    for ( int i = 0; i < 50; i++ ) // 50 is arbitrary
        printf("\n");
}

void exec(char** args){
    pid_t  pid;
    int    status;

    if ((pid = fork()) < 0) {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) {
            execvp(*args, args) ;
            fprintf(stderr,"%s: command not found\n",args[0]);
            exit(1);
        }
    else {
        while (wait(&status) != pid)
            ;
    }
}

void changeDIR(char* arg){
    if(arg==NULL){
        chdir(HOME);
        return;
    }
    if(chdir(arg)!=0)
        fprintf(stderr,"Could not find path\n");
}


char **parseLine(char *line,int* argc) {
    char **argv = NULL;
    int i = 0;
    argv = (char **) malloc(ARGBUFF * sizeof(char *));
    for (int i = 0; i < ARGBUFF; i++) {
        argv[i] = NULL;
    }
    char *token = strtok(line, DELIM);
    if(token==NULL){
        purge(line,argv);
        return NULL;
    }
    argv[i++] = strdup(token);
    while ((token = strtok(NULL, DELIM))) {
        argv[i++] = strdup(token);
    }
    *argc=i;

    return argv;

}


char *getLineInput() {

    /*when buff is set to 0 and *line to NULL, getline() will auto-malloc*/
    size_t buffSize = 0;
    size_t characters;
    char *line = NULL;
    characters = getline(&line, &buffSize, stdin);
    line[characters - 1] = '\0';
    return line;

}

//void printParams(char **argv,int argc) {
//    printf("Params are :");
//    for (int i = 0; i < argc; i++) {
//        printf("%s ", argv[i]);
//    }
//    printf("\n");
//}

void start() {
    char *lineRead = NULL;
    char **argv = NULL;
    int argc=0;
    while (TRUE) {
        printf("DSHELL$:\t");
        lineRead = getLineInput();
        argv = parseLine(lineRead,&argc);
        if(!lineRead|| !argv){
            continue;
        }
        if (!strcmp(lineRead, "exit")) {
            purge(lineRead, argv);
            break;
        }
        if(!strcmp(lineRead,"cd")){
            changeDIR(argv[1]);
            purge(lineRead,argv);
            continue;
        }
        if(!strcmp(lineRead,"clear")){
            clear();
            purge(lineRead,argv);
            continue;;
        }
            exec(argv);
            purge(lineRead, argv);

    }


}

int main(int argc, char *argv[]) {
    start();

    return 0;
}

