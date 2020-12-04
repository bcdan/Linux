#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define ARGBUFF 64
#define CWDMAX 2048
#define HOME "/home"
#define BLUB "\e[44m"
#define GRN "\e[0;32m"
#define reset "\e[0m"

void start();


char *getLineInput();

char **parseLine(char *line, int *argc, char *delim);

int exec(char **args, int argc, char *line, int typeOfFunc);

void purge(char *line, char **args);

int changeDIR(char **arg);

int exitShell(char **arg);

int help(char **arg);

int execRegular(char **args);

int execRedirect(char **args, int option);

int execAsync(char **args, int argc);

int execPipe(char **args, int argc);

int getNumOfParamChars();

void printParams(char **argv, int argc);

void purgeArgsOnly(char **args);

int isBasicFunc(const char *line);

int isRedirectFunc(const char *line);

int isAsyncPipeFunc(const char *line);

void removeSpaces(char *s);

/*------------------------ */
char *regularDELIM = " \t\n\r\a";

char asyncPipeChars[] = {
        '&', '|'
};
char redirectChars[] = {
        '<', '>'
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

};

int (*asyncPipeFuncs[])(char **, int) = {
        &execAsync,
        &execPipe
};

enum Options {
    IN = 0, OUT, PIPE, ASYNC
};

enum types {
    BASIC = 0, REDIRECT, ASYNCPIPE, OTHER
};

/*--------------------------*/





void removeSpaces(char *s) {
    const char *d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while ((*s++ = *d++));

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

int execPipe(char **args, int argc) {
    int fd[argc][2];
    int numOfArgv = 0;
    pid_t pid;
    char **argv = NULL;
    int status;
    for (int i = 0; i < argc; i++) {
        argv = parseLine(args[i], &numOfArgv, regularDELIM);
        if (i != argc - 1) {
            if (pipe(fd[i]) < 0) {
                fprintf(stderr, "Failed to create pipe\n");
                return -1;
            }
        }
        if ((pid = fork()) < 0) {
            fprintf(stderr, "fork error\n");
            return EXIT_FAILURE;
        } else if (!pid) {
            if (i != argc - 1) {
                dup2(fd[i][STDOUT_FILENO], STDOUT_FILENO);
                close(fd[i][STDIN_FILENO]);
                close(fd[i][STDOUT_FILENO]);
            }
            if (i != 0) {
                dup2(fd[i - 1][STDIN_FILENO],STDIN_FILENO);
                close(fd[i - 1][STDOUT_FILENO]);
                close(fd[i - 1][STDIN_FILENO]);
            }
            execvp(argv[0], argv);
            fprintf(stderr, "could not execute pipe\n");
            return -1;
        }
        if (i != 0) {
            close(fd[i - 1][STDIN_FILENO]);
            close(fd[i - 1][STDOUT_FILENO]);
        }
        wait(&status);
        purgeArgsOnly(argv);
    }
    return 1;

}


int execAsync(char **args, int argc) {
    char **argv = NULL;
    pid_t pid, wpid;
    int status;
    int sizeOfArgv = 0;
    for (int i = 0; i < argc; i++) {
        argv = parseLine(args[i], &sizeOfArgv, regularDELIM);
        if ((pid = fork()) < 0) {
            fprintf(stderr, "fork error\n");
            return EXIT_FAILURE;
        }
        if (!pid) {
            execvp(argv[0], argv);
            fprintf(stderr, "could not execute &\n");
            return EXIT_FAILURE;
        }

    }
    for (int i = 0; i < argc; i++) {
        wpid = waitpid(pid, &status, 0);
        printf("[%d] %d\n", i + 1, wpid);
    }
    purgeArgsOnly(argv);
    return 1;
}


int execRegular(char **args) {
    pid_t pid;
    int status;

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        return EXIT_FAILURE; // todo :maybe change this later
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

int isRedirectFunc(const char *line) {
    int sizeOfParams = 2;
    for (int i = 0; i < sizeOfParams; i++) {
        if (strchr(line, redirectChars[i])) {
            return i;
        }
    }
    return -1;

}

int isAsyncPipe(const char *line) {
    int sizeOfParams = 2;
    for (int i = 0; i < sizeOfParams; i++) {
        if (strchr(line, asyncPipeChars[i])) {
            return i;
        }
    }
    return -1;

}

int execRedirect(char **args, int option) {
    int status, fd, argc = 0;
    pid_t pid;
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
        wait(&status);

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

int exec(char **args, int argc, char *line, int typeOfFunc) {
    if (!args) {
        return 1;
    }

    switch (typeOfFunc) {
        case BASIC: {
            for (int i = 0; i < 3; i++) {
                if (strstr(line, basicBashFuncs[i]))
                    return basicFuncs[i](args);
            }
            break;
        }
        case REDIRECT: {
            for (int i = 0; i < 2; i++) {
                if (strchr(line, redirectChars[i]))
                    return paramFuncs[i](args, i);

            }
        }
        case ASYNCPIPE: {
            for (int i = 0; i < 2; i++) {

                if (strchr(line, asyncPipeChars[i]))
                    return asyncPipeFuncs[i](args, argc);
            }
        }

        case OTHER: {
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

int help(char **arg) {
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
    int argc = 0; //todo: use this -> very useful
    int status = 1;
    int choice = -1;
    char cwd[CWDMAX];

    while (status) {
        printf(BLUB"DSHELL$:"reset);
        getcwd(cwd, sizeof(cwd)) != NULL ? printf(GRN"%s$:"reset, cwd) : fprintf(stderr, "Current path error\n");
        lineRead = getLineInput();
        int basic = isBasicFunc(lineRead);
        int redirect = isRedirectFunc(lineRead);
        int asyncPipe = isAsyncPipe(lineRead);
        choice = basic != -1 ? BASIC : redirect != -1 ? REDIRECT : asyncPipe != -1 ? ASYNCPIPE : OTHER;
        switch (choice) {
            case BASIC : {
                argv = parseLine(lineRead, &argc, regularDELIM);
                status = exec(argv, argc, lineRead, BASIC);
                break;
            }
            case REDIRECT: {
                argv = parseLine(lineRead, &argc, &redirectChars[redirect]);
                status = exec(argv, argc, lineRead, REDIRECT);
                break;
            }
            case ASYNCPIPE: {
                argv = parseLine(lineRead, &argc, &asyncPipeChars[asyncPipe]);
                status = exec(argv, argc, lineRead, ASYNCPIPE);
                break;
            }
            case OTHER: {
                argv = parseLine(lineRead, &argc, regularDELIM);
                status = exec(argv, argc, lineRead, OTHER);
                break;
            }
            default: {
                status = 0;
                break;
            }

        }
        purge(lineRead, argv);
    }


}

int main(int argc, char *argv[]) {


    start();

    return 0;
}

