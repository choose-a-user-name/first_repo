#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int** stationInfos;
int** toFromMap;
int** distMap;
int** busInfos;
pthread_mutex_t* mutexLocks;
int S = 0;
int B = 0;
int T = 0;
int globalDelivered = 0;
int globalTotal = 0;
int globalReturnFlag = 0;
int absValue(int x){if(x<0) return -x; return x;}

void* runThread(void * args){
    int id = *(int *)(args);
    srand(id);
    int indx = busInfos[id][0];
    int row = indx / S;
    int col = indx % S;
    while(!globalReturnFlag){
        int high = 0;
        pthread_mutex_lock(&(mutexLocks[row]));
        indx = busInfos[id][0];
        for(int i = 0; i < S; i++){
            if(toFromMap[row][i] > high){
                high = toFromMap[row][i];
                col = i;
            }
        }
        high = 0;
        if(col == indx % S){
            for(int i = 0; i < S; i++){
                if( i != row){
                    if(stationInfos[i][2] > high){
                        high = stationInfos[i][2];
                        col = i;
                    }
                }
            }
        }
        if(col == indx % S){
            col = rand() % S;
        }
        busInfos[id][1] = toFromMap[row][col];
        busInfos[id][2] += toFromMap[row][col];
        stationInfos[row][2] -= toFromMap[row][col];
        toFromMap[row][col] = 0;
        busInfos[id][0] = row * S + col;
        pthread_mutex_unlock(&(mutexLocks[row]));    
        sleep(distMap[row][col] / 4.0);
        row = col;
    }
    return NULL;
}

int main (int argc, char** argv){
    if(argc != 2){
        printf("\nUsage is: %s <file_path>\n",argv[0]);
        exit(-1);
    }
    else{
        printf("The file path is: %s\n",argv[1]);
        FILE* ifptr;
        ifptr = fopen(argv[1],"r");
        if(ifptr != NULL){
            printf("File %s was opened!\n",argv[1]);
            int boolTest = 0;
            char buf[81];
            char* dummy;
            dummy = fgets(buf, 80, ifptr);
            if(dummy != NULL) S = atoi(buf);
            strcpy(buf,"");
            stationInfos = malloc((S) * sizeof(int*));
            toFromMap = malloc((S) * sizeof(int*));
            distMap =   malloc((S) * sizeof(int*));
            mutexLocks = malloc((S) * sizeof(pthread_mutex_t));
            dummy = fgets(buf, 80, ifptr);
            if(dummy != NULL) B = atoi(buf);
            strcpy(buf,"");
            busInfos = malloc((B) * sizeof(int*));
            dummy = fgets(buf, 80, ifptr);
            if(dummy != NULL) T = atoi(buf);
            strcpy(buf,"");
            for(int i = 0; i < B; i++){
                busInfos[i] = malloc((3) * sizeof(int));
                busInfos[i][0] = 0;
                busInfos[i][1] = 0;
                busInfos[i][2] = 0;
            }
            for(int i = 0; i < S; i++){
                pthread_mutex_init(&(mutexLocks[i]), NULL);  
                stationInfos[i] = malloc((3) * sizeof(int));
                toFromMap[i] = malloc((S) * sizeof(int));
                distMap[i] = malloc((S) * sizeof(int));
                stationInfos[i][0] = 0;
                stationInfos[i][1] = 0;
                stationInfos[i][2] = 0;
                boolTest = fscanf(ifptr, "%80s",buf);
                stationInfos[i][0] = atoi(buf);
                strcpy(buf,"");
                boolTest = fscanf(ifptr, "%80s", buf);
                stationInfos[i][1] = atoi(buf);
                strcpy(buf, "");
            }
            for(int i = 0; i < S; i++){
                for(int ii = 0; ii < S; ii++){
                    toFromMap[i][ii] = 0;
                    distMap[i][ii] = absValue(stationInfos[i][0] - stationInfos[ii][0]) + 
                                     absValue(stationInfos[i][1] - stationInfos[ii][1]);
                }
            }
            int numPassengers = 0, xCoord = 0, yCoord = 0;
            do{
                boolTest = fscanf(ifptr, "%80s", buf);
                if(boolTest){
                    numPassengers = atoi(buf);
                }
                strcpy(buf,"");
                boolTest = fscanf(ifptr, "%80s", buf);
                if(boolTest){
                    xCoord = atoi(buf);                    
                }
                strcpy(buf,"");
                boolTest = fscanf(ifptr, "%80s", buf);
                if(boolTest){
                    yCoord = atoi(buf);
                }
                strcpy(buf,"");
                if(!feof(ifptr)){
                   if(xCoord >= S || yCoord >= S){
                        printf("Error! Invaid(to large) grid-coords (%d,%d)!",xCoord,yCoord); 
                        exit(-1);
                    }
                    toFromMap[xCoord][yCoord] += numPassengers;
                }      
            }while(!feof(ifptr));
            fclose(ifptr);
            printf("\nFile %s is closed.\n",argv[1]);
            printf("\nBeginning Situation:\nIn distance matrix form:");
            printf("\nRows = Source Station\nColumns = Destination.");
            printf("\nIntegers = Number of people waiting.");
            for(int i = 0; i < S; i++){
                printf("\n");
                for(int ii = 0; ii < S; ii++){
                    stationInfos[i][2] += toFromMap[i][ii];
                    if (toFromMap[i][ii] == 0) printf("  _");
                    else printf("  %d",toFromMap[i][ii]);
                }
                printf("    Station %d's total: %d",i,stationInfos[i][2]);
                globalTotal += stationInfos[i][2];
            }
            printf("\n\n");
            int threadData[B];
            pthread_t tids[B];
            for(int i = 0; i < B; i++){
                threadData[i] = i;
                pthread_create(tids + i, NULL, runThread, threadData + i);
            }
            int timer = 0;
            while(timer < T) { 
                timer++;
                sleep(1); 
                printf("\nAfter %d second(s):",timer);
                for(int i = 0; i < S; i++){
                    printf("\n %d @ (%d, %d):",i,stationInfos[i][0],stationInfos[i][1]);
                    printf(" has %d people waiting.",stationInfos[i][2]);
                } 
            }
            globalReturnFlag = 1;
            printf("\n\nEnding buses finishing last trip.");
            sleep(1);
            printf("\n");
            for(int i = 0; i < B; i++) {
                pthread_join(tids[i], NULL);
                globalDelivered += busInfos[i][2];
                printf("\nBus %d delivered %d people",i,busInfos[i][2]);
            }
            printf("\n\n%d people delivered, %d remain.\n",globalDelivered, globalTotal - globalDelivered);
            printf("\nEnding Situation:");
            for(int i = 0; i < S; i++){
                printf("\n");
                for(int ii = 0; ii < S; ii++){
                    if (toFromMap[i][ii] == 0) printf("  _");
                    else printf("  %d",toFromMap[i][ii]);
                }
                printf("Station %d's total: %d",i,stationInfos[i][2]);
            }
            printf("\n\n");
            for(int i = 0; i < S; i++) {
                free(stationInfos[i]);
                free(toFromMap[i]);
                free(distMap[i]);
                pthread_mutex_destroy(&(mutexLocks[i]));
            }
            free(distMap);
            free(stationInfos);
            free(toFromMap);
            free(mutexLocks);          
            for(int i = 0; i < B; i++) {
                free(busInfos[i]);                
            }
            free(busInfos);
        }
        else{
            printf("File path %s does not exist!\n",argv[1]);
        }
    }
    return 0;
}
//gcc --std=c99 -Wall -O2 -o 3 3.c -pthread
