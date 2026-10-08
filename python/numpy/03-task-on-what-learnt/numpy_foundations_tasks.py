"""
NumPy foundations practice.

Run:
    python numpy_foundations_tasks.py

Rules:
- Fill only the TODO functions.
- Do not change the tests.
- Prefer NumPy operations over Python loops.
- Tasks 1 and 2 MUST return views sharing memory with the input.
- Task 3 MUST return an independent copy.
- Tasks 4 and 5: do not use np.tile or np.repeat.
- Task 6: no Python loops.
- Task 7 intentionally requires reading NumPy documentation.

You may use NumPy docs freely.
"""

from __future__ import annotations

import traceback

import numpy as np


# ---------------------------------------------------------------------------
# Task 1 — stepped 2D view
# ---------------------------------------------------------------------------

def stepped_view(a: np.ndarray) -> np.ndarray:
    """
    Given a 2D array, return a VIEW containing:
      - rows 1, 3, 5, ...
      - columns 0, 2, 4, ...

    Example logical selection:
        a[1::2, ::2]

    Requirement:
        The result must share memory with `a`.
    """
    raise NotImplementedError


# ---------------------------------------------------------------------------
# Task 2 — reverse without copying
# ---------------------------------------------------------------------------

def reverse_columns_view(a: np.ndarray) -> np.ndarray:
    """
    Return a VIEW where every row is reversed.

    Example:
        [[1, 2, 3],
         [4, 5, 6]]

    becomes:
        [[3, 2, 1],
         [6, 5, 4]]

    Requirement:
        No copy. At least one stride should become negative.
    """
    raise NotImplementedError


# ---------------------------------------------------------------------------
# Task 3 — advanced indexing copy
# ---------------------------------------------------------------------------

def reordered_rows_copy(a: np.ndarray) -> np.ndarray:
    """
    Return rows in this order:

        3, 0, 2

    Requirement:
        Use advanced indexing.
        The result must NOT share memory with `a`.
    """
    raise NotImplementedError


# ---------------------------------------------------------------------------
# Task 4 — broadcasting: outer-style calculation
# ---------------------------------------------------------------------------

def add_every_pair(x: np.ndarray, y: np.ndarray) -> np.ndarray:
    """
    Given:

        x.shape == (N,)
        y.shape == (M,)

    return an array with shape:

        (N, M)

    where:

        result[i, j] == x[i] + y[j]

    Example:
        x = [1, 2]
        y = [10, 20, 30]

    result:
        [[11, 21, 31],
         [12, 22, 32]]

    Constraints:
        - no Python loops
        - no np.tile
        - no np.repeat

    Hint:
        Think about where a size-1 axis must be inserted.
    """
    raise NotImplementedError


# ---------------------------------------------------------------------------
# Task 5 — real broadcasting across several axes
# ---------------------------------------------------------------------------

def calibrate(samples: np.ndarray,
              channel_gain: np.ndarray,
              time_offset: np.ndarray) -> np.ndarray:
    """
    Shapes:

        samples.shape      == (devices, time, channels)
        channel_gain.shape == (channels,)
        time_offset.shape  == (time,)

    Calculate:

        result[d, t, c]
            =
        samples[d, t, c] * channel_gain[c] + time_offset[t]

    Constraints:
        - no Python loops
        - no np.tile
        - no np.repeat

    You will need to make `time_offset` broadcast along
    the device and channel dimensions.
    """
    raise NotImplementedError


# ---------------------------------------------------------------------------
# Task 6 — docs task: reductions, axis and keepdims
# ---------------------------------------------------------------------------

def normalize_rows(a: np.ndarray) -> np.ndarray:
    """
    Normalize every row so that its elements sum to 1.

    Example:
        [[1.0, 1.0],
         [1.0, 3.0]]

    becomes:
        [[0.5,  0.5 ],
         [0.25, 0.75]]

    Constraints:
        - no Python loops
        - do not manually reshape using known row/column counts

    Research if necessary:
        ndarray.sum / np.sum
        axis
        keepdims

    Assume every row has a non-zero sum.
    """
    raise NotImplementedError


# ---------------------------------------------------------------------------
# Task 7 — docs task: Cartesian selection with np.ix_
# ---------------------------------------------------------------------------

def select_rectangle(a: np.ndarray,
                     rows: np.ndarray,
                     columns: np.ndarray) -> np.ndarray:
    """
    Return the Cartesian product of the requested rows and columns.

    Example:

        rows    = [0, 2]
        columns = [1, 3]

    should mean:

        [
            [a[0, 1], a[0, 3]],
            [a[2, 1], a[2, 3]],
        ]

    NOT pairwise indexing.

    Requirement:
        Use np.ix_.

    Research:
        numpy.ix_

    Think about why:

        a[rows, columns]

    means something different.
    """
    raise NotImplementedError


# ===========================================================================
# Tests — do not modify
# ===========================================================================

def test_stepped_view() -> None:
    a = np.arange(36, dtype=np.int32).reshape(6, 6)
    b = stepped_view(a)

    expected = np.array([
        [6, 8, 10],
        [18, 20, 22],
        [30, 32, 34],
    ], dtype=np.int32)

    assert np.array_equal(b, expected)
    assert b.shape == (3, 3)
    assert np.shares_memory(a, b), "Task 1 must return a view."

    old = int(a[1, 0])
    b[0, 0] = 999
    assert a[1, 0] == 999
    a[1, 0] = old


def test_reverse_columns_view() -> None:
    a = np.arange(12, dtype=np.int32).reshape(3, 4)
    b = reverse_columns_view(a)

    expected = np.array([
        [3, 2, 1, 0],
        [7, 6, 5, 4],
        [11, 10, 9, 8],
    ], dtype=np.int32)

    assert np.array_equal(b, expected)
    assert np.shares_memory(a, b), "Task 2 must return a view."
    assert any(stride < 0 for stride in b.strides), (
        "Expected a negative stride."
    )


def test_reordered_rows_copy() -> None:
    a = np.arange(20, dtype=np.int32).reshape(4, 5)
    b = reordered_rows_copy(a)

    expected = np.array([
        [15, 16, 17, 18, 19],
        [0, 1, 2, 3, 4],
        [10, 11, 12, 13, 14],
    ], dtype=np.int32)

    assert np.array_equal(b, expected)
    assert not np.shares_memory(a, b), "Task 3 must return a copy."

    b[0, 0] = 999
    assert a[3, 0] == 15


def test_add_every_pair() -> None:
    x = np.array([1, 2, 3], dtype=np.int64)
    y = np.array([10, 20, 30, 40], dtype=np.int64)

    result = add_every_pair(x, y)

    expected = np.array([
        [11, 21, 31, 41],
        [12, 22, 32, 42],
        [13, 23, 33, 43],
    ])

    assert result.shape == (3, 4)
    assert np.array_equal(result, expected)


def test_calibrate() -> None:
    samples = np.array([
        [
            [1.0, 2.0, 3.0],
            [4.0, 5.0, 6.0],
        ],
        [
            [7.0, 8.0, 9.0],
            [10.0, 11.0, 12.0],
        ],
    ])

    channel_gain = np.array([10.0, 100.0, 1000.0])
    time_offset = np.array([0.5, 5.0])

    result = calibrate(samples, channel_gain, time_offset)

    expected = np.empty_like(samples)
    for d in range(samples.shape[0]):
        for t in range(samples.shape[1]):
            for c in range(samples.shape[2]):
                expected[d, t, c] = (
                    samples[d, t, c] * channel_gain[c]
                    + time_offset[t]
                )

    assert result.shape == samples.shape
    assert np.allclose(result, expected)


def test_normalize_rows() -> None:
    a = np.array([
        [1.0, 1.0, 2.0],
        [1.0, 3.0, 6.0],
        [5.0, 5.0, 10.0],
    ])

    result = normalize_rows(a)

    assert result.shape == a.shape
    assert np.allclose(result.sum(axis=1), np.ones(a.shape[0]))

    expected = np.array([
        [0.25, 0.25, 0.50],
        [0.10, 0.30, 0.60],
        [0.25, 0.25, 0.50],
    ])

    assert np.allclose(result, expected)


def test_select_rectangle() -> None:
    a = np.arange(30).reshape(5, 6)

    rows = np.array([0, 3, 4])
    columns = np.array([1, 2, 5])

    result = select_rectangle(a, rows, columns)

    expected = np.array([
        [1, 2, 5],
        [19, 20, 23],
        [25, 26, 29],
    ])

    assert result.shape == (3, 3)
    assert np.array_equal(result, expected)
    assert not np.shares_memory(a, result)


TESTS = [
    test_stepped_view,
    test_reverse_columns_view,
    test_reordered_rows_copy,
    test_add_every_pair,
    test_calibrate,
    test_normalize_rows,
    test_select_rectangle,
]


def main() -> None:
    failures = 0

    for test in TESTS:
        name = test.__name__.removeprefix("test_")

        try:
            test()
        except Exception:
            failures += 1
            print(f"[FAIL] {name}")
            traceback.print_exc()
            print()
        else:
            print(f"[ OK ] {name}")

    print()
    if failures == 0:
        print("All tasks passed.")
    else:
        print(f"{failures} task(s) failed.")


if __name__ == "__main__":
    main()
