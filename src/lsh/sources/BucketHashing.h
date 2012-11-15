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

#ifndef BUCKETHASHING_INCLUDED
#define BUCKETHASHING_INCLUDED

// An entry (point) in a bucket of points (a bucket is specified by a
// vector in integers of length k). There is link to the actual point
// stored in the entry, as well as link to the next entry in the
// bucket.
typedef struct _BucketEntryT {
  //PPointT point;
  Int32T pointIndex;
  _BucketEntryT *nextEntry;
} BucketEntryT, *PBucketEntryT;

// The type definition for a bucket. A bucket is a container for
// points that all have the same value for hash function g (function g
// is a vector of K LSH functions).
typedef struct _GBucketT {
  // These controlValues are used instead of the full k-vector (value
  // of the hash function g) describing the bucket. With a high
  // probability all buckets will have different pairs of
  // controlValues.
  Uns32T controlValue1;

  // The bucket entries (stored in a linked list).
  BucketEntryT firstEntry;
  _GBucketT *nextGBucketInChain;
} GBucketT, *PGBucketT;

typedef struct _LinkPackedGBucketT {
  Uns32T controlValue1;
  Int32T indexStart;
} LinkPackedGBucketT, *PLinkPackedGBucketT;

typedef struct _PackedGBucketT {
  Uns32T controlValue1;
  Int32T indexStart;
  Int32T nPointsInBucket;
} PackedGBucketT, *PPackedGBucketT;

// Number of bits reserved for storing the #points in a bucket
#define N_BITS_FOR_BUCKET_LENGTH (32 - 2 - N_BITS_PER_POINT_INDEX)

// 2^N_BITS_FOR_BUCKET_LENGTH - 1
#define MAX_NONOVERFLOW_POINTS_PER_BUCKET ((1U << N_BITS_FOR_BUCKET_LENGTH) - 1)

// how many fields of N_BITS_FOR_BUCKET_LENGTH bits are needed to store a 32-bit (unsigned) integer.
#define N_FIELDS_PER_INDEX_OF_OVERFLOW ((32 + N_BITS_FOR_BUCKET_LENGTH - 1) / N_BITS_FOR_BUCKET_LENGTH)

typedef union _HybridChainEntryT {
  Uns32T controlValue1;
  struct _OverloadedPoint {
    Uns32T isLastBucket : 1;
    Uns32T bucketLength : N_BITS_FOR_BUCKET_LENGTH;
    Uns32T isLastPoint : 1;
    Uns32T pointIndex : N_BITS_PER_POINT_INDEX;
  } point;
} HybridChainEntryT, *PHybridChainEntryT;

typedef union _GeneralizedPGBucket {
  PGBucketT llGBucket;
  PLinkPackedGBucketT linkGBucket;
  PPackedGBucketT packedGBucket;
  PHybridChainEntryT hybridGBucket;
} GeneralizedPGBucket;

typedef struct _PointsListEntryT {
  PPointT point;
  Int32T nextPoint;
} PointsListEntryT;

// A big number (>> max #  of points)
#define INDEX_START_EMPTY 1000000000U

// 4294967291 = 2^32-5
#define UH_PRIME_DEFAULT 4294967291U

// 2^29
#define MAX_HASH_RND 536870912U

// 2^32-1
#define TWO_TO_32_MINUS_1 4294967295U

// Whether to use the same hash functions (for universal hashing) or
// not. If using the same hash functions, then we can precompute some
// of the hash values and reuse them.
#define USE_SAME_UHASH_FUNCTIONS TRUE

#define USE_PRECOMPUTED_HASHES USE_SAME_UHASH_FUNCTIONS

// Two hash functions used: main one and a control one.
#define UHF_NUMBER_OF_HASHES 2

#define UHF_MAIN_INDEX 0

#define UHF_CONTROL1_INDEX 1

// Number of precomputed Uns32T words needed to store precomputed
// hashes of a (part of a) bucket description (more precisely of a <u>
// function).  It is 2*2 because: need 2 words for each of 1) the main
// hash; 2) control value 1 hash function (2 words per hash function
// because a <u> function can occupy two positions in the bucket
// vector).
#define N_PRECOMPUTED_HASHES_NEEDED (UHF_NUMBER_OF_HASHES * 2)

// An universal hash table with collision solved by chaining. The
// chains and the buckets are stored using either singly linked lists
// or static arrays (depending on the value of the field <typeHT>).
typedef struct _UHashStructureT {
  // The type of the hash table (can take values HT_*). when
  // <typeHT>=HT_LINKED_LIST, chains&buckets are linked lists. when
  // <typeHT>=HT_PACKED, chains&buckets are static arrays. when
  // <typeHT>=HT_STATISTICS, chains are static arrays and buckets only
  // count # of elements.  when <typeHT>=HT_HYBRID_CHAINS, a chain is
  // a "hybrid" array that contains both the buckets and the points
  // (the an element of the chain array is of type
  // <HybridChainEntryT>). all chains are conglamerated in the same
  // array <hybridChainsStorage>.
  IntT typeHT;

  // The array containing the hash slots of the universal hashing.
  union _hashTableT {
    PGBucketT *llHashTable;
    PackedGBucketT **packedHashTable;
    LinkPackedGBucketT **linkHashTable;
    PHybridChainEntryT *hybridHashTable;
  } hashTable;

  // The sizes of each of the chains of the hashtable (used only when
  // typeHT=HT_PACKED or HT_STATISTICS.
  IntT *chainSizes;

  union _bucketPoints{
    PPointT *pointsArray;
    PointsListEntryT *pointsList;
  } bucketPoints;

  HybridChainEntryT *hybridChainsStorage;

  // The size of hashTable.
  Int32T hashTableSize;

  // Number of elements(buckets) stored in the hash table in total (that
  // is the number of non-empty buckets).
  Int32T nHashedBuckets;

  Int32T nHashedPoints;

  // Unused (but allocated) instances of the corresponding
  // structs. May be reused if needed (instead of allocated new
  // memory).
  PGBucketT unusedPGBuckets;
  PBucketEntryT unusedPBucketEntrys;

  Uns32T prime; // the prime used for the universal hash functions.
  IntT hashedDataLength;// the number of IntT's in an element from U (U is the set of values to hash).

  // The hash functions used for the universal hashing.  

  // The main hash function (that defines the index
  // of the slot in the table).
  // The type of the hash function is: h_{a}(k) = ((a\cdot k)mod p)mod hashTableSize.
  Uns32T *mainHashA;

  // Control hash functions: used to compute/check the <controlValue>s
  // of <GBucket>s.
  // The type of the hash function is: h_{a}(k) = (a\cdot k)mod p
  Uns32T *controlHash1;
} UHashStructureT, *PUHashStructureT;

#define HT_LINKED_LIST 0

#define HT_PACKED 1

#define HT_STATISTICS 2

#define HT_HYBRID_CHAINS 3

#define CHAIN_INIT_SIZE 0
#define CHAIN_RESIZE_RATIO 1.5



PUHashStructureT newUHashStructure(IntT typeHT, Int32T hashTableSize, IntT bucketVectorLength, BooleanT useExternalUHFs, Uns32T *(&mainHashA), Uns32T *(&controlHash1), PUHashStructureT modelHT);

void clearUHashStructure(PUHashStructureT uhash);

void optimizeUHashStructure(PUHashStructureT uhash, PointsListEntryT *(&auxPtsList));

void freeUHashStructure(PUHashStructureT uhash, BooleanT freeHashFunctions);

void addBucketEntry(PUHashStructureT uhash, IntT nBucketVectorPieces, Uns32T firstBucketVector[], Uns32T secondBucketVector[], Int32T pointIndex);

GeneralizedPGBucket getGBucket(PUHashStructureT uhash, IntT nBucketVectorPieces, Uns32T firstBucketVector[], Uns32T secondBucketVector[]);

void precomputeUHFsForULSH(PUHashStructureT uhash, Uns32T *uVector, IntT length, Uns32T *result);

#endif
