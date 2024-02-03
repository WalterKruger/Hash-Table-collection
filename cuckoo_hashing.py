from random import randint

class CuckooHashTable:
    def __init__(self, size:int = 8, load_factor:float = 0.5):
        self.elements = 0
        self.load_factor = load_factor          # Maximum allowed full-ness

        self.table1 = [None] * (size // 2)
        self.table2 = [None] * (size // 2)

        self.seed1 = 1
        self.seed2 = randint(2, (2<<31) - 1)    # Random for testing purposes
    
    def __len__(self): return self.elements

    def __str__(self):
        dict_str = ""
        for keyValue in self.items():
            dict_str += f"{repr(keyValue[0])}: {repr(keyValue[1])}, "
        
        return "{"+str(dict_str[:-2])+"}"
    __repr__ = __str__      # Some methods use this calls for printing

    def __iter__(self):
        self.iter_table = self.table1   # Table to currently search
        self.iter_index = 0             # Index into that table
        return self
    
    def __next__(self):
        while True:
            # Step foward until find non-none
            for i in range(self.iter_index, len(self.iter_table)):
                if self.iter_table[i] is not None:
                    self.iter_index = i+1           # Step foward as 'i' is at visted
                    return self.iter_table[i][0]    # Only return key
            
            # Restart search at table2
            if self.iter_table == self.table1:
                self.iter_table = self.table2
                self.iter_index = 0
                continue

            del self.iter_table
            del self.iter_index
            raise StopIteration
        


    def hash_function(self, keyToHash, seed:int) -> int:
        PRIME = 65537
        MAX_BITS = (1<<31) - 1    # Largest 32-bit (signed) representation

        # Create hash by "rolling" all characters
        hash = 0
        for char in str(keyToHash):
            hash = (hash * PRIME * seed + ord(char)) % MAX_BITS

        return hash % len(self.table1)
    
    def isOverLoaded(self) -> bool:
        return self.load_factor < self.elements / ( len(self.table1) + len(self.table2) )
    
    def items(self) -> list[tuple]:
        return [keyValue for keyValue in (self.table1 + self.table2) if keyValue is not None]

    def __setitem__(self, keyToInsert:str, dataToAdd):
        if self.isOverLoaded(): self.doubleTableSize()

        keyValue_toInsert = (keyToInsert, dataToAdd)
        print(f"\nInserting {keyValue_toInsert}...")

        # Hash into both tables, searching for empty slots or matching keys 
        for tableToSearch, tableSeed in zip([self.table1, self.table2] , [self.seed1, self.seed2]):
            bucket = self.hash_function(keyToInsert, tableSeed)
            # If bucket is empty, simply place the data there
            if tableToSearch[bucket] is None:
                tableToSearch[bucket] = keyValue_toInsert
                self.elements += 1

                print(f"\tEmpty slot! \t{bucket}")
                return

            # If the key matches, just replace the existing value
            if tableToSearch[bucket][0] == keyToInsert:
                print(f"\tReplacing value [{tableToSearch[bucket][1]} -> {dataToAdd}]")

                tableToSearch[bucket] = keyValue_toInsert
                return
        
        current_table, other_table = self.table2, self.table1   # Just calculated table2's hash

        # Keep evicting keys to the other table until an open slot is found
        for maxFails in range(20):
            # Evict the current key and insert the new key into alternate table
            evictedKeyValue = current_table[bucket]
            current_table[bucket] = keyValue_toInsert

            print(f"\tEvicting {evictedKeyValue} with {keyValue_toInsert}\t{bucket}")

            # Find if place in other table is open for evicted
            bucket = self.hash_function(evictedKeyValue[0], (self.seed1) if(other_table == self.table1)else (self.seed2) )

            if other_table[bucket] is None:
                other_table[bucket] = evictedKeyValue
                self.elements += 1

                print(f"\tEmpty slot found! \t{bucket}")
                return

            # Swap tables used for next loop
            current_table, other_table = other_table, current_table
            keyValue_toInsert = evictedKeyValue

        # If it failed too many times (a cycle?), resize both table
        print("\nFAILED TOO MANY TIMES")
        # Find anywhere to put homeless keyValue pair
        if   None in self.table1: self.table1[self.table1.index(None)] = keyValue_toInsert
        elif None in self.table2: self.table2[self.table2.index(None)] = keyValue_toInsert

        self.doubleTableSize()

    # Re-hashes all elements into a double sized table
    def doubleTableSize(self):
        print(f"\nDOUBLING TABLE SIZE! {len(self.table1)} -> {len(self.table1)*2}")
        # Save all current key-values
        keyValues = self.items()

        # Clear and double current table
        self.table1 = [None] * ( len(self.table1) * 2 )
        self.table2 = [None] * ( len(self.table2) * 2 )

        self.elements = 0   # Re-adding will double count

        for key, value in keyValues:
            self.__setitem__(key, value)
    
    def __getitem__(self, keyToFind):
        # Search both table for matching key
        for tableToSearch, tableSeed in zip([self.table1, self.table2], [self.seed1, self.seed2]):
            bucket = self.hash_function(keyToFind, tableSeed)
            #print(f"{keyToFind} -> {bucket}")

            if tableToSearch[bucket] == None: continue

            if tableToSearch[bucket][0] == keyToFind:
                return tableToSearch[bucket][1]
        
        raise KeyError(f"{repr(keyToFind)} isn't in table")
    
    # TODO: May want to shrink table when below 1/4 load factor...
    def pop(self, keyToRemove):
        # Search both table for matching key
        for tableToSearch, tableSeed in zip([self.table1, self.table2], [self.seed1, self.seed2]):
            bucket = self.hash_function(keyToRemove, tableSeed)

            if tableToSearch[bucket] == None: continue

            if tableToSearch[bucket][0] == keyToRemove:
                removedValue = tableToSearch[bucket][1]
                tableToSearch[bucket] = None

                self.elements -= 1
                return removedValue
        
        raise KeyError(keyToRemove)


def main():
    # Example usage:
    hTable = CuckooHashTable(size=4, load_factor=0.75)

    # Random strings
    """for samples in range(20):
        to_hash = ""
        for char_lenght in range( randint(3, 6) ):
            to_hash += chr(randint(ord('a'), ord('z')))
        
        cuckoo_table.insert(to_hash)"""

    stringToHash = "A much later advance was the development of the idea that 0 can be considered as a number, with its own numeral. The use of a 0 digit in place-value notation (within other numbers) dates back as early as 700 BCE by the Babylonians, who omitted such a digit when it would have been the last symbol in the number. The Olmec and Maya civilizations used 0 as a separate number as early as the 1st century BCE, but this usage did not spread beyond Mesoamerica. The use of a numeral 0 in modern times originated with the Indian mathematician Brahmagupta in 628 CE. However, 0 had been used as a number in the medieval computus (the calculation of the date of Easter), beginning with Dionysius Exiguus in 525 CE, without being denoted by a numeral. Standard Roman numerals do not have a symbol for 0; instead, nulla (or the genitive form nullae) from nullus, the Latin word for none, was employed to denote a 0 value."

    for i, string in enumerate(stringToHash.split(" ")):
        hTable[string] = i

    hTable["Gödel"] = 999

    #print("\nTable 1:", cuckoo_table.table1)
    #print("\nTable 2:", cuckoo_table.table2)

    # Verify that all inputted strings can be retrieved
    #for string in stringToHash.split(" "): print(hTable[string])

    print(hTable.pop("Gödel"))     # Does pop() work?

    print(hTable)
    #for key, value in hTable.items(): print(f"{repr(key)}: {repr(value)}")

    # Custom iterator test
    #for key in hTable: print(key)

    print( hTable.elements )
    print( len(hTable.table1) + len(hTable.table2) )


if __name__ == "__main__": main()