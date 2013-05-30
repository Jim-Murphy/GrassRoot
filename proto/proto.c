#include <gst/gst.h>
  
static int active_channel = 0;

static gboolean 
do_switch (GstElement * pipeline)
{
  int old_channel;
  int new_channel;
  GstElement *vselect;
  GstElement *aselect;
  GstStateChangeReturn ret;
  gchar *new_name;
  gchar *old_name;
  GstPad *vnew_pad;
  GstPad *anew_pad;
  GstPad *vold_pad;
  GstPad *aold_pad;
  gint64 v_stoptime, a_stoptime;
  gint64 v_starttime, a_starttime;
  gint64 v_runningtime, a_runningtime;
  gint64 starttime, stoptime;

// For now, simply assume it's 0 or 1, and other channel is 0 when active is 1, and vice versa

// GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "before");

  old_channel    = active_channel;     // Channel we're about to leave
  active_channel = active_channel?0:1; // Swap the channel indicator
  new_channel    = active_channel;     // Channel we're switching to

  /* get the named pad */
  new_name = g_strdup_printf ("sink%d", new_channel);
  old_name = g_strdup_printf ("sink%d", old_channel);
 
  g_print("Switching from pad %s to pad %s\n", old_name, new_name);

  /* find the selector */
  vselect = gst_bin_get_by_name (GST_BIN (pipeline), "vselector");
  if (!vselect) {
     g_print("Input selector not found\n");
  }
  /* get the video pads */
  vnew_pad = gst_element_get_static_pad (vselect, new_name);
  if (!vnew_pad) {
     g_print("Input selector pad %s not found\n", new_name);
  }
  vold_pad = gst_element_get_static_pad (vselect, old_name);
  if (!vold_pad) {
     g_print("Input selector pad %s not found\n", old_name);
  }

//AUDIO  aselect = gst_bin_get_by_name (GST_BIN (pipeline), "aselector");
//AUDIO  if (!aselect) {
//AUDIO     g_print("Audio input selector not found\n");
//AUDIO  }

//AUDIO  /* get the audio pads */
//AUDIO  anew_pad = gst_element_get_static_pad (aselect, new_name);
//AUDIO  if (!anew_pad) {
//AUDIO     g_print("Input selector pad %s not found\n", new_name);
//AUDIO  }
//AUDIO  aold_pad = gst_element_get_static_pad (aselect, old_name);
//AUDIO  if (!aold_pad) {
//AUDIO     g_print("Input selector pad %s not found\n", old_name);
//AUDIO  }

  /* First, send the BLOCK signal to each selector */

  g_signal_emit_by_name (vselect, "block", &v_stoptime);
//AUDIO  g_signal_emit_by_name (aselect, "block", &a_stoptime);

//AUDIO  if (v_stoptime > a_stoptime) {
            stoptime = v_stoptime;
//AUDIO  } else {
//AUDIO     stoptime = a_stoptime;
//AUDIO  }

  g_object_get (G_OBJECT (vnew_pad), "running-time", &v_runningtime, NULL);

//STOPTIME  gst_pad_get_property (anew_pad, (const char *)"running-time", &a_runningtime);
  
//STOPTIME  if (v_runningtime < a_runningtime) {
               starttime = v_runningtime;
//STOPTIME  } else {
//STOPTIME     starttime = a_runningtime;
//STOPTIME  } 

  g_print("Stopped at %"G_GINT64_FORMAT"\n", stoptime);
  g_print("Starting at %"G_GINT64_FORMAT"\n", starttime);

  g_signal_emit_by_name (vselect, "switch", vnew_pad, stoptime, (gint64) -1 );
//AUDIO  g_signal_emit_by_name (aselect, "switch", anew_pad, stoptime, starttime);

//  gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

//  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "after");

  g_free (new_name);
  g_free (old_name);

  return TRUE;
}



static void
cb_new_source_pad (GstElement *element,
	    GstPad     *pad,
	    gpointer    data)
{
  g_print("Input selector pad added\n");
  GstCaps *padCaps;
  const GstStructure *capStr;
 
  /* What kind of pad? */
  padCaps = gst_pad_get_caps(pad);
  if ( g_strrstr(gst_caps_to_string(padCaps),"video") ) {
//     link_new_pad (element, pad, "vq1", "vdepay1", "vdec1", "tol1", "selector", v_channel++, 'v');
  } else {
//     link_new_pad (element, pad, "aq1", "adepay1", "adec1", "", "aselector", a_channel++, 'a');
  }


}


int main(int argc, char *argv[]) {

  GstElement *pipeline;

  GstElement *vsource1, *vsource2; 
  GstElement *vq101, *vq102, *vq103, *vq104, *vq105, *vq106, *vq107;
  GstElement *vq201, *vq202, *vq203, *vq204, *vq205, *vq206, *vq207;
  GstElement *vqout1, *vqout2, *vqout3;
  GstElement *vscale101, *vscale102;
  GstElement *vscale201, *vscale202;
  GstElement *vrate101, *vrate102, *vrate201;
  GstElement *vcapsfil101, *vcapsfil102, *vcapsfil103, *vcapsfil104;
  GstElement *vcapsfil201, *vcapsfil202, *vcapsfil203;
  GstElement *ffcolor1, *ffcolor2, *ffcolorout;
  GstElement *vselector;
  GstElement *vtee2;
  GstElement *vbox;
  GstElement *vmix;
  GstElement *vtextover;
  GstElement *imagesink;

  GstPad  *teeSrc1;
  GstPad  *selSink1;
  GstPad  *genSink;
  GstPad  *genSrc;

  GstCaps *fullscreencaps;
  GstCaps *pipcaps;

  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  GMainLoop *loop;
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
   
  /* Create the elements */

  vsource1     = gst_element_factory_make ("videotestsrc",       "vsource1");
  vq101        = gst_element_factory_make ("queue",              "vq101");
  vq102        = gst_element_factory_make ("queue",              "vq102");
  vq103        = gst_element_factory_make ("queue",              "vq103");
  vq104        = gst_element_factory_make ("queue",              "vq104");
  vq105        = gst_element_factory_make ("queue",              "vq105");
  vq106        = gst_element_factory_make ("queue",              "vq106");
  vq107        = gst_element_factory_make ("queue",              "vq107");
  vscale101    = gst_element_factory_make ("videoscale",         "vscale101");
  vscale102    = gst_element_factory_make ("videoscale",         "vscale102");
  vrate101     = gst_element_factory_make ("videorate",          "vrate101");
  vrate102     = gst_element_factory_make ("videorate",          "vrate102");
  vcapsfil101  = gst_element_factory_make ("capsfilter",         "vcapsfil101");
  vcapsfil102  = gst_element_factory_make ("capsfilter",         "vcapsfil102");
  vcapsfil103  = gst_element_factory_make ("capsfilter",         "vcapsfil103");
  vcapsfil104  = gst_element_factory_make ("capsfilter",         "vcapsfil104");
  ffcolor1     = gst_element_factory_make ("ffmpegcolorspace",   "ffcolor1");

  vsource2      = gst_element_factory_make ("videotestsrc",       "vsource2");
  vq201        = gst_element_factory_make ("queue",              "vq201");
  vq202        = gst_element_factory_make ("queue",              "vq202");
  vq203        = gst_element_factory_make ("queue",              "vq203");
  vq204        = gst_element_factory_make ("queue",              "vq204");
  vq205        = gst_element_factory_make ("queue",              "vq205");
  vq206        = gst_element_factory_make ("queue",              "vq206");
  vq207        = gst_element_factory_make ("queue",              "vq207");
  vscale201    = gst_element_factory_make ("videoscale",         "vscale201");
  vscale202    = gst_element_factory_make ("videoscale",         "vscale202");
  vrate201     = gst_element_factory_make ("videorate",          "vrate201");
  vcapsfil201  = gst_element_factory_make ("capsfilter",         "vcapsfil201");
  vcapsfil202  = gst_element_factory_make ("capsfilter",         "vcapsfil202");
  vcapsfil203  = gst_element_factory_make ("capsfilter",         "vcapsfil203");
  ffcolor2     = gst_element_factory_make ("ffmpegcolorspace",   "ffcolor2");
  vtee2        = gst_element_factory_make ("tee",                "vtee2");

  vselector    = gst_element_factory_make ("input-selector",     "vselector");
  vqout1       = gst_element_factory_make ("queue",              "vqout1");
  vqout2       = gst_element_factory_make ("queue",              "vqout2");
  vqout3       = gst_element_factory_make ("queue",              "vqout3");
  ffcolorout   = gst_element_factory_make ("ffmpegcolorspace",   "ffcolorout");
  vmix         = gst_element_factory_make ("videomixer",         "vmix");
  vbox         = gst_element_factory_make ("videobox",           "vbox");
  vtextover    = gst_element_factory_make ("textoverlay",        "vtextover");
  imagesink    = gst_element_factory_make ("ximagesink",         "imagesink");

  /* Create the empty pipeline */

  pipeline = gst_pipeline_new ("demo-pipeline");
   
  if (  
     !vsource1 || !vsource2 || 
     !vq101 || !vq102 || !vq103 || !vq104 || !vq105 || !vq106 || !vq107 ||
     !vq201 || !vq202 || !vq203 || !vq204 || !vq205 || !vq206 || !vq207 ||
     !vqout1 || !vqout2 || !vqout3 ||
     !vscale101 || !vscale102 ||
     !vscale201 || !vscale202 ||
     !vrate101 || !vrate102 || !vrate201 ||
     !ffcolor1 || !ffcolor2 || !ffcolorout ||
     !vselector || !vtee2 || !vbox || !vmix || !vtextover ||
     !vcapsfil101 || !vcapsfil102 || !vcapsfil103 || !vcapsfil104 ||
     !vcapsfil201 || !vcapsfil202 || !vcapsfil203 ||
     !imagesink ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */

  gst_bin_add_many (GST_BIN (pipeline), vsource1, vq101, vscale101, vcapsfil101,
                    vq102, ffcolor1, vq103, vrate101, vcapsfil102, vq104, vselector, vq105,
                    vscale102, vcapsfil103, vq106, vrate102, vcapsfil104, vq107,
                    vqout1, vtextover, vqout2, vmix, ffcolorout, vqout3, imagesink, NULL);

  gst_bin_add_many (GST_BIN (pipeline), vsource2, vq201, vscale201, vcapsfil201, 
                    vq202, ffcolor2, vq203, vrate201, vcapsfil202, vtee2, vq204,
                    vq205, vscale202, vcapsfil203, vq206, vbox, vq207, NULL);


  if (gst_element_link_many (vsource1, vq101, vscale101, vcapsfil101,
                             vq102, ffcolor1, vq103, vrate101, vcapsfil102, vq104, vselector, vq105,
                             vscale102, vcapsfil103, vq106, vrate102, vcapsfil104, vq107,
                             vqout1, vtextover, vqout2, vmix, ffcolorout, vqout3, imagesink, NULL) != TRUE )  {
     g_printerr ("Video Channel 1  could not be linked.\n");
     gst_object_unref (pipeline);
     return -1;
  }

  if (gst_element_link_many (vsource2, vq201, vscale201, vcapsfil201, 
                         vq202, ffcolor2, vq203, vrate201, vcapsfil202, vtee2, vq204,
                         vscale202, vcapsfil203, vq206, vbox, vq207, vmix, NULL) != TRUE ) {
     g_printerr ("Video Channel 2  could not be linked.\n");
     gst_object_unref (pipeline);
     return -1;
  }

  teeSrc1 = gst_element_get_request_pad (vtee2, "src1");
  if (!teeSrc1) {
     g_printerr ("Unable to get Src1 on Vtee\n");
  }

  genSink = gst_element_get_static_pad (vq205, "sink");
  if (!genSink) {
     g_printerr ("Unable to get Sink on Q205\n");
  }

  if (gst_pad_link (teeSrc1, genSink))  {
     g_printerr ("Video Channel 2 could not link Tee to Q205.\n");
     gst_object_unref (pipeline);
     return -1;
  }

  genSrc = gst_element_get_static_pad (vq205, "src");
  if (!genSrc) {
     g_printerr ("Unable to get src on Q205\n");
  }
  selSink1 = gst_element_get_request_pad (vselector, "sink1");
  if (!selSink1) {
     g_printerr ("Unable to get sink1 on Selctor\n");
  }

  if (gst_pad_link (genSrc,selSink1))  {
     g_printerr ("Video Channel 2 could not link Q205 to Selector.\n");
     gst_object_unref (pipeline);
     return -1;
  }
  

  /* Set up the various caps filters */

  fullscreencaps = gst_caps_new_simple ("video/x-raw-yuv",
			   "width", G_TYPE_INT, 640,
			   "height", G_TYPE_INT, 360,
                           "framerate", GST_TYPE_FRACTION, 15, 1,
			   NULL);
  g_object_set (G_OBJECT (vcapsfil101), "caps", fullscreencaps, NULL);
  g_object_set (G_OBJECT (vcapsfil102), "caps", fullscreencaps, NULL);
  g_object_set (G_OBJECT (vcapsfil103), "caps", fullscreencaps, NULL);
  g_object_set (G_OBJECT (vcapsfil201), "caps", fullscreencaps, NULL);
  g_object_set (G_OBJECT (vcapsfil202), "caps", fullscreencaps, NULL);

  pipcaps = gst_caps_new_simple ("video/x-raw-yuv",
			   "width", G_TYPE_INT, 320,
			   "height", G_TYPE_INT, 180,
                           "framerate", GST_TYPE_FRACTION, 15, 1,
			   NULL);
  g_object_set (G_OBJECT (vcapsfil203), "caps", pipcaps, NULL);

  g_object_set (vsource1, "pattern", 0, NULL);
  g_object_set (vsource1, "is-live", TRUE, NULL);
  g_object_set (vsource2, "pattern", 18, NULL);
  g_object_set (vsource2, "is-live", TRUE, NULL);

  g_object_set (vq101, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq102, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq103, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq104, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq105, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq106, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq107, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq201, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq202, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq203, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq204, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq205, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq206, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq207, "max-size-bytes", 1000000000, NULL);
  g_object_set (vqout1, "max-size-bytes", 1000000000, NULL);
  g_object_set (vqout2, "max-size-bytes", 1000000000, NULL);
  g_object_set (vqout3, "max-size-bytes", 1000000000, NULL);

  g_object_set (vtextover, "text", "GrassRoot Media", NULL);
  g_object_set (vtextover, "valign", "bottom", NULL);
  g_object_set (vtextover, "shaded-background", FALSE, NULL);
  g_object_set (vtextover, "font-desc", "Sans Bold 20", NULL);

  g_object_set (vbox, "left", -320, NULL);
  g_object_set (vbox, "top", 0, NULL);
  g_object_set (vbox, "border-alpha", 0, NULL);


//  /* listen for newly created pads */
//  g_signal_connect (vsource1, "pad-added", G_CALLBACK (cb_new_source_pad), NULL);

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  
  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "start");

  /* Wait until error or EOS */

  loop=g_main_loop_new (NULL, FALSE);
  g_timeout_add (10000, (GSourceFunc) do_switch, pipeline);
  g_main_loop_run (loop);

  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS );
  
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
  gst_caps_unref (fullscreencaps);
  gst_caps_unref (pipcaps);
  return 0;
}
