#define _POSIX_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int global_PIDS[4];
static int puck_count = 0;
static int global_goal_scorer = 0;

static void fault_handler(int signum){
    puck_count += 1;
    printf("%ld had puck %d time(s)\n",(long)getpid(), puck_count);
    if(global_goal_scorer){
        printf("and scored a goal!\n");
    }else{
        kill(global_PIDS[(rand()%4)],SIGUSR1);
    }
};

static void fault_handler2(int signum){
    puck_count += 1;
    printf("%ld had puck %d time(s)\n",(long)getpid(), puck_count);
};

int main(int argc, char** argv){
    pid_t pid = getpid();/*argv[1];*/
    if (argc != 3){
        printf("\npassed %d args to %s", argc, argv[0]);
        exit(-1);
    }
    else{
        printf("\n%s has pid %ld",argv[1], (long)pid);
        if(!strcmp(argv[1],argv[2])){
            global_goal_scorer = 1;
        }
        sigset_t mask;
        sigemptyset(&mask);
        struct sigaction sa1 = {
            .sa_handler = fault_handler,
            .sa_mask = mask,
            .sa_flags = 0
        };
        sigaction(SIGUSR1, &sa1, NULL);
        struct sigaction sa2 = {
            .sa_handler = fault_handler2,
            .sa_mask = mask,
            .sa_flags = 0
        };
        sigaction(SIGUSR2, &sa2, NULL);
        unsigned int SIZE = 2048;
        char buf[SIZE];
        ssize_t boolRead = read(0, buf, SIZE);
        int boolConverted = 0;
        boolRead = 0;
        int fd1 = open("/proc/sys/kernel/pid_max", O_RDONLY);
        long maxPid = 0;
        strcpy(buf,"");
        boolRead = read(fd1, buf, SIZE);
        if(boolRead){
            maxPid = atol(buf);
        }
        else{
            perror("no pid max");
            exit(-1);
        }
        close(fd1);
        char str1[] = "/proc/";
        char str2[] = "/cmdline";
        int thisPID = 0;
        unsigned int tempCtr = 0;
        for(int ii = 0; ii < 4; ii = ii + 1) global_PIDS[ii] = 0;
        for(int i = 1; i <= maxPid; i = i + 1){
            char str3[17];
            char buf2[SIZE];
            for(int iii = 0; iii < SIZE; iii = iii + 1) buf2[iii] = '\0';
            boolConverted = sprintf(str3,"%d",i);
            if (boolConverted) {
                boolConverted = sprintf(buf, "%s%s%s",str1,str3,str2);
                if(boolConverted){
                    fd1 = open(buf, O_RDONLY);
                    boolRead = read(fd1,buf2,SIZE);
                    if(boolRead){
                        if(!strcmp(argv[0],buf2)){
                            if(i != (long)getpid()){
                                global_PIDS[tempCtr] = i;
                                tempCtr = (tempCtr + 1)%4;
                            }
                            else{
                                thisPID = i;
                            }
                        }
                    }
                    close(fd1);
                }
                else{
                    perror("Error!  sprintf failed!\n");
                    exit(-1);
                }
            }
            else{
                perror("Error!  sprintf failed!\n");
                exit(-1);
            }
        }
        printf(" | other pids:");
        for(int ii = 0; ii < 4; ii = ii + 1) printf("%d ", global_PIDS[ii]);
        printf("\n");
        srand(thisPID);
        while (boolRead != 0){
            strcpy(buf,"");
            boolRead = read(0, buf, SIZE);
        } 
    }
    return 0;
}/*compile with gcc -std=c99 -Wall -O2 -o a player.c*/
