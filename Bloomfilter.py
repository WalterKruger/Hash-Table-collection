from sys import getsizeof
from random import randint  # For generating seeds
from math import log        # For generating seeds

class BloomFilter:
    def __init__(self, elementsToAdd:int, bitsPerElement:int = 10):

        self.bitarray = bytearray( (elementsToAdd * bitsPerElement + 7) // 8 )
        
        self.seeds = []
        # Get the optimal number of unique random seeds
        while len(self.seeds) < bitsPerElement * log(2):
            randomNum = randint(1, (1<<31) - 1)
            if randomNum not in self.seeds:
                self.seeds.append(randomNum)
    

    def hash(self, key:str, seed:int):
        prime = 65537
        max_bits = (1<<31) - 1    # Largest 32-bit representation
        hash = 0
        for char in str(key):
            hash = (hash * prime * seed + ord(char)) % max_bits
        return hash % (len(self.bitarray) * 8)
    
    def setBit(self, bitPos:int):
        BITS_IN_BYTE = 8

        self.bitarray[bitPos // BITS_IN_BYTE] |= 1 << (bitPos % BITS_IN_BYTE)

    def add(self, key:str):
        print(key, end="")
        # Hash n unquie locations, which represent the key
        for seed in self.seeds:
            print(f" {self.hash(key, seed)},", end="")
            self.setBit(self.hash(key, seed))
        
        print("")

    def getBit(self, bitPos:int) -> bool:
        BITS_IN_BYTE = 8
        bitMask = 1 << (bitPos % BITS_IN_BYTE)

        return self.bitarray[bitPos // BITS_IN_BYTE] & bitMask != 0
    
    # Python's 'in' dunder method
    def __contains__(self, key:str) -> bool:
        print(key, end="\t")
        # If any location doesn't match
        for seed in self.seeds:
            print(f"{self.hash(key, seed)}, ", end="")
            if self.getBit(self.hash(key, seed)) == False:
                return False
        
        return True




def main():
    """nums = [1,2,3]

    print(f"Bytearray 2: {getsizeof(bytearray([1,2]))}")
    print(f"Bytearray 3: {getsizeof(bytearray([1,2,3]))}")

    print(f"\nInt array 3: {getsizeof(nums)}")"""


    keys = ["Walter", "Place", "John", "Mark"]

    a = BloomFilter(elementsToAdd=len(keys))
    print(a.seeds)

    for key in keys: a.add(key)

    # Print bits in byte array
    index = 0
    for byte in a.bitarray:
        for bit in reversed("0" * ( 8 - len(bin(byte)[2:]) ) + bin(byte)[2:]):
            print(f" {index}: {bit}", end="\t")
            index += 1
        print("")
            

    print("\n#GET ('in' dunder method)")
    for key in keys + ["Apple"]:
        print(key in a)


if __name__ == "__main__": main()