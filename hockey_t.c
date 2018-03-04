#define _POSIX_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#define NUM_CHILD 5

int main(int argc, char** argv){
    if (argc != 2){
        printf("Error!  Usage is: %s <C|LW|RW|D|D2>\n", argv[0]);
        exit(-1);
    }
    else{
        char *members[] = {"C","LW","RW","D","D2", NULL};
        int pipeFD[5][2];
        int boolArgIsValid = 0;
        pid_t pids[5];
        for(int l = 0; l < NUM_CHILD; l = l + 1){
            pids[l] = 0;
            pipeFD[l][0] = 0;
            pipeFD[l][1] = 0;
            if (!strcmp(argv[1],members[l])) boolArgIsValid = 1;
        }    
        if (boolArgIsValid == 0){
            printf("Error!  Usage is: %s <C|LW|RW|D|D2>\n", argv[0]);
            exit(-1);
        }      
        for(int i = 0; i < NUM_CHILD; i = i + 1){
            int retVal = 0;
            retVal = pipe(pipeFD[i]);
            pids[i] = fork();
            if (pids[i] == 0){
                if(!retVal){
                    int dupVal = dup2(pipeFD[i][0], 0);
                    if (!dupVal){
                       close(pipeFD[i][0]);
                       close(pipeFD[i][1]);
                    }
                    execlp("./player","player",members[i],argv[1], (char*)NULL);
                }else{
                    perror("Error!  Pipe failed!\n");
                    exit(-1);
                }
            }
            else if ( (long)getpid() < 0 ){
                perror("Error!  Fork failed!\n");
                exit(-1);
            }else{
                close(pipeFD[i][0]);
                ssize_t sizeOfMsg = write(pipeFD[i][1], members[i], 4);
                if (sizeOfMsg < 1){
                    printf("\nDid not write msg.");
                }
                if(i == NUM_CHILD - 1){
                    sleep(2);
                    int choice = 1;
                    while(choice != 2){
                        sleep(1);
                        retVal = 0;
                        printf("\nMain Menu:");
                        printf("\n0. Pass Puck to Player");
                        printf("\n1. Show Player Statistics");
                        printf("\n2. End Game\nChoose an option: ");
                        int temp = scanf("%d",&choice);
                        if(!temp){
                            printf("\nThe choices are 0,1,2\n");
                            perror("\nError!  Scanf failed\n");
                            exit(-1);
                        }
                        if (choice < 0 || choice > 2){ 
                            printf("\nThe choices are 0,1,2\n");
                        }
                        else{
                            switch(choice){
                                case 0:
                                    for(int ii = 0; ii < NUM_CHILD; ii = ii + 1){
                                        printf("%d. %s\n",ii,members[ii]);
                                    }
                                    printf("Choose a player: ");
                                    retVal = scanf("%d",&choice);
                                    if(choice < 0 || choice > 4){
                                        printf("The choices are 1-5\n");
                                    }
                                    else {
                                        printf("pid %d is %ld\n",choice, (long)pids[choice]);
                                    }
                                    kill(pids[choice],SIGUSR1);
                                    choice = 0;
                                    break;
                                case 1:
                                    for(int ii = 0; ii < NUM_CHILD; ii = ii + 1){
                                        printf("%d. %s\n",ii,members[ii]);
                                    }
                                    printf("Choose a player: ");
                                    retVal = scanf("%d",&choice);
                                    if(choice < 0 || choice > 4){
                                        printf("The choices are 1-5\n");
                                    }
                                    else {
                                        printf("pid %d is %ld\n",choice, (long)pids[choice]);
                                    }
                                    kill(pids[choice],SIGUSR2);
                                    choice = 0;
                                    break;
                                case 2:
                                    printf("\nBye\n ");
                                    break;
                                default:
                                    printf("\nChoose 0, 1 or 2 please.\n ");
                                    break;
                            }
                        }
                    }
                    for (int k = 0; k < i+1; k = k + 1){
                        close(pipeFD[k][1]);     
                    }
                    for (int k = 0; k < i+1; k = k + 1){
                        retVal = 0;
                        pid_t tempPID = waitpid(pids[k], &retVal, 0);
                        if (tempPID > 0){
                            printf("\n%ld is dead.",(long)tempPID);
                        }
                        else{
                            printf("\nError on pid %ld.",(long)tempPID);
                        }
                    }
                }
            }
        }  
    }
    return 0;
}/*compile with gcc --std=c99 -Wall -O2 -o a team.c*/

