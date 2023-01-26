#include <algorithm>

// Generate random data
void initRandom(float *values, int N) {
    for (int i=0; i<N; i++)
    {
        // random input values
        values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;
    }
}

// Generate data that gives high relative speedup
void initGood(float *values, int N) {
    for (int i=0; i<N; i++)
    {
        // Todo: Choose values
        values[i] = 1.0f;
    }
}

// Generate data that gives low relative speedup
void initBad(float *values, int N) {
    for (int i=0; i<N; i++)
    {
        // Todo: Choose values
        //values[i] = 1.0f;
        if(i%2 == 0){
            values[i] = 0.625f;
        }
        else{
            values[i] = 173.0f;
        }
    }
}


oid initBad(float *values, int N) {
    for (int i=0; i<N; i++)
    {   
        // Todo: Choose values
        values[i] = 1.0f;
    
        if(i%1433==0){

            values[i] = 173.0f;
        }
        else{
                if(i%343==0){
                        values[i] = 356.0f;
                }
    
    
                else{
                        values[i] = 1.0f;
                }
        }    

    }   
}
