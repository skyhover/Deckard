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

/*
  The main entry file containing the main() function. The main()
  function parses the command line parameters and depending on them
  calls the correspondin functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include "headers.h"

#define N_SAMPLE_QUERY_POINTS 100

// The data set containing all the points.
PPointT *dataSetPoints = NULL;
// Number of points in the data set.
IntT nPoints = 0;
// The dimension of the points.
IntT pointsDimension = 0;
// The value of parameter R (a near neighbor of a point <q> is any
// point <p> from the data set that is the within distance
// <thresholdR>).
//RealT thresholdR = 1.0;

// The succes probability of each point (each near neighbor is
// reported by the algorithm with probability <successProbability>).
RealT successProbability = 0.9;

// Same as <thresholdR>, only an array of R's (for the case when
// multiple R's are specified).
RealT *listOfRadii = NULL;
IntT nRadii = 0;

RealT *memRatiosForNNStructs = NULL;

char sBuffer[600000];

/*
  Prints the usage of the LSHMain.
 */
void usage(char *programName){
  printf("Usage: %s #pts_in_data_set #queries dimension successProbability radius data_set_file query_points_file max_available_memory [-c|-p params_file]\n", programName);
}

inline PPointT readPoint(FILE *fileHandle){
  PPointT p;
  RealT sqrLength = 0;
  FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
  FAILIF(NULL == (p->coordinates = (RealT*)MALLOC(pointsDimension * sizeof(RealT))));
  for(IntT d = 0; d < pointsDimension; d++){
    FSCANF_REAL(fileHandle, &(p->coordinates[d]));
    sqrLength += SQR(p->coordinates[d]);
  }
  fscanf(fileHandle, "%[^\n]", sBuffer);
  p->index = -1;
  p->sqrLength = sqrLength;
  return p;
}

// Reads in the data set points from <filename> in the array
// <dataSetPoints>. Each point get a unique number in the field
// <index> to be easily indentifiable.
void readDataSetFromFile(char *filename){
  FILE *f = fopen(filename, "rt");
  FAILIF(f == NULL);
  
  //fscanf(f, "%d %d ", &nPoints, &pointsDimension);
  //FSCANF_DOUBLE(f, &thresholdR);
  //FSCANF_DOUBLE(f, &successProbability);
  //fscanf(f, "\n");
  FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));
  for(IntT i = 0; i < nPoints; i++){
    dataSetPoints[i] = readPoint(f);
    dataSetPoints[i]->index = i;
  }
}

// Tranforming <memRatiosForNNStructs> from
// <memRatiosForNNStructs[i]=ratio of mem/total mem> to
// <memRatiosForNNStructs[i]=ratio of mem/mem left for structs i,i+1,...>.
void transformMemRatios(){
  RealT sum = 0;
  for(IntT i = nRadii - 1; i >= 0; i--){
    sum += memRatiosForNNStructs[i];
    memRatiosForNNStructs[i] = memRatiosForNNStructs[i] / sum;
    //DPRINTF("%0.6lf\n", memRatiosForNNStructs[i]);
  }
  ASSERT(sum <= 1.000001);
}


int compareInt32T(const void *a, const void *b){
  Int32T *x = (Int32T*)a;
  Int32T *y = (Int32T*)b;
  return (*x > *y) - (*x < *y);
}

/*
  The main entry to LSH package. Depending on the command line
  parameters, the function computes the R-NN data structure optimal
  parameters and/or construct the R-NN data structure and runs the
  queries on the data structure.
 */
int main(int nargs, char **args){
  if(nargs < 9){
    usage(args[0]);
    exit(1);
  }

  //initializeLSHGlobal();

  // Parse part of the command-line parameters.
  nPoints = atoi(args[1]);
  IntT nQueries = atoi(args[2]);
  pointsDimension = atoi(args[3]);
  successProbability = atof(args[4]);
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
    FAILIF(NULL == (memRatiosForNNStructs = (RealT*)MALLOC(nRadii * sizeof(RealT))));
    for(IntT i = 0; i < nRadii; i++){
      FSCANF_REAL(radiiFile, &listOfRadii[i]);
      ASSERT(listOfRadii[i] > 0);
      FSCANF_REAL(radiiFile, &memRatiosForNNStructs[i]);
      ASSERT(memRatiosForNNStructs[i] > 0);
    }
  }else{
    nRadii = 1;
    FAILIF(NULL == (listOfRadii = (RealT*)MALLOC(nRadii * sizeof(RealT))));
    FAILIF(NULL == (memRatiosForNNStructs = (RealT*)MALLOC(nRadii * sizeof(RealT))));
    listOfRadii[0] = thresholdR;
    memRatiosForNNStructs[0] = 1;
  }
  DPRINTF("No. radii: %d\n", nRadii);
  //thresholdR = atof(args[5]);
  availableTotalMemory = atol(args[8]);

  if (nPoints > MAX_N_POINTS) {
    printf("Error: the structure supports at most %d points (%d were specified).\n", MAX_N_POINTS, nPoints);
    fprintf(ERROR_OUTPUT, "Error: the structure supports at most %d points (%d were specified).\n", MAX_N_POINTS, nPoints);
    exit(1);
  }

  readDataSetFromFile(args[6]);
  DPRINTF("Allocated memory (after reading data set): %d\n", totalAllocatedMemory);

  Int32T nSampleQueries = N_SAMPLE_QUERY_POINTS;
  PPointT sampleQueries[nSampleQueries];
  Int32T sampleQBoundaryIndeces[nSampleQueries];
  if ((nargs < 9) || (strcmp("-c", args[9]) == 0)){
    // In this cases, we need to generate a sample query set for
    // computing the optimal parameters.

    // Generate a sample query set.
    FILE *queryFile = fopen(args[7], "rt");
    if (strcmp(args[7], ".") == 0 || queryFile == NULL || nQueries <= 0){
      // Choose several data set points for the sample query points.
      for(IntT i = 0; i < nSampleQueries; i++){
	sampleQueries[i] = dataSetPoints[genRandomInt(0, nPoints - 1)];
      }
    }else{
      // Choose several actual query points for the sample query points.
      nSampleQueries = MIN(nSampleQueries, nQueries);
      Int32T sampleIndeces[nSampleQueries];
      for(IntT i = 0; i < nSampleQueries; i++){
	sampleIndeces[i] = genRandomInt(0, nQueries - 1);
      }
      qsort(sampleIndeces, nSampleQueries, sizeof(*sampleQueries), compareInt32T);
      //printIntVector("sampleIndeces: ", nSampleQueries, sampleIndeces);
      Int32T j = 0;
      for(Int32T i = 0; i < nQueries; i++){
	if (i == sampleIndeces[j]){
	  sampleQueries[j] = readPoint(queryFile);
	  j++;
	  while (i == sampleIndeces[j]){
	    sampleQueries[j] = sampleQueries[j - 1];
	    j++;
	  }
	}else{
	  fscanf(queryFile, "%[^\n]", sBuffer);
	  fscanf(queryFile, "\n");
	}
      }
      nSampleQueries = j;
      fclose(queryFile);
    }

    // Compute the array sampleQBoundaryIndeces that specifies how to
    // segregate the sample query points according to their distance
    // to NN.
    sortQueryPointsByRadii(pointsDimension,
			   nSampleQueries,
			   sampleQueries,
			   nPoints,
			   dataSetPoints,
			   nRadii,
			   listOfRadii,
			   sampleQBoundaryIndeces);
  }

  RNNParametersT *algParameters = NULL;
  PRNearNeighborStructT *nnStructs = NULL;
  if (nargs > 9) {
    // Additional command-line parameter is specified.
    if (strcmp("-c", args[9]) == 0) {
      // Only compute the R-NN DS parameters and output them to stdout.
      
      printf("%d\n", nRadii);
      transformMemRatios();
      for(IntT i = 0; i < nRadii; i++){
	// which sample queries to use
	Int32T segregatedQStart = (i == 0) ? 0 : sampleQBoundaryIndeces[i - 1];
	Int32T segregatedQNumber = nSampleQueries - segregatedQStart;
	if (segregatedQNumber == 0) {
	  // XXX: not the right answer
	  segregatedQNumber = nSampleQueries;
	  segregatedQStart = 0;
	}
	ASSERT(segregatedQStart < nSampleQueries);
	ASSERT(segregatedQStart >= 0);
	ASSERT(segregatedQStart + segregatedQNumber <= nSampleQueries);
	ASSERT(segregatedQNumber >= 0);
	RNNParametersT optParameters = computeOptimalParameters(listOfRadii[i],
								successProbability,
								nPoints,
								pointsDimension,
								dataSetPoints,
								segregatedQNumber,
								sampleQueries + segregatedQStart,
								(Uns32T)((availableTotalMemory - totalAllocatedMemory) * memRatiosForNNStructs[i]));
	printRNNParameters(stdout, optParameters);
      }
      exit(0);
    } else if (strcmp("-p", args[9]) == 0) {
      // Read the R-NN DS parameters from the given file and run the
      // queries on the constructed data structure.
      if (nargs < 10){
	usage(args[0]);
	exit(1);
      }
      FILE *pFile = fopen(args[10], "rt");
      FAILIFWR(pFile == NULL, "Could not open the params file.");
      fscanf(pFile, "%d\n", &nRadii);
      DPRINTF1("Using the following R-NN DS parameters:\n");
      DPRINTF("N radii = %d\n", nRadii);
      FAILIF(NULL == (nnStructs = (PRNearNeighborStructT*)MALLOC(nRadii * sizeof(PRNearNeighborStructT))));
      FAILIF(NULL == (algParameters = (RNNParametersT*)MALLOC(nRadii * sizeof(RNNParametersT))));
      for(IntT i = 0; i < nRadii; i++){
	algParameters[i] = readRNNParameters(pFile);
	printRNNParameters(stderr, algParameters[i]);
	nnStructs[i] = initLSH_WithDataSet(algParameters[i], nPoints, dataSetPoints);
      }

      pointsDimension = algParameters[0].dimension;
      FREE(listOfRadii);
      FAILIF(NULL == (listOfRadii = (RealT*)MALLOC(nRadii * sizeof(RealT))));
      for(IntT i = 0; i < nRadii; i++){
	listOfRadii[i] = algParameters[i].parameterR;
      }
    } else{
      // Wrong option.
      usage(args[0]);
      exit(1);
    }
  } else {
    FAILIF(NULL == (nnStructs = (PRNearNeighborStructT*)MALLOC(nRadii * sizeof(PRNearNeighborStructT))));
    // Determine the R-NN DS parameters, construct the DS and run the queries.
    transformMemRatios();
    for(IntT i = 0; i < nRadii; i++){
      // XXX: segregate the sample queries...
      nnStructs[i] = initSelfTunedRNearNeighborWithDataSet(listOfRadii[i], 
							   successProbability, 
							   nPoints, 
							   pointsDimension, 
							   dataSetPoints, 
							   nSampleQueries, 
							   sampleQueries, 
							   (Uns32T)((availableTotalMemory - totalAllocatedMemory) * memRatiosForNNStructs[i]));
    }
  }

  DPRINTF1("X\n");

  IntT resultSize = nPoints;
  PPointT *result = (PPointT*)MALLOC(resultSize * sizeof(*result));
  PPointT queryPoint;
  FAILIF(NULL == (queryPoint = (PPointT)MALLOC(sizeof(PointT))));
  FAILIF(NULL == (queryPoint->coordinates = (RealT*)MALLOC(pointsDimension * sizeof(RealT))));

  FILE *queryFile = fopen(args[7], "rt");
  FAILIF(queryFile == NULL);
  TimeVarT meanQueryTime = 0;
  for(IntT i = 0; i < nQueries; i++){

    RealT sqrLength = 0;
    // read in the query point.
    for(IntT d = 0; d < pointsDimension; d++){
      FSCANF_REAL(queryFile, &(queryPoint->coordinates[d]));
      sqrLength += SQR(queryPoint->coordinates[d]);
    }
    queryPoint->sqrLength = sqrLength;
    //printRealVector("Query: ", pointsDimension, queryPoint->coordinates);

    // get the near neighbors.
    IntT nNNs = 0;
    for(IntT r = 0; r < nRadii; r++){
      nNNs = getRNearNeighbors(nnStructs[r], queryPoint, result, resultSize);
      printf("Total time for R-NN query at radius %0.6lf (radius no. %d):\t%0.6lf\n", (double)(listOfRadii[r]), r, timeRNNQuery);
      meanQueryTime += timeRNNQuery;

      if (nNNs > 0){
	printf("Query point %d: found %d NNs at distance %0.6lf (radius no. %d). NNs are:\n", i, nNNs, (double)(listOfRadii[r]), r);
	for(IntT j = 0; j < nNNs; j++){
	  ASSERT(result[j] != NULL);
	  printf("%09d\tdist:%0.6lf\n", result[j]->index, distance(pointsDimension, queryPoint, result[j]));
	  CR_ASSERT(distance(pointsDimension, queryPoint, result[j]) <= listOfRadii[r]);
	  //DPRINTF("Distance: %lf\n", distance(pointsDimension, queryPoint, result[j]));
	  //printRealVector("NN: ", pointsDimension, result[j]->coordinates);
	}
	break;
      }
    }
    if (nNNs == 0){
      printf("Query point %d: no NNs found.\n", i);
    }
  }
  if (nQueries > 0){
    meanQueryTime = meanQueryTime / nQueries;
    printf("Mean query time: %0.6lf\n", (double)meanQueryTime);
  }

  //freePRNearNeighborStruct(nnStruct);

  return 0;
}
