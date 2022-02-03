#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <time.h>
#include <stdexcept>
#include "omp.h"

using namespace std; 

#define GAME_SIZE 3
#define MAX_CHILD 4

//  __ __ __
// |__|__|__|
// |__|__|__|
// |__|__|__|
//

vector<string> states_checked;
volatile bool found_flag = false;

// ********************************** Problema - PUZZLE **********************************//
struct state
{
    int game_matrix[GAME_SIZE][GAME_SIZE];
    int zero_index[2];

    state(){}

    state(int actual_matrix[GAME_SIZE][GAME_SIZE]){
        for(int i=0; i<GAME_SIZE; i++)
            for(int j=0; j<GAME_SIZE; j++){
                if(actual_matrix[i][j] == 0){
                    zero_index[0] = i;
                    zero_index[1] = j;
                }
                game_matrix[i][j] = actual_matrix[i][j];
            }
    }
    
};

bool validateState (struct state actual_state){
    for(int i=0; i<GAME_SIZE; i++)
        for(int j=0; j<GAME_SIZE; j++)
            if( actual_state.game_matrix[i][j] != j+(GAME_SIZE*i)) return false;    
    return true;
}

//       1
//       __
//  4   |__|   2
//      
//       3

bool verifyMovement (struct state actual_state, int movement){
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch(movement){
        case 1:
            return i_0 != 0;
            break;
        case 2:
            return j_0 != GAME_SIZE-1;
            break;
        case 3:
            return i_0 != GAME_SIZE-1;
            break;
        case 4:
            return j_0 != 0;
            break;
        default:
            return false;
            break;
    }
}

struct state generateState (struct state actual_state, int movement){
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch(movement){
        case 1:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0-1][j_0];
            actual_state.game_matrix[i_0-1][j_0] = 0;
            
            actual_state.zero_index[0] = i_0-1;
            break;
        case 2:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0+1];
            actual_state.game_matrix[i_0][j_0+1] = 0;

            actual_state.zero_index[1] = j_0+1;
            break;
        case 3:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0+1][j_0];
            actual_state.game_matrix[i_0+1][j_0] = 0;

            actual_state.zero_index[0] = i_0+1;
            break;
        case 4:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0-1];
            actual_state.game_matrix[i_0][j_0-1] = 0;

            actual_state.zero_index[1] = j_0-1;
            break;
    }

    return actual_state;
}

struct state generatePreviousState (struct state actual_state, int movement){
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch(movement){
        case 1:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0+1][j_0];
            actual_state.game_matrix[i_0+1][j_0] = 0;

            actual_state.zero_index[0] = i_0+1;
            break;
        case 2:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0-1];
            actual_state.game_matrix[i_0][j_0-1] = 0;

            actual_state.zero_index[1] = j_0-1;
            break;
        case 3:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0-1][j_0];
            actual_state.game_matrix[i_0-1][j_0] = 0;
            
            actual_state.zero_index[0] = i_0-1;
            break;
        case 4:
            actual_state.game_matrix[i_0][j_0] = actual_state.game_matrix[i_0][j_0+1];
            actual_state.game_matrix[i_0][j_0+1] = 0;

            actual_state.zero_index[1] = j_0+1;
            break;
            
    }

    return actual_state;
}

string generateStateHash(struct state actual_state){
    string hash_id = "";
    for(int i=0; i<GAME_SIZE; i++)
        for(int j=0; j<GAME_SIZE; j++)
            hash_id += to_string(actual_state.game_matrix[i][j]);

    return hash_id;
}

bool verifyStateChecked(struct state actual_state){
    string hash_id = generateStateHash(actual_state);
    return find(states_checked.begin(), states_checked.end(), hash_id) != states_checked.end();
}

string print_state(struct state actual_state){

    string state_print = "";

    for(int i=0; i<GAME_SIZE; i++){
        for(int j=0; j<GAME_SIZE; j++)
            state_print += (to_string(actual_state.game_matrix[i][j]) + " ");
        state_print += "\n";
    }

    return state_print;
}


// ********************************** Problema - PUZZLE **********************************//

// ************************************* Algoritmo BFS *************************************//
struct node
{
    vector<int> secuence;
    struct state node_state;

    node(){}

    node(struct state problem_state){
        node_state = problem_state;
    }
};


vector<node> createChildren(struct node actual_node){

    struct state actual_state = actual_node.node_state;
    vector<node> children;
        
    for(int i=1; i<=MAX_CHILD; i++){
        if(verifyMovement(actual_state, i)){
            struct state new_state = generateState(actual_state, i);
            if(!verifyStateChecked(new_state)){
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


struct node breadthFirstSearch(vector<node> node_queue, string (*generateStateHash)(struct state), 
                               bool (*validateState)(struct state), bool (*verifyMovement)(struct state, int),
                               struct state (*generateState)(struct state, int), bool (*verifyStateChecked)(struct state)){

    vector<string> states_checked_parallel;

    while(!node_queue.empty()){
        
        //Obtener primero
        struct node actual_node = node_queue.front();
        if(found_flag) return actual_node;

        //Borrar primero
        node_queue.erase(node_queue.begin());

        // Revisar si es valido 
        if (validateState(actual_node.node_state)){
            found_flag = true;
            return actual_node;
        }
        
        //Crear nodos del árbol
            struct state actual_state = actual_node.node_state;
            vector<node> children;
                
            for(int i=1; i<=MAX_CHILD; i++){
                if(verifyMovement(actual_state, i)){
                    struct state new_state = generateState(actual_state, i);

                    
                    string hash_id = generateStateHash(new_state);
                    bool verifyPepe = find(states_checked_parallel.begin(), states_checked_parallel.end(), hash_id) != states_checked_parallel.end();


                    if(!verifyPepe){
                        states_checked_parallel.push_back(hash_id);

                        struct node new_node(new_state);
                        new_node.secuence = actual_node.secuence;
                        new_node.secuence.push_back(i);

                        children.push_back(new_node);
                    }
                }
            }
        // vector<node> children = createChildren(actual_node);
        node_queue.insert(node_queue.end(), children.begin(), children.end());
    }

    throw invalid_argument("ERROR: No se encontro una solucion para el estado inicial ingresado");
}





void printSolutionStates(struct node node_solution){
    struct state state_movement = node_solution.node_state;
    string secuence_print = print_state(state_movement) + " \n";

    for (int i=node_solution.secuence.size()-1; i>=0; i--){
        state_movement = generatePreviousState(state_movement, node_solution.secuence[i]);
        secuence_print = print_state(state_movement) + " \n" + secuence_print;
    }

    cout << secuence_print << endl;
    cout << "Movimientos minimos requeridos: " << node_solution.secuence.size() << endl;
}


// ************************************* Algoritmo BFS *************************************//

// **************************************** Paralelo ****************************************//
vector<int> distributeResources(int min, int max){    
    vector<int> resources;
    int min_per_num = max/min;
    int additional = max % min;

    for(int i=0; i<min; i++)
        resources.push_back(min_per_num + (additional>i));

    return resources;
}


vector<vector<node>> distributeThreads(int num_threads, vector<node> nodes){

    if(num_threads == 1) return {nodes};
    if(nodes.size() != 1 ) throw invalid_argument("ERROR: La lista ingresada tiene más de un nodo");


    struct node actual_node = nodes.back();
    if (validateState(actual_node.node_state)) return {{actual_node}};

    // Se Crea el vector de respuesta y se generan los hijos del nodo
    vector<vector<node>> threads_queues;
    vector<node> children = createChildren(actual_node);
    int num_child = children.size();

    if(num_child == 0) throw invalid_argument("ERROR: El nodo no tiene hijos posibles");

    // Repartir hijos en los hilos
    if(num_child >= num_threads){
        vector<int> child_per_threads = distributeResources(num_threads, num_child);
        
        for (int& i: child_per_threads){
            vector<node> thread_queue;
            for(int j=0; j<i; j++){
                thread_queue.push_back(children.back());
                children.pop_back();
            }
            threads_queues.push_back(thread_queue);
        }
    }
    // Repartir hilos en los hijos
    else{
        vector<int> threads_per_child = distributeResources(num_child, num_threads);

        for (int i = 0; i<threads_per_child.size(); i++){
            vector<node> new_nodes = {children[i]};
            vector<vector<node>> new_queues = {new_nodes};

            if(threads_per_child[i] > 1){
                new_queues = distributeThreads(threads_per_child[i], new_nodes);
                if(new_queues.size() == 1) return new_queues;
            }
            
            threads_queues.insert( threads_queues.end(), new_queues.begin(), new_queues.end());
        }
    }

    return threads_queues;
}
// **************************************** Paralelo ****************************************//

int main()
{
    clock_t start=clock();

    int num_thread = 16;

    // Genera el estado inicial
    int initial_game_matrix[3][3] = {
        {7,0,3},
        {1,2,5},
        {4,8,6}
    };

    struct state initial_state(initial_game_matrix);
    struct node initial_node(initial_state);

    //Crea el primer nodo del arbol y de la cola, y se marca como visitado
    vector<node> node_queue = {initial_node};
    states_checked.push_back(generateStateHash(initial_state));

    struct node node_solution = initial_node;
    
    if(num_thread == 1){
        node_solution = breadthFirstSearch( node_queue, 
                                            generateStateHash, 
                                            validateState, 
                                            verifyMovement,
                                            generateState,
                                            verifyStateChecked
                                          );
    }else{
        // Secuencialmente se busca una solución mientras se contruyen los subconjuntos
        // de nodos que evaluará cada hilo en caso de no encontrar dicha solución
        vector<vector<node>> nodes_per_thread = distributeThreads(num_thread, node_queue);

        if(nodes_per_thread.size()==1)
            node_solution = nodes_per_thread[0][0];
        else{
            if(nodes_per_thread.size() != num_thread) throw invalid_argument("ERROR: No hay suficientes nodos asignados");

            #pragma omp parallel num_threads(num_thread) shared(found_flag)
            {
                #pragma omp for
                for(int i=0; i<num_thread; i++){
                    struct node solution  = breadthFirstSearch( nodes_per_thread[i], 
                                                                generateStateHash, 
                                                                validateState, 
                                                                verifyMovement,
                                                                generateState,
                                                                verifyStateChecked
                                                               );

                    if(validateState(solution.node_state)){
                        node_solution = solution;
                    }
                }
            }
        }
    }
    
    printSolutionStates(node_solution);

    //Se hace la impresión de la duración del programa
    clock_t end = clock();
    double elapsedTime = (double(end-start)/CLOCKS_PER_SEC);
    cout << "Tiempo de ejecucion: " << elapsedTime << endl;
        
    return 0;
}