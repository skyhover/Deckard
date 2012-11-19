
/* This code is used to compute the grouping ranges for vectors.
 * It does not need a vector file as inputs; its usage:
 * ./thisfile <distance> <lowerbound> <upperbound> [flag-output_range]
 *
 * So, it only generates a set of ranges. Pls use another code to dispatch
 * the vectors so it's more flexible to how merge certain ranges together.
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
const double epsilon = 1e-6;

const long max_num_ranges=1024*1024*1024; // could the size of a vector be so huge?
int num_ranges=1024*2; // may be dynamically increased
void *enlarge_array(long* arr, int * cap)
{
    assert( *cap < *cap+1024 ); // help reduce integer overflow
    *cap = *cap + 1024;
    return realloc(arr, *cap*sizeof(long));
}

int main (int argc, char *argv[])
{
  double d, c;
  long lo, hi;
  long *lows, *highs;
  long ri = 0;
  long i, j;

  if ( argc!=4 ) {
    fprintf(stderr, "Usage: %s <dist> <lowerbound> <upperbound>\n", argv[0]);
    exit(127);
  }

  d = atof(argv[1]);
  lo = atol(argv[2]);
  hi = atol(argv[3]);
  assert( d>=0.0 );
  assert( lo>=1 ); // 0 is meaningless for vectors
  assert( hi>=lo );

  lows = (long *)malloc(sizeof(long)*num_ranges);
  highs = (long *)malloc(sizeof(long)*num_ranges);
  assert( lows!=NULL && highs!=NULL );

  if ( fabs(d)<epsilon )
    c = -1;
  else
    c = d;
  
  /* the first range: */
  lows[0] = 0;
  highs[0] = (long)ceil(lo+d); // "ceil" is more coservative than "floor"; if ranges are always integers, "floor" should be better.
  ri = 1; // assume ( max_num_ranges>2 );
  while ( ri<max_num_ranges && lows[ri-1]<=hi ) {
    /* compute the range for a new group */
    if ( ri >= max_num_ranges-1 ) {
      fprintf(stderr, "ERR: Reach %ld ranges. The last range is set to include all rest sizes.\n", max_num_ranges);
      if ( c<0 )
	lows[ri] = (long)ceil(highs[ri-1]-c);
      else
	lows[ri] = (long)ceil(highs[ri-1]-c*lows[ri-1]/lo);
    } else {
      /* enlarge the arrays if they are too small: */
      if ( ri >= num_ranges ) {
	lows = enlarge_array(lows, &num_ranges);
	highs = realloc(highs, num_ranges*sizeof(long));
	assert( lows!=NULL && highs!=NULL );
      }
      if ( ri == 1 ) {
	/* begin the second range: */
	if ( c<0 ) {
	  /* do not to worry about false clone transitions or scalable "c": */
	  lows[ri] = (long)ceil(lo+d-c);
	  highs[ri] = (long)ceil((lo+d)*lows[ri]/lo);
	} else {
	  lows[ri] = (long)ceil(lo+d-c);
	  highs[ri] = (long)ceil(lo+2*d+d*d/lo-c*c/lo+c+1);
	}
      } else {
	if ( c<0 ) {
	  lows[ri] = (long)ceil(highs[ri-1]-c);
	  highs[ri] = (long)ceil((lo+d)*lows[ri]/lo);
	} else {
	  lows[ri] = (long)ceil(highs[ri-1]-c*lows[ri-1]/lo);
	  highs[ri] = (long)ceil((lo+d+c)*highs[ri-1]/lo-(d*c+c*c)*lows[ri-1]/lo/lo+1);
	}
      }
      ri++;
    }
  }
  highs[ri-1] = -1; // "-1" means all rest sizes

  /* output the ranges:
   * first line (the parameters): <dist> <low> <high> <num_ranges> <first_high> <last_low>
   * other lines: <range_id> <low> <high> <dist_for_reference> */
  printf("%.8g\t%ld\t%ld\t%ld\t%ld\t%ld\n", d, lo, hi, ri, highs[0], lows[ri-1]);
  printf("%6ld\t\t%ld\t%ld\t%g\n", (long)1, lows[0], highs[0], d);
  for ( i=1; i<ri; i++) {
    printf("%6ld\t\t%ld\t%ld\t%g\n", i+1, lows[i], highs[i], sqrt(lows[i])/sqrt(lo)*d);
  }
  return 0;
}

