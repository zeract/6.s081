#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(int argc,char *argv[]){
    int p[2];
    pipe(p);
    char buf[512];
    int n;
    if(fork()==0){
        
        n = read(p[0],buf,6);
        if(n!=6){
            fprintf(2,"child read error\n");
            exit(1);
        }
        fprintf(2,"%d: received ping\n",getpid());
        
        if(write(p[1],buf,6)!=6){
            fprintf(2,"child write error\n");
            exit(1);
        }
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    else{
        n=write(p[1],"hello ",6);
        if(n!=6){
            fprintf(2,"parent write error\n");
            exit(1);
        }
        close(p[1]);
        wait(0);
        n = read(p[0],buf,6);
        if(n!=6){
            fprintf(2,"parent read error\n");
            exit(1);
        }
        fprintf(2,"%d: received pong\n",getpid());
        close(p[0]);
        exit(0);
    }
}