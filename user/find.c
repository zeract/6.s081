#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
    //static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    //different with ls,don't need blank-padded name
    return p;
}

void find_directory(char *path,char *name){
    int fd;
    struct dirent de;
    struct stat st;
    char buf[512],*p;
    if((fd = open(path,0))<0){
        fprintf(2,"cannot open the directory!\n");
        exit(1);
    }
    if(fstat(fd,&st)<0){
        fprintf(2, "find: cannot stat \n");
        close(fd);
        exit(1);
    }
    switch(st.type){
        case T_FILE:
            if(strcmp(fmtname(path),name)==0){
                printf("%s\n",path);
                //path[strlen(path)-1]='\n';
                //write(1,path,strlen(path));
                //write(1, "\n", 1);
            }
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
            }
            strcpy(buf, path);
            p = buf+strlen(buf);  //p is the pointer of buf,so p can change buf
            *p++ = '/';
            
            
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);  //copy the mem data
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            if(strcmp(fmtname(buf),".")!=0&&strcmp(fmtname(buf),"..")!=0){
                
                find_directory(buf,name); //使用buf而不是fmtname(buf)
                
                
            }
            }
            
            break;
        }
    close(fd);
    
}
int main(int argc,char *argv[]){
    if(argc<3){
        fprintf(2,"find system call is error!\n");
        exit(1);
    }
    find_directory(argv[1],argv[2]);
    exit(0);
    return 0;
}