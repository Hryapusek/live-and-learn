
Setting pipeline to PAUSED ...
Pipeline is PREROLLING ...

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = event   ******* (identity0:sink) E (type: stream-start (10254), GstEventStreamStart, stream-id=(string)399fca9d3e72f7be018bd5f5b95a861c, flags=(GstStreamFlags)GST_STREAM_FLAG_NONE, group-id=(uint)1;) 0x5cb7e1e65500

/GstPipeline:pipeline0/GstVideoTestSrc:videotestsrc0.GstPad:src: caps = video/x-raw, format=(string)I420, width=(int)320, height=(int)240, framerate=(fraction)25/1, multiview-mode=(string)mono, pixel-aspect-ratio=(fraction)1/1, interlace-mode=(string)progressive

/GstPipeline:pipeline0/GstCapsFilter:capsfilter0.GstPad:src: caps = video/x-raw, format=(string)I420, width=(int)320, height=(int)240, framerate=(fraction)25/1, multiview-mode=(string)mono, pixel-aspect-ratio=(fraction)1/1, interlace-mode=(string)progressive

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = event   ******* (identity0:sink) E (type: caps (12814), GstEventCaps, caps=(GstCaps)"video/x-raw\,\ format\=\(string\)I420\,\ width\=\(int\)320\,\ height\=\(int\)240\,\ framerate\=\(fraction\)25/1\,\ multiview-mode\=\(string\)mono\,\ pixel-aspect-ratio\=\(fraction\)1/1\,\ interlace-mode\=\(string\)progressive";) 0x5cb7e1e655e0

/GstPipeline:pipeline0/GstIdentity:identity0.GstPad:src: caps = video/x-raw, format=(string)I420, width=(int)320, height=(int)240, framerate=(fraction)25/1, multiview-mode=(string)mono, pixel-aspect-ratio=(fraction)1/1, interlace-mode=(string)progressive

/GstPipeline:pipeline0/GstFakeSink:fakesink0.GstPad:sink: caps = video/x-raw, format=(string)I420, width=(int)320, height=(int)240, framerate=(fraction)25/1, multiview-mode=(string)mono, pixel-aspect-ratio=(fraction)1/1, interlace-mode=(string)progressive

/GstPipeline:pipeline0/GstIdentity:identity0.GstPad:sink: caps = video/x-raw, format=(string)I420, width=(int)320, height=(int)240, framerate=(fraction)25/1, multiview-mode=(string)mono, pixel-aspect-ratio=(fraction)1/1, interlace-mode=(string)progressive

/GstPipeline:pipeline0/GstCapsFilter:capsfilter0.GstPad:sink: caps = video/x-raw, format=(string)I420, width=(int)320, height=(int)240, framerate=(fraction)25/1, multiview-mode=(string)mono, pixel-aspect-ratio=(fraction)1/1, interlace-mode=(string)progressive

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = event   ******* (identity0:sink) E (type: segment (17934), GstEventSegment, segment=(GstSegment)"segment, flags=(GstSegmentFlags)GST_SEGMENT_FLAG_NONE, rate=(double)1, applied-rate=(double)1, format=(GstFormat)time, base=(guint64)0, offset=(guint64)0, start=(guint64)0, stop=(guint64)18446744073709551615, time=(guint64)0, position=(guint64)0, duration=(guint64)18446744073709551615;";) 0x5cb7e1e656c0

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = chain   ******* (identity0:sink) (115200 bytes, dts: none, pts: 0:00:00.000000000, duration: 0:00:00.040000000, offset: 0, offset_end:  1, flags: 00000040 discont , meta: none) 0x5cb7e1e6c5a0

Pipeline is PREROLLED ...
Setting pipeline to PLAYING ...
Redistribute latency...
New clock: GstSystemClock

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = chain   ******* (identity0:sink) (115200 bytes, dts: none, pts: 0:00:00.040000000, duration: 0:00:00.040000000, offset: 1, offset_end:  2, flags: 00000000 , meta: none) 0x5cb7e1e6c6c0

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = chain   ******* (identity0:sink) (115200 bytes, dts: none, pts: 0:00:00.080000000, duration: 0:00:00.040000000, offset: 2, offset_end:  3, flags: 00000000 , meta: none) 0x5cb7e1e6c5a0

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = chain   ******* (identity0:sink) (115200 bytes, dts: none, pts: 0:00:00.120000000, duration: 0:00:00.040000000, offset: 3, offset_end:  4, flags: 00000000 , meta: none) 0x5cb7e1e6c6c0

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = chain   ******* (identity0:sink) (115200 bytes, dts: none, pts: 0:00:00.160000000, duration: 0:00:00.040000000, offset: 4, offset_end:  5, flags: 00000000 , meta: none) 0x5cb7e1e6c5a0

/GstPipeline:pipeline0/GstIdentity:identity0: last-message = event   ******* (identity0:sink) E (type: eos (28174), ) 0x5cb7e1e65730
Got EOS from element "pipeline0".
Execution ended after 0:00:00.000652916
Setting pipeline to NULL ...
Freeing pipeline ...

I see the five buffers here. The duration of each is 0.04. First pts is zero. Last is 0.16. Which corresponds to the expected delta.

Experiment B

Here i see something interesting. I see that the sync does not allow frames to be processed before the presentation time. But who is stopping them? Fakesink? And what sync is even used for? Who wants to display the video immediately without knowing the time it should be played? I mean for ordinary data processing maybe. But with videosink sync=False is just weird you know.

Experiment C
Live means we can not produce frames faster than we see them alive(fps).
