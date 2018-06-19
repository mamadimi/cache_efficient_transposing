
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
#include "testingStruct.h"

#ifndef __cilk
#include <cilk/cilk_stub.h>
#endif
#include <cilk/cilk.h>

#define ITERATIONS 4

using namespace std;

template< class T > void simple_transposing_bucketing(T *Ain,T *Aout,int *permutationVector,int n,int buckets){

    int sizeOfSubBuckets = n/buckets;
    if(buckets==1){
            cilk_for(int i=0;i<n;i++){
                memcpy(&Aout[permutationVector[i]],&Ain[i],sizeof(T));
            }
    }
    else{
        for(int j=0;j<buckets;j++){

                cilk_for(int i=j*sizeOfSubBuckets;i<(j+1)*sizeOfSubBuckets;i++){
                    memcpy(&Aout[permutationVector[i]],&Ain[i],sizeof(T));
                }
        }
    }
}

int decideBucketsForStage3(int N){
    /*
     * The return value is an experimental value.
     */

    if(N<=pow(2,23)) return 1;
    if(N<=pow(2,24)) return 32;
    if(N<=pow(2,25)) return 256;
    if(N<=pow(2,26)) return 512;
}
void read_from_file(int *X, char *filename, int N){

    FILE *fp = fopen(filename, "r+");

    int size = fread(X, sizeof(int), N, fp);

    fclose(fp);

}

double improvedSerialReadFromInput(testingStruct *Ain, testingStruct *Aout, int *permutationVector,int n,int number_of_buckets,int* internalSubBucketsSizes){

    int oneBucketElements = (int)(n/number_of_buckets);

    int *intermediatePermutationVector = new int[n];
    testingStruct *AIntermediate = new testingStruct[n];

    int oneSubBucketElements = n/(number_of_buckets*number_of_buckets);

    timeval startwtime, endwtime;
    double duration=0;

    gettimeofday (&startwtime, NULL);

    //FIRST STAGE - Rearrange elements into correct buckets

    /**
     * Every bucket is splitted in number_of_buckets subbuckets and data are sorted in order to be in correct places.
     */

        cilk_for(int j=0;j<number_of_buckets;j++){

            int subBucketSize = oneBucketElements/number_of_buckets ;

            int *subBucketsStarts = new int[number_of_buckets];

            for(int i=0;i<number_of_buckets;i++){
                subBucketsStarts[i] = j*oneBucketElements + i*subBucketSize;
            }

            testingStruct *curr_bucket = &Ain[j*oneBucketElements];
            int *permutPoiterIN = &permutationVector[j*oneBucketElements];

            for(int i=0;i<oneBucketElements;i++){
              
              // For every element in bucket identify the correct bin that nedds to go
              // If some sub buckets are full, put the element in the nearest subbuckets with space.
                int bin = (int)(permutPoiterIN[i]/oneBucketElements);

                if( (subBucketsStarts[ bin ] - j*oneBucketElements) / subBucketSize >= bin+1){
                    for(int k=1;;k++){
                        if(bin+k < number_of_buckets ){
                            int temp_bin = bin + k;
                            if( (subBucketsStarts[ temp_bin ] - j*oneBucketElements) / subBucketSize < temp_bin+1){
                                bin=temp_bin;
                                break;
                            }
                        }
                        if(bin-k >= 0 ){
                            int temp_bin = bin - k;
                            if( (subBucketsStarts[ temp_bin ] - j*oneBucketElements) / subBucketSize < temp_bin+1){
                                bin=temp_bin;
                                break;
                            }
                        }
                    }
                }

                int pos = subBucketsStarts[ bin ] ;

                AIntermediate[ pos ] = curr_bucket[i];

                intermediatePermutationVector[ pos ] = permutPoiterIN[i];

                ++subBucketsStarts[bin];
            }

            delete[] subBucketsStarts;
        }


    //SECOND STAGE - Move the subbuckets to the correct bucket

        cilk_for(int i=0;i<number_of_buckets-1;i++){

            int *permtemp = new int[oneSubBucketElements];
            testingStruct *Atemp = new testingStruct[oneSubBucketElements];

            //swap with other buckets

            for(int j=i+1;j<number_of_buckets;j++){

                memcpy(Atemp, &AIntermediate[j*oneBucketElements+i*oneSubBucketElements] , oneSubBucketElements*sizeof(testingStruct));

                memcpy(&AIntermediate[j*oneBucketElements+i*oneSubBucketElements] , &AIntermediate[i*oneBucketElements+j*oneSubBucketElements], oneSubBucketElements*sizeof(testingStruct));

                memcpy(&AIntermediate[i*oneBucketElements+j*oneSubBucketElements], Atemp, oneSubBucketElements*sizeof(testingStruct));

                memcpy(permtemp, &intermediatePermutationVector[j*oneBucketElements+i*oneSubBucketElements] , oneSubBucketElements*sizeof(int));

                memcpy(&intermediatePermutationVector[j*oneBucketElements+i*oneSubBucketElements] , &intermediatePermutationVector[i*oneBucketElements+j*oneSubBucketElements], oneSubBucketElements*sizeof(int));

                memcpy(&intermediatePermutationVector[i*oneBucketElements+j*oneSubBucketElements], permtemp, oneSubBucketElements*sizeof(int));

            }

            delete[] permtemp;
            delete[] Atemp;

        }

    //THIRD STAGE - Apply the classic method to permutate every subbucket correctly.

    simple_transposing_bucketing(AIntermediate,Aout,intermediatePermutationVector,n,decideBucketsForStage3(n));

    gettimeofday (&endwtime, NULL);

    duration = (double)((endwtime.tv_usec - startwtime.tv_usec)/1.0e6
    + endwtime.tv_sec - startwtime.tv_sec);


    delete[] intermediatePermutationVector;
    delete[] AIntermediate;

    return duration;

}

int main(int argc, char *argv[]){


    srand (time(NULL));

    unsigned long int n = atoi(argv[1]);

    unsigned long int  number_of_buckets = atoi(argv[2]);

    double averageDuration=0;


    for(int reps=0;reps<ITERATIONS;reps++){

        testingStruct *Ain = new testingStruct[n];
        testingStruct *Aout = new testingStruct[n];

        int *permutationVector = new int[n];
        int *internalSubBucketsSizes= new int[number_of_buckets*number_of_buckets];

        for(int i=0;i<number_of_buckets*number_of_buckets;i++){
            internalSubBucketsSizes[i] = 0;
        }

        read_from_file(permutationVector, (char*)"permutationVector.txt", n);

        cilk_for(int i=0;i<n;i++){
            Ain[i].c = i;
        }

        double duration=0;

        duration = improvedSerialReadFromInput(Ain,Aout,permutationVector,n,number_of_buckets,internalSubBucketsSizes);

        averageDuration+=duration;

        delete[] Ain;
        delete[] Aout;
        delete[] permutationVector;
        delete[] internalSubBucketsSizes;

    }

    averageDuration=averageDuration/ITERATIONS;

    cout<<averageDuration<<endl;

    return 0;
}
