/**
 * @author RookieHPC
 * @brief Original source code at https://www.rookiehpc.com/mpi/docs/mpi_igather.php
 **/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
 
/**
 * @brief Illustrates how to use a gather in a non-blocking way.
 * @details This application is meant to be run with 4 MPI processes. Every MPI
 * process begins with a value, the MPI process 0 is picked to gather all these
 * values and print them. It can be visualised as follows:
 *
 * +-----------+ +-----------+ +-----------+ +-----------+
 * | Process 0 | | Process 1 | | Process 2 | | Process 3 |
 * +-+-------+-+ +-+-------+-+ +-+-------+-+ +-+-------+-+ 
 *   | Value |     | Value |     | Value |     | Value |   
 *   |   0   |     |  100  |     |  200  |     |  300  |   
 *   +-------+     +-------+     +-------+     +-------+   
 *            \            |     |            /
 *             \           |     |           /
 *              \          |     |          /
 *               \         |     |         /
 *                \        |     |        /
 *                 \       |     |       /
 *                +-----+-----+-----+-----+
 *                |  0  | 100 | 200 | 300 |
 *                +-----+-----+-----+-----+
 *                |       Process 0       |
 *                +-----------------------+
 **/
int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
 
    // Get number of processes and check that 4 processes are used
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(size != 4)
    {
        printf("This application is meant to be run with 4 MPI processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
 
    // Determine root's rank
    int root_rank = 0;
 
    // Get my rank
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
    // Define my value
    int my_value = my_rank * 100;
    printf("Process %d, my value = %d.\n", my_rank, my_value);
 
    // Request handler
    MPI_Request request;
 
    if(my_rank == 0)
    {
        int buffer[4];
 
        // Launch the gather
        MPI_Igather(&my_value, 1, MPI_INT, buffer, 1, MPI_INT, root_rank, MPI_COMM_WORLD, &request);
 
        // Do another job while the gather progresses
        // ...
 
        // Wait for the gather to complete
        // MPI_Wait(&request, MPI_STATUS_IGNORE);
        printf("Values collected on process %d: %d, %d, %d, %d.\n", my_rank, buffer[0], buffer[1], buffer[2], buffer[3]);
    }
    else if(my_rank == 1){}
    else
    {
        // Launch the gather
        MPI_Igather(&my_value, 1, MPI_INT, NULL, 0, MPI_INT, root_rank, MPI_COMM_WORLD, &request);
 
        // Do another job while the gather progresses
        // ...
 
        // Wait for the gather to complete
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
 
    MPI_Finalize();
 
    return EXIT_SUCCESS;
}
