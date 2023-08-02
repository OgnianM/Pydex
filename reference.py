import numpy as np


arr = [i for i in range(10)]

print(arr[:]) # slice
print(arr[2:]) # pos start
print(arr[-4:]) # neg start
print(arr[:5]) # pos end
print(arr[:-3]) # neg end
print(arr[2:5]) # pos start and end
print(arr[-4:-1]) # neg start and end
print(arr[2:5:2]) # pos start, end, and step
print(arr[-4:-1:2]) # neg start, end, and step

print(arr[::-3]) # slice
print(arr[2::-1]) # pos start
print(arr[-4::-4]) # neg start
print(arr[:5:2]) # pos end
print(arr[:-3:-3]) # neg end
print(arr[2:5:5]) # pos start and end
print(arr[-4:-1:-1]) # neg start and end
print(arr[2:5:2]) # pos start, end, and step
print(arr[-4:-1:2]) # neg start, end, and step

