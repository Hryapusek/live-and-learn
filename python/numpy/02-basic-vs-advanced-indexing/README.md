a =
[
    [ 0,  1,  2,  3,  4],
    [ 5,  6,  7,  8,  9],
    [10, 11, 12, 13, 14],
    [15, 16, 17, 18, 19]
]

b - view because its classic start:end:step selection
b =
[
    [ 5,  7,  9],
    [15, 17, 19]
]

c - copy because advanced indexing is used
c =
[
    [ 5,  6,  7,  8,  9],
    [15, 16, 17, 18, 19]
]

d - same as c
d =
[
    [ 0,  2,  4],
    [ 5,  7,  9],
    [10, 12, 14],
    [15, 17, 19]
]

e - same as c
e =
[
    [10, 11, 12, 13, 14],
    [ 0,  1,  2,  3,  4],
    [15, 16, 17, 18, 19]
]

This is copy because the striding would be too complicated. It would be more cheap to just copy the whole array. If someone needs such hard view - he could write the proxy class himself then
