#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
//-------Colors macros------
#define MAG   "\x1B[35m"
#define GRN   "\x1B[32m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"
//--------------------------
#define CURRENT_FOLDER "."
#define BUFF_SIZE 256


struct fileDescription{
    char permissions[11]; // 10 chars + /0 -> acts as a string to describe a file's permission
    char* groupId;
    char* userID;
    int numOfLinks,month,day,year,hour,min;
    unsigned long size; //size of file in bytes
};
//two structs that make life easier when dealing with files
struct fileInfo{
    __mode_t st_mode;
    struct fileDescription desc;
};

const char * Months[] = {"Jan", "Feb", "Mar", "Apr", "May" ,"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void permissionsToString(struct fileInfo* info);
void getInfo(DIR *d);
struct fileInfo constructInfo(const char * name);
void printInfo(struct fileInfo* info,const char* name);
void init(int argc,char** argv);

int main(int argc, char *argv[]){
    DIR *d=NULL;
    if(argc < 2){ //no arguments => show contents of current folder "."
        d = opendir(CURRENT_FOLDER);
        getInfo(d);
    }else{
        init(argc,argv);
    }
    closedir(d);
    return 0;
}

void init(int argc,char** argv) {
    DIR *d = NULL;
    struct stat s;
    for(int i = 1 ; argc>1;i++,argc--){
        int status = lstat(argv[i], &s);
        if(status == -1 ){ // check for valid path
            fprintf(stderr, "ls: cannot access '%s': No such file or directory\n", argv[i]);
            continue;
        }
        if (S_ISDIR(s.st_mode)) { // is it a directory?
            chdir(argv[i]); // change dir to the one received by arg
            printf("\nPath: '%s'\n", argv[i]);
            d = opendir(CURRENT_FOLDER); // open the current dir ^
            getInfo(d);
            closedir(d);
        } else{
            struct fileInfo temp = constructInfo(argv[i]);
            printInfo(&temp,argv[i]);
        }
    }
}

// initialize fileInfo struct that holds all the info of a file
// like an i-node only cleaner and readable
struct fileInfo constructInfo(const char* name){
    struct fileInfo info;
    struct stat sb;
    struct tm* t;
    struct group *grp;
    struct passwd *pwd;
    lstat(name,&sb);
    grp = getgrgid(sb.st_gid);
    pwd = getpwuid(sb.st_uid);
    info.desc.userID=pwd->pw_name;
    info.desc.groupId=grp->gr_name;
    info.desc.numOfLinks=sb.st_nlink;
    info.desc.size=sb.st_size;
    t = localtime(&sb.st_ctime);
    info.desc.day=t->tm_mday;
    info.desc.month=t->tm_mon;
    info.desc.min=t->tm_min;
    info.desc.hour=t->tm_hour;
    info.desc.year=t->tm_year+1900;
    info.st_mode=sb.st_mode;
    permissionsToString(&info);
    return info;

}
void getInfo(DIR *d){ // iterate and scan for all files/folders and gather all info
    struct dirent *entry=NULL;
    struct fileInfo temp;

    while((entry = readdir(d)) != NULL){
        temp=constructInfo(entry->d_name);
        printInfo(&temp,entry->d_name);
    }
    printf("\n");

}
void printInfo(struct fileInfo* info,const char* name){
    printf("%2s ",info->desc.permissions);
    printf("%5d ", info->desc.numOfLinks);
    printf("%5s %5s ",info->desc.userID,info->desc.groupId);
    printf("%10ld ", info->desc.size);
    printf("%2s %2d %2d %2.2d:%.2d ",Months[info->desc.month], info->desc.day,info->desc.year, info->desc.hour, info->desc.min);

    if(info->desc.permissions[0]=='l'){ //if a link
        ssize_t size;
        char linkBuff[BUFF_SIZE];
        printf(MAG"%s "RESET,name );// if soft link -> magenta
        size = readlink(name, linkBuff, BUFF_SIZE);
        linkBuff[size] = '\0';
        printf("-> "BLU "%s\n" RESET, linkBuff);
    }
    else if(info->desc.permissions[9]=='x' && info->desc.permissions[0]!='d') //if executable
        printf(GRN"%s\n"RESET, name);
    else if(info->desc.permissions[0]=='d')
        printf(BLU"%s\n"RESET, name); //if directory -> blue
    else
        printf("%s\n", name);


}

void permissionsToString(struct fileInfo* info){
    int i = 0;
    info->desc.permissions[i++]=S_ISDIR(info->st_mode)?'d':
                                S_ISFIFO(info->st_mode)?'p':
                                S_ISLNK(info->st_mode)?'l':'-';
    info->desc.permissions[i++]=(S_IRUSR & info->st_mode) ? 'r' : '-';
    info->desc.permissions[i++]=(S_IWUSR & info->st_mode) ? 'w' : '-';
    info->desc.permissions[i++]=(S_IXUSR & info->st_mode) ? 'x' : '-';
    info->desc.permissions[i++]=(S_IRGRP & info->st_mode) ? 'r' : '-';
    info->desc.permissions[i++]=(S_IWGRP & info->st_mode) ? 'w' : '-';
    info->desc.permissions[i++]=(S_IXGRP & info->st_mode) ? 'x' : '-';
    info->desc.permissions[i++]=(S_IROTH & info->st_mode) ? 'r' : '-';
    info->desc.permissions[i++]=(S_IWOTH & info->st_mode ) ? 'w' : '-';
    info->desc.permissions[i++]=(S_IXOTH & info->st_mode) ? 'x' : '-';
    info->desc.permissions[i]='\0';

}
