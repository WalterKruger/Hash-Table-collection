#include <stdio.h>
#include <stdlib.h> // Dynamic mem
#include <string.h> // For hash's key
#include <math.h>   // Power

// Malloc with null return check
#define malloc_safe(variable, type, bytes) do {\
    variable = (type)malloc(bytes);\
    if(variable==NULL) {\
        perror("Memory allocation failed!\n"); exit(0);}\
    } while(0)

// Calloc with null return check
#define calloc_safe(variable, type, arr_len) do {\
    variable = (type*)calloc(arr_len, sizeof(type));\
    if(variable==NULL) {\
        perror("Memory allocation failed!\n"); exit(0);}\
    } while(0)

struct Hash_Data {
    char *key;
    int value;

    struct Hash_Data *next;
};

struct Hash_Data* createNode(const char *key, const int value) {
    struct Hash_Data* nodeToAdd;
    malloc_safe(nodeToAdd, struct Hash_Data*, sizeof(struct Hash_Data));
    // Key's lenght can be variable
    calloc_safe(nodeToAdd->key, char, strlen(key)+1);

    strcpy(nodeToAdd->key, key);
    nodeToAdd->value = value;
    nodeToAdd->next = NULL;

    return nodeToAdd;
}

struct HashTable {
    struct Hash_Data *table;

    float load_factor;
    size_t elements;
    size_t size;
};

void HTable_set();


size_t hash_fun(const char *key) {
        const unsigned PRIME = 65537;
        const unsigned MAX_BITS = ~0;    // Largest 32-bit representation
        unsigned hash = 0;
        // For every character
        for (size_t i=0; key[i] != '\0'; i++) {
            hash = (hash * PRIME + (int)key[i]) % MAX_BITS;
        }
        return hash;
}

struct HashTable* createHTable() {
    const size_t STARTING_SIZE = 8;
    const float LOAD_FACTOR = 0.75;

    struct HashTable* NewTable;
    malloc_safe(NewTable, struct HashTable*, sizeof(struct HashTable));
    // Table size can be variable
    //calloc_safe(NewTable->table, struct Hash_Data, STARTING_SIZE);
    NewTable->table = (struct Hash_Data*)calloc(STARTING_SIZE, sizeof(struct Hash_Data));

    NewTable->load_factor = LOAD_FACTOR;
    NewTable->size = STARTING_SIZE;
    NewTable->elements = 0;
    
    return NewTable;
}

int HTable_overloaded(struct HashTable *hTable) {
    return (hTable->load_factor) < ((float)hTable->elements / hTable->size);
}

void Htable_extend(struct HashTable *hTable) {
    printf("\n##Overloaded! Extending table...\n\n");
    struct Hash_Data *bucketLL;
    struct Hash_Data itemsCopy[hTable->elements-1];
    size_t save_i = 0;

    // For every "bucket"
    for (size_t bucket=0; bucket<hTable->size; bucket++) {
        if (hTable->table[bucket].key == NULL) {  continue;  }
        // Step through the LinkedList, and save a copy of every item
        bucketLL = &hTable->table[bucket];
        for (;bucketLL != NULL; bucketLL = bucketLL->next) {
            itemsCopy[save_i++] = *bucketLL;
        }
    }
    // Clear the memory for, then allocate double the size
    free(hTable->table);
    hTable->size *= 2;
    calloc_safe(hTable->table, struct Hash_Data, hTable->size);

    hTable->elements = 0;   // Prevents double counting

    for (size_t i=0; i < sizeof(itemsCopy)/sizeof(itemsCopy[0]); i++) {
        HTable_set(itemsCopy[i].key, itemsCopy[i].value, hTable);
    }
}

void HTable_set(const char *keyToAdd, const int valueToAdd, struct HashTable *hTable) {
    hTable->elements++;
    if (HTable_overloaded(hTable)) {  Htable_extend(hTable);  }

    const size_t bucket = hash_fun(keyToAdd) % hTable->size;

    // If bucket is empty
    if (hTable->table[bucket].key == NULL) {
        printf("Adding new element to start of bucket {'%s': %i}\n", keyToAdd, valueToAdd);
        hTable->table[bucket] = *createNode(keyToAdd, valueToAdd);
        return;
    }

    struct Hash_Data *cur_node = &hTable->table[bucket];

    // When a nodes key matches the keyToAdd
    if (strcmp(cur_node->key, keyToAdd)==0) {
        foundMatchingKey:

        printf("Replacing value in bucket {'%s': %i -> %i}\n", keyToAdd, cur_node->value, valueToAdd);
        cur_node->value = valueToAdd;
        hTable->elements--; // Not adding a "new" element
        return;
    }

    // Go to end of bucket's LinkedList
    for (;cur_node->next != NULL; cur_node = cur_node->next) {
        if (strcmp(cur_node->key, keyToAdd)==0) {
            goto foundMatchingKey;
        }
    }
    printf("Adding to end of bucket {'%s': %i}\n", keyToAdd, valueToAdd);
    // Add new value to end of LinkedList
    cur_node->next = createNode(keyToAdd, valueToAdd);
    
}

int HTable_get(const char *keyToFind, struct HashTable *hTable) {
    const size_t bucket = hash_fun(keyToFind) % hTable->size;
    
    // If key is at head of bucket
    if ( strcmp(hTable->table[bucket].key, keyToFind)==0 ) {
            return hTable->table[bucket].value;
        }

    struct Hash_Data *cur_node = &hTable->table[bucket];

    // Search bucket's LinkedList
    for (;cur_node != NULL; cur_node = cur_node->next) {
        if ( strcmp(cur_node->key, keyToFind)==0 ) {
            return cur_node->value;
        }
    }

    perror("Item not in table!\n");
    exit(0);
}

void LL_append(struct Hash_Data *head, struct Hash_Data *toAdd) {
    
    struct Hash_Data *cur_node = head;
    while (cur_node->next != NULL) {  cur_node = cur_node->next;  }

    cur_node->next = toAdd;
}

int main() {
    /*
    struct Hash_Data *head = createNode("Head", 1);
    LL_append(head, createNode("Middle", 2));
    LL_append(head, createNode("tail", 3));

    printf("Head:\t{'%s': %d}\n", head->key, head->value);
    printf("Mid:\t{'%s': %d}\n", head->next->key, head->next->value);
    printf("Tail:\t{'%s': %d}\n", head->next->next->key, head->next->next->value);
    */
    
    struct HashTable *people = createHTable();
    HTable_set("Mike", 20, people);
    HTable_set("John", 7, people);
    HTable_set("Mike", 27, people);

    HTable_set("Peter", 1, people);
    HTable_set("Walter", 999, people);
    HTable_set("Ivils", 0, people);
    HTable_set("Fortasax", -1, people);

    printf("%i\n", HTable_get("Mike", people));
    printf("%i\n", HTable_get("John", people));
    printf("%i\n", HTable_get("Peter", people));
    printf("%i\n", HTable_get("Walter", people));
    printf("%i\n", HTable_get("Ivils", people));
    printf("%i\n", HTable_get("Fortasax", people));

    HTable_set("Patrica", 124, people);

}