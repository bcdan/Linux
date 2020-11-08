#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#define BUFF_SIZE 256


struct fileDescription{
    char permissions[11]; // 10 chars + /0
    char* groupId;
    char* userID;
    int numOfLinks,month,day,year,hour,min;
    unsigned long size;
};

struct fileInfo{
    struct stat sb;
    struct tm* t;
    struct group *grp;
    struct passwd *pwd;
    struct fileDescription desc;
};

void permissionsToString(struct fileInfo* info);
void getInfo(DIR *d);
struct fileInfo constructInfo(const char * name);
void printInfo(struct fileInfo* info,const char* name);
void init(int argc,char** argv);


const char * Monthes[] = {"Jan", "Feb", "Mar", "Apr", "May" ,"Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void permissionsToString(struct fileInfo* info){
    int i = 0;
    info->desc.permissions[i++]=S_ISDIR(info->sb.st_mode)?'d':
                 S_ISFIFO(info->sb.st_mode)?'p':
                 S_ISLNK(info->sb.st_mode)?'l':'-';
    info->desc.permissions[i++]=(S_IRUSR & info->sb.st_mode) ? 'r' : '-';
    info->desc.permissions[i++]=(S_IWUSR & info->sb.st_mode) ? 'w' : '-';
    info->desc.permissions[i++]=(S_IXUSR & info->sb.st_mode) ? 'x' : '-';
    info->desc.permissions[i++]=(S_IRGRP & info->sb.st_mode) ? 'r' : '-';
    info->desc.permissions[i++]=(S_IWGRP & info->sb.st_mode) ? 'w' : '-';
    info->desc.permissions[i++]=(S_IXGRP & info->sb.st_mode) ? 'x' : '-';
    info->desc.permissions[i++]=(S_IROTH & info->sb.st_mode) ? 'r' : '-';
    info->desc.permissions[i++]=(S_IWOTH & info->sb.st_mode ) ? 'w' : '-';
    info->desc.permissions[i++]=(S_IXOTH & info->sb.st_mode) ? 'x' : '-';
    info->desc.permissions[i]='\0';

}

void printInfo(struct fileInfo* info,const char* name){
    printf("%s ",info->desc.permissions);
    printf("%d\t", info->desc.numOfLinks);
    printf("%s\t", info->desc.userID);
    printf("%s ",info->desc.groupId);
    printf("%10.0lu\t", info->desc.size);
    printf("%s %d\t%d %2.2d:%2.2d ",Monthes[info->desc.month], info->desc.day,info->desc.year, info->desc.hour, info->desc.min);
    if(S_ISLNK(info->sb.st_mode)){ // if soft link paint cyan
        ssize_t size;
        char link_read[BUFF_SIZE];
        printf(CYN"%s "RESET,name );
        size = readlink(name, link_read, BUFF_SIZE);
        link_read[size] = '\0';
        printf("->"BLU "%s\n" RESET, link_read);
    }
    else if((S_IXGRP & info->sb.st_mode) || (S_IXUSR & info->sb.st_mode) || (S_IXOTH & info->sb.st_mode))
        printf(BLU"%s\n"RESET, name); //if executable -> paint blue
    else if(S_ISFIFO(info->sb.st_mode))
        printf( BLU"%s\n",RESET, name );
    else
        printf("%s\n", name);


}
// initialize fileInfo struct that holds all the info of a file
// like an i-node only nicer
struct fileInfo constructInfo(const char* name){
    struct fileInfo info;
    lstat(name, &info.sb);
    info.grp = getgrgid(info.sb.st_gid);
    info.pwd = getpwuid(info.sb.st_uid);
    info.desc.userID=info.pwd->pw_name;
    info.desc.groupId=info.grp->gr_name;
    info.desc.numOfLinks=info.sb.st_nlink;
    info.desc.size=info.sb.st_size;
    info.t = localtime(&info.sb.st_ctime);
    info.desc.day=info.t->tm_mday;
    info.desc.month=info.t->tm_mon;
    info.desc.min=info.t->tm_min;
    info.desc.hour=info.t->tm_hour;
    info.desc.year=info.t->tm_year+1900;
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

}
void init(int argc,char** argv) {
    DIR *d = NULL;
    char buf[BUFF_SIZE];
    struct stat s;
    int i = 1;
    while (argc-- > 1) {
        lstat(argv[i], &s);
        if (S_ISDIR(s.st_mode)) {
           // getcwd(buf, sizeof(buf)); ----------Not necessary----------
            chdir(argv[i]); // change dir to the one received by arg
            printf("%s\n", argv[i]);
            d = opendir("."); // open the current dir ^
            getInfo(d);
            //chdir(buf); // ----------Not necessary----------
        } else{
            struct fileInfo temp = constructInfo(argv[i]);
            printInfo(&temp,argv[i]);
        }
        i++;
    }
}

int main(int argc, char *argv[]){
    if(argc < 2){
        DIR *d=NULL;
        d = opendir(".");
        getInfo(d);
    }else
        init(argc,argv);
    return 0;
}