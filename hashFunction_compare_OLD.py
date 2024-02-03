import random as r

def mean(array: list):
    sum_total = 0
    for number in array:
        sum_total += number
    return sum_total/len(array)

def djb2_hash(input_str):
    prime = 65537
    max_bits = 2**31 - 1 # Max 32-bit representation

    hash_value = 0
    for i, char in enumerate(input_str):
        #hash_value = ((hash_value << 5) + hash_value) + ord(char)
        #hash_value = (hash_value * prime + ord(char)) % max_bits

        hash_value = int(hash_value + (( 432 ^ ord(char)) * prime) / pow(2, 64))
    
        #hash_value += (ord(char) * (prime ** i))
        #hash_value = hash_value ^ ord(char)*prime
    #print(hash_value)
    return hash_value % 100

hash_count = [0]*100

for samples in range(100000):
    to_hash = ""
    for char_lenght in range(r.randint(1, 5)):
        to_hash += str(r.randint(ord('A'), ord('z')))
    
    #hash_count[hash(to_hash) % 100] += 1
    hash_count[djb2_hash(to_hash)] += 1
    #print( f"{djb2_hash(to_hash)}\t<- {to_hash}" )

count_mean = mean(hash_count)

empty_buckets = 0
std = 0
for i in range(len(hash_count)):
    if hash_count[i] == 0: empty_buckets += 1

    std += (hash_count[i] - count_mean)**2

    print(f"{i}: {hash_count[i]}\t", end="")
std = (std/len(hash_count))**0.5

print(f"\n\nEMPTY: {empty_buckets}")
print(f"Standard deviation: {std}")