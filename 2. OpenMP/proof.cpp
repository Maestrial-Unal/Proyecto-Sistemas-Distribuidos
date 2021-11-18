#include <iostream>
#include <time.h>
#include "omp.h"


using namespace std;

int main()
{   
    clock_t start=clock();

    int a[56] = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};
    int b[56] = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};
    int c[56];

    volatile bool flag = false;
    
    #pragma omp parallel num_threads(4) shared(flag)
    {
        #pragma omp for schedule(static,1)
        for(int i=0; i<56; i++){
            if(flag) continue;
            
            c[i]=a[i]+b[i];
            cout << i << " -> "<< omp_get_thread_num() << " : " << c[i] <<"\n";
            
            if(c[i]==8) flag = true;
            
        }
    }  
    // for(int i=0; i<4; i++){
    //     cout << c[i] << endl;
    // }    
    

    clock_t end = clock();
    double elapsedTime = (double(end-start)/CLOCKS_PER_SEC);
    cout << "Tiempo de ejecucion: " << elapsedTime << endl;
    return 0;
}
