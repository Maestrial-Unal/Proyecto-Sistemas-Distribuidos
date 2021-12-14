
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>

#include <vector>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <time.h>
#include <stdexcept>
#include <cmath>

using namespace std;

#define GAME_SIZE 3
#define MAX_CHILD 4

vector<string> states_checked;
//__device__ volatile __managed__ bool  isFinished = false;
__device__ bool  isFinished = false;
__managed__ int  total_threads = 640;
int blocks = 10;

cudaError_t bfsWithCuda(__int8* memory, struct node* nodes, int* heigths);

struct state
{
    int game_matrix[GAME_SIZE][GAME_SIZE];
    int zero_index[2];

    state() {}

    state(int actual_matrix[GAME_SIZE][GAME_SIZE]) {
        for (int i = 0; i < GAME_SIZE; i++)
            for (int j = 0; j < GAME_SIZE; j++) {
                if (actual_matrix[i][j] == 0) {
                    zero_index[0] = i;
                    zero_index[1] = j;
                }
                game_matrix[i][j] = actual_matrix[i][j];
            }
    }

};

bool validateState(struct state actual_state) {
    for (int i = 0; i < GAME_SIZE; i++)
        for (int j = 0; j < GAME_SIZE; j++)
            if (actual_state.game_matrix[i][j] != j + (GAME_SIZE * i)) return false;
    return true;
}

string generateStateHash(struct state actual_state) {
    string hash_id = "";
    for (int i = 0; i < GAME_SIZE; i++)
        for (int j = 0; j < GAME_SIZE; j++)
            hash_id += to_string(actual_state.game_matrix[i][j]);

    return hash_id;
}

bool verifyStateChecked(struct state actual_state) {
    string hash_id = generateStateHash(actual_state);
    return find(states_checked.begin(), states_checked.end(), hash_id) != states_checked.end();
}

struct node
{
    vector<int> secuence;
    struct state node_state;

    node() {}

    node(struct state problem_state) {
        node_state = problem_state;
    }
};

__device__ void dev_printState(__int8 matrix[3][3]) {
    printf("Hilo numero: %d\n%d %d %d \n%d %d %d \n%d %d %d \n\n",
        threadIdx.x + blockIdx.x * blockDim.x,
        matrix[0][0],
        matrix[0][1],
        matrix[0][2],
        matrix[1][0],
        matrix[1][1],
        matrix[1][2],
        matrix[2][0],
        matrix[2][1],
        matrix[2][2]);
}

__global__ void addKernel(__int8* memory, struct node* nodes, int* heights)
{
    int idx = threadIdx.x + blockIdx.x * blockDim.x;

    int const registryMemorySpace = 6000; //4000 para 18 movimientos // Se puede hacer variable de acuerdo con el numero de hilos por bloque (?)
    int const stackSize = (registryMemorySpace*2)/3;
    int const maxMemorySize = registryMemorySpace/3;
    
    __int8 stack[stackSize][3][3]; //Max 1820
    __int8 nodeMovements[stackSize];
    __int8 re_memory[maxMemorySize][3][3];

    int actualIndex = 0;
    int size = 1;
    int memorySize = 1;

    int nodesChecked = 1; //DEBUG
    int nodesCreated = 1; //DEBUG
    int resetTimes = 0;   //DEBUG
    int isUseful = 0;     //DEBUG
    int memoryNeeded = 0; //DEBUG

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            stack[actualIndex][i][j] = nodes[idx].node_state.game_matrix[i][j];
            re_memory[0][i][j] = memory[(j + (i * 3)) + (idx * 9)];
        }
    }
    nodeMovements[actualIndex] = heights[idx];

    while (size < registryMemorySpace && !isFinished) {

        bool ansFound = true;
        int i_0 = -1;
        int j_0 = -1;

        for (int i = 0; i < 3; i++) {
            //if (!ansFound && i_0 != -1 && j_0 != -1) break;
            for (int j = 0; j < 3; j++) {
                if (stack[actualIndex][i][j] != j + (3 * i)) ansFound = false;
                if (stack[actualIndex][i][j] == 0) {
                    i_0 = i;
                    j_0 = j;
                }
            }
        }

        if (ansFound) {
            printf("\nHILO %d: Encuentra una solucion con %d movimientos y con un size de pila de %d\n\n", idx, nodeMovements[actualIndex], size);
            dev_printState(stack[actualIndex]);
            dev_printState(stack[0]);
            isFinished = true;
        }else {
            for (int i = 1; i <= 4; i++) {
                
                int provIndex = (actualIndex + size) % registryMemorySpace; //Se consideran los límites de la memoria de la stack
                bool isValid = false;

                if (provIndex==0) resetTimes++; //DEBUG

                switch (i) {
                    case 1:
                        isValid = i_0 != 0;
                        break;
                    case 2:
                        isValid = j_0 != 2;
                        break;
                    case 3:
                        isValid = i_0 != 2;
                        break;
                    case 4:
                        isValid = j_0 != 0;
                        break;
                }

                if (isValid && size<= registryMemorySpace) {

                    for (int j = 0; j < 3; j++) {
                        for (int k = 0; k < 3; k++) {
                            stack[provIndex][j][k] = stack[actualIndex][j][k];
                        }
                    }
                    nodeMovements[provIndex] = nodeMovements[actualIndex]+1;

                    switch (i) {
                        case 1:
                            stack[provIndex][i_0][j_0] = stack[provIndex][i_0 - 1][j_0];
                            stack[provIndex][i_0 - 1][j_0] = 0;
                            break;
                        case 2:
                            stack[provIndex][i_0][j_0] = stack[provIndex][i_0][j_0 + 1];
                            stack[provIndex][i_0][j_0 + 1] = 0;
                            break;
                        case 3:
                            stack[provIndex][i_0][j_0] = stack[provIndex][i_0 + 1][j_0];
                            stack[provIndex][i_0 + 1][j_0] = 0;
                            break;
                        case 4:
                            stack[provIndex][i_0][j_0] = stack[provIndex][i_0][j_0 - 1];
                            stack[provIndex][i_0][j_0 - 1] = 0;
                            break;
                    }

                    bool found = false;
                    for (int j = 0; j < memorySize; j++) {
                        if (!found) {
                            found = true;
                            for (int k = 0; k < 3; k++) {
                                for (int m = 0; m < 3; m++) {
                                    if(found) found = re_memory[j][k][m] == stack[provIndex][k][m];
                                }
                            }
                        }
                    }

                    if (!found) {
                        size++;
                        nodesCreated++; //DEBUG
                    }
                    else  isUseful++;     //DEBUG
                }
            }

            if (memorySize < maxMemorySize) {
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        re_memory[memorySize][i][j] = stack[actualIndex][i][j];
                    }
                }
                memorySize++;
            }else memoryNeeded++; //DEBUG

            size--;
            actualIndex = (actualIndex + 1) % registryMemorySpace;
            nodesChecked++; //DEBUG
        }
    }

    if (!isFinished && idx == 1) {
        printf("--- NO SE ENCUENTRA UNA SOLUCION, SE NECESITA MAS MEMORIA ---\n\n");
    }

    //printf("El hilo %d ha evaluado %d nodos, en ese numero se han generado en total %d nodos.\nSe ha sobreescrito la posicion 0 de la cola %d veces.\nLa memoria compartida fue util %d veces, pero se necesito mas memoria %d veces.\n\Por otro lado, la memoria se utilizo %d veces.\n\n", idx, nodesChecked, nodesCreated, resetTimes, isUseful, memoryNeeded, memorySize);
    //DEBUG

    return;
}

struct state generateState(struct state actual_state, int movement) {
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch (movement) {
    case 1:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0 - 1][j_0];
        actual_state.game_matrix[i_0 - 1][j_0] = 0;

        actual_state.zero_index[0] = i_0 - 1;
        break;
    case 2:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0 + 1];
        actual_state.game_matrix[i_0][j_0 + 1] = 0;

        actual_state.zero_index[1] = j_0 + 1;
        break;
    case 3:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0 + 1][j_0];
        actual_state.game_matrix[i_0 + 1][j_0] = 0;

        actual_state.zero_index[0] = i_0 + 1;
        break;
    case 4:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0 - 1];
        actual_state.game_matrix[i_0][j_0 - 1] = 0;

        actual_state.zero_index[1] = j_0 - 1;
        break;
    }

    return actual_state;
}

bool verifyMovement(struct state actual_state, int movement) {
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch (movement) {
    case 1:
        return i_0 != 0;
        break;
    case 2:
        return j_0 != GAME_SIZE - 1;
        break;
    case 3:
        return i_0 != GAME_SIZE - 1;
        break;
    case 4:
        return j_0 != 0;
        break;
    default:
        return false;
        break;
    }
}

vector<node> createChildren(struct node actual_node) {

    struct state actual_state = actual_node.node_state;
    vector<node> children;

    for (int i = 1; i <= MAX_CHILD; i++) {
        if (verifyMovement(actual_state, i)) {
            struct state new_state = generateState(actual_state, i);
            if (!verifyStateChecked(new_state)) {
                states_checked.push_back(generateStateHash(new_state));

                struct node new_node(new_state);
                new_node.secuence = actual_node.secuence;
                new_node.secuence.push_back(i);

                children.push_back(new_node);
            }
        }
    }

    return children;
}

void print_state(struct state actual_state) {
    for (int i = 0; i < GAME_SIZE; i++) {
        for (int j = 0; j < GAME_SIZE; j++)
            printf("%d ", actual_state.game_matrix[i][j]);
        printf("\n");
    }
    printf("\n");
}

vector<node> distributeThreads(int num_threads, vector<node> nodes) {

    struct node initial_node = nodes.front();
    states_checked.push_back(generateStateHash(initial_node.node_state));

    while (nodes.size() < num_threads) {
        struct node actual_node = nodes.front();

        if (validateState(actual_node.node_state)) return { actual_node };
        vector<node> children = createChildren(actual_node);
        

        if (nodes.size()-1 + children.size() > num_threads) break;
        nodes.erase(nodes.begin());
        nodes.insert(nodes.end(), children.begin(), children.end());
    }
    printf("Se formaron %d nodos\n",nodes.size());
    return nodes;
}

struct state generatePreviousState(struct state actual_state, int movement) {
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch (movement) {
    case 1:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0 + 1][j_0];
        actual_state.game_matrix[i_0 + 1][j_0] = 0;

        actual_state.zero_index[0] = i_0 + 1;
        break;
    case 2:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0 - 1];
        actual_state.game_matrix[i_0][j_0 - 1] = 0;

        actual_state.zero_index[1] = j_0 - 1;
        break;
    case 3:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0 - 1][j_0];
        actual_state.game_matrix[i_0 - 1][j_0] = 0;

        actual_state.zero_index[0] = i_0 - 1;
        break;
    case 4:
        actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0 + 1];
        actual_state.game_matrix[i_0][j_0 + 1] = 0;

        actual_state.zero_index[1] = j_0 + 1;
        break;

    }

    return actual_state;
}

int main()
{
    clock_t start = clock();

    int initial_game_matrix[3][3] = {{ 0,3,5 },
                                     { 7,1,2 },
                                     { 4,8,6 }};

    /*int initial_game_matrix[3][3] = {{3,2,5}, //0.4 secs
                                     {6,0,8},
                                     {7,1,4}};*/

    /*int initial_game_matrix[3][3] = {{1,2,5},
                                     {3,4,8},
                                     {6,0,7}};*/

    struct state initial_state(initial_game_matrix);
    struct node initial_node(initial_state);

    vector<node> distributed_nodes = distributeThreads(total_threads, { initial_node });

    if (distributed_nodes.size() == 1 && validateState(distributed_nodes[0].node_state)) {
        printf("No se tuvo que hacer paralelo\n");
        //Se encontró la solución
    }
    else {
        printf("Se hace paralelo\n");
        printf("Nodos ejecutados %I64u\n\n", distributed_nodes.size());
        total_threads = distributed_nodes.size();

        vector<int> nodesHeigth;
        vector<__int8> memory;

        for (int i = 0; i < total_threads; i++) {
            nodesHeigth.push_back((int)distributed_nodes[i].secuence.size());
            struct state parent = generatePreviousState(distributed_nodes[i].node_state, distributed_nodes[i].secuence.back());

            for (int j = 0; j < 3; j++)
                for (int k = 0; k < 3; k++)
                    memory.push_back(parent.game_matrix[j][k]);
        }

        cudaError_t cudaStatus = bfsWithCuda(&memory[0], &distributed_nodes[0], &nodesHeigth[0]);
        if (cudaStatus != cudaSuccess) {
            fprintf(stderr, "addWithCuda failed!");
            return 1;
        }

        cudaStatus = cudaDeviceReset();
        if (cudaStatus != cudaSuccess) {
            fprintf(stderr, "cudaDeviceReset failed!");
            return 1;
        }

    }
    clock_t end = clock();
    double elapsedTime = (double(end - start) / CLOCKS_PER_SEC);
    cout << "Tiempo de ejecucion: " << elapsedTime << endl;

    return 0;
}

// Función auxiliar para el lanzamiento del kernel
cudaError_t bfsWithCuda(__int8* memory, struct node* nodes, int* heigths)
{
    __int8* dev_memory = 0;
    struct node* dev_nodes = 0;
    int* dev_heights = 0;
    
    cudaError_t cudaStatus;

    // Elegir GPU
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    //Asignar espacio en la GPU
    cudaStatus = cudaMalloc((void**)&dev_memory, total_threads * 9 * sizeof(__int8));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_nodes, total_threads * sizeof(struct node));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_heights, total_threads * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    //Copiar los valores de entrada en la GPU
    cudaStatus = cudaMemcpy(dev_memory, memory, total_threads * 9 * sizeof(__int8), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_nodes, nodes, total_threads * sizeof(struct node), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_heights, heigths, total_threads * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    //Llamar el kernel
    int threads_per_block = ceil(((double)total_threads) / ((double) blocks));
    printf("Numero de hilos por bloque: %d\n", threads_per_block);

    addKernel<<<blocks, threads_per_block >>>(dev_memory, dev_nodes, dev_heights);
    

    // Verificar errores de lanzamiento del kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // Espera a que termine el kernel y retorna algún error encontrado
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    //Free GPU memory
Error:
    cudaFree(dev_memory);
    cudaFree(dev_nodes);
    cudaFree(dev_heights);
    
    return cudaStatus;
}