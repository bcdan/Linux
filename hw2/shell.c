//Oren Or 203985262
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#define BUF_LEN 1024

//for testing
void print2dArr(char** arr,int size){
	int i;
	printf("size: %d\n",size);
	for(i=0;i<size;i++){
		printf("%s ",arr[i]);
	}
	printf("\n");
}

void free2dArr(char** arr,int size){
	int i;
	for(i=0;i<size;i++){
		if(arr[i]!=NULL)
		free(arr[i]);
	}
	if(arr!=NULL)
	free(arr);
}

char** copy2dArrPartly(char** arr,int size,int stop_index){
	int i;
	if(stop_index>size)
	stop_index=size;
	char** ret=(char**)malloc(stop_index*sizeof(char*));
	for(i=0;i<stop_index;i++){
		ret[i]=(char*)malloc((1+strlen(arr[i]))*sizeof(char));
		strcpy(ret[i],arr[i]);
	}
	return ret;
}

int countWords(char * str) 
{ 
    int state = 0, cnt = 0;
    while (*str) 
    { 
        if (*str == ' ' || *str == '\n' || *str == '\t') 
		state = 0; 
        else if (state == 0) 
        { 
            state = 1; 
            ++cnt; 
		} 
        ++str; 
	}   
    return cnt; 
} 

void exe(char** args,int size,bool background,int in_file_index,int out_file_index){
	int status,fdin=-1,fdout=-1,stop_index=size;
	char** new_arr;
	pid_t pid;
	
	pid = fork();
	if(pid==-1){
		perror("fork\n");
		free2dArr(args,size);
		exit(EXIT_FAILURE);
	}
	if(background){
		stop_index=size-1;
	}
	if(pid == 0) {	
		if(out_file_index!=-1){
			stop_index=out_file_index-1;
			close(1);
			fdout = open(args[out_file_index], O_WRONLY | O_CREAT , 0766);
			if(fdout==-1){
				perror("fdout");
			}			
		}	
		if(in_file_index!=-1){
			stop_index=in_file_index-1;
			close(0);
			fdin = open(args[in_file_index], O_RDONLY);
			if(fdin==-1){
				perror("fdin");
			}	
		}
		if(stop_index!=size){
			new_arr=copy2dArrPartly(args,size,stop_index);
			if(execvp(new_arr[0], new_arr)==-1){
				free2dArr(new_arr,stop_index);
				perror("exec smaller array\n");
			}
		}		
		if(execvp(args[0], args)==-1){
			perror("exec original array\n");
		}
	}
	if(!background){
		wait(&status);
	}
	if(fdin!=-1)
	close(fdin);
	if(fdout!=-1)
	close(fdout);
	free2dArr(args,size);
	if(new_arr!=NULL)
	free2dArr(new_arr,stop_index);
}

void changeDirectory(char* arg,int size){
	if(size>1){
		char path[BUF_LEN];
		getcwd(path,BUF_LEN);
		strcat(path, "/");
		strcat(path, arg);
		if(chdir(path)==-1){
			perror("chdir with arg\n");
		}
	}
	else{
		struct passwd *pw = getpwuid(getuid());
		char *homedir = pw->pw_dir;
		if(chdir(homedir)==-1){
			perror("chdir home\n"); 
		} 
	}  
}

void getInput(char* buffer){
	int count=0,i=0,out_file_index=-1,in_file_index=-1;
	char *token, **arr,*pos;
	size_t size;
	bool background=false,cdFlag=false;
	
	// Read contents from stdin 
	printf("# ");
	if (getline(&buffer, &size, stdin) == -1) {
		perror("getline error\n");
		exit(EXIT_FAILURE);
	}
	count=countWords(buffer);
	if(count==0)
		return;
	//for null at the end
	arr=(char**)malloc((count+1)*sizeof(char*));
	if(arr==NULL){
		perror("arr malloc\n");
		exit(EXIT_FAILURE);
	}
	//split line
	token = strtok(buffer, " ");
	for(i=0;i<count;i++)
	{
		if(token==NULL){
			perror("token null");
			free2dArr(arr,count+1);
			exit(EXIT_FAILURE);
		}
		if((strcmp(token,"exit")==0||(strcmp(token,"exit\n")==0))&&i==0){
			free(arr);
			exit(EXIT_SUCCESS);
		}
		if((strcmp(token,"cd")==0||strcmp(token,"cd\n")==0)&&i==0){
			cdFlag=true;
		}		
		//check "&" for background thread
		if(strcmp(token,"&\n")==0||strcmp(token,"&")==0){
			if(i!=count-1){
				perror("&\n");
				free2dArr(arr,count+1);			
				exit(EXIT_FAILURE);
			}
			background=true;
		}		
		//check "<" for in file
		else if(strcmp(token,"<")==0){
			if(i==count-1){
				perror("error in <\n");
				free2dArr(arr,count+1);		
				exit(EXIT_FAILURE); 
			}			
			in_file_index=i+1;
		}
		//check ">" for out file
		else if(strcmp(token,">")==0){
			if(i==count-1){
				perror("error in >\n");
				free2dArr(arr,count+1);	
				exit(EXIT_FAILURE);
			}
			out_file_index=i+1;
		}
		arr[i]=(char*)malloc((strlen(token)+1)*sizeof(char));
		if(arr[i]==NULL){
			free2dArr(arr,count+1);
			perror("token malloc\n");
			exit(EXIT_FAILURE);
		}
		stpcpy(arr[i],token);
		//remove '\n'
		if(i==count-1){
			if ((pos=strchr(arr[i], '\n')) != NULL) 
			*pos = '\0';
		}
		token = strtok(NULL, " ");
	}
	if(cdFlag){
		changeDirectory(arr[1],count);
		return;
	}
	exe(arr,count,background,in_file_index,out_file_index);
}

int main(int argc, char *argv[]) {	
	char buffer[BUF_LEN]; 
	while(1){
		getInput(buffer);
	}
	return EXIT_SUCCESS;
}