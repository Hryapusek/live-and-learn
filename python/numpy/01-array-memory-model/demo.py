import numpy as np


a = np.arange(12, dtype=np.int32).reshape(3, 4)

print(a)
print("shape:   ", a.shape)
print("strides: ", a.strides)
print("itemsize:", a.itemsize)

print()

b = a[:, ::2]

print(b)
print("shape:   ", b.shape)
print("strides: ", b.strides)

print()

b[0, 0] = 999

print(a)