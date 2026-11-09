# Task 002 — Encoding H.264 into a Matroska file

## Goal

Build a GStreamer pipeline that generates raw video, encodes it as H.264, and writes it to a Matroska (`.mkv`) file. Inspect how the negotiated caps change along the pipeline, then determine whether `h264parse` is required in this particular case.

**Test configuration**

| Setting | Value |
| --- | --- |
| Source | `videotestsrc`, `pattern=ball` |
| Frames | `125` (5 seconds of media at 25 FPS) |
| Resolution | `640 × 360` |
| Framerate | `25/1` |
| Raw pixel format | `I420` |
| Encoder | `x264enc`, `bitrate=2048` (kbit/s) |
| Maximum keyframe interval | `25` frames |
| Container | Matroska |
| Output file | `example.mkv` |

## Experiment A — Build the encoding pipeline

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=125 \
    ! capsfilter caps="video/x-raw,format=I420,framerate=25/1,width=640,height=360" \
    ! videoconvert \
    ! x264enc bitrate=2048 key-int-max=25 \
    ! h264parse \
    ! matroskamux \
    ! filesink location="example.mkv"
```

**Result:** The pipeline ran and reached playback; the task of producing the file was completed. The element name is `h264parse` (not `h246parse` as initially typed in the notes).

### Data flow

```text
videotestsrc   → video/x-raw (I420)
videoconvert   → video/x-raw (I420)
x264enc        → video/x-h264 (AVC, AU-aligned)
h264parse      → video/x-h264 (parsed, AVC, AU-aligned)
matroskamux    → video/x-matroska
filesink       → example.mkv
```

`videoconvert` remained at **I420 → I420** in this run, so no pixel-format change was needed. The important representation change happens at `x264enc`: **raw frames → encoded H.264**. `matroskamux` then packages the compressed video into a Matroska container; it does not encode the frames again.

## Experiment B — Inspect negotiated caps

The output from `gst-launch-1.0 -v` was long, so only the fields relevant to this exercise are recorded below.

| Pad | Negotiated media and noteworthy fields |
| --- | --- |
| `videotestsrc:src` | `video/x-raw`, `I420`, `640×360`, `25/1` |
| `videoconvert:sink` | `video/x-raw`, `I420`, `640×360`, `25/1` |
| `videoconvert:src` | `video/x-raw`, `I420`, `640×360`, `25/1` |
| `x264enc:sink` | `video/x-raw`, `I420`, `640×360`, `25/1` |
| `x264enc:src` | `video/x-h264`, `stream-format=avc`, `alignment=au`, `profile=high`, `level=3` |
| `h264parse:src` | `video/x-h264`, `stream-format=avc`, `alignment=au`, `parsed=true` |
| `matroskamux:video_0` | `video/x-h264`, `stream-format=avc`, `alignment=au` |
| `matroskamux:src` | `video/x-matroska` |
| `filesink:sink` | `video/x-matroska` |

### Additional metadata exposed by the parser

Compared with the encoder's output caps, `h264parse` reported:

```text
parsed=true
chroma-format=4:2:0
bit-depth-luma=8
bit-depth-chroma=8
```

This shows that the parser can inspect and describe the encoded bitstream, even when its input and output are both `video/x-h264`.

### Notes for later study

These fields appeared during inspection, but I haven't fully studied them yet:

| Field | Brief reminder / question to revisit |
| --- | --- |
| `codec_data` | Binary H.264 initialization information (including SPS/PPS); why does the decoder need it? |
| `stream-format=avc` | How does AVC representation differ from `avc3` and `byte-stream`? |
| `alignment=au` | Buffers are aligned to access units; how is this different from NAL alignment? |
| `profile=high` | Identifies supported H.264 coding features, **not** a pixel/color format. |
| `level=3` | Identifies decoder capability limits; which limits matter? |
| `pixel-aspect-ratio=1/1` | Square pixels; distinct from the image's width-to-height ratio. |
| `interlace-mode=progressive` | Progressive frames versus interlaced video. |
| `colorimetry`, `chroma-site` | Color interpretation and chroma sample positioning; revisit with color formats. |
| `streamheader` | Container header information exposed by `matroskamux`. |

There is no need to memorize these fields to complete this exercise.

### Other observed messages

```text
Redistribute latency...
Pipeline is PREROLLED ...
Setting pipeline to PLAYING ...
New clock: GstSystemClock
```

These introduce pipeline timing, preroll, and clocks. They are topics for a later exercise.

## Experiment C — Remove `h264parse`

Repeat the pipeline without the parser:

```bash
gst-launch-1.0 -v \
    videotestsrc pattern=ball num-buffers=125 \
    ! capsfilter caps="video/x-raw,format=I420,framerate=25/1,width=640,height=360" \
    ! videoconvert \
    ! x264enc bitrate=2048 key-int-max=25 \
    ! matroskamux \
    ! filesink location="example-no-parser.mkv"
```

**Observation:** The pipeline still negotiated successfully, reached `PREROLLED` / `PLAYING`, and delivered `video/x-matroska` to `filesink`.

The key caps remained compatible:

```text
x264enc:src        video/x-h264, stream-format=avc, alignment=au
matroskamux:video_0 video/x-h264, stream-format=avc, alignment=au
matroskamux:src     video/x-matroska
filesink:sink       video/x-matroska
```

**Conclusion:** `h264parse` is **not required for caps compatibility in this specific pipeline**: `x264enc` already provides the AVC representation and AU alignment accepted by `matroskamux`. The parser can still supply metadata and perform useful normalization in other pipelines.

The similar caps do **not** establish that the two output files are byte-for-byte identical, only that both pipelines negotiated compatible media formats. Full container validation or a file comparison was not part of these notes.

## Takeaways

1. An element may be present without changing the media format: here, `videoconvert` negotiated I420 on both sides.
2. `x264enc` crosses the **raw → compressed** boundary (`video/x-raw` → `video/x-h264`).
3. `h264parse` understands the structure of encoded H.264 and can add metadata; it is not always mandatory.
4. `matroskamux` crosses the **encoded stream → container** boundary (`video/x-h264` → `video/x-matroska`).
5. The verbose caps output is useful evidence, but retaining only the key negotiated values makes the experiment easier to revisit.

## Next topic

`GstBuffer`, PTS/DTS, preroll, clocks, and why five seconds of **media time** need not equal five seconds of **wall-clock time**.
