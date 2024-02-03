# include <stdio.h>     // Printf
# include <stdint.h>    // For exact width objects

#include <stdlib.h>     // Random "strings"
#include <math.h>       // Messuring performance

# define MAX_LEN 20
# define TABLE_SIZE 100


#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )


/*
    Hashes a single character (for the rolling hash function)
*/
const unsigned AES_PRIME = 65537;

unsigned betterCustom(unsigned hash, char charInStr) {
    return hash * AES_PRIME + (int)charInStr;
}

unsigned betCusSeed(unsigned hash, char charInStr) {
    return hash * AES_PRIME * rand() + (int)charInStr;
}

unsigned shiftAddition(unsigned hash, char charInStr) {
    return ((hash << 5) + hash) + (int)charInStr;
}

unsigned stragerSEED(unsigned hash, char charInStr) {
    return hash + (( (rand()+1) ^ charInStr) * AES_PRIME) / pow(2, 64);
}

unsigned badCustom(unsigned hash, char charInStr) {
    return hash ^ (int)charInStr * AES_PRIME;
}

// For every char: hash = function(hash, char)
unsigned rollingHash(const char *key, unsigned (*hashChar)(unsigned, char)) {
    const unsigned MAX_BITS = ~0;    // Largest 32-bit representation

    unsigned hash = AES_PRIME;
    // For every character
    for (size_t i=0; key[i] != '\0'; i++) {
        hash = (*hashChar)(hash, key[i]) % MAX_BITS;
    }
    return hash;
}

const uint8_t bytePermutations[256] = {
    251, 175, 119, 215, 81, 14, 79, 191, 103, 49, 181, 143, 186, 157,  0,
    232, 31, 32, 55, 60, 152, 58, 17, 237, 174, 70, 160, 144, 220, 90, 57,
    223, 59,  3, 18, 140, 111, 166, 203, 196, 134, 243, 124, 95, 222, 179,
    197, 65, 180, 48, 36, 15, 107, 46, 233, 130, 165, 30, 123, 161, 209, 23,
    97, 16, 40, 91, 219, 61, 100, 10, 210, 109, 250, 127, 22, 138, 29, 108,
    244, 67, 207,  9, 178, 204, 74, 98, 126, 249, 167, 116, 34, 77, 193,
    200, 121,  5, 20, 113, 71, 35, 128, 13, 182, 94, 25, 226, 227, 199, 75,
    27, 41, 245, 230, 224, 43, 225, 177, 26, 155, 150, 212, 142, 218, 115,
    241, 73, 88, 105, 39, 114, 62, 255, 192, 201, 145, 214, 168, 158, 221,
    148, 154, 122, 12, 84, 82, 163, 44, 139, 228, 236, 205, 242, 217, 11,
    187, 146, 159, 64, 86, 239, 195, 42, 106, 198, 118, 112, 184, 172, 87,
    2, 173, 117, 176, 229, 247, 253, 137, 185, 99, 164, 102, 147, 45, 66,
    231, 52, 141, 211, 194, 206, 246, 238, 56, 110, 78, 248, 63, 240, 189,
    93, 92, 51, 53, 183, 19, 171, 72, 50, 33, 104, 101, 69, 8, 252, 83, 120,
    76, 135, 85, 54, 202, 125, 188, 213, 96, 235, 136, 208, 162, 129, 190,
    132, 156, 38, 47, 1, 7, 254, 24, 4, 216, 131, 89, 21, 28, 133, 37, 153,
    149, 80, 170, 68, 6, 169, 234, 151
};
uint8_t pearsonHash(const char* strToHash) {
    uint8_t hash = 0;

    for (size_t charI=0; strToHash[charI] != '\0'; charI++)
        hash = bytePermutations[hash ^ strToHash[charI] ];
    
    return hash;
}

uint32_t ElfHash(const char* strToHash) {
    uint32_t hash = 0, high;

    while (*strToHash) {
        hash = (hash << 4) + *strToHash++;
        if (high = hash & 0xF0000000) 
            hash ^= high >> 24;
        hash &= ~high;
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

uint64_t fnv_1a_SEED(const char* strToHash) {
    const uint64_t FNV_offset_basis = 0xcbf29ce484222325 + rand();
    const uint64_t FNV_prime = 0x100000001b3;

    uint64_t hash = FNV_offset_basis;
    for (size_t charI=0; strToHash[charI] != '\0'; charI++) {
        hash = hash ^ strToHash[charI];
        hash = hash * FNV_prime;
    }
    return hash;
}

uint32_t jenkins_one_at_a_time_hash(const uint8_t* key) {
    size_t i = 0;
    uint32_t hash = 0;
    while (key[i] != '\0') {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

uint32_t SuperFastHash(const char*strToHash, int len) {
    uint32_t hash = len, tmp;
    int rem;

    //if (len <= 0 || data == NULL) return 0; // Test data will always be false

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (strToHash);
        tmp    = (get16bits (strToHash+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        strToHash  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (strToHash);
                hash ^= hash << 16;
                hash ^= ((signed char)strToHash[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (strToHash);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*strToHash;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}




unsigned rndNumber(const unsigned lowerBound, const unsigned upperBound) {
    const unsigned PRIME = 65537;
    return (rand()*PRIME) % (upperBound - lowerBound + 1) + lowerBound;
}

int main() {
    char rndString[MAX_LEN + 1];

    unsigned bucket[TABLE_SIZE] = {0}; // Store how many collisions occur at each bucket

    const size_t TOTAL_STRINGS = 10000;
    const double meanFullness = TOTAL_STRINGS / TABLE_SIZE; // Fullness if uniform

    size_t strPlace;    // Place in the random string

    // Get the number of times a bucket is sellected
    for (size_t i=0; i<TOTAL_STRINGS; i++) {
        // Generate random string
        for (strPlace = 0; strPlace < rndNumber(3, MAX_LEN); strPlace++)
            rndString[strPlace] = rndNumber((int)'a', (int)'z');
        rndString[strPlace + 1] = '\0'; // Terminate string

        //printf("%u:\t(%s)\n", rollingHash(rndString, betCusSeed) % 5, rndString);

        bucket[SuperFastHash(rndString, strPlace) % TABLE_SIZE] += 1;
        //bucket[rollingHash(rndString, stragerSEED) % TABLE_SIZE] += 1;
        //bucket[jenkins_one_at_a_time_hash(rndString) % TABLE_SIZE] += 1;
        
        //bucket[ElfHash(rndString) % TABLE_SIZE] += 1;
        //bucket[fnv_1a_SEED(rndString) % TABLE_SIZE] += 1;
    }
    // BEST size = 100: 1297, 31, 59, 1, 211, 401, 23, 11, 7, 9, 3
    // BEST size = 64:  Pretty much the same

    // Get messures of uniformity for bucket fill-levels
    double meanSqrError = 0, variance = 0;

    for (size_t i=0; i<TABLE_SIZE; i++) {
        printf("%.2f\t ", bucket[i] / meanFullness);

        meanSqrError += pow(abs( meanFullness - bucket[i] ), 3) / TOTAL_STRINGS;
        variance += pow(meanFullness - bucket[i], 2) / (TOTAL_STRINGS - 1);
    }

    printf("\n\nMeanCubedError: %.2f\n", meanSqrError);
    printf("Std. deviation: %.2lf\n", pow(variance, 0.5));
    
    printf("\nHash string: %u\n", rollingHash("Hello, world!", stragerSEED) % TABLE_SIZE);

}