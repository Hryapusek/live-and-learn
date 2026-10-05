import numpy as np

a = np.arange(12, dtype=np.int32).reshape(3, 4)

b = a.T

c = a[:, ::-1]

d = a[::2, ::2]