import numpy as np

a = np.arange(20, dtype=np.int32).reshape(4, 5)

b = a[1:4:2, ::2]

c = a[[1, 3]]

d = a[:, [0, 2, 4]]

e = a[[3, 0, 2], :]
