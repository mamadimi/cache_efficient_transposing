#include "stdlib.h"
#include "iostream"
#include <algorithm>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <math.h>

using namespace std;

/**
 * Create a random permutation vector by swaping randomly two matrix's indexes
 *
 */
int shuffle(int *permutationVector,float permutationPercentage, int n){
    //Init permutationVector
    if(n<=1) return 0;

    if((permutationPercentage*n)>=n){
        random_shuffle(&permutationVector[0],&permutationVector[n]);
        return 0;
    }
    bool *isSwaped = new bool[n] ;

    for(int i=0;i<n;i++){
        isSwaped[i] = false;
    }

    int countSwaps = 0;

    for(;;){
        int swap1 = rand()%n;
        int swap2 = rand()%n;

        if((swap1!=swap2) && isSwaped[swap1]==false && isSwaped[swap2]==false ){
            int temp = permutationVector[swap1];
            permutationVector[swap1] = permutationVector[swap2];
            permutationVector[swap2] = temp;

            isSwaped[swap1] = true;
            isSwaped[swap2] = true;

            countSwaps+=2;

            if (countSwaps>=(int)(permutationPercentage*n)) break;
        }
    }

    delete [] isSwaped;

    return 0;

}

void save_table(int *X, int N){

  FILE *fp;

  char filename[200];

  sprintf(filename, "permutationVector.txt");

  fp = fopen(filename, "w+");

  fwrite(X, sizeof(int), N, fp);

  fclose(fp);

}


int main(int argc, char *argv[]){

    srand (time(NULL));

    unsigned long int  n = atoi(argv[1]);

    int localPermBuckets = pow(10,atoi(argv[2])); //The buckets of permutation vector on which the permutations are strigthly local.

    float permutationPercentage = (float)(atoi(argv[3]))/100; //The percentage of the matrix that is actually permutated.

    int *permutationVector = new int[n];

    for(int i=0;i<n;i++){
        permutationVector[i] = i;
    }

    //Create a random shuffle vector

    for(int i=0;i<localPermBuckets;i++){
         shuffle(&permutationVector[i*(n/localPermBuckets)],permutationPercentage,(int)(n/localPermBuckets));
    }

    save_table(permutationVector,n);

    delete[] permutationVector;

    return 0;
}
