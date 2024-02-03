#include <stdio.h>
#include <stdlib.h>     // Dynamic mem allocation
#include <string.h>     // Byte operations (memcpy/memcmp)
#include <stdint.h>     // For mem efficant ints
#include <math.h>       // Calculating optimal seed count
#include <stdbool.h>    // For 'is in set?' return

#define BITS_IN_BYTE 8

// Memory allocation functions that check for null pointer return
void* malloc_safe(const size_t BYTES) {
    void *allocatedMem = malloc(BYTES);
    if (allocatedMem == NULL) {
        perror("Memory allocation failed!"); abort();
    }
    return allocatedMem;
}

void* calloc_safe(const size_t TYPE_SIZE, const size_t ELEMENTS) {
    void *arrayPrt = calloc(ELEMENTS, TYPE_SIZE);
    if (arrayPrt == NULL) {
        perror("Memory allocation failed!"); abort();
    }
    return arrayPrt;
}

struct CBloomFilter {
    void* intArray;
    size_t intArrBuckets;
    unsigned short intTypeBytes;

    unsigned short* seeds;
    uint8_t seedsCount;
};

struct CBloomFilter* createBloomFilter(const size_t elementsToAdd, const unsigned short slotsPerElement) {
    struct CBloomFilter* NewBFilter = (struct CBloomFilter*)malloc_safe(sizeof(struct CBloomFilter));

    NewBFilter->intTypeBytes = 1;

    NewBFilter->intArrBuckets = elementsToAdd * slotsPerElement;
    NewBFilter->intArray = calloc_safe(sizeof(uint8_t), NewBFilter->intArrBuckets);

    NewBFilter->seedsCount = ceilf(slotsPerElement * log(2));
    NewBFilter->seeds = (unsigned short*)malloc_safe(sizeof(unsigned short) * NewBFilter->seedsCount);
    // Populate seed array with random seeds
    for (size_t i=0; i < NewBFilter->seedsCount; i++)
        NewBFilter->seeds[i] = (rand() % (unsigned short)~0 ) + 1;

    return NewBFilter;
}

unsigned hash_fun(const char* keyToHash, const unsigned SEED) {
    unsigned PRIME = 65537, MAX_BITS = ~0, hash = 0;
    // For every char in key
    for (size_t i=0; keyToHash[i] != '\0'; i++)
        hash = (hash * PRIME * SEED + keyToHash[i]) % MAX_BITS;
    
    return hash;
}

void* addressFromBucket (struct CBloomFilter* bFilter, const size_t bucket) {
    return (int8_t*)bFilter->intArray + bucket * bFilter->intTypeBytes;
}

void increaseIntSize(struct CBloomFilter* bFilter) {
    printf("\n##Doubling Size of the filter\n");

    // Print bit array
    for (size_t i=0; i < bFilter->intArrBuckets; i++) {
        uint32_t valueInBucket = 0;
        memcpy(&valueInBucket, addressFromBucket(bFilter, i), bFilter->intTypeBytes);
        printf("%-3zu\t ", valueInBucket);
    }
    
    // Save a copy of the current array
    void* intArrCpy = malloc_safe(bFilter->intArrBuckets * bFilter->intTypeBytes);
    memcpy(intArrCpy, bFilter->intArray, bFilter->intArrBuckets * bFilter->intTypeBytes);

    // Increase the size of the int's type
    const size_t oldTypeSize = bFilter->intTypeBytes;
    bFilter->intTypeBytes *= 2;

    // Re-size current array to be size of new type (double)
    free(bFilter->intArray);
    bFilter->intArray = calloc_safe(bFilter->intTypeBytes, bFilter->intArrBuckets);


    // Put old items back into array
    for (size_t i=0; i < bFilter->intArrBuckets; i++)
        memcpy(addressFromBucket(bFilter, i), (int8_t*)intArrCpy + i * oldTypeSize, oldTypeSize);
    
    free(intArrCpy);
    
}

void Bloom_add(struct CBloomFilter* bFilter, char* keyToAdd) {
    uint32_t valueOfBucket = 0;
    // For every seed in the filter, hash to an index
    for (size_t i=0; i < bFilter->seedsCount; i++) {
        unsigned bucket = hash_fun(keyToAdd, bFilter->seeds[i]) % bFilter->intArrBuckets;

        memcpy(&valueOfBucket, addressFromBucket(bFilter, bucket), bFilter->intTypeBytes);
        valueOfBucket += 1;
        memcpy(addressFromBucket(bFilter, bucket), &valueOfBucket, bFilter->intTypeBytes);

        // Increase type size to prevent int overflow
        if (valueOfBucket + 1 == 1 << (bFilter->intTypeBytes * BITS_IN_BYTE) ) {
            printf("Voilated: %u\b", valueOfBucket);
            increaseIntSize(bFilter);
        }

        //printf("%u, ", bucket);
    }
}

bool keyInBloom(struct CBloomFilter* bFilter, char* keyToFind) {
    // Has to be as large as largest int array type
    const uint32_t zeroCompare = 0;

    // Hash every seed, and fail check if any bucket is empty
    for (size_t i=0; i < bFilter->seedsCount; i++) {
        unsigned bucket = hash_fun(keyToFind, bFilter->seeds[i]) % bFilter->intArrBuckets;
        
        if (memcmp(addressFromBucket(bFilter, bucket), &zeroCompare, bFilter->intTypeBytes) == 0)
            return false;
    }
    return true;
}

// Don't remove keys you aren't 100% sure are already in the filter to prevent false negatives
void removeFromBloom(struct CBloomFilter* bFilter, char* keyToRemove) {
    if (!keyInBloom(bFilter, keyToRemove)) {
        printf("\nError: key to remove not in filter!"); abort();
    }
    uint32_t valueOfBucket = 0;
    // Hash key will all seeds and remove one from each bucket
    for (size_t i=0; i < bFilter->seedsCount; i++) {
        unsigned bucket = hash_fun(keyToRemove, bFilter->seeds[i]) % bFilter->intArrBuckets;

        memcpy(&valueOfBucket, addressFromBucket(bFilter, bucket), bFilter->intTypeBytes);
        valueOfBucket -= 1;
        memcpy(addressFromBucket(bFilter, bucket), &valueOfBucket, bFilter->intTypeBytes);
    }
}


unsigned rndNumber(const unsigned lowerBound, const unsigned upperBound) {
    const unsigned PRIME = 65537;
    return (rand()*PRIME) % (upperBound - lowerBound + 1) + lowerBound;
}

int main() {

    srand(((int)__TIME__[6] << 0) & ((int)__TIME__[7] << 1)); // Seed random using time
    const unsigned TOTAL_STRINGS = 10000, MAX_LEN = 20;

    struct CBloomFilter* myFilter = createBloomFilter(10, 10);

    // Print all the seeds
    for (unsigned i=0; i < myFilter->seedsCount; i++) printf("%u\n", myFilter->seeds[i]);

    printf("BitArrLenght: %u\n", myFilter->intArrBuckets);

    // Add a bunch of random strings to the filter as a test
    char rndString[MAX_LEN];
    for (size_t strPlace, i=0; i<TOTAL_STRINGS; i++) {
        // Generate random string
        for (strPlace = 0; strPlace < rndNumber(3, MAX_LEN); strPlace++)
            rndString[strPlace] = rndNumber('a', 'z');
        rndString[strPlace + 1] = '\0'; // Terminate string
        
        //printf("\n\n%s\n\t", rndString);
        Bloom_add(myFilter, rndString);
        //printf("\n\tIn filter? %u", keyInBloom(myFilter, rndString));

        //removeFromBloom(myFilter, rndString);
        //printf("\n\tAfter removed? %u", keyInBloom(myFilter, rndString));
    }
    
    printf("\nIs 'Mike' in filter? %u\n", keyInBloom(myFilter, "Mike")); 
    
    // Print bit array
    uint32_t valueInBucket = 0;
    for (size_t i=0; i < myFilter->intArrBuckets; i++) {
        memcpy(&valueInBucket, addressFromBucket(myFilter, i), myFilter->intTypeBytes);
        printf("%-3zu\t ", valueInBucket);
    }
    //Manually increase table size
    //increaseIntSize(myFilter);

}