import sys
import os

nums = ['0', '5', '10', '15']

nums_dict =  {}
for k in nums:
    nums_dict[k] = [] 

with open("aws_fib_per_node.csv", "r") as f:
    for line in f.readlines():
        num, rest = line.split(',', maxsplit=1)
        if num in nums:
            nums_dict[num].append(line)

for k in nums_dict.keys():
    with open(f"lambda_{k}.csv", 'w+') as f:
        for line in nums_dict[k]:
            f.write(line)

