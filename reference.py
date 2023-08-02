import numpy as np


def print_cxx_style(np_arr):
    if np_arr.shape == ():
        print(np_arr, end=", ")
        return

    print("{", end="")
    for i in range(np_arr.shape[0]):
        print_cxx_style(np_arr[i])
    print("}, ")





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

arr2 = np.array([[[1,2,3], [4,5,6], [7,8,9]],
                 [[10,11,12], [13,14,15], [16,17,18]],
                 [[19,20,21], [22,23,24], [25,26,27]]])


print_cxx_style(arr2[:,1::-1,1])
print_cxx_style(arr2[::-1,1::-1,1])
