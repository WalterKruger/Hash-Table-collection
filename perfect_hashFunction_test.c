# include <stdio.h>     // Printf
# include <stdint.h>    // For exact width objects
# include <stdbool.h>
# include <string.h>

#include <stdlib.h>     // Random "strings"
#include <math.h>       // Messuring performance

# define MAX_LEN 20


/*
    Hashes a single character (for the rolling hash function)
*/
const unsigned AES_PRIME = 65537;

unsigned betterCustom(unsigned hash, char charInStr, unsigned SEED) {
    return hash * SEED * AES_PRIME + (int)charInStr;
}

unsigned stragerSEED(unsigned hash, char charInStr, const unsigned SEED) {
    return hash + (( SEED ^ charInStr) * AES_PRIME) / pow(2, 64);
}

unsigned shiftAddition(unsigned hash, char charInStr) {
    return ((hash << 5) + hash) + (int)charInStr;
}


// For every char: hash = function(hash, char)
unsigned rollingHash(const char *key, const unsigned SEED) {
    const unsigned MAX_BITS = ~0;    // Largest 32-bit representation

    unsigned hash = SEED;
    // For every character
    for (size_t i=0; key[i] != '\0'; i++) {
        //hash = ( hash * SEED * PRIME_ROLLING + key[i] ) % MAX_BITS;
        hash = stragerSEED(hash, key[i], 42) % MAX_BITS;
    }
    return hash;
}

uint64_t fnv_1a(const char* strToHash) {
    const uint64_t FNV_offset_basis = 0xcbf29ce484222325;
    const uint64_t FNV_prime = 0x100000001b3;

    uint64_t hash = FNV_offset_basis;
    for (size_t charI=0; strToHash[charI] != '\0'; charI++) {
        hash = hash ^ strToHash[charI];
        hash = hash * FNV_prime;
    }
    return hash;
}




unsigned rndNumber(const unsigned lowerBound, const unsigned upperBound) {
    const unsigned PRIME = 65537;
    return (rand()*PRIME) % (upperBound - lowerBound + 1) + lowerBound;
}

void* calloc_safe(const size_t ELEMENTS, const size_t EL_SIZE) {
    void* arrToAlloc = calloc(ELEMENTS, EL_SIZE);

    if(arrToAlloc == NULL) {
        perror("Memory allocation failed!");exit(0);
    }
    return arrToAlloc;
}

int main() {

    const size_t TABLE_SIZE = 100;

    size_t strPlace;    // Place in the random string
    char randKeys[TABLE_SIZE][MAX_LEN+1];

    // Get array of random strings
    for (size_t keyPlace=0; keyPlace < TABLE_SIZE; keyPlace++) {
        for (strPlace = 0; strPlace < rndNumber(3, MAX_LEN); strPlace++)
            randKeys[keyPlace][strPlace] = rndNumber((int)'a', (int)'z');
        
        randKeys[keyPlace][strPlace + 1] = '\0'; // Terminate string
    }
    

    size_t BUCKET_SIZE = TABLE_SIZE * 5;
    

    char bucketOccupied[BUCKET_SIZE][MAX_LEN+1];
    unsigned trysUntilPerfect = 0;
    
    bool collisions = true;
    while (collisions) {
        collisions = false;

        if(trysUntilPerfect++ == 100000) abort();

        for (size_t i=0; i<BUCKET_SIZE; i++)
            strcpy( bucketOccupied[i], "\0\0\0\0\0" );
        
        unsigned curSeed = rand()+1;

        // Hash every key
        for (size_t i=0; i<TABLE_SIZE; i++) {
            size_t cur_hash = rollingHash(randKeys[i], curSeed) % BUCKET_SIZE;

            if ( bucketOccupied[cur_hash][0] != '\000' && strcmp(bucketOccupied[cur_hash], randKeys[i]) != 0 ) {
                printf("Collision! (%s with %s) at %u)\n", randKeys[i], bucketOccupied[cur_hash], cur_hash);

                collisions = true;
                break;
            }
            strcpy( bucketOccupied[cur_hash], randKeys[i] );
        }
    }
    printf("Took %u trys to find a no-collision hash!\n", trysUntilPerfect);

}