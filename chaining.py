
class HashTable:
    def __init__(self, load_factor:float=0.75, default_size:int=8):
        self.elements = 0   # Table starts empty
        
        if not(load_factor > 0):
            raise ValueError("Load factor: Must be positive")
        self.load_factor = load_factor

        self.table = [None]*default_size
    
    def __len__(self): return self.elements

    def __str__(self):
        dict_str = ""
        for item in self.items():
            dict_str += f"{repr(item[0])}: {repr(item[1])}, "
        
        return "{"+str(dict_str[:-2])+"}"

    def items(self):
        item_list = []
        for bucket in self.table:
            if not bucket: continue
            for item in bucket:
                item_list.append(item)
        
        return item_list
    
    def over_loaded(self):
        return self.load_factor < self.elements/len(self.table)
    
    def extend(self):
        # Create a copy of all elements
        to_re_add = self.items()
        # Create new table, double the size
        self.table = [None] * (len(self.table)*2)

        # Re-add all elements
        self.elements = 0   # Re-adding will double count
        for item in to_re_add:
            self.__setitem__(item[0], item[1])
    
    def hash(self, key):
        prime = 65537
        max_bits = 2**31 - 1    # Largest 32-bit representation
        hash = 0
        for char in str(key):
            hash = (hash * prime + ord(char)) % max_bits
        return hash % len(self.table)
    
    def __setitem__(self, key, value):
        if not hasattr(key, "__hash__"):
            raise TypeError(f"unhashable type: {type(key)}")

        self.elements += 1
        if self.over_loaded(): self.extend()

        bucket = self.hash(key)
        key_value = (key, value)

        # If bucket is un-used
        if not self.table[bucket]:
            self.table[bucket] = [key_value]
            return
        
        # Search bucket for item
        inBucket = False    # Is key in bucket already?
        key_i = 0           # Index in bucket's list
        for item in self.table[bucket]:
            if item[0] == key:
                inBucket = True
                break
            key_i += 1

        if inBucket:    # Replace current's value
            self.table[bucket][key_i] = key_value
        else:           # Add to end of bucket
            self.table[bucket].append(key_value)
    
    def __getitem__(self, key):
        bucket = self.hash(key)
        if self.table[bucket]:
            for item in self.table[bucket]:
                if item[0] == key: return(item[1])
        
        raise KeyError(f"{repr(key)} isn't in table")


# Example use case
a = HashTable()

a[1] = "One"
a["Two"] = 2
a["Three"] = 3
a["Four"] = 4
print(a.table)
a["Five"] = 5
a["Six"] = 6
a["Seven"] = 7


print(a[1])
print(a)