from random import randint  # For generating testing strings


class RH_Node:
    def __init__(self, key:str, value):
        self.key = key
        self.value = value

        self.offSet = 0    # Off set from original position

    def __str__(self): return str((self.key, self.offSet))
    __repr__ = __str__


class RH_HashTable:
    def __init__(self, default_size:int =8, load_factor:float = 0.9):
        
        self.table = [None] * default_size

        self.load_factor = load_factor
        self.maxOffSet = 0

        self.elements = 0
    
    def __len__(self): return self.elements

    def __str__(self):
        objAsString = ""
        for bucketNode in self.table:
            if bucketNode == None: continue
            objAsString += f"{repr(bucketNode.key)}: {repr(bucketNode.value)}, "
        
        return "{"+objAsString[:-2]+"}"
    
    def hash_function(self, keyToHash) -> int:
        PRIME = 65537
        MAX_BITS = (1<<31) - 1    # Largest 32-bit (signed) representation

        # Create hash by "rolling" all characters
        hash = 0
        for char in str(keyToHash):
            hash = (hash * PRIME + ord(char)) % MAX_BITS

        return hash % len(self.table)
    
    def items(self) -> list[tuple]:
        return [(node.key, node.value) for node in self.table if node != None]

    def doubleTableSize(self):
        print(f"\n##DOUBLING TABLE SIZE {len(self.table)} -> {len(self.table)*2}")
        keyValues = self.items()

        self.table = [None] * (len(self.table) * 2)

        self.elements = 0   # To prevent double counting
        self.maxOffSet = 0  # New table may have diffrent max offset

        for key, value in keyValues:
            self.__setitem__(key, value)

    
    def isOverLoaded(self) -> bool:
        return self.load_factor < self.elements / len(self.table)
    
    def __setitem__(self, keyToAdd, dataToAdd):

        bucket = self.hash_function(keyToAdd)

        print(f"\nInserting {keyToAdd} -> {bucket}", end="")

        # Find open bucket and if key is already hash funtion
        nodeToPlace = RH_Node(keyToAdd, dataToAdd)
        closestOpenI = None
        
        for offSet in range(len(self.table)):
            searchI = (bucket + offSet) % len(self.table)    # So it will wrap to start
            print(f"\n\t{searchI}   ", end="")

            # Always check if matches current key
            if self.table[searchI] != None:
                if self.table[searchI].key == keyToAdd:
                    # Replace the value of the matching key
                    print(f"Replacing value {self.table[searchI].value} -> {dataToAdd}", end="")

                    self.table[searchI].value = dataToAdd
                    return
            
            # We don't need to search for an empty bucket if we already found one 
            if closestOpenI != None:
                # Matching key must be within the max off-set
                if offSet > self.maxOffSet: break
                continue

            # Save the index of closest open bucket
            if self.table[searchI] == None:
                print("FOUND OPEN!", end="")
                closestOpenI = searchI
                # Matching key must be within the max off-set
                if offSet > self.maxOffSet: break
                continue        # Still need to search for matching key

            print(f"Occupied! [{nodeToPlace.offSet} vs {self.table[searchI].offSet}]", end="")

            # Swap toPlace with smaller offSet node to reduce average offset
            if nodeToPlace.offSet > self.table[searchI].offSet:
                print(f" Now placing {self.table[searchI]}", end="")

                self.table[searchI], nodeToPlace = (nodeToPlace, self.table[searchI])

            nodeToPlace.offSet += 1
            # Node may be the new greatest offset
            if nodeToPlace.offSet > self.maxOffSet:
                self.maxOffSet = nodeToPlace.offSet
            

        print("")   # For new line as prevous
        self.table[closestOpenI] = nodeToPlace

        # Check for overloaded now as we had to search for matching key
        self.elements += 1
        if self.isOverLoaded(): self.doubleTableSize()

    def __getitem__(self, keyToFind):
        bucket = self.hash_function(keyToFind)

        for offSet in range(self.maxOffSet + 1):
            searchI = (bucket + offSet) % len(self.table)    # So it will wrap to start

            if self.table[searchI] == None: continue

            if self.table[searchI].key == keyToFind:
                return self.table[searchI].value
        
        raise KeyError(f"'{keyToFind}' not in table")
    
    # Remove a node by its key and return the removed's value
    def pop(self, keyToRemove):
        bucket = self.hash_function(keyToRemove)

        for offSet in range(self.maxOffSet + 1):
            searchI = (bucket + offSet) % len(self.table)    # So it will wrap to start

            if self.table[searchI] == None: continue

            if self.table[searchI].key == keyToRemove:
                removedValue = self.table[searchI].value
                
                self.table[searchI] = None
                self.elements -= 1

                return removedValue
        
        raise KeyError(f"'{keyToRemove}' not in table")

def main():
    a = RH_HashTable(default_size=16, load_factor=0.9)

    # Generate random strings
    """randomStrs = []
    for totalStrings in range(1000):
        currentString = ""
        for char_lenght in range( randint(3, 6) ):
            currentString += chr(randint(ord('a'), ord('z')))
        
        randomStrs.append(currentString)
        
    for i, randomSample in enumerate(randomStrs):
        a[randomSample] = i"""

    stringToHash = "A much later advance was the development of the idea that 0 can be considered as a number, with its own numeral. The use of a 0 digit in place-value notation (within other numbers) dates back as early as 700 BCE by the Babylonians, who omitted such a digit when it would have been the last symbol in the number. The Olmec and Maya civilizations used 0 as a separate number as early as the 1st century BCE, but this usage did not spread beyond Mesoamerica. The use of a numeral 0 in modern times originated with the Indian mathematician Brahmagupta in 628 CE. However, 0 had been used as a number in the medieval computus (the calculation of the date of Easter), beginning with Dionysius Exiguus in 525 CE, without being denoted by a numeral. Standard Roman numerals do not have a symbol for 0; instead, nulla (or the genitive form nullae) from nullus, the Latin word for none, was employed to denote a 0 value."

    for i, string in enumerate(stringToHash.split(" ")):
        a[string] = i

    #for i in range(30): a[i] = 0

    print(a.table)            # __str__ test
    print(a.maxOffSet)
    print(len(a.table))

    """randomStrs = tuple(set(randomStrs))    # Make set to remove duplicates
    for i, string in enumerate(randomStrs):
        print(f"{string}:\t{a.pop(randomStrs[i])}")

    print(a)"""

    


if __name__ == "__main__": main()
