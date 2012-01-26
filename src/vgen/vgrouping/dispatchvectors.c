/*
 * 
 * Copyright (c) 2007-2012,
 *   Lingxiao Jiang         <lxjiang@ucdavis.edu>
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
/* for stat(): */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* TODO: modularization:
 * - separate the core of dispatch from I/O
 *   -- output file name generation; length limitation
 * - separate "scaling" from dispatch
 * - refactor common utilities (e.g., dir_exists)
 * - refactor command line processing
 * - scripts, instead of C, may be more flexible for changes and maintanence
 *   -- BSD license makes IP issues irrelevant
 */

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

void outputavector(FILE* out, const avector* av, long info)
{
  //  fprintf(out, "%ld\t%ld\n%s%s", info, av->size, av->note, av->vector);
  fprintf(out, "%s%s", av->note, av->vector);
}

bool dir_exists(const char * path)
{
  struct stat mystat;
  int res;

  res = stat(path, &mystat);
  if (res != 0) {
    fprintf(stderr, "Can't read %s.\n", path);
    return FALSE;
  } else if (S_ISDIR(mystat.st_mode)) {  // or use mystat.st_mode & 0x4000
    return TRUE;
  } else {
    fprintf(stderr, "%s exists, but it's not a directory\n", path);
    return FALSE;
  }
}

bool read_size_ranges(FILE* rf, long **lows, long ** highs, int* num_ranges,
		      double * d, long * lo, long * hi)
{
  long fhi = 0, llo = 0;
  char * line=NULL; int llen1 = 0;
  int rid = 0, rcounter = 0;
  int negative_range = 0;

  assert( rf!=NULL );

  // first line: <dist> <low> <high> <num_ranges> <first_high> <last_low>
  assert( getline(&line, &llen1, rf)>0 );
  assert( sscanf(line, "%lg\t%ld\t%ld\t%d\t%ld\t%ld", d, lo, hi, num_ranges, &fhi, &llo)==6 );
  assert( *d>=0. );
  assert( *lo>=1 );
  assert( *hi>=*lo );
  assert( *num_ranges>=2 );
  assert( fhi>=*lo );
  assert( llo+*d>=fhi );
  //fprintf(stdout, "%lg\t%ld\t%ld\t%d\t%ld\t%ld\n", *d, *lo, *hi, *num_ranges, fhi, llo);

  /* init lows and highs: */
  if ( *lows==NULL || *highs==NULL ) {
    *lows = (long *)malloc(sizeof(long)*(*num_ranges));
    *highs = (long *)malloc(sizeof(long)*(*num_ranges));
  } else {
    *lows = (long *)realloc(*lows, sizeof(long)*(*num_ranges));
    *highs = (long *)realloc(*highs, sizeof(long)*(*num_ranges));
  }
  assert( *lows!=NULL && *highs!=NULL );

  while ( getline(&line, &llen1, rf)>0 && rcounter<*num_ranges ) {
    //fprintf(stdout, "==>%s", line);
    assert( sscanf(line, "%d\t%ld\t%ld", &rid, &((*lows)[rcounter]), &((*highs)[rcounter]))==3 );
    assert( (*lows)[rcounter]>=0 );
    assert( (*highs)[rcounter]>=(*lows)[rcounter] || (*highs)[rcounter]<0 );
    if ( (*highs)[rcounter]<0 )
      negative_range++;
    //fprintf(stdout, "%d\t%ld\t%ld\n", rid, (*lows)[rcounter], (*highs)[rcounter]);
    assert( ++rcounter==rid );
  }
  if ( rcounter!=*num_ranges ) {
    fprintf(stderr, "ERROR in the range file: 'num_ranges' does not match line counters: %d vs. %d\n", num_ranges, rcounter);
    return FALSE;
  } else if ( negative_range<1 ) {
    fprintf(stderr, "ERROR: no -1 in the ranges?\n");
    return FALSE;
  } else if ( negative_range>1 ) {
    fprintf(stderr, "ERROR: more than one -1 in the ranges?\n");
    return FALSE;
  } else
    return TRUE;
}

int dispatch(const avector * vec, const long const * lows, const long const * highs, const int num_ranges,
	     const double d, const long lo, const long hi, const char * destdir, const char * fprefix)
{
  // group file names: destdir/fprefix_g<rid>_d_lo_hi
#define BASENAME_LEN 40
  char vecgroupname[FILENAME_MAX];
  char * psuffix, *pslash;
  int rid, rcounter = 0;
  bool last_range = FALSE;

  assert( vec!=NULL );
  assert( destdir!=NULL );

  if ( ! dir_exists(destdir) ) {
    fprintf(stderr, "ERROR: directory does not exist: %s\n", destdir);
    return 0;
  }
  if ( vec->size<=0 )
    return 0;

  /* prepare for the group names: */
  strncpy(vecgroupname, destdir, FILENAME_MAX);
  vecgroupname[FILENAME_MAX-1] = 0;
  psuffix = vecgroupname + strlen(vecgroupname);
  assert( psuffix>vecgroupname );
  assert( psuffix-vecgroupname < FILENAME_MAX-BASENAME_LEN ); // save space for actual names.
  pslash = strrchr(vecgroupname, '/');
  if ( pslash==NULL || pslash+1!=psuffix )
    *psuffix++ = '/';
  *psuffix = 0;

  for ( rid=0; rid<num_ranges; rid++ ) {
    if ( vec->size < lows[rid] )
      break;
    else if ( vec->size <= highs[rid] || highs[rid]<0 ) {
      // save the vector into this range:
      FILE * vgfile = NULL;
      snprintf(psuffix, BASENAME_LEN, "%s_g%d_%.8g_%ld_%ld", fprefix, rid+1, d, lo, hi);
      vgfile = fopen(vecgroupname, "a"); // NOTE: old group files are kept.
      if ( vgfile == NULL ) {
	fprintf(stderr, "Can't append to vector group file '%s'\n", vecgroupname);
	continue;
      }

      outputavector(vgfile, vec, 0);

      fclose(vgfile);
      rcounter++;
      if ( highs[rid]<0 ) {
	if ( last_range )
	  fprintf(stderr, "WARN: more than one -1 in the ranges; current rid = %d\n", rid+1);
	last_range = TRUE;
      }
    } else
      continue;
  }

  return rcounter;
}

int main (int argc, char *argv[])
{
  /* for reading the ranges: */
  FILE * rangefile = NULL;
  long *lows=NULL, *highs=NULL;
  int num_ranges = 0;
  double d = 0.;
  long lo = 0, hi = 0;
  /* for reading vectors: */
  FILE * vecfile = NULL;
  const char * outdir = "./vectorgroups/";
  const char * fileprefix = "vdb";
  long vid = 0, dispatched = 0;
  size_t llen1 = 0, llen2 = 0;
  avector onevector;
  regex_t num_node, node_kind;
  regmatch_t matches[1];

  if ( argc!=3 && argc!=4 && argc!=5 ) {
    fprintf(stderr, "Usage: %s <range file> <vec file> [<outdir> [fileprefix]]\n", argv[0]);
    exit(127);
  }

  rangefile = fopen(argv[1], "r");
  if ( rangefile==NULL ) {
    fprintf(stderr, "Can't open the range file: %s\n", argv[1]);
    exit(127);
  }
  vecfile = fopen(argv[2], "r");
  if ( vecfile==NULL ) {
    fprintf(stderr, "Can't open the vec file: %s\n", argv[2]);
    exit(127);
  }
  if ( argc>=4 ) {
    outdir = argv[3];
  }
  if ( ! dir_exists(outdir) ) {
    exit(127);
  }
  if ( argc>=5 ) {
    fileprefix = argv[4];
  }

  assert(0 == regcomp(&num_node, "NUM_NODE:([0-9]+)", REG_EXTENDED) );
  assert(0 == regcomp(&node_kind, "NODE_KIND:([0-9]+)", REG_EXTENDED) );

  read_size_ranges(rangefile, &lows, &highs, &num_ranges, &d, &lo, &hi);
  fclose(rangefile);
  rangefile = NULL;

  initavector(&onevector);
  while ( getline(&(onevector.note), &llen1, vecfile)>0 ) {
    char temp;
    int start, end;

    if ( getline(&(onevector.vector), &llen2, vecfile)<=0 ) {
      fprintf(stderr, "Reading stopped at line %ld in vector file %s. Should be checked there!\n", 2*(vid+1), argv[2]);
    }

    /* get node kind */
    if (0 != regexec(&node_kind, onevector.note, 1, matches, 0) ) {
      fprintf(stderr, "Line %ld has no node_kind in %s?\n", 2*(vid+1)-1, argv[2]);
    } else {
      start = matches[0].rm_so;
      end = matches[0].rm_eo;
      assert( start!=-1 );
      temp = onevector.note[end];
      onevector.note[end] = '\0';
      onevector.kind = atoi(onevector.note+start+10);
      onevector.note[end] = temp;
    }

    /* get num_node */
    if (0 != regexec(&num_node, onevector.note, 1, matches, 0) ) {
      fprintf(stderr, "Why line %ld has no NUM_NODE in %s?\n", 2*(vid+1)-1, argv[2]);
      continue;
    }
    start = matches[0].rm_so;
    end = matches[0].rm_eo;
    assert( start!=-1 );
    temp = onevector.note[end];
    onevector.note[end] = '\0';
    onevector.size = atoi(onevector.note+start+9);
    if ( onevector.size<=0 ) {
      fprintf(stderr, "Warning: why line %ld has NUM_NODE<=0? Skip the vector in %s.\n", 2*(vid+1)-1, argv[2]);
      continue;
    }
    assert( onevector.size>0 );
    onevector.note[end] = temp;

    /* one vector is read in, now dispatch it into group(s): */
    vid++;
    dispatched += dispatch(&onevector, lows, highs, num_ranges, d, lo, hi, outdir, fileprefix);
  }

  /* finalize: */
  clearavector(&onevector);
  fclose(vecfile);

  fprintf(stdout, "Total %ld vectors read in; %ld vectors dispatched into %d ranges (actual groups may be many fewer).\n", vid, dispatched, num_ranges);
  return 0;
}

