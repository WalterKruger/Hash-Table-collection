#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h> // Dynamic mem
#include <stdint.h> // For exactly 8 bit objects
#include <string.h> // For hash's key

#include <math.h>   // For hash fun

// Malloc with null return check
#define malloc_safe(variable, type, bytes) do {\
    variable = (type)malloc(bytes);\
    if(variable==NULL) {\
        perror("Memory allocation failed!"); exit(0);}\
    } while(0)

// Calloc with null return check
#define calloc_safe(variable, type, arr_len) do {\
    variable = (type*)calloc(arr_len, sizeof(type));\
    if(variable==NULL) {\
        perror("Memory allocation failed!");exit(0);}\
    } while(0)

#define realloc_safe(variable, type, arr_len) do {\
    variable = (type*)realloc(variable, arr_len * sizeof(type));\
    if(variable==NULL) {\
        perror("Memory allocation failed!"); exit(0);}\
    } while(0)


/*
    Single bit bool array
*/
const size_t BYTES_BITS = 8;

uint8_t* createBitArray(const size_t elements) {
    const size_t byte_size = (elements-1) / BYTES_BITS + 1;

    uint8_t *boolArray;
    calloc_safe(boolArray, uint8_t, byte_size);
    return boolArray;
}

void setBit(int8_t *boolArray, const size_t bit_position, const bool toSetAs) {
    const size_t ARR_I = bit_position / BYTES_BITS;
    const uint8_t bitMask = 1 << (bit_position % BYTES_BITS);

    // (~bitMask & *boolArry) sets the position to be modifed to zero
    boolArray[ARR_I] = (bitMask * toSetAs) | ( ~bitMask & boolArray[ARR_I] );
}

bool getBit(const uint8_t *boolArray, const size_t bit_position) {
    const uint8_t bitMask = 1 << (bit_position % BYTES_BITS);

    // The shift at the end isn't needed as only 0 returns false (bool type)
    return (boolArray[bit_position / BYTES_BITS] & bitMask) >> bit_position;
}

/*
    HashTable
*/

struct Hash_Data {
    char *key;
    int value;

    struct Hash_Data *next;
    struct Hash_Data *previous;
};

struct Hash_Data* createNode(const char *key, const int value) {
    struct Hash_Data* nodeToAdd;
    malloc_safe(nodeToAdd, struct Hash_Data*, sizeof(struct Hash_Data));
    // Key's lenght can be variable
    calloc_safe(nodeToAdd->key, char, strlen(key)+1);

    strcpy(nodeToAdd->key, key);
    nodeToAdd->value = value;

    nodeToAdd->next = NULL;
    nodeToAdd->previous = NULL;

    return nodeToAdd;
}

struct HashTable {
    size_t elements;

    struct Hash_Data **table;
    size_t size;
    unsigned seed;

    struct Hash_Data *head;
    struct Hash_Data *tail;
};

struct HashTable* createHTable() {
    const size_t STARTING_SIZE = 8;

    struct HashTable* NewTable;
    malloc_safe(NewTable, struct HashTable*, sizeof(struct HashTable));
    // Table size can be variable
    calloc_safe(NewTable->table, struct Hash_Data*, STARTING_SIZE);

    NewTable->size = STARTING_SIZE;
    NewTable->elements = 0;

    NewTable->head = NULL;
    NewTable->tail = NULL;

    RAND_MAX;
    NewTable->seed = rand()+1; // Can't be zero
    
    return NewTable;
}

unsigned stragerSEED(unsigned hash, char charInStr, unsigned SEED) {
    return hash + (( SEED ^ charInStr) * 65537) / pow(2, 64);
}

size_t hash_fun(const char *key, const unsigned SEED) {
    const unsigned PRIME = 65537;
    const unsigned MAX_BITS = ~0;    // Largest representation

    // For every character
    unsigned hash = 42;
    for (size_t i=0; key[i] != '\0'; i++) {
        //hash = (hash * PRIME * SEED + (int)key[i]) % MAX_BITS;
        hash = stragerSEED(hash, key[i], SEED) % MAX_BITS;
    }
    return hash;
}

void hTable_reHash(struct HashTable *hTable) {
    //printf("\n## Extending table...\n");

    // The more full the table is, the less trys it will do
    unsigned max_fails = 100;
    unsigned failsAtSize = 0;
    uint8_t *bucketOccupied = createBitArray(hTable->size);

    //printf("## Max fails: %d\n", max_fails);

    for (size_t i=0; i<hTable->size; i++) hTable->table[i]=NULL;

    bool collisions = true;
    while (collisions) {
        collisions = false;
        // If failed too many times, double the size of the table
        if (failsAtSize++ > max_fails) {
            printf("## Limit exceeded! Table size '%d' -> '%d'\n", hTable->size, hTable->size*2);
            hTable->size *= 2;
            //// realloc() doesn't initialize to NULL
            //free(hTable->table);
            //calloc_safe(hTable->table, struct Hash_Data*, hTable->size);
            free(bucketOccupied);
            bucketOccupied = createBitArray(hTable->size);

            failsAtSize = 0;
            max_fails = 100 - 100*hTable->elements / hTable->size;  // Table size has changed
            printf("## Max fails: %d\n", max_fails);
        }

        hTable->seed = rand()+1;

        // ReHash every key by stepping through LinkedList
        struct Hash_Data *cur_node = hTable->head;
        for (;cur_node != NULL; cur_node = cur_node->next) {
            size_t cur_hash = hash_fun(cur_node->key, hTable->seed) % hTable->size;

            // hTable->table[cur_hash] != NULL
            if (getBit(bucketOccupied, cur_hash) == true) {
                printf("\tCollision! ['%s': %d]\n", cur_node->key, cur_hash);
                collisions = true;
                break;
            }
            setBit(bucketOccupied, cur_hash, true);
            //hTable->table[cur_hash] = cur_node;
        }
    }
    // Clears current table then reallocate to increased size
    free(hTable->table);
    calloc_safe(hTable->table, struct Hash_Data*, hTable->size);
    // ReHash then add all nodes
    struct Hash_Data*nodeToReAdd = hTable->head;
    for (;nodeToReAdd != NULL; nodeToReAdd = nodeToReAdd->next) {
        size_t cur_hash = hash_fun(nodeToReAdd->key, hTable->seed) % hTable->size;
        hTable->table[cur_hash] = nodeToReAdd;
    }

    free(bucketOccupied);
    printf("## Got new table after '%d' tries! [Seed: %d]\n\n", failsAtSize, hTable->seed);
}

void HTable_set(const char *keyToAdd, const int valueToAdd, struct HashTable *hTable) {
    size_t bucket = hash_fun(keyToAdd, hTable->seed) % hTable->size;
    struct Hash_Data *bucketData = hTable->table[bucket];

    // Is key already in table (with NULL safety check)
    if ((bucketData == NULL)? 0 : strcmp(bucketData->key, keyToAdd) == 0) {
        // Just replace existing value
        printf("Replacing {'%s': %i -> %i}\n", keyToAdd, bucketData->value, valueToAdd);
        bucketData->value = valueToAdd;
        return;
    }

    struct Hash_Data *nodeToAdd = createNode(keyToAdd, valueToAdd);

    // Initialise or append to LinkedList
    if (hTable->head == NULL) {
        hTable->head = nodeToAdd;
        hTable->tail = nodeToAdd;
    } else {
        nodeToAdd->previous = hTable->tail;
        hTable->tail->next = nodeToAdd;

        hTable->tail = hTable->tail->next;
    };

    hTable->elements += 1;
    printf("Adding {'%s': %i}\n", keyToAdd, valueToAdd);

    // Check for collisions
    if (bucketData == NULL) {
        hTable->table[bucket] = hTable->tail;
        return;
    }

    printf("Collision! ['%s' with '%s' ]\n", keyToAdd, bucketData->key);
    hTable_reHash(hTable);

}

int HTable_get(const char *keyToFind, const struct HashTable *hTable) {
    const size_t bucket = hash_fun(keyToFind, hTable->seed) % hTable->size;
    const struct Hash_Data *bucketData = hTable->table[bucket];

    // Is key already in table (with NULL safety check)
    if ((bucketData == NULL)? 0 : strcmp(bucketData->key, keyToFind) == 0) {
        return bucketData->value;
    }
    perror("Key not found!"); exit(-1);
}

void HTable_del(const char *keyToDel, struct HashTable *hTable) {
    const size_t bucket = hash_fun(keyToDel, hTable->seed) % hTable->size;
    struct Hash_Data *bucketData = hTable->table[bucket];

    if (bucketData == NULL) {
        perror("Key not found!"); exit(-1);
    }
    printf("Removing: {'%s': %d}\n", bucketData->key, bucketData->value);

    bucketData->previous->next = bucketData->next;
    bucketData->next->previous = bucketData->previous;

    free(bucketData);
    hTable->table[bucket] = NULL;
}

void print_HTable(const struct HashTable *hTable) {
    struct Hash_Data *cur_node = hTable->head;

    for (;cur_node != NULL; cur_node = cur_node->next) {
        printf("{'%s': %d} -> ", cur_node->key, cur_node->value);
    }
    printf("NULL\n");
}


unsigned rndNumber(const unsigned lowerBound, const unsigned upperBound) {
    const unsigned PRIME = 65537;
    return (rand()*PRIME) % (upperBound - lowerBound + 1) + lowerBound;
}

int main() {
    /*
    size_t buckets = 2;
    int **bucketOccupied;
    calloc_safe(bucketOccupied, int*, buckets);
    //for (size_t i=0; i<buckets; i++) {bucketOccupied[i]=0;}

    for (int i=0;i<buckets;i++) {  printf("%d: %p\n", i, bucketOccupied[i]);  }

    buckets *= 4;
    free(bucketOccupied);
    calloc_safe(bucketOccupied, int*, buckets);
    for (int i=0;i<buckets;i++) {  printf("\n%d: %p", i, bucketOccupied[i]);  }
    */
    
    struct HashTable *people = createHTable();
    printf("Seed: %d\n\n", people->seed);
    
    size_t MAX_LEN = 20;
    char rndString[MAX_LEN];
    size_t strPlace;
    
    for (size_t i=0; i<20; i++) {
        for (strPlace = 0; strPlace < rndNumber(3, MAX_LEN); strPlace++)
            rndString[strPlace] = rndNumber((int)'a', (int)'z');
        rndString[strPlace + 1] = '\0'; // Terminate string

        HTable_set(rndString, rand(), people);
    }
    

    HTable_set("Mike", 20, people);
    HTable_set("John", 7, people);
    HTable_set("Mike", 27, people);    
    HTable_set("Peter", 1, people);

    HTable_set("Walter", 999, people);
    HTable_set("Ivils", 0, people);
    HTable_set("Fortasax", -1, people);
    HTable_set("Dire", 546, people);
    HTable_set("Sally", -6, people);
    /*
    printf("%i\n", HTable_get("Mike", people));
    printf("%i\n", HTable_get("John", people));
    printf("%i\n", HTable_get("Peter", people));
    printf("%i\n", HTable_get("Walter", people));
    printf("%i\n", HTable_get("Ivils", people));
    printf("%i\n", HTable_get("Fortasax", people));
    */
    //HTable_set("Patrica", 124, people);
    printf("\n");

    print_HTable(people);

    printf("\nSize: %u\n", people->size);
    //HTable_del("John", people);
    //print_HTable(people);

    
}