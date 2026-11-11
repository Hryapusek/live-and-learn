// Live and Learn — GStreamer Task 007: Pad probes
//
// Goal:
//   A. Observe the first five buffers on a pad.
//   B. Temporarily BLOCK that pad, then unblock it from the main thread.
//
// This is an exercise, not a finished solution. Search for "TODO".
// The starter compiles and runs even before you finish the TODOs.
//
// Build (Debian, GStreamer development package installed):
//   g++ -std=c++17 -Wall -Wextra -O0 -g main.cpp -o pad_probes \
//       $(pkg-config --cflags --libs gstreamer-1.0)
// Run:
//   ./pad_probes

#include <gst/gst.h>
#include <gst/gstpad.h>

#include <chrono>
#include <iostream>
#include <atomic>

namespace {

struct ProbeState {
    std::atomic<unsigned> observed_buffers = 0;
};

constexpr auto max_buffers = 5;

// A DATA probe. Called in a GStreamer streaming thread.
GstPadProbeReturn on_buffer(GstPad* /*pad*/,
                            GstPadProbeInfo* info,
                            gpointer user_data)
{
    std::cout << "------ on_buffer -------" << std::endl;

    auto& state = *static_cast<ProbeState*>(user_data);

    auto buffer = GST_PAD_PROBE_INFO_BUFFER(info);

    // TODO 1: Get the buffer from 'info' using GST_PAD_PROBE_INFO_BUFFER(info).
    //         A probe gives you a borrowed pointer: do NOT unref the buffer.

    state.observed_buffers.fetch_add(1, std::memory_order_release);

    // TODO 2: For each buffer, increment state.observed_buffers and print:
    //           - gst_buffer_get_size(buffer) (bytes)
    //           - GST_BUFFER_PTS(buffer) (convert to ms if valid)
    //         Hint: GST_CLOCK_TIME_IS_VALID(pts), GST_TIME_AS_MSECONDS(pts).

    std::cout << "buffer size: " << gst_buffer_get_size(buffer) << std::endl;

    auto pts = GST_BUFFER_PTS(buffer);
    if (GST_CLOCK_TIME_IS_VALID(pts)) {
        std::cout << GST_TIME_AS_MSECONDS(pts) << std::endl;;
    }

    std::cout << "------ on_buffer end -------" << std::endl;

    // TODO 3: After observing five buffers, REMOVE this probe.
    //         Hint: return GST_PAD_PROBE_REMOVE.
    //         Otherwise return GST_PAD_PROBE_OK.

    // if (state.observed_buffers > max_buffers) {
    //     return GST_PAD_PROBE_REMOVE;
    // }
    return GST_PAD_PROBE_OK;
}

// A BLOCKING probe. Also runs in a streaming thread, on the next matching buffer.
GstPadProbeReturn on_blocked(GstPad* /*pad*/,
                             GstPadProbeInfo* info,
                             gpointer /*user_data*/)
{
    std::cout << "------ on_blocked -------" << std::endl;
    // TODO 4: Print a message that the next buffer has reached the blocked pad.
    //         Bonus: print its PTS. Don't sleep or wait here!
    auto buffer = GST_PAD_PROBE_INFO_BUFFER(info);

    auto pts = GST_BUFFER_PTS(buffer);
    if (GST_CLOCK_TIME_IS_VALID(pts)) {
        std::cout << GST_TIME_AS_MSECONDS(pts) << std::endl;;
    }

    std::cout << "------ on_blocked end -------" << std::endl;

    // IMPORTANT: For a blocking probe, OK keeps the probe installed and
    // leaves dataflow BLOCKED until another thread removes the probe.
    // PASS would let this item through, then block again on the next item.
    return GST_PAD_PROBE_OK;
}

void print_error(GstMessage* message)
{
    GError* error = nullptr;
    gchar* debug = nullptr;
    gst_message_parse_error(message, &error, &debug);

    std::cerr << "GStreamer error: "
              << (error ? error->message : "unknown error") << '\n';
    if (debug)
        std::cerr << "Details: " << debug << '\n';

    if (error)
        g_error_free(error);
    g_free(debug);
}

} // namespace

int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    // 'tap' is an identity element, purely to give us a named pad to probe.
    // This live source normally produces one buffer every 40 ms (25 FPS).
    constexpr const char* description =
        "videotestsrc pattern=ball is-live=true num-buffers=100 "
        "! video/x-raw,format=I420,width=320,height=240,framerate=25/1 "
        "! identity name=tap silent=true "
        "! fakesink sync=false";

    GError* parse_error = nullptr;
    GstElement* pipeline = gst_parse_launch(description, &parse_error);
    if (!pipeline || parse_error) {
        std::cerr << "Cannot construct pipeline: "
                  << (parse_error ? parse_error->message : "unknown error") << '\n';
        if (parse_error)
            g_error_free(parse_error);
        if (pipeline)
            gst_object_unref(pipeline);
        return 1;
    }

    GstElement* tap = gst_bin_get_by_name(GST_BIN(pipeline), "tap");
    GstPad* pad = tap ? gst_element_get_static_pad(tap, "src") : nullptr;
    if (tap)
        gst_object_unref(tap);

    if (!pad) {
        std::cerr << "Cannot find tap:src pad\n";
        gst_object_unref(pipeline);
        return 1;
    }

    ProbeState probe_state;
    gst_pad_add_probe(pad,
                      GST_PAD_PROBE_TYPE_BUFFER,
                      on_buffer,
                      &probe_state,
                      nullptr);

    GstBus* bus = gst_element_get_bus(pipeline);
    if (gst_element_set_state(pipeline, GST_STATE_PLAYING)
        == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Cannot start pipeline\n";
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(bus);
        gst_object_unref(pad);
        gst_object_unref(pipeline);
        return 1;
    }

    using Clock = std::chrono::steady_clock;
    const auto started = Clock::now();
    bool tried_to_block = false;
    bool tried_to_unblock = false;
    gulong block_probe_id = 0;
    bool finished = false;
    bool success = false;

    // The main thread remains responsive because we poll the bus with a
    // short timeout. The data/blocking callbacks run in a streaming thread.
    while (!finished) {
        const auto elapsed = Clock::now() - started;

        if (!tried_to_block && elapsed >= std::chrono::seconds(1)) {
            tried_to_block = true;
            std::cout << "[main] 1 second elapsed: time to block the pad\n";

            block_probe_id = gst_pad_add_probe(
                pad,
                static_cast<GstPadProbeType>(GST_PAD_PROBE_TYPE_BLOCK | GST_PAD_PROBE_TYPE_BUFFER),
                on_blocked,
                nullptr,
                nullptr
            );

            // TODO 5: Install a BLOCK | BUFFER probe on 'pad'.
            //         Use on_blocked as the callback and store the returned
            //         probe ID in block_probe_id.
            //         Hint: gst_pad_add_probe(...).
            //         Cast the bitwise combination to GstPadProbeType.
        }

        if (tried_to_block && !tried_to_unblock
            && elapsed >= std::chrono::seconds(2)) {
            tried_to_unblock = true;
            std::cout << "[main] 2 seconds elapsed: time to unblock the pad\n";

            if (block_probe_id != 0) {
                gst_pad_remove_probe(pad, block_probe_id);
                block_probe_id = 0;
            }
            // TODO 6: If block_probe_id != 0, remove the probe from 'pad'
            //         using gst_pad_remove_probe(pad, block_probe_id).
            //         Then set block_probe_id = 0.
        }

        GstMessage* message = gst_bus_timed_pop_filtered(
            bus,
            50 * GST_MSECOND,
            static_cast<GstMessageType>(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));

        if (!message)
            continue;

        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
            print_error(message);
            finished = true;
        } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
            std::cout << "[main] EOS — source finished\n";
            success = true;
            finished = true;
        }

        gst_message_unref(message);
    }

    // Safety cleanup: don't leave a blocked streaming thread during shutdown.
    if (block_probe_id != 0)
        gst_pad_remove_probe(pad, block_probe_id);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pad);
    gst_object_unref(pipeline);

    return success ? 0 : 1;
}
