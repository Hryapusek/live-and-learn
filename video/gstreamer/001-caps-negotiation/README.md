# 001 — GStreamer: Caps Negotiation

**Goal:** Learn how raw-video formats are negotiated, how `capsfilter` constrains them, and when `videoconvert` is required.

**Tools:** `gst-launch-1.0`, `gst-inspect-1.0`  
**Status:** Four experiments completed

## Concepts

| Term | Meaning |
| --- | --- |
| Element | A component that produces, consumes, or processes media. |
| Source (`src`) pad | An element's output connection. |
| Sink (`sink`) pad | An element's input connection. |
| Pad template | Declares which pads and formats an element may support. |
| Caps (`GstCaps`) | Describe a media type and properties such as pixel format, dimensions, and framerate. |
| Caps negotiation | The process of choosing an actual, mutually supported format. |
| `capsfilter` | Restricts the allowed caps; it does not convert data. |
| `videoconvert` | Converts between raw-video pixel formats when needed. |

`capsfilter caps="video/x-raw,format=RGB"` and `video/x-raw,format=RGB` are equivalent pipeline descriptions.

## 0. Baseline: no explicit caps

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=100 \
    ! videoconvert \
    ! autovideosink
```

**Observed negotiated caps:**

| Pad | Format | Resolution | Framerate |
| --- | --- | --- | --- |
| `videotestsrc:src` | `YUY2` | 320×240 | 30/1 |
| `videoconvert:sink` | `YUY2` | 320×240 | 30/1 |
| `videoconvert:src` | `YUY2` | 320×240 | 30/1 |
| Selected `xvimagesink:sink` | `YUY2` | 320×240 | 30/1 |

### Question: where did these values come from?

**Answer:** They are the *result of negotiation*, not arbitrary defaults of `videoconvert`.

- `videotestsrc` can generate many raw formats. Its fixation logic prefers **320×240 at 30 FPS** when nothing else constrains those fields.
- `autovideosink` selected **`xvimagesink`** on this machine. Downstream sink capabilities/preferences influence which pixel format is chosen; here that was `YUY2`.
- `videoconvert` did **not** have to change the pixel format: its input and output caps both ended up `YUY2`.

Do **not** assume GStreamer always takes “the first format supported by both.” Elements can advertise ordered preferences, query downstream, fixate fields, and negotiate dynamically. Different sinks or systems can give different results.

## 1. Control the source

**Task:** Produce `I420`, 640×480, 25 FPS using an explicit capsfilter.

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=100 \
    ! capsfilter caps="video/x-raw,format=I420,framerate=25/1,width=640,height=480" \
    ! videoconvert \
    ! autovideosink
```

**Observed:**

```text
videotestsrc:src    → I420, 640×480, 25/1
capsfilter0:src     → I420, 640×480, 25/1
videoconvert:sink  → I420, 640×480, 25/1
videoconvert:src   → I420, 640×480, 25/1
xvimagesink:sink   → I420, 640×480, 25/1
```

**Conclusion:** The source successfully negotiated the requested caps. Because the selected video sink accepted `I420`, no pixel-format conversion was needed in this run.

## 2. Force I420 → RGB conversion

**Task:** Constrain the input to `I420`, then require `RGB` after `videoconvert`. Use `fakesink` to avoid display compatibility influencing the experiment.

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=100 \
    ! capsfilter caps="video/x-raw,format=I420,framerate=25/1,width=640,height=480" \
    ! videoconvert \
    ! capsfilter caps="video/x-raw,format=RGB" \
    ! fakesink
```

**Observed:**

```text
videoconvert:sink → I420, 640×480, 25/1
videoconvert:src  → RGB,  640×480, 25/1
fakesink:sink     → RGB,  640×480, 25/1
```

**Conclusion:** The input and output pads have different pixel formats. `videoconvert` performs the conversion; `capsfilter` only restricts what is allowed.

### Unexpected failure with `autovideosink`

When `fakesink` was not substituted, the pipeline failed during preroll:

```text
ERROR: ... GstVideoTestSrc:videotestsrc0: Internal data stream error.
streaming stopped, reason not-negotiated (-4)
ERROR: pipeline doesn't want to preroll.
```

**Interpretation:** The display-dependent pipeline failed to negotiate a usable format in that configuration. Since the original run selected `xvimagesink`, the display sink and its capabilities are relevant suspects, but this log alone does not prove the exact underlying constraint. `fakesink` removes the display-dependent restriction. The error is reported at the source because dataflow failed; it does not necessarily mean `videotestsrc` itself is defective.

## 3. Remove the converter: deliberate link failure

**Task:** Require incompatible formats with no element capable of converting between them.

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=100 \
    ! capsfilter caps="video/x-raw,format=I420,framerate=25/1,width=640,height=480" \
    ! capsfilter caps="video/x-raw,format=RGB" \
    ! fakesink
```

**Observed:**

```text
WARNING: erroneous pipeline: could not link capsfilter0 to capsfilter1
```

**Conclusion:** The two adjacent filters require different pixel formats, and neither transforms the data. Their allowed caps do not intersect, so GStreamer cannot link them.

Even though the CLI prefixes this with `WARNING`, **pipeline construction failed**. This is distinct from the `not-negotiated (-4)` error in experiment 2, which appeared during streaming/preroll after the pipeline had been created.

## 4. Reverse conversion: RGB → I420

**Task:** Reverse the conversion, while retaining the same dimensions and framerate.

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=100 \
    ! capsfilter caps="video/x-raw,format=RGB" \
    ! videoconvert \
    ! capsfilter caps="video/x-raw,format=I420,framerate=25/1,width=640,height=480" \
    ! fakesink
```

**Observed:**

```text
videotestsrc:src   → RGB,  640×480, 25/1
videoconvert:sink  → RGB,  640×480, 25/1
videoconvert:src   → I420, 640×480, 25/1
fakesink:sink      → I420, 640×480, 25/1
```

**Interesting observation:** The `25/1` framerate and `640×480` dimensions specified *after* `videoconvert` affected the upstream `videotestsrc` caps.

**Why:** Negotiation takes the constraints of connected elements into account, not just the element immediately following a filter. `videoconvert` changes pixel representation, but does not itself change frame rate or resolution here; `videotestsrc` can generate the required dimensions and framerate directly.

## Main takeaways

1. Pad templates show **possible** caps; `gst-launch -v` shows **negotiated** caps.
2. A capsfilter **constrains** media format; it doesn't convert it.
3. A converter only needs to change the format when the input/output requirements differ.
4. Negotiation can propagate requirements **upstream**, even though buffers ordinarily flow **downstream**.
5. A pipeline can fail during **linking** or later during **runtime negotiation**; distinguish these while debugging.
6. Format selection without explicit filters depends on available elements, their preferences, and their negotiation behavior.

## Follow-up experiments

- Remove `videoconvert` from the *baseline* pipeline and check whether `videotestsrc` can negotiate directly with `autovideosink`.
- Keep `RGB` between `videoconvert` and another `videoconvert` before `autovideosink`. Does adding a second converter resolve the display-dependent failure?
- Inspect `gst-inspect-1.0 xvimagesink` and compare supported formats with `RGB`.
- For deeper debugging, run with `GST_DEBUG=GST_CAPS:5` and examine the caps queries and decisions.

## References

- [GStreamer: Caps negotiation](https://gstreamer.freedesktop.org/documentation/plugin-development/advanced/negotiation.html)
- [GStreamer: capsfilter](https://gstreamer.freedesktop.org/documentation/coreelements/capsfilter.html)
- [GStreamer: videoconvert](https://gstreamer.freedesktop.org/documentation/videoconvertscale/videoconvert.html)
- [GStreamer: autovideosink](https://gstreamer.freedesktop.org/documentation/autodetect/autovideosink.html)
- [GStreamer: videotestsrc](https://gstreamer.freedesktop.org/documentation/videotestsrc/)

---

*These are notes from actual local experiments. Negotiated formats and display-sink behavior may vary across machines and GStreamer versions.*
