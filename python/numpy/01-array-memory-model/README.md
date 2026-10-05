# NumPy Array Memory Model — Session 01

## Initial array

```python
import numpy as np

a = np.arange(12, dtype=np.int32).reshape(3, 4)
```

Logically, `a` looks like:

```text
[
    [0,  1,  2,  3],
    [4,  5,  6,  7],
    [8,  9, 10, 11]
]
```

Each element is an `int32`, so every element occupies **4 bytes**.

A useful simplified model of a NumPy array is:

```cpp
struct ndarray_like
{
    std::byte* logical_origin;

    std::vector<std::size_t> shape;
    std::vector<std::ptrdiff_t> strides;

    DType dtype;

    // Ownership / reference to backing storage.
};
```

The important idea is that the logical representation of an array is described by metadata such as its **shape**, **strides**, **dtype**, and starting address.

---

# Task 1

## 1. What should `a.strides` be?

```python
a.strides == (16, 4)
```

A stride tells us how many **bytes** the memory address changes when the corresponding array index increases by one.

For a two-dimensional array:

```text
address(a[i, j])
    =
logical_origin
    + i * strides[0]
    + j * strides[1]
```

For `a`:

```text
strides[0] = 4 elements × 4 bytes = 16 bytes
strides[1] = 1 element  × 4 bytes = 4 bytes
```

Therefore:

```text
a.strides = (16, 4)
```

For example:

```text
address(a[2, 3])

offset =
    2 × 16
  + 3 × 4
  = 44 bytes
```

---

## 2. What happens after slicing with `::2`?

```python
b = a[:, ::2]
```

The first `:` selects every row.

The second `::2` selects every second column, starting from column zero.

Therefore:

```text
b =
[
    [0,  2],
    [4,  6],
    [8, 10]
]
```

Its shape is:

```python
b.shape == (3, 2)
```

---

## 3. What are `b.strides`?

My initial answer was:

```text
(8, 4)
```

This was wrong.

The correct answer is:

```python
b.strides == (16, 8)
```

The important realization is:

> Strides are not determined only by the logical shape of an array.

`b` is a view into `a`.

Moving to the next row still requires moving through one complete original row:

```text
b[0,0] = a[0,0]
b[1,0] = a[1,0]

distance = 16 bytes
```

Moving to the next column skips one original element:

```text
b[0,0] = a[0,0]
b[0,1] = a[0,2]

distance = 8 bytes
```

Therefore:

```text
shape   = (3, 2)
strides = (16, 8)
```

A newly allocated contiguous array with the same logical values could instead have:

```text
shape   = (3, 2)
strides = (8, 4)
```

So two arrays can have identical logical contents and shape while having completely different memory layouts.

---

## 4. Does `b` contain a copy?

No.

Basic NumPy slicing normally creates a **view** into existing memory.

Therefore `b` refers to the same underlying storage as `a`.

A useful check is:

```python
np.shares_memory(a, b)
```

which returns `True`.

Checking:

```python
b.base is None
```

can also provide information about ownership, but `.base` should not be treated as a universal test for whether two particular arrays share memory. Views can form chains and may reference some earlier backing object.

Explicit operations include:

```python
arr.copy()
arr.view()
```

A copy receives independent storage, while a view continues referring to existing storage.

---

## 5. What happens after modifying `b`?

```python
b[0, 0] = 999
```

Because `b` shares its storage with `a`, this changes the same memory location represented by:

```python
a[0, 0]
```

Therefore `a` becomes:

```text
[
    [999, 1,  2,  3],
    [4,   5,  6,  7],
    [8,   9, 10, 11]
]
```

---

# Bonus: Can an array change its logical layout without moving its data?

Yes.

An `ndarray` can represent many different logical layouts simply by changing metadata such as:

```text
shape
strides
logical origin
```

The logical elements of an array do **not** necessarily occupy contiguous locations.

For example:

```python
b = a[:, ::2]
```

logically selects:

```text
0       2
4       6
8      10
```

even though the skipped elements still exist between them in the backing storage.

Therefore an ndarray should not be thought of simply as:

```text
contiguous buffer + dimensions
```

A better model is:

```text
backing storage
    +
logical starting address
    +
shape
    +
strides
    +
dtype
```

---

# Task 2

Given:

```python
a = np.arange(12, dtype=np.int32).reshape(3, 4)
```

```text
a =
[
    [0,  1,  2,  3],
    [4,  5,  6,  7],
    [8,  9, 10, 11]
]
```

## `b = a.T`

The transpose is:

```text
b =
[
    [0, 4,  8],
    [1, 5,  9],
    [2, 6, 10],
    [3, 7, 11]
]
```

Its metadata is:

```text
shape   = (4, 3)
strides = (4, 16)
```

No element data needs to be rearranged.

The original strides:

```text
(16, 4)
```

are effectively exchanged:

```text
(4, 16)
```

Therefore `b` is a **view**.

---

## `c = a[:, ::-1]`

The result is:

```text
c =
[
    [3,  2,  1, 0],
    [7,  6,  5, 4],
    [11, 10, 9, 8]
]
```

My initial assumption was that this required a copy because it was unclear how normal strides could describe reversed memory.

That assumption was wrong.

The metadata is:

```text
shape   = (3, 4)
strides = (16, -4)
```

The crucial idea is that **strides can be negative**.

For the first row, the logical starting point becomes the element containing `3`:

```text
memory:

0    1    2    3
              ^
              c[0,0]
```

Then each increment of the second logical index moves backward by four bytes:

```text
c[0,0] = 3
c[0,1] = 2   -> -4 bytes
c[0,2] = 1   -> -4 bytes
c[0,3] = 0   -> -4 bytes
```

Therefore:

```text
address(c[i,j])
    =
logical_origin
    + i × 16
    + j × (-4)
```

No data needs to be copied or reversed physically.

`c` is a **view**.

---

## `d = a[::2, ::2]`

The first `::2` selects every second row:

```text
row 0
row 2
```

The second `::2` selects every second column:

```text
column 0
column 2
```

Therefore:

```text
d =
[
    [0,  2],
    [8, 10]
]
```

Its shape is:

```text
shape = (2, 2)
```

Moving between logical rows skips two original rows:

```text
2 × 16 = 32 bytes
```

Moving between logical columns skips two original columns:

```text
2 × 4 = 8 bytes
```

Therefore:

```text
strides = (32, 8)
```

Again, no copy is necessary.

`d` is a **view**.

---

# Main conclusions

### Strides describe address movement

For a two-dimensional array:

```text
address(i, j)
    =
logical_origin
    + i × stride[0]
    + j × stride[1]
```

Strides are measured in **bytes**, not elements.

### Shape does not determine memory layout

Two arrays can have:

```text
shape = (3, 2)
```

while having completely different strides and physical layouts.

### Views can skip memory

```python
a[:, ::2]
```

can represent every second element without creating another buffer.

### Views can traverse memory backwards

```python
a[:, ::-1]
```

uses a negative stride.

### Transposition does not necessarily move data

```python
a.T
```

can be implemented by changing shape and strides.

### Logical layout and physical layout are separate concepts

This is the central idea from this session.

An `ndarray` is not simply a multidimensional container of values. It is a logical interpretation of some memory according to metadata describing how that memory should be traversed.