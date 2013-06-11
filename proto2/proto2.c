#include <gst/gst.h>
  
static GstElement *pipeline;

static int v_channel = 0;

static int active_channel = 0;

static gboolean
do_switch (GstElement * pipeline)
{
  int other_channel;
  GstElement *select;
  GstStateChangeReturn ret;
  gchar *name;
  gchar *othername;
  GstPad *pad;
  GstPad *otherPad;
  gint64 v_stoptime;
  gint64 v_starttime;
  gint64 v_runningtime;
  gint64 starttime, stoptime;

  other_channel  = active_channel ? 0 : 1;
  active_channel = active_channel ? 0 : 1;

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "foo");


  /* find the selector */
  select = gst_bin_get_by_name (GST_BIN (pipeline), "selector");

  if (!select) {
     g_print("Input selector not found\n");
  }

  /* get the named pad */
  name = g_strdup_printf ("sink%d", active_channel);
  othername = g_strdup_printf ("sink%d", other_channel);
  pad = gst_element_get_static_pad (select, name);
  otherPad = gst_element_get_static_pad (select, othername);

  if (!pad) {
     g_print("Input selector pad %s not found\n", name);
  }
  if (!otherPad) {
     g_print("Input selector pad %s not found\n", othername);
  }

  /* set the active pad */

  g_signal_emit_by_name (select, "block", &v_stoptime);
  stoptime = v_stoptime;

  g_object_get (G_OBJECT(pad), "running-time", &v_runningtime, NULL);
  starttime = v_runningtime;

  g_signal_emit_by_name (select, "switch", pad, stoptime, starttime);

  g_free (name);

  return TRUE;
}


int main(int argc, char *argv[]) {
  GstElement *source, *source2, *sink, *selector, *aselector, *asink; 
  GstElement *vdec1, *vdec2, *vdepay1, *vdepay2,  *adec1, *adec2, *adepay1, *adepay2;  
  GstElement *vq1, *vq2, *aq1, *aq2, *voq, *aoq;
  GstElement *tol1, *tol2, *tolo, *aresample, *vrate;
  GstElement *vtee, *atee;
  GstElement *aenc, *aconv, *vconv, *venc, *aencq, *vencq, *vmuxq, *amuxq, *mux, *rtmpq;
  GstElement *filesink, *rtmpsink;
  GstBus *bus;
  GstPad *srcpad, *sinkpad, *pad, *pad0, *pad1;
  GstMessage *msg;
  GstStateChangeReturn ret;
  GMainLoop *loop;
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
   
  /* Create the elements */
  source      = gst_element_factory_make ("souphttpsrc",       "source");
  source2     = gst_element_factory_make ("souphttpsrc",       "source2");
  selector    = gst_element_factory_make ("input-selector","selector");
  vdec1       = gst_element_factory_make ("jpegdec",       "vdec1");
  vdec2       = gst_element_factory_make ("jpegdec",       "vdec2");
  vq1         = gst_element_factory_make ("queue",         "vq1");
  vq2         = gst_element_factory_make ("queue",         "vq2");
  voq         = gst_element_factory_make ("queue",         "voq");
  tol1        = gst_element_factory_make ("timeoverlay",   "tol1");
  tol2        = gst_element_factory_make ("timeoverlay",   "tol2");
  tolo        = gst_element_factory_make ("timeoverlay",   "tolo");
  vrate       = gst_element_factory_make ("videorate",     "vrate");
  vtee        = gst_element_factory_make ("tee",           "vtee");
  venc        = gst_element_factory_make ("ffenc_flv",     "venc");
  vconv       = gst_element_factory_make ("ffmpegcolorspace",  "vconv");
  vencq       = gst_element_factory_make ("queue",         "vencq");
  vmuxq       = gst_element_factory_make ("queue",    "vmuxq");
  mux         = gst_element_factory_make ("flvmux",        "mux");
//  filesink    = gst_element_factory_make ("filesink",      "filesink");
  rtmpsink    = gst_element_factory_make ("rtmpsink",      "rtmpsink");
//  sink        = gst_element_factory_make ("autovideosink", "sink");



  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
   
  if (!pipeline || !source || !source2 || !selector || 
      !vdec1 || !vdec2 || 
      !vq1 || !vq2 || !voq || !tol1 || !tol2 || !tolo || !vtee ||
      !vconv || !venc || !vencq || !vmuxq || !mux ||
//      !sink
//      !filesink  
      !rtmpsink 
      ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */

  gst_bin_add (GST_BIN (pipeline), source);
  gst_bin_add (GST_BIN (pipeline), source2);
  gst_bin_add (GST_BIN (pipeline), selector);
  gst_bin_add (GST_BIN (pipeline), vdec1);
  gst_bin_add (GST_BIN (pipeline), vdec2);
  gst_bin_add (GST_BIN (pipeline), vq1);
  gst_bin_add (GST_BIN (pipeline), vq2);
  gst_bin_add (GST_BIN (pipeline), voq);
  gst_bin_add (GST_BIN (pipeline), tol1);
  gst_bin_add (GST_BIN (pipeline), tol2);
  gst_bin_add (GST_BIN (pipeline), tolo);
  gst_bin_add (GST_BIN (pipeline), vtee);
  gst_bin_add (GST_BIN (pipeline), venc);
  gst_bin_add (GST_BIN (pipeline), vconv);
  gst_bin_add (GST_BIN (pipeline), vencq);
  gst_bin_add (GST_BIN (pipeline), vmuxq);
  gst_bin_add (GST_BIN (pipeline), mux);
//  gst_bin_add (GST_BIN (pipeline), filesink);
  gst_bin_add (GST_BIN (pipeline), rtmpsink);
//  gst_bin_add (GST_BIN (pipeline), sink);
  gst_bin_add (GST_BIN (pipeline), vrate);

/*****   VIDEO 1 INPUT SIDE  ***/

  if (gst_element_link_many (source, vq1, vdec1, selector, NULL) != TRUE) {
    g_printerr ("Video input 1  pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   VIDEO 2 INPUT SIDE  ***/

  if (gst_element_link_many (source2, vq2, vdec2, selector, NULL) != TRUE) {
    g_printerr ("Video input 2  pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   VIDEO OUTPUT SIDE ****/

  if (gst_element_link_many (selector, vencq, venc, vmuxq, mux, voq, rtmpsink, NULL) != TRUE) {
    g_printerr ("Video output pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }


  /* Modify the source's properties */

  g_object_set (source, "location", "http://66.184.211.231/mjpg/video.mjpg", NULL);
  g_object_set (source2, "location", "http://webcam1.coloradocollege.edu/mjpg/video.mjpg", NULL);
//  g_object_set (source2, "location", "http://128.153.6.47/mjpg/video.mjpg", NULL);
  g_object_set (source, "do-timestamp", TRUE, NULL);
  g_object_set (source2, "do-timestamp", TRUE, NULL);
  g_object_set (source, "is-live", TRUE, NULL);
  g_object_set (source2, "is-live", TRUE, NULL);

  g_object_set (selector, "sync-streams", TRUE, NULL);

  g_object_set (rtmpsink, "location",
  "rtmp://1.7669465.fme.ustream.tv/ustreamVideo/7669465/dX3r3M2m3mAfLwfA7hCa9YQXc3FntQum flashver=FME/2.5 (compatible; FMSc 1.0)",NULL);
  g_object_set (rtmpsink, "sync", FALSE, NULL);

  g_object_set (tol1, "halign","left", NULL);
  g_object_set (tol1, "valign","top", NULL);
  g_object_set (tol1, "text","Input 1:", NULL);
  g_object_set (tol1, "shaded-background",TRUE, NULL);
  
  g_object_set (tol2, "halign","left", NULL);
  g_object_set (tol2, "valign","top", NULL);
  g_object_set (tol2, "text","Input 2:", NULL);
  g_object_set (tol2, "shaded-background",TRUE, NULL);

  g_object_set (tolo, "halign","right", NULL);
  g_object_set (tolo, "valign","top", NULL);
  g_object_set (tolo, "text","OUTPUT TIME:", NULL);
  g_object_set (tolo, "shaded-background",TRUE, NULL);

  g_object_set (vq1, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq2, "max-size-bytes", 1000000000, NULL);
  g_object_set (vmuxq, "max-size-bytes", 1000000000, NULL);
  g_object_set (vencq, "max-size-bytes", 1000000000, NULL);
  g_object_set (voq,   "max-size-bytes", 1000000000, NULL);

//  g_object_set (filesink,   "location", "stuff.flv", NULL);


  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  
  /* Wait until error or EOS */

  loop=g_main_loop_new (NULL, FALSE);
  g_timeout_add (15000, (GSourceFunc) do_switch, pipeline);
  g_main_loop_run (loop);

  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
  
  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;
    
    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }
  
  
  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
