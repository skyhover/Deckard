/*
 * 
 * Copyright (c) 2007-2018, University of California / Singapore Management University
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu> <lxjiang@smu.edu.sg>
 *   Ghassan Misherghi      <ghassanm@ucdavis.edu>
 *   Zhendong Su            <su@ucdavis.edu>
 *   Stephane Glondu        <steph@glondu.net>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>

typedef int bool;
#define FALSE 0
#define TRUE 1

typedef struct avector_tag {
  int kind; /* experiments show that it's no use (only three kinds: 0 (unkonwn), 65 (stmt), 131 (extdef) */
  long size;
  char * note;
  char * vector;
  bool scaled;
} avector;

void initavector(avector * av)
{
  assert ( av!=NULL );
  av->kind = 0;
  av->size = 0;
  av->note = NULL;
  av->vector = NULL;
  av->scaled = FALSE;
}

void clearavector(avector * av)
{
  if ( av==NULL )
    return;
  av->kind = 0;
  av->size = 0;
  if ( av->note!=NULL ) {
    free(av->note);
    av->note = NULL;
  }
  if ( av->vector!=NULL ) {
    free(av->vector);
    av->vector = NULL;
  }
  av->scaled = FALSE;
}

int compare_avector(const void * a, const void * b) 
{
  //  if ( ((avector*)a)->kind == ((avector*)b)->kind )
    return ((avector*)a)->size - ((avector*)b)->size;
    //  else
    //    return ((avector*)a)->kind - ((avector*)b)->kind;
}

int pointsDimension = 0;
double * scaledfloatvec = NULL;
int dimensionofavector(const char * line)
     /* assume input is a string of digits */
{
  const char *p = line; int dim = 0;

  if ( p==NULL )
    return 0;

  while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
  while (*p != '\0') {
    while (*p != ' ' && *p!='\t' && *p!='\r' && *p!='\n' && *p != '\0') p++;
    dim ++;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
  }

  return dim;
}

bool scaleavector(avector* av, long srange, long xrange)
{
  char * scaledvec = NULL;
  int dim = 0;
  char *t = av->vector;

  av->scaled = TRUE;

  if ( t==NULL )
    return FALSE;

  if ( pointsDimension==0 )
    pointsDimension = dimensionofavector(av->vector);
  if ( pointsDimension==0 )
    return FALSE;

  if ( scaledfloatvec==NULL ) {
    scaledfloatvec = (double*)malloc(sizeof(double)*pointsDimension);
    assert( scaledfloatvec!=NULL );
  }

  for (dim=0, t=av->vector; *t != '\0' && dim < pointsDimension; dim++) {
    while ( !isdigit(*t) && *t != '\0' && *t != '.') t++; /* skip over the leading blanks; */
    //    scaledfloatvec[dim] = strtof(t, &t); /* why this doesn't work? Why worked in LSH? Because g++ works fine; it's a bug in gcc/glibc... */
    sscanf(t, "%lg", &scaledfloatvec[dim]); /* get the number; */
    while ( isdigit(*t) || *t=='.') t++; /* skip over the number; */
    scaledfloatvec[dim] = scaledfloatvec[dim]*srange/xrange;
  }

#define PRECISION_LEN 13
  scaledvec = (char*)malloc((1+pointsDimension)*PRECISION_LEN); /* may be too many... */
  if ( scaledvec==NULL ) {
    fprintf(stderr, "Not enough memory to scale all vectors. Try to free some vectors of previous groups...\n");
    return FALSE;
  }

  t = scaledvec;
  for (dim=0; dim<pointsDimension; dim++) {
    int inc = snprintf(t, PRECISION_LEN, "%lg ", scaledfloatvec[dim]);
    if ( inc>0 ) {
      t += inc;
    } else {
      t[0] = '\t'; t++;
    }
    assert( t-scaledvec<=pointsDimension*PRECISION_LEN ); /* a little conservative. */
  }
  t[0] = '\n'; t[1] = '\0';

  free(av->vector);		/* why this seems no use for saving memory? TOFIX. */
  av->vector=scaledvec;
  return TRUE;
}

void outputavector(FILE* out, avector* av, long info)
{
  //  fprintf(out, "%ld\t%ld\n%s%s", info, av->size, av->note, av->vector);
  fprintf(out, "%s%s", av->note, av->vector);
}

long vectorno = 1000000;
int main (int argc, char *argv[])
{
  avector *vectors = NULL;
  FILE * vecfile = NULL;
  long vid = 0;
  size_t len = 0;
  regex_t num_node, node_kind;
  regmatch_t matches[1];

  if ( argc!=3 && argc!=4 && argc!=6 && argc!=7 ) { 
    fprintf(stderr, "Usage: vectorsort <vec file> <#vector> [ flag_dist | {srange dist orange [flag_tofile]} ]\n");
    exit(1);
  }

  vectorno = atol(argv[2]);
  assert(vectorno>0);

  vecfile = fopen(argv[1], "r");
  if ( vecfile==NULL ) {
    fprintf(stderr, "Can't open the vec file: %s\n", argv[1]);
    exit(1);
  }

  vectors = (avector*)malloc(sizeof(avector)*(vectorno+1));
  if ( vectors==NULL ) {
    fprintf(stderr, "Memory malloc error for %ld vectors.\n", vectorno);
    exit(1);
  }

  assert(0 == regcomp(&num_node, "NUM_NODE:([0-9]+)", REG_EXTENDED) );
  assert(0 == regcomp(&node_kind, "NODE_KIND:([0-9]+)", REG_EXTENDED) );

  initavector(&vectors[vid]);
  while ( getline(&(vectors[vid].note), &len, vecfile)>=0 ) {
    char temp;
    int start, end;

    if ( getline(&(vectors[vid].vector), &len, vecfile)<=0 ) {
      fprintf(stderr, "Reading stopped at line %ld. Should be checked there!\n", 2*(vid+1));
    }

    /* get node kind */
    if (0 != regexec(&node_kind, vectors[vid].note, 1, matches, 0) ) {
      fprintf(stderr, "Line %ld has no node_kind?\n", 2*(vid+1)-1);
    } else {
      start = matches[0].rm_so;
      end = matches[0].rm_eo;
      assert( start!=-1 );
      temp = vectors[vid].note[end];
      vectors[vid].note[end] = '\0';
      vectors[vid].kind = atoi(vectors[vid].note+start+10);
      vectors[vid].note[end] = temp;
    }

    /* get num_node */
    if (0 != regexec(&num_node, vectors[vid].note, 1, matches, 0) ) {
      fprintf(stderr, "Why line %ld has no NUM_NODE?\n", 2*(vid+1)-1);
      continue;
    }
    start = matches[0].rm_so;
    end = matches[0].rm_eo;
    assert( start!=-1 );
    temp = vectors[vid].note[end];
    vectors[vid].note[end] = '\0';
    vectors[vid].size = atoi(vectors[vid].note+start+9);
    //    fprintf(stderr, "No. %ld Vec of size %ld from %s\n", vid+1, vectors[vid].size, vectors[vid].note+start+9);
    if ( vectors[vid].size<=0 ) {
      fprintf(stderr, "Warning: why line %ld has NUM_NODE<=0? Skip the vector.\n", 2*(vid+1)-1);
      clearavector(&vectors[vid]);
      continue;
    }
    assert( vectors[vid].size>0 );
    vectors[vid].note[end] = temp;

    vid++;
    if ( vid>vectorno ) {
      fprintf(stderr, "ERR: maximal #vec (%ld) is reached. Stop reading here.\n", vectorno);
      break;
    }
    initavector(&vectors[vid]);
  }

  fclose(vecfile);
  pointsDimension = dimensionofavector(vectors[0].vector); /* get the dimension of the vector for later use. */

  qsort(vectors, vid, sizeof(avector), compare_avector);

  if ( argc==3 ) {
    // output sorted vectors:
    long groups = 0, i;
    for(i = 0; i<vid; i++) {
      outputavector(stdout, &vectors[i], i);
    }
  } else if ( argc==4 ) {
    // output sizes of vectors and their numbers of occurrence:
    long i, startsize = vectors[0].size -1, nsize = 0;
    for ( i=0; i<vid; i++ ) {
      if ( vectors[i].size > startsize ) {
	// start a new size/#occurrence:
	if ( nsize > 0 )
	  fprintf(stdout, "%ld\t%ld\n", startsize, nsize);
	startsize = vectors[i].size;
	nsize = 1;
      } else
	nsize++;
    }
    if ( nsize > 0 ) // last size
      fprintf(stdout, "%ld\t%ld\n", startsize, nsize);
  } else {
#define MAX_NUM_GROUP 1024*2 // only need hundreds if d>0 and c>=0. And 1024*1024 may be too big for some machines, such as my C640 and msg.
    long s; double d, c;
    long al[MAX_NUM_GROUP], au[MAX_NUM_GROUP];
    long groups = 0, i;
    assert ( argc==6 || argc==7 );
    s = atol(argv[3]);
    d = atof(argv[4]);
    c = atof(argv[5]);
    assert(s>0);	      /* the upper bound of the first range */
    assert(d>=0 && d<s); /* maximal tolerated distances among clones in the first group; d>=s is not very meaningful */
    if ( !(d>0 && c<d && c>=0) && !(d==0 && c==-1) ) {  /* the overlapped range. c<-1 may skip some vectors. */
      fprintf(stderr, "Warning: c=%g is not good for this case, ", c);
      if ( d==0 )
	fprintf(stderr, "better to use c==-1.\n");
      else
	fprintf(stderr, "better to use c in [0,%g)\n", d);
    }

    /* compute all the group ranges */
    al[0] = 0, au[0] = (long)ceil(s+d);
    groups = 1;
    while ( al[groups-1]<=vectors[vid-1].size && au[groups-1]<=vectors[vid-1].size ) {
      /* compute the range for a new group */
      if ( groups >= MAX_NUM_GROUP ) {
	fprintf(stderr, "ERR: Can't handle more than %ld groups. The last group is set to include all rest vectors.\n", groups);
	au[groups-1] = vectors[vid-1].size+1;
	break;
      }

      if ( groups == 1 ) {
	/* begin the second group: */
	if ( c<0 ) {
	  /* do not to worry about false clone transitions or scalable "c": */
	  al[groups] = (long)ceil(s+d-c);
	  au[groups] = (long)ceil((s+d)*al[groups]/s);
	} else {
	  al[groups] = (long)ceil(s+d-c);
	  au[groups] = (long)ceil(s+2*d+d*d/s-c*c/s+c+1);
	}
      } else {
	if ( c<0 ) {
	  al[groups] = (long)ceil(au[groups-1]-c);
	  au[groups] = (long)ceil((s+d)*al[groups]/s);
	} else {
	  al[groups] = (long)ceil(au[groups-1]-c*al[groups-1]/s);
	  au[groups] = (long)ceil((s+d+c)*au[groups-1]/s-(d*c+c*c)*al[groups-1]/s/s+1);
	}
      }
      groups++;
    }

    /* output the group ranges: */
    if ( argc==6 ) {
      for ( i=0; i<groups; i++) {
        printf("%2ld & %ld\t& %ld\t& \\\\\\hline\n", i, al[i], au[i]);
      }
      return 0;
    }

    /* print out the vectors according the group ranges: Currently it
       has bug if c<-1. */
    {
      long curral = -1, currau = 0, nextal = al[0], gid = -1; /* a virtual starting point. */
      long nexti = 0, groupsize = 0;
#define OUTPUT2FILE TRUE
      char * groupfilename = (char*)malloc(sizeof(char)*(strlen(argv[1])+strlen(argv[3])+strlen(argv[4])+20));
      FILE * groupfile = stdout;
      assert( OUTPUT2FILE==FALSE || groupfilename!=NULL );

      /* output the ranges into a file for later usage: */
      sprintf(groupfilename, "%s_ranges_%s_%s", argv[1], argv[4], argv[3]);
      groupfile = fopen(groupfilename, "w");
      if ( groupfile == NULL ) {
	fprintf(stderr, "Can't open range file `%s' for writing.\n", groupfilename);
	groupfile = stdout;
      }
      for ( i=0; i<groups; i++) {
        fprintf(groupfile, "%ld\t%ld\t%ld\n", i, al[i], au[i]);
      }
      if ( groupfile!=stdout && groupfile!=stderr ) {
	fclose(groupfile);
	groupfile = stdout;
      }

      /* output vector groups: */
      for ( i=0; gid<groups && i<vid; i++ ) {
	if ( nextal<=vectors[i].size && (i==0 || nextal>vectors[i-1].size) ) {
	  /* record the starting position of the next group: */
	  nexti = i;
	}
	if ( vectors[i].size<=currau ) {
	  if ( vectors[i].size>=curral ) {
// comment the scaling out to make it more natural (use different distances during clustering later on) .
/* 	    if ( vectors[i].scaled==FALSE && curral>0 ) */
/* 	      scaleavector(&vectors[i], s, curral); */
	    outputavector(groupfile, &vectors[i], i);
	    groupsize++;
	  } /* here may skip some vectors when c<-1. */
	  /* An optimization to save some memory: */
	  if ( vectors[i].size<nextal )
	    clearavector(&vectors[i]);
	} else {
	  /* begin a new group: */
	  gid++;
	  sprintf(groupfilename, "%s_g%ld_%s_%s", argv[1], gid, argv[4], argv[3]);
	  if ( OUTPUT2FILE==TRUE && groupfile!=NULL
	       && groupfile!=stdout && groupfile!=stderr ) {
	    /* close the previous group file: */
	    fclose(groupfile);	/* The last group file may be left un-closed... */
	  }
	  printf("Size of Prev Group: %ld\n", groupsize);
	  printf("==== group No. %ld, file %s ====\n", gid, groupfilename);
	  groupfile = fopen(groupfilename, "w");
	  if ( groupfile==NULL ) {
	    fprintf(stderr, "Can't open %s_g%ld for writing. Use stdout instead.\n", argv[1], gid);
	    groupfile = stdout;
	  }
	  groupsize = 0;

	  if ( nextal<=currau )
	    /* the next group is overlapped with the current, so need to set back i: */
	    i = nexti -1;	/* -1 is due to i++; */
	  else
	    i = i -1; /* -1 is because vector[i] is not outputted yet. */
	  curral = al[gid], currau = au[gid];
	  nextal = gid==groups-1 ? vectors[vid-1].size+1 : al[gid+1]; /* there is no nextal for the last group. */
	}
      }

      free(groupfilename);
    }
  }

  return 0;
}
