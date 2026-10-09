"""
NumPy computation checkpoint
============================

Run:
    python numpy_computation_checkpoint.py

Rules
-----
- Fill only functions marked TASK.
- Do not modify tests.
- No Python loops/comprehensions unless explicitly allowed.
- Do not use np.vectorize.
- NumPy docs are allowed and expected.
- Only NumPy; no scipy/pandas/numba.

Mental model to practice:
    shapes -> broadcasting -> elementwise work -> reductions -> output shape
           -> dtype -> ownership/temporaries

Research list
-------------
Read as needed, not all up front:
- NumPy ufuncs
- ufunc out= and where=
- reductions: axis, tuple axes, keepdims
- np.argmax
- np.count_nonzero
- np.add.accumulate
- np.divide(..., out=..., where=...)
- np.isfinite
- np.nanmean / np.nanstd
- np.select
- reduction dtype=
- np.lib.stride_tricks.sliding_window_view

There are 16 required tasks and 2 optional challenges.
"""

from __future__ import annotations

import ast
import inspect
import textwrap
import traceback
from typing import Callable

import numpy as np


# ===========================================================================
# A. UFUNCS / OUTPUT CONTROL
# ===========================================================================

# TASK 1
def affine_transform(x: np.ndarray, scale: float, offset: float) -> np.ndarray:
    """Return x * scale + offset without Python iteration.

    Think: does this expression necessarily become one fused native loop?
    """
    """
        SO: Does it even matter what ndim has x?
    """
    return x * scale + offset


# TASK 2
def affine_into(x: np.ndarray, scale: float, offset: float, out: np.ndarray) -> np.ndarray:
    """Compute x * scale + offset into caller-provided storage.

    Requirements:
    - use ufunc out=
    - avoid a full-size temporary for x * scale
    - return exactly `out`

    Read: np.multiply, np.add, ufunc out=
    """
    np.multiply(x, scale, out=out)
    np.add(out, offset, out=out)
    return out


# TASK 3
def nonnegative_sqrt(x: np.ndarray) -> np.ndarray:
    """sqrt(x) for x>=0, otherwise 0.

    Requirements:
    - floating result
    - use np.sqrt with BOTH out= and where=
    - do not evaluate sqrt on negative entries

    Read carefully: ufunc where=. What happens to positions where where=False
    if you did not initialize/provide output storage?
    """
    # The danger is that numbers that are less than 0 are just ignored completely and that means that output storage might contain garbage depending on how it was initalized
    result = np.zeros(x.size, dtype=x.dtype)
    np.sqrt(x, where=(x>=0), out=result)
    return result


# ===========================================================================
# B. REDUCTIONS / AXES
# ===========================================================================

# TASK 4
def normalize_rows_l1(a: np.ndarray) -> np.ndarray:
    """Normalize each row so every row sums to 1.

    Assume row sums are non-zero.
    Requirement: reduction + keepdims=True.
    """
    return a / a.sum(axis=1, keepdims=True)


# TASK 5
def center_last_axis(a: np.ndarray) -> np.ndarray:
    """Subtract the mean of the LAST axis from every element.

    Must work for any ndim >= 1.
    Do not hard-code dimensions.
    Read/use: axis=-1, keepdims=True.
    """
    return a - a.mean(axis=-1, keepdims=True)


# TASK 6
def mean_channel_profile(samples: np.ndarray) -> np.ndarray:
    """samples.shape == (devices, time, channels).

    Return one mean value per channel by averaging over devices AND time.
    Output shape: (channels,)

    Read: tuple passed to axis=.
    """
    return samples.mean(axis=(0, 1))


# TASK 7
def strongest_channel(spectra: np.ndarray) -> np.ndarray:
    """spectra.shape == (measurements, channels).

    Return the INDEX of the strongest channel for each measurement.
    Output shape: (measurements,)

    Read: np.argmax(axis=...).
    """
    return np.argmax(spectra, axis=1)


# TASK 8
def cumulative_energy(samples: np.ndarray) -> np.ndarray:
    """Cumulative sum along the LAST axis.

    Requirement: use np.add.accumulate, NOT np.cumsum.
    Read: ufunc.accumulate.
    """
    # print(np.add.reduce(samples, axis=-1, keepdims=True))
    # I thought at first that you want me just to sum all the numbers in last axes
    # When i saw np.add.accumulate i confused it with the std accumulate and thought its just a sum
    # Now i see what this actually is
    # Im gonna read about np.cumsum now just to know what it is
    # Well i read about it, it seems like its the same thing
    # Nope, not exactly. Well i mean it is. You just wanted me to deeplearn the ufuncs with the accumulate because its not only about add, its also about other ufuncs that have accumulate. I understand
    # I havent encountered a real example of accumulate usage, but i guess it might be a part of some another algorithm
    return np.add.accumulate(samples, axis=-1) # same as np.cumsum


# ===========================================================================
# C. MASKS / CONDITIONAL COMPUTATION
# ===========================================================================

# TASK 9
def select_range(a: np.ndarray, low: float, high: float) -> np.ndarray:
    """Return values satisfying low <= value < high using a boolean mask.

    Also know whether the returned result is a view or copy.
    """
    # That was really tough to be honest
    mask = np.full(a.shape, True)
    np.logical_and(mask, a >= low, out=mask)
    np.logical_and(mask, a < high, out=mask)
    return a[mask]


# TASK 10
def sanitize_readings(a: np.ndarray, low: float, high: float) -> np.ndarray:
    """Return floating output where invalid readings become NaN.

    Valid means:
        finite AND low <= value <= high

    Requirement: use np.where.
    Read: np.isfinite, np.where.
    """
    result = np.array(a.shape, dtype=np.float64)
    mask = np.isfinite(a)
    np.logical_and(mask, a >= low, out=mask)
    np.logical_and(mask, a < high, out=mask)
    return np.where(mask, a, np.nan)


# TASK 11
def valid_count_per_device(samples: np.ndarray, low: float, high: float) -> np.ndarray:
    """samples.shape == (devices, time, channels).

    Count valid scalar readings per device.
    Valid: finite AND low <= x <= high.
    Output shape: (devices,)

    Requirement: use np.count_nonzero and reduce time+channel axes together.
    """
    raise NotImplementedError


# TASK 12
def safe_divide(numerator: np.ndarray, denominator: np.ndarray) -> np.ndarray:
    """Elementwise numerator / denominator, but 0 where denominator == 0.

    Broadcasting must work.
    Requirements: np.divide with BOTH out= and where=.

    Think: why can
        np.where(denominator != 0, numerator / denominator, 0)
    still perform problematic division before selection?
    """
    raise NotImplementedError


# ===========================================================================
# D. COMPOSITION
# ===========================================================================

# TASK 13
def weighted_channel_mean(spectra: np.ndarray, weights: np.ndarray) -> np.ndarray:
    """spectra.shape == (measurements, channels)
       weights.shape == (channels,)

    Return one weighted mean per measurement:
        sum(spectrum * weights) / sum(weights)

    No tile/repeat, no loops.
    """
    raise NotImplementedError


# TASK 14
def standardize_channels(samples: np.ndarray) -> np.ndarray:
    """samples.shape == (devices, time, channels).

    For each channel independently:
    - mean over devices AND time
    - std over devices AND time
    - result = (samples - mean) / std

    Assume non-zero std for every channel.
    Requirement: tuple axes + keepdims=True.
    """
    raise NotImplementedError


# TASK 15
def stable_softmax(logits: np.ndarray) -> np.ndarray:
    """logits.shape == (batch, classes).

    Compute row-wise softmax, but make it numerically stable.

    Read:
    - "numerically stable softmax"
    - np.exp
    - row max reduction + keepdims

    Must work around values near 1000 without overflow.

    Explain to yourself: why does subtracting the row maximum not change
    the softmax probabilities?
    """
    raise NotImplementedError


# TASK 16
def pairwise_distances(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """a.shape == (N, dimensions)
       b.shape == (M, dimensions)

    Return Euclidean distance for every pair: shape (N, M).

    No loops/scipy.
    Use broadcasting + reduction.

    Hint:
        a[:, None, :]
        b[None, :, :]

    Determine the difference shape and which axis represents coordinates.
    """
    raise NotImplementedError


# ===========================================================================
# OPTIONAL RESEARCH CHALLENGES
# ===========================================================================

# OPTIONAL 1
def optional_piecewise_calibration(x: np.ndarray) -> np.ndarray:
    """Piecewise function:

        x < 0       -> -1
        0 <= x <100 -> x * 0.1
        x >= 100    -> 10 + sqrt(x - 100)

    Requirement: use np.select.
    Read: np.select.
    Compare mentally with nested np.where calls.
    """
    raise NotImplementedError


# OPTIONAL 2
def optional_moving_average(x: np.ndarray, window: int) -> np.ndarray:
    """1D moving average using overlapping windows.

    Example:
        [1,2,3,4,5], window=3 -> [2,3,4]

    Requirement:
    - use np.lib.stride_tricks.sliding_window_view
    - no loops

    Read carefully about sliding_window_view.
    Think about:
    - output/window shape
    - strides
    - whether windows physically copy data
    - how overlapping logical windows can share storage
    """
    raise NotImplementedError


# ===========================================================================
# TEST HELPERS — DO NOT MODIFY
# ===========================================================================

def assert_no_python_iteration(fn: Callable) -> None:
    source = textwrap.dedent(inspect.getsource(fn))
    tree = ast.parse(source)
    banned = (
        ast.For, ast.AsyncFor, ast.While,
        ast.ListComp, ast.SetComp, ast.DictComp, ast.GeneratorExp,
    )
    assert not any(isinstance(node, banned) for node in ast.walk(tree)), (
        f"{fn.__name__}: Python iteration/comprehension detected"
    )


def assert_uses(fn: Callable, *tokens: str) -> None:
    source = inspect.getsource(fn)
    missing = [token for token in tokens if token not in source]
    assert not missing, f"{fn.__name__}: expected use of {missing}"


# ===========================================================================
# TESTS — DO NOT MODIFY
# ===========================================================================

def test_affine_transform():
    assert_no_python_iteration(affine_transform)
    x = np.array([[1., 2.], [3., 4.]])
    assert np.allclose(affine_transform(x, 2.5, -1), [[1.5, 4], [6.5, 9]])


def test_affine_into():
    assert_no_python_iteration(affine_into)
    assert_uses(affine_into, "out=")
    x = np.arange(6, dtype=float).reshape(2, 3)
    out = np.empty_like(x)
    result = affine_into(x, 3, 2, out)
    assert result is out
    assert np.allclose(out, x * 3 + 2)


def test_nonnegative_sqrt():
    assert_no_python_iteration(nonnegative_sqrt)
    assert_uses(nonnegative_sqrt, "out=", "where=")
    x = np.array([-9., -1., 0., 1., 4., 9.])
    result = nonnegative_sqrt(x)
    assert np.issubdtype(result.dtype, np.floating)
    assert np.allclose(result, [0, 0, 0, 1, 2, 3])


def test_normalize_rows_l1():
    assert_no_python_iteration(normalize_rows_l1)
    assert_uses(normalize_rows_l1, "keepdims=True")
    a = np.array([[1., 1., 2.], [2., 3., 5.]])
    result = normalize_rows_l1(a)
    assert result.shape == a.shape
    assert np.allclose(result.sum(axis=1), 1)


def test_center_last_axis():
    assert_no_python_iteration(center_last_axis)
    a = np.arange(24, dtype=float).reshape(2, 3, 4)
    result = center_last_axis(a)
    assert result.shape == a.shape
    assert np.allclose(result.mean(axis=-1), 0)


def test_mean_channel_profile():
    assert_no_python_iteration(mean_channel_profile)
    a = np.arange(24, dtype=float).reshape(2, 3, 4)
    result = mean_channel_profile(a)
    assert result.shape == (4,)
    assert np.allclose(result, a.mean(axis=(0, 1)))


def test_strongest_channel():
    assert_no_python_iteration(strongest_channel)
    spectra = np.array([[1,5,2,3], [9,1,2,3], [1,2,3,10]])
    assert np.array_equal(strongest_channel(spectra), [1,0,3])


def test_cumulative_energy():
    assert_no_python_iteration(cumulative_energy)
    assert_uses(cumulative_energy, "np.add.accumulate")
    a = np.array([[1,2,3], [10,20,30]])
    assert np.array_equal(cumulative_energy(a), [[1,3,6], [10,30,60]])


def test_select_range():
    assert_no_python_iteration(select_range)
    a = np.array([-5,0,3,8,10,20])
    result = select_range(a, 3, 10)
    assert np.array_equal(result, [3,8])
    assert not np.shares_memory(a, result)


def test_sanitize_readings():
    assert_no_python_iteration(sanitize_readings)
    assert_uses(sanitize_readings, "np.where")
    a = np.array([-100., -5., 10., 200., np.nan, np.inf])
    result = sanitize_readings(a, -10, 100)
    expected = np.array([np.nan, -5., 10., np.nan, np.nan, np.nan])
    assert np.allclose(result, expected, equal_nan=True)


def test_valid_count_per_device():
    assert_no_python_iteration(valid_count_per_device)
    assert_uses(valid_count_per_device, "np.count_nonzero")
    samples = np.array([
        [[1.,2.,np.nan], [3.,1000.,4.]],
        [[5.,6.,7.], [np.inf,-1000.,8.]],
    ])
    assert np.array_equal(valid_count_per_device(samples, 0, 10), [4,4])


def test_safe_divide():
    assert_no_python_iteration(safe_divide)
    assert_uses(safe_divide, "np.divide", "out=", "where=")
    numerator = np.array([[10.,20.,30.], [40.,50.,60.]])
    denominator = np.array([2.,0.,10.])
    with np.errstate(divide="raise", invalid="raise"):
        result = safe_divide(numerator, denominator)
    assert np.allclose(result, [[5.,0.,3.], [20.,0.,6.]])


def test_weighted_channel_mean():
    assert_no_python_iteration(weighted_channel_mean)
    spectra = np.array([[10.,20.,30.], [1.,2.,3.]])
    weights = np.array([1.,2.,1.])
    result = weighted_channel_mean(spectra, weights)
    assert result.shape == (2,)
    assert np.allclose(result, [20.,2.])


def test_standardize_channels():
    assert_no_python_iteration(standardize_channels)
    rng = np.random.default_rng(42)
    samples = rng.normal(
        loc=np.array([10.,100.,-50.]),
        scale=np.array([2.,5.,10.]),
        size=(4,20,3),
    )
    result = standardize_channels(samples)
    assert result.shape == samples.shape
    assert np.allclose(result.mean(axis=(0,1)), 0, atol=1e-12)
    assert np.allclose(result.std(axis=(0,1)), 1, atol=1e-12)


def test_stable_softmax():
    assert_no_python_iteration(stable_softmax)
    logits = np.array([[1000.,1001.,1002.], [-1000.,-1000.,-999.]])
    with np.errstate(over="raise", invalid="raise"):
        result = stable_softmax(logits)
    assert result.shape == logits.shape
    assert np.allclose(result.sum(axis=1), 1)
    assert np.all(np.isfinite(result))
    assert np.allclose(result, stable_softmax(logits + 12345.0))


def test_pairwise_distances():
    assert_no_python_iteration(pairwise_distances)
    a = np.array([[0.,0.], [3.,4.]])
    b = np.array([[0.,0.], [0.,4.], [3.,0.]])
    result = pairwise_distances(a, b)
    assert result.shape == (2,3)
    assert np.allclose(result, [[0.,4.,3.], [5.,3.,4.]])


def test_optional_piecewise_calibration():
    assert_no_python_iteration(optional_piecewise_calibration)
    assert_uses(optional_piecewise_calibration, "np.select")
    x = np.array([-4.,0.,50.,99.,100.,109.,200.])
    expected = np.array([-1.,0.,5.,9.9,10.,13.,20.])
    assert np.allclose(optional_piecewise_calibration(x), expected)


def test_optional_moving_average():
    assert_no_python_iteration(optional_moving_average)
    assert_uses(optional_moving_average, "sliding_window_view")
    x = np.array([1.,2.,3.,4.,5.])
    assert np.allclose(optional_moving_average(x, 3), [2.,3.,4.])


REQUIRED_TESTS = [
    test_affine_transform,
    test_affine_into,
    test_nonnegative_sqrt,
    test_normalize_rows_l1,
    test_center_last_axis,
    test_mean_channel_profile,
    test_strongest_channel,
    test_cumulative_energy,
    test_select_range,
    test_sanitize_readings,
    test_valid_count_per_device,
    test_safe_divide,
    test_weighted_channel_mean,
    test_standardize_channels,
    test_stable_softmax,
    test_pairwise_distances,
]

OPTIONAL_TESTS = [
    test_optional_piecewise_calibration,
    test_optional_moving_average,
]


def run_tests(tests: list[Callable], title: str) -> tuple[int, int]:
    print(f"\n=== {title} ===")
    passed = 0
    for test in tests:
        name = test.__name__.removeprefix("test_")
        try:
            test()
        except NotImplementedError:
            print(f"[TODO] {name}")
        except Exception:
            print(f"[FAIL] {name}")
            traceback.print_exc()
            print()
        else:
            passed += 1
            print(f"[ OK ] {name}")
    return passed, len(tests)


def main() -> None:
    rp, rt = run_tests(REQUIRED_TESTS, "Required tasks")
    op, ot = run_tests(OPTIONAL_TESTS, "Optional research challenges")
    print(f"\nRequired: {rp}/{rt}")
    print(f"Optional: {op}/{ot}")
    print("\nRequired checkpoint passed." if rp == rt else "\nKeep going.")


if __name__ == "__main__":
    main()
