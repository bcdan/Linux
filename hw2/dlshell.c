#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define TRUE 1
#define ARGBUFF 64
#define CWDMAX 2048
#define HOME "/home"
#define BLUB "\e[44m"
#define GRN "\e[0;32m"
#define reset "\e[0m"

void start();

char *getLineInput();

char **parseLine(char *line, int *argc, char *delim);

int exec(char **args, char *line,int typeOfFunc);

void purge(char *line, char **args);

int changeDIR(char **arg);

int exitShell(char **arg);

int help(char **arg);

int execRegular(char **args);

int execRedirect(char **args, int option);

int getNumOfParamChars();

void printParams(char **argv, int argc);

void purgeArgsOnly(char **args);

int isBasicFunc(const char *line);

int isParamFunc(const char *line);

void removeSpaces(char * s);

/*------------------------ */
char *regularDELIM = " \t\n\r\a";
char *redirectDELIM = " ><|";
char paramChars[] = {
        '<', '>',
        '&', '|'
};

char *basicBashFuncs[] = {
        "cd",
        "exit",
        "help"
};

int (*basicFuncs[])(char **) ={
        &changeDIR,
        &exitShell,
        &help
};

int (*paramFuncs[])(char **, int) = {
        &execRedirect,
        &execRedirect,
        //&execPipe,
        //&execAsync

};

enum Options {
    IN = 0, OUT, PIPE, ASYNC
};

/*--------------------------*/





void removeSpaces(char* s){
        const char* d = s;
        do {
            while (*d == ' ') {
                ++d;
            }
        } while (*s++ = *d++);

    }



int getNumOfParamChars() {
    return sizeof(paramChars) / sizeof(paramChars[0]);
}

void purgeArgsOnly(char **args) {
    if (args != NULL) {
        for (int i = 0; i < ARGBUFF; i++) {
            if (args[i] != NULL)
                free(args[i]);
        }
        free(args);
    }
}

int execRegular(char **args) {
    pid_t pid;
    int status;

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        return EXIT_FAILURE;
        //exit(1);
    } else if (!pid) {
        execvp(*args, args);
        fprintf(stderr, "%s: command not found\n", args[0]);
//        exit(1);
        return EXIT_FAILURE;
    } else {
        wait(&status);
    }
    return 1;
}


int isBasicFunc(const char *line) {
    for (int i = 0; i < 3; i++) {
        if (strstr(line, basicBashFuncs[i]))
            return 1;

    }
    return -1;
}

int isParamFunc(const char* line){
    int sizeOfParams = getNumOfParamChars();
    for (int i = 0; i < sizeOfParams; i++) {
        if (strchr(line, paramChars[i])) {
            return i;
        }
    }
    return -1;

}

int execRedirect(char **args, int option) {
    int status, fd, argc = 0;
    pid_t pid, wpid;
    char **argv = parseLine(args[0], &argc, " ");
    removeSpaces(args[1]);

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    } else if (!pid) {
        switch (option) {
            case IN: {
                fd = open(args[1], O_RDONLY);

                if (fd < 0) {
                    fprintf(stderr, "could not open file: %s\n", args[1]);
                    return -1;
                }
                dup2(fd, 0);
                break;
            }
            case OUT: {
                printf("ARGS 1 is : %s\n", args[1]);
                fd = open(args[1], O_WRONLY | O_CREAT, 0644);
                if (fd < 0) {
                    fprintf(stderr, "could not open file: %s\n", args[1]);
                    return -1;
                }
                dup2(fd, 1);
                break;

            }
            default:
                return 0;
        }
        execvp(argv[0], argv);
        fprintf(stderr, "%s: command not found\n", args[0]);
        exit(1);
    } else {
        //wait(&status);
        do {

            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    }
    purgeArgsOnly(argv);

    return 1;


}


void purge(char *line, char **args) {
    if (args != NULL) {
        for (int i = 0; i < ARGBUFF; i++) {
            if (args[i] != NULL)
                free(args[i]);
        }
        free(args);
    }
    if (line != NULL) {
        free(line);
    }

}

int exec(char **args, char *line,int typeOfFunc) {
    if (!args) {
        return 1;
    }

    switch(typeOfFunc){
        case 0:{
            for(int i = 0 ;i < 3 ; i++){
                if(strstr(line, basicBashFuncs[i]))
                    return basicFuncs[i](args);
            }
            break;
        }
        case 1:{
            int size = getNumOfParamChars();
            for (int i = 0; i < size; i++)
                if (strchr(line, paramChars[i]))
                    return paramFuncs[i](args, i);

        }
        case 2:{
            return execRegular(args);
        }
    }

return 1;

}

int changeDIR(char **arg) {
    if (arg[1] == NULL) {
        chdir(HOME);
        return 1;
    }
    if (chdir(arg[1]) != 0) {
        fprintf(stderr, "Could not find path\n");
        return -1;
    }
    return 1;
}


int exitShell(char **arg) {
    return 0;
}

int help(char **arg){
    printf("Hi my name's Dan Levy and this is my shell\n");
    return 1;
}


char **parseLine(char *line, int *argc, char *delim) {
    char **argv = NULL;
    char *tempString = strdup(line);
    char *token = strtok(tempString, delim);
    int i = 0;

    argv = (char **) malloc(ARGBUFF * sizeof(char *));
    for (int j = 0; j < ARGBUFF; j++) {
        argv[j] = NULL;
    }
    if (token == NULL) {
        purge(tempString, argv);
        return NULL;
    }
    argv[i++] = strdup(token);
    while ((token = strtok(NULL, delim))) {
        argv[i++] = strdup(token);
    }
    *argc = i;
    free(tempString);

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


void printParams(char **argv, int argc) {
    printf("Params are:\n");
    for (int i = 0; i < argc; i++) {
        printf("%s \n", argv[i]);
    }
    printf("\n");
}

void start() {

    char *lineRead = NULL;
    char **argv = NULL;
    int argc = 0;
    int status = 1;
    int choice = -1;
    char cwd[CWDMAX];
    enum types {BASIC=0,PARAM,OTHER};
    while (status) {
        printf(BLUB"DSHELL$:"reset);
        getcwd(cwd, sizeof(cwd)) != NULL ? printf(GRN"%s$:"reset, cwd) : fprintf(stderr, "Current path error\n");
        lineRead = getLineInput();
        int basic = isBasicFunc(lineRead);
        int param = isParamFunc(lineRead);
        choice = basic!= -1 ? 0 : param != -1 ? 1 : 2;
        switch (choice) {
            case 0 :{
                argv = parseLine(lineRead, &argc, regularDELIM);
                status = exec (argv,lineRead,BASIC);
                break;
            }
            case 1:{
                argv = parseLine(lineRead, &argc, &paramChars[param]);
                status = exec(argv, lineRead,PARAM);
                break;
            }
            case 2:{
                argv = parseLine(lineRead, &argc, regularDELIM);
                status = exec (argv,lineRead,OTHER);
                break;
            }
            default:{
                status = 0;
                break;
            }

        }
        purge(lineRead,argv);
    }



}

int main(int argc, char *argv[]) {
//    puts("\t&&&&&&&\n"
//         "\t&\n"
//         "\t&\n"
//         "\t&&&&&&&\n"
//         "\t&\n"
//         "\t&\n"
//         "\t&&&&&&& * * * \n"
//    );

    start();

    return 0;
}

