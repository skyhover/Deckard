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
 * Modified by: Stephane Glondu (stephane.glondu@dptinfo.ens-cachan.fr)
 * Modified by: Lingxiao Jiang (lxjiang@ucdavis.edu, lxjiang@smu.edu.sg)
 */

/*
  The main entry file containing the main() function. The main()
  function parses the command line parameters and depending on them
  calls the corresponding functions.
 */

#include <cstdio>
#include <cstdlib>
#include <sys/times.h>
#include <sys/types.h>
#include <cctype>
#include <regex.h>
#include <unistd.h>
#include "headers.h"

#define N_SAMPLE_QUERY_POINTS 100

static int DEBUG_LEVEL = 1;

// The data set containing all the points.
PPointT *dataSetPoints = NULL;
PPointT *queryDataSetPoints = NULL;
// Number of points in the data set.
IntT nPoints = 0;
IntT nQueryPoints = 0;

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

regex_t preg[ENUM_PPROP_LAST_NOT_USED];

// Will merge prefetch consecutive vectors from input file
int prefetch = 1;

// The second heuristic for detecting bugs.
// return true when there are inconsistent changes.
bool inconsistentIDchanges(char *v1, char *v2)
{
  regex_t id;
  regmatch_t pmatch[1];
  //char *id1, *id2;
  int nChanged=0, nUnchanged=0;

  if ( v1==v2 )
    return false;
  else if ( v1==NULL || v2==NULL )
    return true;

  FAILIF(0 != regcomp(&id, "([^,}[:blank:]]+)", REG_EXTENDED));

  int a, b, c, d;
  while ( regexec(&id, v1, 1, pmatch, 0) == 0 &&
       (a=pmatch[0].rm_so) != -1 ) {
    char t1 = v1[(b=pmatch[0].rm_eo)];
    v1[b] = '\0';
    if ( regexec(&id, v2, 1, pmatch, 0) == 0 &&
	 (c=pmatch[0].rm_so) != -1 ) {
      char t2 = v2[(d=pmatch[0].rm_eo)];
      v2[d] = '\0';
      if ( strcmp(v1, v2)==0 )
	nUnchanged++;
      else
	nChanged++;
      v2[d] = t2;
      v1[b] = t1;
      v2 += d;
      v1 += b;
    } else {
      nChanged++;
      v1[b] = t1;
      v1 += b;
      break;
    }
  }
  while ( regexec(&id, v1, 1, pmatch, 0) == 0 &&
          (a=pmatch[0].rm_so) != -1 ) {
    nChanged++;
    v1 += pmatch[0].rm_eo;
  }

  while ( regexec(&id, v2, 1, pmatch, 0) == 0 &&
	  (c=pmatch[0].rm_so) != -1 ) {
    nChanged++;
    v2 += pmatch[0].rm_eo;
  }

  // need quantified condition here.

  return false;
}

/*
  Prints the usage of the LSHMain.
 */
void usage(int code, const char *programName) {
  printf("Usage: %s [options: see source code] data_set_file [params_file]\n", programName);
  exit(code);
}

struct CommandLineParameters {
   bool computeParameters;
   char *paramsFile;
   // Parameters for filtering:
   bool no_filtering;
   bool bug_detecting;
   int upperBound;
   int lowerBound;
   int minNumNodes;
   int min_nVars;
   int max_num_diff_vars;
   float max_num_diff_nVars_diff;
   float max_nVars_diff;
   bool interfiles;
   int min_lines;
   CommandLineParameters();
};

CommandLineParameters::CommandLineParameters()
{
   computeParameters = false;
   paramsFile = NULL;
   no_filtering = false;
   bug_detecting = true;
   upperBound = 0;
   lowerBound = 2;
   minNumNodes = 0;
   min_nVars = 0;
   max_num_diff_vars = 16;
   max_num_diff_nVars_diff = 0.5;
   max_nVars_diff = 0.35;
   interfiles = false;
   min_lines = 0;
}

void printPoint(FILE* out, const PPointT p)
{
      fprintf(out, "FILE %s LINE:%d:%d NODE_KIND:%d nVARs:%d NUM_NODE:%d TBID:%d TEID:%d\n",
            p->filename, p->prop[ENUM_PPROP_LINE-1], p->prop[ENUM_PPROP_OFFSET-1],
            p->prop[ENUM_PPROP_NODE_KIND-1], p->prop[ENUM_PPROP_nVARs-1],
            p->prop[ENUM_PPROP_NUM_NODE-1], p->prop[ENUM_PPROP_TBID-1], p->prop[ENUM_PPROP_TEID-1]);
}

inline PPointT readPoint2(char *line, char *comment){
  
  PPointT p;
  RealT sqrLength = 0;
  FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
  FAILIF(NULL == (p->coordinates = (RealT*)MALLOC(pointsDimension * sizeof(RealT))));
  IntT d;
  char *t;

  if (comment != NULL) {
    int a, b;
    regmatch_t pmatch[2];
     
    if (regexec(&preg[ENUM_PPROP_FILE], comment, 2, pmatch, 0) == 0 &&
	(a = pmatch[1].rm_so) != -1) {
      b = pmatch[1].rm_eo;
      FAILIF(NULL == (p->filename = (char*)MALLOC(b-a+1)));
      memmove(p->filename, comment + a, b-a);
      p->filename[b-a] = '\0';
    }
   
    for (int i = 1; i < ENUM_PPROP_LAST_NOT_USED; i++) {
      if (regexec(&preg[i], comment, 2, pmatch, 0) == 0 &&
	  (a = pmatch[1].rm_so) != -1) {
	b = pmatch[1].rm_eo;
	char t = comment[b];
	comment[b] = '\0';
	p->prop[i-1] = atoi(comment + a);
	comment[b] = t;
	if ( i==ENUM_PPROP_OIDs ) {
	  // memeory bottleneck now
// 	  int c = pmatch[0].rm_so, d = pmatch[0].rm_eo;
// 	  FAILIF(NULL == (p->oids = (char*)MALLOC(d-c+1)));
// 	  memmove(p->oids, comment + c, d-c);
// 	  p->oids[d-c] = '\0';
	  p->oids = NULL;
	}
      } else {
	p->prop[i-1] = 0;
	if ( i==ENUM_PPROP_OIDs )
	  p->oids = NULL;
      }
    }

    p->prop[ENUM_PPROP_OFFSET-1] = p->prop[ENUM_PPROP_OFFSET-1] - p->prop[ENUM_PPROP_LINE-1] +1; // the line range.
  }

  for (d = 0, t = line; *t != '\0' && d < pointsDimension; d++) {
    while ( !isdigit(*t) && *t != '\0' && *t != '.') t++;
    p->coordinates[d] = strtof(t, &t); // TOFIX: certain versions of gcc have bugs for strtof.
    sqrLength += SQR(p->coordinates[d]);
  }

  p->index = -1;
  p->sqrLength = sqrLength;
  return p;
}

// Linked list structure for PPointT
typedef struct TPPointTList_s TPPointTList;
struct TPPointTList_s {
  PPointT hd;
  TPPointTList *tl;
};

// Reads in the data set points from <filename> into the array
// <dsPoints>, and set dNum and dDim (if was zero). Each point get a unique number in the field
// <index> (within this file only) to be easily identifiable.
void readDataSetFromFile2(const char *filename, PPointT*(&dsPoints), IntT& dNum, IntT& dDim)
{
  int prefetchSize = 0;
  TPPointTList *prefetchStart;		// beginning of prefetch queue
  TPPointTList *prefetchEnd;		// new cell at the end of prefetch queue
  TPPointTList *pointsStart;		// beginnning of point list
  FAILIF(NULL == (prefetchEnd = (TPPointTList*)MALLOC(sizeof(TPPointTList))));
  prefetchEnd->tl = NULL;
  pointsStart = prefetchStart = prefetchEnd;

  dNum = 0;

  FILE *f = fopen(filename, "rt");
  FAILIF(f == NULL);
  
  char *line = NULL, *comment = NULL;
  size_t bufferLength = 0;
  ssize_t lineLength;

  while ((lineLength = getline(&line, &bufferLength, f)) > 0) {
    if (line[0] == ';' ) {
      // such a line is no use and could cause problems for the code
      // which auto-detects the number of points now. -Pi 10/30/05. TODO
      fprintf(stderr, "Warning: no fully-supported format around line %d\n", dNum*2);
      continue;
    } else if (line[0] == '#') {
      // the line is a comment
      if (comment != NULL)
         free(comment);
      comment = line;
      if (comment[lineLength-1] == '\n')
         comment[lineLength-1] = '\0';
      line = NULL;
      bufferLength = 0;
    } else {
      // the line is a point
      if (dDim == 0) {
         // compute the dimension
         char *p = line;
         while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
         while (*p != '\0') {
           while (*p != ' ' && *p!='\t' && *p!='\r' && *p!='\n' && *p != '\0') p++;
           dDim++;
           while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
         }
      }

      // add the new point to the prefetch queue
      prefetchEnd->hd = readPoint2(line, comment);

      if (prefetchSize < prefetch)
         prefetchSize++;
      if (prefetchSize == prefetch) {
         dNum++;
         // sum up list prefetch into the prefetchStart->hd.
         // Performance problem here - Pi
         for (TPPointTList *i = prefetchStart->tl; i != NULL; i = i->tl) {
           for (int j = 0; j < dDim; j++) {
             prefetchStart->hd->coordinates[j] += i->hd->coordinates[j];
           }
           prefetchStart->hd->prop[ENUM_PPROP_FILE-1] += i->hd->prop[ENUM_PPROP_FILE-1];
         }
         // allocate the next cell and move the prefetch window
         FAILIF(NULL == (prefetchEnd->tl = (TPPointTList*)MALLOC(sizeof(TPPointTList))));
         prefetchEnd = prefetchEnd->tl;
         prefetchEnd->tl = NULL;
         prefetchStart = prefetchStart->tl;
      } else {
         ASSERT(prefetchSize < prefetch);
         // allocate the next cell
         FAILIF(NULL == (prefetchEnd->tl = (TPPointTList*)MALLOC(sizeof(TPPointTList))));
         prefetchEnd = prefetchEnd->tl;
         prefetchEnd->tl = NULL;
      }
    } // end of new point handling
  } // end of file
  fclose(f);

  FAILIFWR(dNum<=0, "No data point in the input file?");
  // put the points in the array and free the point list
  FAILIF(NULL == (dsPoints = (PPointT*)MALLOC(dNum * sizeof(PPointT))));
  for(IntT i = 0; i < dNum; i++) {
    ASSERT(pointsStart != NULL);
    dsPoints[i] = pointsStart->hd;
    dsPoints[i]->index = i;
    TPPointTList *cur = pointsStart->tl;
    free(pointsStart);
    pointsStart = cur;
  } // dNum == the number of total points - prefetch + 1
  if (comment != NULL)
     free(comment);

  // free the remaining prefetch queue
  while (pointsStart != NULL) {
    TPPointTList *cur = pointsStart->tl;
    free(pointsStart);
    pointsStart = cur;
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

#define ENUM_BUCKETS

#define CHECK_INT(v) { \
  if (v <= 0) { \
    fprintf(stderr, "Incorrect integer value for variable %s\n", #v); \
    usage(1, argv[0]); \
  }}
#define CHECK_FLOAT(v) { \
  if (v < 1e-3) { \
    fprintf(stderr, "Incorrect float value for variable %s\n", #v); \
    usage(1, argv[0]); \
  }}

RNNParametersT *algParameters = NULL;
PRNearNeighborStructT *nnStructs = NULL;

bool readParamsFile(const char *paramsFile)
{
  FILE *pFile = fopen(paramsFile, "rt");
  if (pFile == NULL) {
    fprintf(stderr,
	    "Warning: could not open %s, will try to compute parameters "
	    "and write them to that file\n", paramsFile);
    return true;
  } else {
    fscanf(pFile, "%d\n", &nRadii);
    fprintf(stderr, "Using the following R-NN DS parameters (from %s):\n", paramsFile);
    fprintf(stderr, "N radii = %d, nPoints = %d\n", nRadii, nPoints);
    FAILIF(NULL == (nnStructs = (PRNearNeighborStructT*)MALLOC(nRadii * sizeof(PRNearNeighborStructT))));
    FAILIF(NULL == (algParameters = (RNNParametersT*)MALLOC(nRadii * sizeof(RNNParametersT))));
    for(IntT i = 0; i < nRadii; i++){
      algParameters[i] = readRNNParameters(pFile);
      printRNNParameters(stderr, algParameters[i]);
      nnStructs[i] = initLSH_WithDataSet(algParameters[i], nPoints, dataSetPoints);
    }
    
    pointsDimension = algParameters[0].dimension;
    if (listOfRadii != NULL) FREE(listOfRadii);
    FAILIF(NULL == (listOfRadii = (RealT*)MALLOC(nRadii * sizeof(RealT))));
    for(IntT i = 0; i < nRadii; i++){
      listOfRadii[i] = algParameters[i].parameterR;
    }
    return false;
  }
}

bool pointIsNotFiltered(PPointT* p, const CommandLineParameters& comParam)
{
   return (*(p))->prop[ENUM_PPROP_NUM_NODE-1] >= comParam.minNumNodes &&
         (*(p))->prop[ENUM_PPROP_nVARs-1] >= comParam.min_nVars &&
         (*(p))->prop[ENUM_PPROP_OFFSET-1] >= comParam.min_lines;
}

int comparePoints(const void *p1, const void *p2)
{
  PPointT a = *(PPointT*)p1;
  PPointT b = *(PPointT*)p2;
  int c = strcmp(a->filename, b->filename);
  if (c)
    return c;
  else if ( a->prop[ENUM_PPROP_nVARs-1] != b->prop[ENUM_PPROP_nVARs-1] )
    return a->prop[ENUM_PPROP_nVARs-1] - b->prop[ENUM_PPROP_nVARs-1];
  else
    return a->prop[ENUM_PPROP_LINE-1] - b->prop[ENUM_PPROP_LINE-1];
}

void enumBuckets(const CommandLineParameters & comParam, PPointT* dsPoints, int dNum, int dDim)
{
  PPointT *result = NULL; // will be updated by getRNearNeighbors
  IntT resultSize = 0; // will be updated by getRNearNeighbors
  PPointT queryPoint = NULL;
  /* PI: why need to allocate memory for queryPoint?
  FAILIF(NULL == (queryPoint = (PPointT)MALLOC(sizeof(PointT))));
  FAILIF(NULL == (queryPoint->coordinates = (RealT*)MALLOC(dDim * sizeof(RealT))));
  */

  TimeVarT meanQueryTime = 0;
  int nQueries = 0;
  bool seen[dNum];
  int nBuckets = 0, nBucketedPoints = 0;

  memset(seen, 0, dNum * sizeof(bool));
  for(IntT i = 0; i < dNum; nQueries++, i++) {
    // find the next unseen point
    while (i < dNum && seen[i])
       i++;
    if (i >= dNum)
       break;
    queryPoint = dsPoints[i];

    // get the near neighbors.
    IntT nNNs = 0;
    for(IntT r = 0; r < nRadii; r++) { // nRadii is always 1 so far.
      nNNs = getRNearNeighbors(nnStructs[r], queryPoint, result, resultSize);
      //printf("Total time for R-NN query at radius %0.6lf (radius no. %d):\t%0.6lf\n", (double)(listOfRadii[r]), r, timeRNNQuery);
      meanQueryTime += timeRNNQuery;

      //printf("\nQuery point %d: found %d NNs at distance %0.6lf (radius no. %d). NNs are:\n",
      //       i, nNNs, (double)(listOfRadii[r]), r);

      // sort by filename, then number of variables, then line number
      qsort(result, nNNs, sizeof(*result), comparePoints);

      // The result array may contain the queryPoint, so do not output it in the following.

      PPointT *cur = result, *end = result + nNNs;

      if ( ! comParam.no_filtering ) { // Filter out certain vectors and clusters.
         while (cur < end)  { /* Shall we discard the rest results and start over for a new point? Not now for the sake of performance... TODO */
            ASSERT(*cur != NULL);

            // Look for the first un-filtered point for the next bucket.
            while ( cur < end ) {
              if ( pointIsNotFiltered(cur, comParam) ) {
                break;
              }
              seen[(*cur)->index] = true;
              cur++;
            }
            if ( cur >= end )
              break;

            bool worthy = false;
            int sizeBucket = 1; // 1 means the first un-filtered point
            PPointT *begin = cur;
            seen[(*begin)->index] = true;
            cur++;
            while ( cur < end &&
                  // look for the next point outside the current file
                  // if interfiles is false; that point is the end of
                  // current bucket (assume vectors in a bucket are
                  // sorted by their filenames already).
                  ( comParam.interfiles || strcmp((*begin)->filename, (*cur)->filename)==0 ) ) {
               if ( pointIsNotFiltered(cur, comParam) ) {
                  // prepare for filtering
                  sizeBucket++;

                  // the first heuristics for bugs AFTER filtering:
                  worthy = worthy || (*begin)->prop[ENUM_PPROP_nVARs-1] != (*cur)->prop[ENUM_PPROP_nVARs-1];

                  // the second heuristics for bugs AFTER filtering:
                  worthy = worthy || inconsistentIDchanges((*begin)->oids, (*cur)->oids); // TODO
               }
               seen[(*cur)->index] = true;
               cur++;
            }

            // output the bucket if:
            //   - there are >= 2 different points
            //   - there are <= upperBound (default 0) && >= lowerBound (default 2) points
            //   - there are >= 2 different numbers of variables
            // and update nBuckets and nBucketedPoints consequently
            if (sizeBucket >= comParam.lowerBound &&
                  (comParam.upperBound < comParam.lowerBound || sizeBucket <= comParam.upperBound) &&
                  ( comParam.bug_detecting ? worthy : true ) ) {
               nBuckets++;
               printf("\n");
               for (PPointT *p = begin; p < cur; p++)  {
                  ASSERT(*p != NULL);
                  if ( pointIsNotFiltered(p, comParam) ) {
                     nBucketedPoints++;

                     // compute the distance to the query point (maybe useless)
                     RealT distance = 0.;
                     for (int i = 0; i < dDim; i++) {
                        RealT t = (*p)->coordinates[i] - queryPoint->coordinates[i];
                        // L1 distance
//                        distance += (t >= 0) ? t : -t;
                        // Pi--L2 distance, LSH uses L2 by default, we should output L2 distance here.
                        distance += t*t;
                     }
                     // L1 distance
//                     printf("%09d\tdist:%0.1lf", (*p)->index, distance);
                     // L2 distance
                     printf("%09d\tdist:%0.1lf\t", (*p)->index, sqrt(distance));
                     printPoint(stdout, *p);
                     //CR_ASSERT(distance(dDim, queryPoint, *p) <= listOfRadii[r]);
                     //DPRINTF("Distance: %lf\n", distance(dDim, queryPoint, result[j]));
                     //printRealVector("NN: ", dDim, result[j]->coordinates);
                  }
               }
            } // end of enumeration of a bucket
         }  // end of !no_filtering
      } else {
         if ( nNNs>=comParam.lowerBound ) { // filter out non-clones anyway
            nBuckets++;
            printf("\n");
            for (PPointT *p = cur; p < end; p++)  {
               ASSERT(*p != NULL);
               nBucketedPoints++;
               seen[(*p)->index] = true;

               // compute the distance to the query point (maybe useless)
               RealT distance = 0.;
               for (int i = 0; i < dDim; i++) {
                  RealT t = (*p)->coordinates[i] - queryPoint->coordinates[i];
                  // L1 distance
//                  distance += (t >= 0) ? t : -t;
                  // Pi--L2 distance, LSH uses L2 by default, we should output L2 distance here.
                  distance += t*t;
               }

               // L1 distance
//               printf("%09d\tdist:%0.1lf", (*p)->index, distance);
               // L2 distance
               printf("%09d\tdist:%0.1lf\t", (*p)->index, sqrt(distance));
               printPoint(stdout, *p);
               //CR_ASSERT(distance(dDim, queryPoint, *p) <= listOfRadii[r]);
               //DPRINTF("Distance: %lf\n", distance(dDim, queryPoint, result[j]));
               //printRealVector("NN: ", dDim, result[j]->coordinates);
            } // end of enumeration of a bucket
         } // end of nNNs>=lowerBound
      }  // end of no_filtering and  exploration of NNs
    } // for (...nRadii...)
  }

  // Simple statistics and finish
  if (nQueries > 0) {
    meanQueryTime = meanQueryTime / nQueries;
    printf("\n%d queries, Mean query time: %0.6lf\n", nQueries, (double)meanQueryTime);
    printf("%d buckets, %d points (out of %d, %.2f %%) in them\n",
      nBuckets, nBucketedPoints, dNum, 100*(float)nBucketedPoints/(float)dNum);
  } else {
    printf("No query\n");
  }

  //freePRNearNeighborStruct(nnStruct);
}

void queryBuckets(const CommandLineParameters& comParam, PPointT* dsPoints, int dNum, int dDim, PPointT* qPoints, int qNum)
{
  PPointT *result = NULL; // will be updated by getRNearNeighbors
  IntT resultSize = 0; // will be updated by getRNearNeighbors
  PPointT queryPoint = NULL;
  /* PI: why need to allocate memory for queryPoint?
  FAILIF(NULL == (queryPoint = (PPointT)MALLOC(sizeof(PointT))));
  FAILIF(NULL == (queryPoint->coordinates = (RealT*)MALLOC(dDim * sizeof(RealT))));
  */

  TimeVarT meanQueryTime = 0;
  int nQueries = 0;
  int nBuckets = 0, nBucketedPoints = 0;

  for(IntT i = 0; i < qNum; nQueries++, i++) {
    queryPoint = qPoints[i];

    // get the near neighbors.
    IntT nNNs = 0;
    for(IntT r = 0; r < nRadii; r++) { // nRadii is always 1 so far.
      nNNs = getRNearNeighbors(nnStructs[r], queryPoint, result, resultSize);
      //printf("Total time for R-NN query at radius %0.6lf (radius no. %d):\t%0.6lf\n", (double)(listOfRadii[r]), r, timeRNNQuery);
      meanQueryTime += timeRNNQuery;

      //printf("\nQuery point %d: found %d NNs at distance %0.6lf (radius no. %d). NNs are:\n",
      //       i, nNNs, (double)(listOfRadii[r]), r);

      // sort by filename, then number of variables, then line number
      qsort(result, nNNs, sizeof(*result), comparePoints);

      PPointT *cur = result, *end = result + nNNs;

      if ( ! comParam.no_filtering ) { // Filter out certain vectors and clusters.
         while (cur < end)  { /* Shall we discard the rest results and start over for a new point? Not now for the sake of performance... TODO */
            ASSERT(*cur != NULL);

            // Look for the first un-filtered point for the next bucket.
            while ( cur < end ) {
              if ( pointIsNotFiltered(cur, comParam) ) {
                break;
              }
              cur++;
            }
            if ( cur >= end )
              break;

            bool worthy = false;
            int sizeBucket = 1+1; // 1+1 means the first un-filtered point plus the query point
            PPointT *begin = cur;
            cur++;
            while ( cur < end &&
                  // look for the next point outside the current file
                  // if interfiles is false; that point is the end of
                  // current bucket (assume vectors in a bucket are
                  // sorted by their filenames already).
                  ( comParam.interfiles || strcmp((*begin)->filename, (*cur)->filename)==0 ) ) {
               if ( pointIsNotFiltered(cur, comParam) ) {
                  // prepare for filtering
                  sizeBucket++;

                  // the first heuristics for bugs AFTER filtering:
                  worthy = worthy || (*begin)->prop[ENUM_PPROP_nVARs-1] != (*cur)->prop[ENUM_PPROP_nVARs-1];

                  // the second heuristics for bugs AFTER filtering:
                  worthy = worthy || inconsistentIDchanges((*begin)->oids, (*cur)->oids); // TODO
               }
               cur++;
            }

            // output the bucket if:
            //   - there are >= 2 different points
            //   - there are <= upperBound (default 0) && >= lowerBound (default 2) points
            //   - there are >= 2 different numbers of variables
            // and update nBuckets and nBucketedPoints consequently
            if (sizeBucket >= comParam.lowerBound &&
                  (comParam.upperBound < comParam.lowerBound || sizeBucket <= comParam.upperBound)
                  && ( comParam.bug_detecting ? worthy : true ) ) {
               nBuckets++;
               printf("\n");
               // print the query point
               printPoint(stdout, queryPoint);
               // print the rest points
               for (PPointT *p = begin; p < cur; p++)  {
                  ASSERT(*p != NULL);
                  if ( pointIsNotFiltered(p, comParam) ) {
                     nBucketedPoints++;

                     // compute the distance to the query point (maybe useless)
                     RealT distance = 0.;
                     for (int i = 0; i < dDim; i++) {
                        RealT t = (*p)->coordinates[i] - queryPoint->coordinates[i];
                        // L1 distance
//                        distance += (t >= 0) ? t : -t;
                        // Pi--L2 distance, LSH uses L2 by default, we should output L2 distance here.
                        distance += t*t;
                     }
                     // L1 distance
//                     printf("%09d\tdist:%0.1lf", (*p)->index, distance);
                     // L2 distance
                     printf("%09d\tdist:%0.1lf\t", (*p)->index, sqrt(distance));
                     printPoint(stdout, *p);
                     //CR_ASSERT(distance(dDim, queryPoint, *p) <= listOfRadii[r]);
                     //DPRINTF("Distance: %lf\n", distance(dDim, queryPoint, result[j]));
                     //printRealVector("NN: ", dDim, result[j]->coordinates);
                  }
               }
            } // end of enumeration of a bucket
         }  // end of !no_filtering
      } else {
         if ( nNNs+1>=comParam.lowerBound ) { // +1 means the query point; filter out non-clones anyway
            nBuckets++;
            printf("\n");
            printPoint(stdout, queryPoint);
            for (PPointT *p = cur; p < end; p++)  {
               ASSERT(*p != NULL);
               nBucketedPoints++;

               // compute the distance to the query point (maybe useless)
               RealT distance = 0.;
               for (int i = 0; i < dDim; i++) {
                  RealT t = (*p)->coordinates[i] - queryPoint->coordinates[i];
                  // L1 distance
//                  distance += (t >= 0) ? t : -t;
                  // Pi--L2 distance, LSH uses L2 by default, we should output L2 distance here.
                  distance += t*t;
               }

               // L1 distance
//               printf("%09d\tdist:%0.1lf", (*p)->index, distance);
               // L2 distance
               printf("%09d\tdist:%0.1lf\t", (*p)->index, sqrt(distance));
               printPoint(stdout, *p);
               //CR_ASSERT(distance(dDim, queryPoint, *p) <= listOfRadii[r]);
               //DPRINTF("Distance: %lf\n", distance(dDim, queryPoint, result[j]));
               //printRealVector("NN: ", dDim, result[j]->coordinates);
            } // end of enumeration of a bucket
         } // end of nNNs>=lowerBound
      }  // end of no_filtering and  exploration of NNs
    } // for (...nRadii...)
  }

  // Simple statistics and finish
  if (nQueries > 0) {
    meanQueryTime = meanQueryTime / nQueries;
    printf("\n%d queries, Mean query time: %0.6lf\n", nQueries, (double)meanQueryTime);
    printf("%d buckets, %d points (out of %d, %.2f %%) in them\n",
      nBuckets, nBucketedPoints, dNum, 100*(float)nBucketedPoints/(float)dNum);
  } else {
    printf("No query\n");
  }

  //freePRNearNeighborStruct(nnStruct);
}


/*
  The main entry to LSH package. Depending on the command line
  parameters, the function computes the R-NN data structure optimal
  parameters and/or construct the R-NN data structure and runs the
  queries on the data structure.
 */
int main(int argc, char *argv[]){

   // TODO: refactor the code and may use XML
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_FILE], "FILE:([^,]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_LINE], "LINE:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_OFFSET], "OFFSET:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_NODE_KIND], "NODE_KIND:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_NUM_NODE], "NUM_NODE:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_NUM_DECL], "NUM_DECL:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_NUM_STMT], "NUM_STMT:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_NUM_EXPR], "NUM_EXPR:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_TBID], "TBID:([-]?[0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_TEID], "TEID:([-]?[0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_nVARs], "VARs:\\{[^}]*\\}([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_CONTEXT_KIND], "CONTEXT_KIND:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_NEIGHBOR_KIND], "NEIGHBOR_KIND:([0-9]+)", REG_EXTENDED));
  FAILIF(0 != regcomp(&preg[ENUM_PPROP_OIDs], "OIDs:\\{[^}]*\\}([0-9]+)", REG_EXTENDED)); // TODO, pair-wise comparision of Vars.

  //initializeLSHGlobal();
  availableTotalMemory = (unsigned int)8e8;  // 800MB by default

  // Parse part of the command-line parameters.
  CommandLineParameters comParam;
  for (int opt; (opt = getopt(argc, argv, "ABl:v:V:e:E:a:m:N:d:p:P:R:M:cFf:b:t:q:")) != -1; ) {
    // Needed: -p -f -R
    switch (opt) {
    case 'A': 
      fprintf(stderr, "Warning: output all clones. Takes more time...\n");
      comParam.no_filtering = true; break;
    case 'B':
      fprintf(stderr, "Warning: no filtering for bugs now.\n");
      comParam.bug_detecting = false; break;
    case 'l': comParam.min_lines = atoi(optarg); break;
    case 'v': comParam.min_nVars = atoi(optarg); break;
    case 'V': comParam.max_num_diff_vars = atoi(optarg); break;
    case 'e': comParam.max_num_diff_nVars_diff = atof(optarg); break;
    case 'E': comParam.max_nVars_diff = atof(optarg); break;
    case 'm': comParam.minNumNodes = atoi(optarg); break;
    case 'b': comParam.lowerBound = atoi(optarg); break;
    case 't': comParam.upperBound = atoi(optarg); break;
    case 'N': nPoints = atoi(optarg); break;
    case 'd': pointsDimension = atoi(optarg); break;
    case 'p': comParam.paramsFile = optarg; break;
    case 'P': successProbability = atof(optarg); break;
    case 'M': availableTotalMemory = atol(optarg); break;
    case 'a': prefetch = atoi(optarg); break;
    case 'c':
      fprintf(stderr, "Warning: will compute parameters\n");
      comParam.computeParameters = true;
      break;
    case 'F':
      fprintf(stderr, "Warning: inter-file clone detection. Takes more time...\n");
      comParam.interfiles = true; break;
    case 'R':
      nRadii = 1;
      FAILIF(NULL == (listOfRadii = (RealT*)MALLOC(nRadii * sizeof(RealT))));
      FAILIF(NULL == (memRatiosForNNStructs = (RealT*)MALLOC(nRadii * sizeof(RealT))));
      listOfRadii[0] = strtod(optarg, NULL);
      memRatiosForNNStructs[0] = 1;
      break;
    case 'f':
      readDataSetFromFile2(optarg, dataSetPoints, nPoints, pointsDimension);
      DPRINTF("Allocated memory (after reading data set): %d\n", totalAllocatedMemory);
      break;
    case 'q':
       readDataSetFromFile2(optarg, queryDataSetPoints, nQueryPoints, pointsDimension);
       fprintf(stderr, "Query data provided: nQueryPoints=%d\n", nQueryPoints);
       break;
    default:
      fprintf(stderr, "Unknown option: -%c\n", opt);
      usage(1, argv[0]);
    }
  }
  
  if (optind < argc) {
    fprintf(stderr, "There are unprocessed parameters left\n");
    usage(1, argv[0]);
  }

  CHECK_INT(availableTotalMemory);
  CHECK_INT(nPoints);
  CHECK_INT(pointsDimension);
  CHECK_INT(nRadii);

  if (nPoints > MAX_N_POINTS) {
    printf("Error: the structure supports at most %d points (%d were specified).\n", MAX_N_POINTS, nPoints);
    fprintf(ERROR_OUTPUT, "Error: the structure supports at most %d points (%d were specified).\n", MAX_N_POINTS, nPoints);
    exit(1);
  }

  if (comParam.computeParameters == false)
     comParam.computeParameters = readParamsFile(comParam.paramsFile);

  if (comParam.computeParameters) {
    Int32T nSampleQueries = N_SAMPLE_QUERY_POINTS;
    PPointT sampleQueries[nSampleQueries];
    Int32T sampleQBoundaryIndeces[nSampleQueries];
    
    // Choose several data set points for the sample query points.
    for(IntT i = 0; i < nSampleQueries; i++){
      sampleQueries[i] = dataSetPoints[genRandomInt(0, nPoints - 1)];
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

    // Compute the R-NN DS parameters
    // if a parameter file is given, output them to that file, and continue
    // otherwise, output them to stdout, and exit

    FILE *fd;
    if (comParam.paramsFile == NULL) {
      fd = stdout;
    } else {
      fd = fopen(comParam.paramsFile, "wt");
      if (fd == NULL) {
         fprintf(stderr, "Unable to write to parameter file %s\n", comParam.paramsFile);
         exit(1);
      }
    }

    fprintf(fd, "%d\n", nRadii);
    transformMemRatios();
    for(IntT i = 0; i < nRadii; i++) {
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
      printRNNParameters(fd, optParameters);
    }
    if (fd == stdout) {
      exit(0);
    } else {
      fclose(fd);
      ASSERT(readParamsFile(comParam.paramsFile) == false);
    }
  }

  // output vector clusters according to the filtering parameters.
  printf("========================= Structure built =========================\n");
  printf("nPoints = %d, Dimension = %d\n", nPoints, pointsDimension);
  printf("no_filtering (0/1) = %d, inter-file (0/1) = %d, prefetch = %d\n", comParam.no_filtering, prefetch, comParam.interfiles);
  printf("*** Filtering Parameters for individual vectors ***\n");
  printf("minNumNodes = %d, min_nVars = %d, min_lines = %d\n", comParam.minNumNodes, comParam.min_nVars, comParam.min_lines);
  printf("*** Filtering Parameters for clusters ***\n");
  printf("lowerBound = %d, upperBound = %d\n", comParam.lowerBound, comParam.upperBound);
  printf("Max num of different nVars = %d, Max diff among different nVars = %g, \nMax diff among the num of different nVars = %g\n",
        comParam.max_num_diff_vars, comParam.max_nVars_diff, comParam.max_num_diff_nVars_diff);

  if ( queryDataSetPoints==NULL )
     enumBuckets(comParam, dataSetPoints, nPoints, pointsDimension);
  else
     queryBuckets(comParam, dataSetPoints, nPoints, pointsDimension, queryDataSetPoints, nQueryPoints);

  return 0;
}

