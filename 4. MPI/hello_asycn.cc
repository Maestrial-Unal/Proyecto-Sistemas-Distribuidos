#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define MSG_LENGTH 15
int main(int argc, char *argv[]){

    int i, tag=1, tasks, iam;
    char message[MSG_LENGTH];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &iam);
    int flag = 0;
    if(iam == 0){
        int flag = 1;
        for(i=1; i< tasks; i++){
            MPI_Ssend(&flag, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else{
        MPI_Request request;
        MPI_Irecv(&flag, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        int ready = 0;

        while(flag == 0){
            // MPI_Test(&request, &ready, MPI_STATUS_IGNORE);
            printf("[Process %d]. Hi %d.\n", iam, ready);
        }
        
        
        printf("[Process %d]. Ready value %d.\n", iam, ready);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
        printf("[Process %d]. I received the value %d.\n", iam, flag);
        
    }
    printf("Termine: %d\n", iam);
    MPI_Finalize();
    return 0;

}
