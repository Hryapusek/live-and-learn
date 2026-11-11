#include <glib-object.h>
#include <gst/gst.h>
#include <gst/gstbus.h>
#include <gst/gstelement.h>
#include <gst/gstmessage.h>
#include <gst/gstobject.h>
#include <gst/gstpipeline.h>

#include <iostream>
#include <array>
#include <algorithm>
#include <functional>

class finally
{
 public:
  finally(std::function<void()> callable) { m_callable = std::move(callable); }

  ~finally() { m_callable(); }

 private:
  std::function<void()> m_callable;
};

template <class Callable>
auto make_finally(Callable&& callable)
{
  return finally(std::move(callable));
}

// auto make_unref_finally ...


auto poll_errors(GstBus* bus)
{
  std::cout << "Polling the errors";

  GstMessage* message = gst_bus_timed_pop_filtered(bus,
                                                   1000,
                                                   static_cast<GstMessageType>(GST_MESSAGE_ERROR));
//   auto message_finally = make_finally([message]() { gst_message_unref(message); });
  if(message) {
    switch(GST_MESSAGE_TYPE(message)) {
      case GST_MESSAGE_ERROR: {  // if parent directory to the file does not exist - the error will
                                 // arise
        GError* error = nullptr;
        gchar* debug = nullptr;
        gst_message_parse_error(message, &error, &debug);
        std::cerr << "Error from " << GST_OBJECT_NAME(GST_MESSAGE_SRC(message)) << ": "
                  << (error ? error->message : "unknown error") << '\n';
        if(debug)
          std::cerr << "Details: " << debug << '\n';
        g_error_free(error);
        g_free(debug);
        break;
      }
    }
  }
}

auto build_pipeline() -> GstElement*
{
  // 1. Create the container.
  GstElement* pipeline = gst_pipeline_new("recording");

  auto bus = gst_element_get_bus(pipeline);
  auto bus_finally = make_finally([bus]() { gst_object_unref(bus); });

  // 2. Create individual elements.
  GstElement* source = gst_element_factory_make("videotestsrc", "source");
  GstElement* filter = gst_element_factory_make("capsfilter", "filter");
  GstElement* converter = gst_element_factory_make("videoconvert", "converter");
  GstElement* filterto = gst_element_factory_make("capsfilter", "filterto");
  GstElement* encoder = gst_element_factory_make("x264enc", "encoder");
  GstElement* parser = gst_element_factory_make("h264parse", "parser");
  GstElement* muxer = gst_element_factory_make("matroskamux", "muxer");
  GstElement* sink = gst_element_factory_make("filesink", "sink");

  std::array<GstElement*, 8> elements {source, filter, converter, filterto, encoder, parser, muxer, sink};

  // Check before using any element.
  if(! pipeline
     || std::any_of(elements.begin(), elements.end(), [](GstElement* e) { return e == nullptr; })) {
    std::cerr << "Could not create all GStreamer elements\n";

    for(auto* element : elements)
      if(element)
        gst_object_unref(element);

    if(pipeline)
      gst_object_unref(pipeline);
    return nullptr;
  }

  // 3. Configure properties.
  g_object_set(G_OBJECT(source), "num-buffers", 125, "is-live", TRUE, nullptr);

  gst_util_set_object_arg(G_OBJECT(source), "pattern", "ball");

  g_object_set(G_OBJECT(encoder), "bitrate", 2'048, "key-int-max", 25, nullptr);

  g_object_set(G_OBJECT(sink), "location", "example.mkv", nullptr);

  // 4. Construct our caps.
  GstCaps* caps = gst_caps_new_simple("video/x-raw",
                                      "format",
                                      G_TYPE_STRING,
                                      "RGB",
                                      "width",
                                      G_TYPE_INT,
                                      640,
                                      "height",
                                      G_TYPE_INT,
                                      360,
                                      "framerate",
                                      GST_TYPE_FRACTION,
                                      25,
                                      1,
                                      nullptr);

  g_object_set(G_OBJECT(filter), "caps", caps, nullptr);
  gst_caps_unref(caps);

    GstCaps* converttocaps = gst_caps_new_simple("video/x-raw",
                                        "format",
                                        G_TYPE_STRING,
                                        "I420", nullptr);
    
  gst_util_set_object_arg(G_OBJECT(filterto), "caps", "video/x-raw, format=I420");

  // 5. Add elements to the pipeline.
  gst_bin_add_many(GST_BIN(pipeline), source, filter, converter filterto, encoder, parser, muxer, sink, nullptr);

  // 6. Link them in dataflow order.
  if(! gst_element_link_many(source, filter, converter, filterto, encoder, parser, muxer, sink, nullptr)) {
    std::cerr << "Could not link elements\n";
    poll_errors(bus);
    gst_object_unref(pipeline);
    return nullptr;
  }

  return pipeline;
}

int main(int argc, char* argv[])
{
  gst_init(&argc, &argv);

  constexpr char const* description = R"(
        videotestsrc pattern=ball is-live=true num-buffers=125
        ! video/x-raw,format=I420,width=640,height=360,framerate=25/1
        ! x264enc bitrate=2048 key-int-max=25
        ! h264parse
        ! matroskamux
        ! filesink location=./somedir/woggilie
    )";

  GstElement* pipeline = build_pipeline();

  if (!pipeline) {
    return 1;
  }

  GstBus* bus = gst_element_get_bus(pipeline);
  GstStateChangeReturn const result = gst_element_set_state(pipeline, GST_STATE_PLAYING);
  bool success = false;

  if(result != GST_STATE_CHANGE_FAILURE) {
    bool finished = false;
    while(! finished) {
      GstMessage* message = gst_bus_timed_pop_filtered(bus,
                                                       GST_CLOCK_TIME_NONE,
                                                       static_cast<GstMessageType>(
                                                         GST_MESSAGE_ERROR | GST_MESSAGE_EOS
                                                         | GST_MESSAGE_STATE_CHANGED));

      if(! message)
        continue;

      switch(GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_STATE_CHANGED:
          if(GST_MESSAGE_SRC(message)
             == GST_OBJECT(
               pipeline)) {  // as we said - the bus accepts messages from everyone, including its
                             // inner elements you feel me, thats why we should explicitely check
                             // that its not some video source state but the whole pipeline state
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
            std::cout << "Pipeline: " << gst_element_state_get_name(old_state) << " -> "
                      << gst_element_state_get_name(new_state) << '\n';
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
          std::cerr << "Error from " << GST_OBJECT_NAME(GST_MESSAGE_SRC(message)) << ": "
                    << (error ? error->message : "unknown error") << '\n';
          if(debug)
            std::cerr << "Details: " << debug << '\n';
          if(error)
            g_error_free(error);
          g_free(debug);
          finished = true;
          break;
        }

        default: break;
      }

      gst_message_unref(message);
    }
  }
  else {
    std::cerr << "Failed to enter PLAYING\n";
    std::cerr << "Gonna discover the messages\n";
    GstMessage* message = gst_bus_timed_pop_filtered(bus,
                                                     GST_CLOCK_TIME_NONE,
                                                     static_cast<GstMessageType>(
                                                       GST_MESSAGE_ERROR));
    if(message) {
      switch(GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {  // if parent directory to the file does not exist - the error
                                   // will arise
          GError* error = nullptr;
          gchar* debug = nullptr;
          gst_message_parse_error(message, &error, &debug);
          std::cerr << "Error from " << GST_OBJECT_NAME(GST_MESSAGE_SRC(message)) << ": "
                    << (error ? error->message : "unknown error") << '\n';
          if(debug)
            std::cerr << "Details: " << debug << '\n';
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
  gst_object_unref(pipeline);  // Because there is a hierarchy. Pipeline is ... while the message
                               // itself is not that deep in hierarchy(just guessing)
  return success ? 0 : 1;
}
