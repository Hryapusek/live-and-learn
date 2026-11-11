# Task 005 — First GStreamer C++ application

A small synchronous example that creates a GStreamer pipeline, records 125 live-paced
synthetic video frames to Matroska/H.264, reads `GstBus` messages, and shuts down on
EOS or error.

## Requirements (Debian/Ubuntu)

```bash
sudo apt install build-essential cmake pkg-config libgstreamer1.0-dev \
    gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly
```

## Build and run

```bash
cmake -S . -B build
cmake --build build
./build/gst_intro
```

The application writes `example.mkv` in the current working directory.
Verify with `gst-discoverer-1.0 example.mkv` if available, or `ffprobe`.

## What to inspect

1. Find the API calls corresponding to *create*, *play*, *listen*, and *clean up*.
2. Notice that `GstBus` messages are not `GstEvent` objects.
3. Find the pipeline `STATE_CHANGED` notifications and the final `EOS` message.
4. Try setting the `filesink` location to a path you cannot write. Confirm that
   the program returns an error and prints the source element.
5. Optional: change `is-live=true` to `is-live=false` and compare execution time.

This deliberately uses `gst_parse_launch()` to keep the first C++ example focused
on the application lifecycle. A later exercise will construct elements explicitly
and introduce asynchronous bus watches in a Qt application.

**Note:** The program has not been compiled or run in the provided generation
environment because the GStreamer development package is not installed there.
