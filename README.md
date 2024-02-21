# Overview
A collection of different implementations of the hash table data structure. They mostly differ by their strategies of collection resolution. 
Some implementations are written in Python well others in C.

### Separate chaining
*Files: `chaining.c` & `chaining.py`*

Each bucket is comprised of a linked list. When a collision occurs, a new node is added to the end of the linked list chain. 
The entire chain must be searched for most operations, but the length of each chain is expected to be short, therefor has an amortized time of $`Θ{(1)}`$. 
Arguably the simplest method.

### ‘Perfect’ hashing
*File: `perfectOrdered_hashTable.c`*

Whenever there is a collision, all items are re-hashed with a separate seed. 
Repeat until there are no more collisions. 
If it fails to find a seed without a collision after ~100 attempts, the table’s size is double. 
Guaranteed worst-case $`O{(1)}`$ search and deletion, but with expensive insertion.

**NOTE: Not working properly. Gets stuck in an infinite loop with modest input sizes**

### Cuckoo hashing
*File: `cuckoo_hashing.py`*

A form of perfect hashing where each item has two potential tables they can occupy. During collisions, the new item evicts the current occupant to its alternate position in the other table. 
This process is repeated until an empty bucket is found, or if too many evictions occur.
The table is doubled in size in the case of the latter.

This ensures that each item has only two potential locations, so guarantees a $`O{(1)}`$ for deletion, and search.
Insertion is a much cheaper than the above perfect strategy, but given the two potential locations, it may have to hash the same item twice.


### Robin Hood
*File: `RobinHood.py`*

A form of open addressing where the distance to their original bucket is minimized on average by "taking from the rich". 

Works by having all items store their offset to the bucket they were originally hashed to. 
When a new item is inserted but there is a collision, it keeps searching adjacent buckets until either an open bucket is found, or another item has a lesser offset. 
The item to add is exchanged with the lesser offset one, and the process is repeated. 
This minimizes the average distance from the original bucket, allowing for relatively fast lookup, deletion, and insertion, even with a very high load factor.

# Additional
### Statistical measuring
*Files: `hashFunction_compare.c` & `hashFunction_compare_OLD.py`*

Measures how well distributed different hash functions are.

## Bloom filters
*File: `Bloomfilter.py`*

A hash table-like probabilistic data structure that uses a bit array to filter out all true positives.
Works by deliberately allowing collisions but assigning multiple buckets/bits per item.
This means that for a false positive to occur, all the buckets that one item is hashed to must already be occupied.

A lot more memory efficient than a hash table with similar performance, because the key to an item doesn’t have to be stored and only a single bit is needed to represent an occupied bucket.

### Counting bloom filter
*File: `CntBloomFilter.c`*

Has the ability count the number of occurrences of a each item.
This specific implementation will dynamically re-size the structure to prevent buckets from overflowing.
Uses a larger, intermediate type to store bucket values when performing comparisons.
