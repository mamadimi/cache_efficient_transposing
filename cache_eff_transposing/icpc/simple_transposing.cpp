#include "stdlib.h"
#include "iostream"
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "testStruct.h"

#ifndef __cilk
#include <cilk/cilk_stub.h>
#endif
#include <cilk/cilk.h>

#define ITERATIONS 4

using namespace std;

void read_from_file(int *X, char *filename, int N){

    FILE *fp = fopen(filename, "r+");

    int size = fread(X, sizeof(int), N, fp);

    fclose(fp);

}

void simple_transposing(rdWrCheckStr *Ain,rdWrCheckStr *Aout,int *permutationVector,int n){


    cilk_for(int i=0;i<n;i++){
        memcpy(&Aout[permutationVector[i]],&Ain[i],sizeof(rdWrCheckStr));

    }

}

int main(int argc, char *argv[]){

    srand (time(NULL));

    unsigned long int n = atoi(argv[1]);

    rdWrCheckStr *Ain = new rdWrCheckStr[n];
    rdWrCheckStr *Aout = new rdWrCheckStr[n];

    int *permutationVector = new int[n];

    //Initialize Ain & Aout
    for(int i=0;i<n;i++){

        Ain[i].c = i;
        Aout[i].c = i;
    }

    //Insert permutationVector from a txt file
    read_from_file(permutationVector, (char*)"permutationVector.txt", n);

    timeval startwtime, endwtime;
    double duration;

    double averageDuration=0;

    for(int reps=0;reps<ITERATIONS;reps++){

        read_from_file(permutationVector, (char*)"permutationVector.txt", n);

        gettimeofday (&startwtime, NULL);

        simple_transposing(Ain,Aout,permutationVector,n);

        gettimeofday (&endwtime, NULL);

        duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
        + endwtime.tv_sec - startwtime.tv_sec);

        averageDuration+=duration;

    }

    averageDuration=averageDuration/ITERATIONS;

    if(duration>0) cout<<averageDuration<<endl;

    delete[] Ain;
    delete[] Aout;
    delete[] permutationVector;

    return 0;
}
