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

int execRegular(char **args, int argc);

int execRedirect(char **args, int option);

int execPipe(char **args, int argc);

void printParams(char **argv, int argc);

void purgeArgsOnly(char **args);

int isBasicFunc(const char *line);

int isRedirectFunc(const char *line);

int isPipeFunc(const char *line);

void removeSpaces(char *s);

int checkBackgroundExec(char **args, int *argc);

char **parseByAmp(char *line, int *argc);//assuming we know we have an ampersand

int execTwoRedirects(char **args, int argc);


/*------------------------ */
char *regularDELIM = " \t\n\r\a";


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


enum Options {
    IN = 0, OUT
};

enum types {
    BASIC = 0, REDIRECT, PIPE, OTHER
};

/*--------------------------*/



int checkBackgroundExec(char **args, int *argc) { // if it has '&' -> return 1 else 0
    char *amp = "&";
    if (!strcmp(args[(*argc) - 1], amp)) {
        free(args[(*argc) - 1]);
        args[(*argc) - 1] = NULL;
        (*argc)--;
        // printParams(args,argc);
        return 1;
    }
    return 0;
}

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

int execTwoRedirects(char **args, int argc) {
    int status, argv_c = 0;
    int output_fd, input_fd;
    pid_t pid;
   // char **argv = parseLine(args[0], &argc, " ");
    removeSpaces(args[1]);
    removeSpaces(args[2]);
    printf("%s\n",args[2]);

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    } else if (!pid) {


        output_fd = open(args[2], O_WRONLY | O_CREAT, 0644);
        if (output_fd < 0) {
            fprintf(stderr, "could not open file: %s\n", args[2]);
            return -1;
        }
        dup2(output_fd, 1);
        close(output_fd);


        input_fd = open(args[1], O_RDONLY);

        if (input_fd < 0) {
            fprintf(stderr, "could not open file: %s\n", args[1]);
            return -1;
        }
        dup2(input_fd, 0);
        close(input_fd);

        execvp(args[0], args);
        fprintf(stderr, "%s: command not found\n", args[0]);
        exit(1);
    } else {
        wait(&status);

    }

    //purgeArgsOnly(argv);

    return 1;

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
                dup2(fd[i - 1][STDIN_FILENO], STDIN_FILENO);
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

int execRegular(char **args, int argc) {
    pid_t pid, wpid;
    int status;
    int argv_c = 0;
    char **argv = NULL;
    for (int i = 0; i < argc; i++) {
        argv = parseLine(args[i], &argv_c, " ");
        removeSpaces(argv[i]);
        int hasAmp = checkBackgroundExec(argv, &argv_c);
        if ((pid = fork()) < 0) {
            fprintf(stderr, "fork error\n");
            return EXIT_FAILURE; // todo :maybe change this later
            //exit(1);
        } else if (!pid) {
            execvp(*argv, argv);
            fprintf(stderr, "%s: command not found\n", argv[0]);
//        exit(1);
            return EXIT_FAILURE;
        } else if (!hasAmp) {
            wait(&status);
        } else {
            wpid = waitpid(pid, &status, WNOHANG);
            if (WIFEXITED((status))) {
                printf("[%d] STARTED \n", wpid);
            }
        }
        purgeArgsOnly(argv);

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


int isPipeFunc(const char *line) {
    return strchr(line, '|') != NULL ? PIPE : -1;
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
            if (strchr(line, '>') && strchr(line, '<')) {
                return execTwoRedirects(args,argc);
            }
            for (int i = 0; i < 2; i++) {
                if (strchr(line, redirectChars[i]))
                    return paramFuncs[i](args, i);

            }
        }
        case PIPE: {
            return execPipe(args, argc);

        }

        case OTHER: {
            return execRegular(args, argc);
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


void printParams(char **argv, int argc) { //for debugging
    printf("Params are:\n");
    for (int i = 0; i < argc; i++) {
        printf("%s \n", argv[i]);
    }
    printf("\n");
}


char **parseByAmp(char *line, int *argc) {//assuming we know we have an ampersand
    char *temp = line;
    char **argv = NULL;
    argv = (char **) malloc(ARGBUFF * sizeof(char *));
    for (int j = 0; j < ARGBUFF; j++) {
        argv[j] = NULL;
    }

    int i = 0;
    while ((*temp)) {
        size_t len = strcspn(temp, "&");
        if (temp[len] == '\0') {
            argv[i++] = strdup(temp);
        } else {
            argv[i++] = strndup(temp, len + 1);
        }
        for (int j = 0; j <= len; j++)
            temp++;
    }

    *argc = i;
    return argv;

}


void start() {

    char *lineRead = NULL;
    char **argv = NULL;
    int argc = 0;
    int status = 1;
    int choice = -1;
    char cwd[CWDMAX];

    while (status) {
        printf(BLUB"DSHELL$:"reset);
        getcwd(cwd, sizeof(cwd)) != NULL ? printf(GRN"%s$:"reset, cwd) : fprintf(stderr, "Current path error\n");
        lineRead = getLineInput();
        int basic = isBasicFunc(lineRead);
        int redirect = isRedirectFunc(lineRead);
        int Pipe = isPipeFunc(lineRead);
        choice = basic != -1 ? BASIC : redirect != -1 ? REDIRECT : Pipe != -1 ? PIPE : OTHER;
        switch (choice) {
            case BASIC : {
                if (strchr(lineRead, '&'))
                    argv = parseByAmp(lineRead, &argc);
                else {
                    argv = parseLine(lineRead, &argc, regularDELIM);
                }
                status = exec(argv, argc, lineRead, BASIC);
                break;
            }
            case REDIRECT: {
                argv = parseLine(lineRead, &argc, &redirectChars[redirect]);
                status = exec(argv, argc, lineRead, REDIRECT);
                break;
            }
            case PIPE: {
                argv = parseLine(lineRead, &argc, "|");
                status = exec(argv, argc, lineRead, PIPE);
                break;
            }
            case OTHER: {
                if (strchr(lineRead, '&'))
                    argv = parseByAmp(lineRead, &argc);
                else {
                    argv = parseLine(lineRead, &argc, regularDELIM);
                }
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

