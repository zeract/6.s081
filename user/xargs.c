#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
int readline(char *new_argv[32], int curr_argc){
	char buf[1024];
	int n = 0;
    
	while(read(0, buf+n, 1)){
		if (n == 1023)
		{
			fprintf(2, "argument is too long\n");
			exit(1);
		}
		if (buf[n] == '\n')
		{
			break;
		}
        
		n++;
	}
    
	buf[n] = 0;
	if (n == 0)return 0;
	int offset = 0;
    
    
	while(offset < n){
		new_argv[curr_argc++] = buf + offset;
		while(buf[offset] != ' ' && offset < n){
			offset++;
		}
		while(buf[offset] == ' ' && offset < n){
			buf[offset++] = 0;
		}
	}
    
	return curr_argc;
}

int main(int argc,char *argv[]){
    //char *argv1[31];
    char *argv2[31];
    //char buf1[512];
    int p[2];
    pipe(p);
    
        
        close(p[0]);
        
        int j=0;
        while(strcmp(argv[j],"xargs")!=0){
            j++;
            
        }
        j++;
        int count=0;
        while(j<argc){
            argv2[count] =argv[j];
            j++;
            count++;
        }

        int curr_argc;
        while((curr_argc = readline(argv2, argc - 1)) != 0)
        {
            argv2[curr_argc] = 0;
            if(fork() == 0){
			exec(argv2[0], argv2);
			fprintf(2, "exec failed\n");
			exit(1);
		    }
		wait(0); 
        }
        /*
        char buf[512];
        int i=0;
        char p[512];
        int index=0;
        while(read(0,buf+i,1)){
            while(buf[i]!='\n'){
                if(i==511){
                    fprintf(2,"argument is too long\n");
                    exit(1);
                }
                if(buf[i]!=' '){
                    p[index] = buf[i];
                    index++;
                    i++;
                }
                else{
                    p[index] ='\0';
                    argv2[count++] = p;
                    index = 0;
                    i++;
                }    
            }
            i++;
        }
        p[index] = '\0';
        argv2[count++] = p;
        argv2[count]=0;
        */
        //close(p[1]);
        //exec(argv2[0],argv2);
    
    
    exit(0);
}