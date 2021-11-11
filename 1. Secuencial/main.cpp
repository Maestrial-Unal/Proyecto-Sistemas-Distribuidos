#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <time.h>

using namespace std;

#define GAME_SIZE 3
#define MAX_CHILD 4

//  __ __ __
// |__|__|__|
// |__|__|__|
// |__|__|__|
//

vector<string> states_checked;

// Problema - PUZZLE
struct state
{
    int game_matrix[GAME_SIZE][GAME_SIZE];
    int zero_index[2];
    string id_hash;

    state(){}

    state(int actual_matrix[GAME_SIZE][GAME_SIZE]){
        // Copy State
        id_hash = "";
        for(int i=0; i<GAME_SIZE; i++)
            for(int j=0; j<GAME_SIZE; j++){
                if(actual_matrix[i][j] == 0){
                    zero_index[0] = i;
                    zero_index[1] = j;
                }
                game_matrix[i][j] = actual_matrix[i][j];
                id_hash += to_string(game_matrix[i][j]);
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

bool verifyStateChecked(struct state actual_state){
    string hash_id = actual_state.id_hash;
    return find(states_checked.begin(), states_checked.end(), hash_id) != states_checked.end();
}


void print_state(struct state actual_state){

    string state_print = "";

    for(int i=0; i<GAME_SIZE; i++){
        for(int j=0; j<GAME_SIZE; j++)
            state_print += (to_string(actual_state.game_matrix[i][j]) + " ");
        state_print += "\n";
    }

    cout<< state_print <<endl;
    return;
}



// Algoritmo
struct node
{
    int actual_child;
    struct state node_state;

    node(){}

    node(struct state problem_state){
        actual_child = 0;
        node_state = problem_state;
    }
};


int main()
{

    clock_t start=clock(); 
    // Generar estado inicial
    
    int initial_game_matrix[3][3] = { {3,1,2},
                                      {6,4,5},
                                      {0,7,8}};

    struct state initial_state(initial_game_matrix);

    struct node initial_node(initial_state);

    vector<node> node_stack;
    node_stack.push_back(initial_node);
    states_checked.push_back(initial_state.id_hash);
    
    while(!node_stack.empty()){
        struct node actual_node = node_stack.back();
        struct state actual_state = actual_node.node_state;
        
        if (validateState(actual_state)){
            break;
        }

        node_stack.pop_back();
        
        actual_node.actual_child++;
        while(actual_node.actual_child <= MAX_CHILD){
            if(verifyMovement(actual_state, actual_node.actual_child)){
                struct state new_state = generateState(actual_state, actual_node.actual_child);

                if(!verifyStateChecked(new_state)){
                    states_checked.push_back(new_state.id_hash);
                    node_stack.push_back(actual_node);

                    struct node new_node(new_state);
                    node_stack.push_back(new_node);
    	            break;
                }
            }
            actual_node.actual_child++;
        }        
    }
    
    for (vector<node>::iterator it = node_stack.begin() ; it != node_stack.end(); ++it){
        print_state((*it).node_state);    
    }

    clock_t end = clock();

    double elapsedTime = (double(end-start)/CLOCKS_PER_SEC);
    cout << "Execution Time: " << elapsedTime << endl;
        
    return 0;
}




// TRASH

/* 
bool childNumber (struct state actual_state){

    int i_0 = actual_state.zero_index[0];
    int j_0 = actual_state.zero_index[1];
    
    //Esquina 2 movimientos
    if(i_0 % GAME_SIZE-1 == 0 && j_0 % GAME_SIZE-1 == 0) return 2;
    
    //Costado no esquina 3 movimientos
    if(i_0 % GAME_SIZE-1 == 0 || j_0 % GAME_SIZE-1 == 0) return 3;

    //Interno 4 movimientos
    return 4;
}
*/


// if(counter>0){
//     print_state(new_state);
//     counter--;
// }
