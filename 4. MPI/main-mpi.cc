#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <time.h>
#include <stdexcept>
#include <cmath>
#include <mpi.h>

using namespace std;

#define GAME_SIZE 3
#define MAX_CHILD 4

//  __ __ __
// |__|__|__|
// |__|__|__|
// |__|__|__|
//

vector<string> states_checked;
// volatile bool found_flag = false;

// ********************************** Problema - PUZZLE **********************************//
struct state
{
    int game_matrix[GAME_SIZE][GAME_SIZE];
    int zero_index[2];

    state() {}

    state(int actual_matrix[GAME_SIZE][GAME_SIZE])
    {
        for (int i = 0; i < GAME_SIZE; i++)
            for (int j = 0; j < GAME_SIZE; j++)
            {
                if (actual_matrix[i][j] == 0)
                {
                    zero_index[0] = i;
                    zero_index[1] = j;
                }
                game_matrix[i][j] = actual_matrix[i][j];
            }
    }
};

bool validateState(struct state actual_state)
{
    for (int i = 0; i < GAME_SIZE; i++)
        for (int j = 0; j < GAME_SIZE; j++)
            if (actual_state.game_matrix[i][j] != j + (GAME_SIZE * i))
                return false;
    return true;
}

//       1
//       __
//  4   |__|   2
//
//       3

bool verifyMovement(struct state actual_state, int movement)
{
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch (movement)
    {
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

struct state generateState(struct state actual_state, int movement)
{
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch (movement)
    {
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

struct state generatePreviousState(struct state actual_state, int movement)
{
    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];

    switch (movement)
    {
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

string generateStateHash(struct state actual_state)
{
    string hash_id = "";
    for (int i = 0; i < GAME_SIZE; i++)
        for (int j = 0; j < GAME_SIZE; j++)
            hash_id += to_string(actual_state.game_matrix[i][j]);

    return hash_id;
}

bool verifyStateChecked(struct state actual_state)
{
    string hash_id = generateStateHash(actual_state);
    return find(states_checked.begin(), states_checked.end(), hash_id) != states_checked.end();
}

string print_state(struct state actual_state)
{

    string state_print = "";

    for (int i = 0; i < GAME_SIZE; i++)
    {
        for (int j = 0; j < GAME_SIZE; j++)
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

    node() {}

    node(struct state problem_state)
    {
        node_state = problem_state;
    }
};

vector<node> createChildren(struct node actual_node)
{

    struct state actual_state = actual_node.node_state;
    vector<node> children;

    for (int i = 1; i <= MAX_CHILD; i++)
    {
        if (verifyMovement(actual_state, i))
        {
            struct state new_state = generateState(actual_state, i);
            if (!verifyStateChecked(new_state))
            {
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

void printSolutionStates(struct node node_solution)
{
    struct state state_movement = node_solution.node_state;
    string secuence_print = print_state(state_movement) + " \n";

    for (int i = node_solution.secuence.size() - 1; i >= 0; i--)
    {
        state_movement = generatePreviousState(state_movement, node_solution.secuence[i]);
        secuence_print = print_state(state_movement) + " \n" + secuence_print;
    }

    cout << secuence_print << endl;
    cout << "Movimientos minimos requeridos: " << node_solution.secuence.size() << endl;
}

void breadthFirstSearch(vector<node> node_queue, int id_process, int num_process, string nodo_name)
{

    vector<string> states_checked_parallel;

    while (!node_queue.empty())
    {
        //Obtener primero
        struct node actual_node = node_queue.front();
        node_queue.erase(node_queue.begin());

        // Revisar si es valido
        if (validateState(actual_node.node_state))
        {
            cout << "Node Solution" << endl;
            cout << "Encontrado por el proceso: " << id_process << endl;
            cout << "Encontrado en el nodo: " << nodo_name << endl;
            cout << "\n " << endl;
            printSolutionStates(actual_node);
            return;
        }

        //Crear nodos del árbol
        struct state actual_state = actual_node.node_state;
        vector<node> children;

        for (int i = 1; i <= MAX_CHILD; i++)
        {
            if (verifyMovement(actual_state, i))
            {
                struct state new_state = generateState(actual_state, i);

                string hash_id = generateStateHash(new_state);
                bool verifyPepe = find(states_checked_parallel.begin(), states_checked_parallel.end(), hash_id) != states_checked_parallel.end();

                if (!verifyPepe)
                {
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

// ********************************** PARALELO MPI **********************************//

vector<node> generate_nodes_for_mpi(struct node initial_node, int num_process)
{
    vector<node> nodes = {initial_node};

    while (nodes.size() < num_process)
    {
        vector<node> all_children;
        for (node &node_parent : nodes)
        {
            vector<node> children = createChildren(node_parent);
            all_children.insert(all_children.end(), children.begin(), children.end());
        }
        nodes = all_children;
    }
    return nodes;
}

vector<int> distribute_nodes(int num_nodes, int num_process)
{
    vector<int> distribution = {};
    while (num_process != 0)
    {
        int num_nodes_process = num_nodes / num_process;
        distribution.push_back(num_nodes_process);
        num_process--;
        num_nodes = num_nodes - num_nodes_process;
    }
    return distribution;
}

int main(int argc, char *argv[])
{

    // ********************************** PARALELO MPI **********************************//

    //Crea el primer nodo del arbol y de la cola, y se marca como visitado

    MPI_Status status;
    int num_process, id_process;
    char nodo_name[MPI_MAX_PROCESSOR_NAME];
    int namelen;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_process);
    MPI_Get_processor_name(nodo_name, &namelen);
    
    // TIME: START
    clock_t start =  clock();

    // Genera el estado inicial
    int initial_game_matrix[3][3] = {
        {7,0,3},
        {1,2,5},
        {4,8,6}
    };

    struct state initial_state(initial_game_matrix);
    struct node initial_node(initial_state);

    // Nodos iniciales
    vector<node> nodes = generate_nodes_for_mpi(initial_node, num_process);
    vector<int> distribution = distribute_nodes(nodes.size(), num_process);

    // Crear Distribucion
    vector<node> nodes_process;
    int index_start;
    int index_end;
    if (id_process == 0)
    {
        index_start = 0;
        index_end = distribution[0];
    }
    else
    {
        index_start = 0;
        for (int i = 0; i < id_process; i++)
            index_start = index_start + distribution[i];

        index_end = index_start + distribution[id_process];
    }
    //Agrega nodos
    for (int i = index_start; i < index_end; i++)
        nodes_process.push_back(nodes[i]);

    // Ejecutar busqueda de la seccion
    breadthFirstSearch(nodes_process, id_process, num_process, nodo_name);


    // TIME: END
    clock_t end = clock();
    double elapsedTime = (double(end - start) / CLOCKS_PER_SEC);
    cout << "Tiempo de ejecucion: " << elapsedTime << endl;
    
    // TERMINAR PROCESO
    MPI_Abort(MPI_COMM_WORLD, -1);
    MPI_Finalize();
    return 0;
}




// Update Variable WITH MPI
// MPI_Gather((void *)&found_flag, 1, MPI_INT, found_flags, 1, MPI_INT, 0, MPI_COMM_WORLD);
// if (id_process == 0)
//     for (int i = 0; i < num_process; i++)
//         found_flag = found_flag + found_flags[i];
// MPI_Bcast((void *)&found_flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
