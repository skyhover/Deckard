/*
 * Copyright (c) 2004-2005 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * MIT grants permission to use, copy, modify, and distribute this software and
 * its documentation for NON-COMMERCIAL purposes and without fee, provided that
 * this copyright notice appears in all copies.
 *
 * MIT provides this software "as is," without representations or warranties of
 * any kind, either expressed or implied, including but not limited to the
 * implied warranties of merchantability, fitness for a particular purpose, and
 * noninfringement.  MIT shall not be liable for any damages arising from any
 * use of this software.
 *
 * Author: Alexandr Andoni (andoni@mit.edu), Piotr Indyk (indyk@mit.edu)
 */

#include <sys/time.h>
#include "headers.h"

#define SQR(a) ((a) * (a))

RealT **points;
int nPoints;
RealT *query;
int nQueries;
int dimension;
RealT R;
RealT p;

RealT *listOfRadii = NULL;
IntT nRadii = 0;

// nearNeighbors[i] is the list of the near neighbors.
int *nearNeighbors;

void usage(char *programName){
  printf("Usage: %s #pts_in_data_set #queries dimension successProbability radius data_set_file queries_file\n", programName);
}

RealT norm(int dimension, RealT *p1){
  RealT result = 0;

  for (int i = 0; i < dimension; i++){
    result += SQR(p1[i]);
  }

  return sqrt(result);
}

// Reads in the data set (points and the initial parameters for <R>)
// in the <points> from the file <filename>.
void readPoints(char *filename){
  FILE *f = fopen(filename, "rt");
  //fscanf(f, "%d %d %lf %lf\n", &nPoints, &dimension, &R, &p);
  points = (RealT**)malloc(nPoints * sizeof(RealT));
  for(int i = 0; i < nPoints; i++){
    points[i] = (RealT*)malloc(dimension * sizeof(RealT));
    for(int d = 0; d < dimension; d++){
      FSCANF_REAL(f, &(points[i][d]));
    }
    //printf("norm (%d): %lf\n", i, norm(dimension, points[i]));
  }
}

// Prints the vector <v> of size <size>. The string <s> appears
// in front.
void printRealVector1(char *s, int size, RealT *v){
  printf("%s", s);
  for(int i = 0; i < size; i++){
    if (i > 0){
      printf(" ");
    }
    printf("%lf", v[i]);
  }

  printf("\n");
}

// Returns the Euclidean distance from point <p1> to <p2>.
RealT dist(RealT *p1, RealT *p2){
  RealT result = 0;

  for (int i = 0; i < dimension; i++){
    result += SQR(p1[i] - p2[i]);
  }

  return SQRT(result);
}

// Returns 1 iff the square of the Euclidean distance from point <p1> to <p2> is <=threshold.
int isDistanceSqrLeq(RealT *p1, RealT *p2, RealT threshold){
  RealT result = 0;

  for (int i = 0; i < dimension; i++){
    result += SQR(p1[i] - p2[i]);
  }

  return result <= threshold;
}

int main(int nargs, char **args){
  if (nargs < 7) {
    usage(args[0]);
    exit(1);
  }

  nPoints = atoi(args[1]);
  nQueries = atoi(args[2]);
  dimension = atoi(args[3]);
  p = atof(args[4]);

  char* endPtr[1];
  RealT thresholdR = strtod(args[5], endPtr);
  if (thresholdR == 0 || endPtr[1] == args[5]){
    // The value for R is not specified, instead there is a file
    // specifying multiple R's.
    thresholdR = 0;

    // Read in the file
    FILE *radiiFile = fopen(args[5], "rt");
    FAILIF(radiiFile == NULL);
    fscanf(radiiFile, "%d\n", &nRadii);
    ASSERT(nRadii > 0);
    FAILIF(NULL == (listOfRadii = (RealT*)MALLOC(nRadii * sizeof(RealT))));
    for(IntT i = 0; i < nRadii; i++){
      FSCANF_REAL(radiiFile, &listOfRadii[i]);
      ASSERT(listOfRadii[i] > 0);
      RealT r;
      FSCANF_REAL(radiiFile, &r);
    }
  }else{
    nRadii = 1;
    FAILIF(NULL == (listOfRadii = (RealT*)MALLOC(nRadii * sizeof(RealT))));
    listOfRadii[0] = thresholdR;
  }
  DPRINTF("No. radii: %d\n", nRadii);

  readPoints(args[6]);
  
  nearNeighbors = (int*)malloc(nPoints * sizeof(int));

  FILE *queryFile = fopen(args[7], "rt");
  //fscanf(queryFile, "%d\n", &nQueries);
  query = (RealT*)malloc(dimension * sizeof(RealT));
  printf("nPoints = %d\n", nPoints);
  //printf("nQueries = %d\n", nQueries);
  for(int i = 0; i < nQueries; i++){
    // read in the query point.
    for(int d = 0; d < dimension; d++){
      FSCANF_REAL(queryFile, &(query[d]));
    }
    //printRealVector1("Query: ", dimension, query);

    IntT nNNs;
    for(int r = 0; r < nRadii; r++){
      TimeVarT time = 0;
      nNNs = 0;
      RealT sqrR = SQR(listOfRadii[r]);
      TIMEV_START(time);
      for(int j = 0; j < nPoints; j++){
	if (isDistanceSqrLeq(query, points[j], sqrR)) {
	  nearNeighbors[nNNs] = j;
	  nNNs++;
	}
	//printf("Distance[dist] (%d): %lf\n", j, dist(query, points[j]));
	//printRealVector1("X: ", dimension, points[j]);
      }
      TIMEV_END(time); // time only finding the near neighbors, and exclude printing from timing.
      printf("Total time for R-NN query at radius %0.6lf (radius no. %d):\t%0.6lf\n", (double)(listOfRadii[r]), r, time);

      if (nNNs > 0){
	printf("Query point %d: found %d NNs at distance %0.6lf (radius no. %d). NNs are:\n", i, nNNs, (double)(listOfRadii[r]), r);
	for(int j = 0; j < nNNs; j++){
	  printf("%09d\n", nearNeighbors[j]);
	  //printRealVector1("NN: ", dimension, points[nearNeighbors[j]]);
	}
	break;
      }
    }
    if (nNNs == 0){
      printf("Query point %d: no NNs found.\n", i);
    }
  }
  
}
