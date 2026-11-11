#include <gst/gst.h>
#include <gst/gstmessage.h>

#include <iostream>

int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    constexpr const char* description = R"(
        videotestsrc pattern=ball is-live=true num-buffers=125
        ! video/x-raw,format=I420,width=640,height=360,framerate=25/1
        ! x264enc bitrate=2048 key-int-max=25
        ! h264parse
        ! matroskamux
        ! filesink location=./somedir/woggilie
    )";

    GError* parse_error = nullptr;
    GstElement* pipeline = gst_parse_launch(description, &parse_error);

    if (parse_error != nullptr || pipeline == nullptr) {
        std::cerr << "Could not build pipeline: "
                  << (parse_error ? parse_error->message : "unknown error") << '\n';
        if (parse_error) g_error_free(parse_error);
        if (pipeline) gst_object_unref(pipeline);
        return 1;
    }

    GstBus* bus = gst_element_get_bus(pipeline);
    const GstStateChangeReturn result = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    bool success = false;

    if (result != GST_STATE_CHANGE_FAILURE) {
        bool finished = false;
        while (!finished) {
            GstMessage* message = gst_bus_timed_pop_filtered(
                bus,
                GST_CLOCK_TIME_NONE,
                static_cast<GstMessageType>(
                    GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_STATE_CHANGED));

            if (!message) continue;

            switch (GST_MESSAGE_TYPE(message)) {
            case GST_MESSAGE_STATE_CHANGED:
                if (GST_MESSAGE_SRC(message) == GST_OBJECT(pipeline)) { // as we said - the bus accepts messages from everyone, including its inner elements you feel me, thats why we should explicitely check that its not some video source state but the whole pipeline state
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed(
                        message, &old_state, &new_state, &pending_state);
                    std::cout << "Pipeline: " << gst_element_state_get_name(old_state)
                              << " -> " << gst_element_state_get_name(new_state) << '\n';
                }
                break;

            case GST_MESSAGE_EOS:
                std::cout << "EOS: recording finalized\n";
                success = true;
                finished = true;
                break;

            case GST_MESSAGE_ERROR: {
                GError* error = nullptr;
                gchar* debug = nullptr;
                gst_message_parse_error(message, &error, &debug);
                std::cerr << "Error from " << GST_OBJECT_NAME(GST_MESSAGE_SRC(message))
                          << ": " << (error ? error->message : "unknown error") << '\n';
                if (debug) std::cerr << "Details: " << debug << '\n';
                if (error) g_error_free(error);
                g_free(debug);
                finished = true;
                break;
            }

            default:
                break;
            }

            gst_message_unref(message);
        }
    } else {
        std::cerr << "Failed to enter PLAYING\n";
        std::cerr << "Gonna discover the messages\n";
        GstMessage* message = gst_bus_timed_pop_filtered(
                bus,
                GST_CLOCK_TIME_NONE,
                static_cast<GstMessageType>(
                    GST_MESSAGE_ERROR));
        if (message) {
            switch (GST_MESSAGE_TYPE(message)) {
                case GST_MESSAGE_ERROR: { // if parent directory to the file does not exist - the error will arise
                    GError* error = nullptr;
                    gchar* debug = nullptr;
                    gst_message_parse_error(message, &error, &debug);
                    std::cerr << "Error from " << GST_OBJECT_NAME(GST_MESSAGE_SRC(message))
                            << ": " << (error ? error->message : "unknown error") << '\n';
                    if (debug) std::cerr << "Details: " << debug << '\n';
                    g_error_free(error);
                    g_free(debug);
                    break;
                }
            }
            gst_message_unref(message);
        }
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline); // Because there is a hierarchy. Pipeline is ... while the message itself is not that deep in hierarchy(just guessing)
    return success ? 0 : 1;
}
