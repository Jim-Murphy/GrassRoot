#include <gst/gst.h>
  
static GstElement *pipeline;

static gboolean 
do_switch (GstElement * pipeline)
{
  int other_channel;
  GstElement *select;
  GstElement *aselect;
  GstStateChangeReturn ret;
  gchar *name;
  gchar *othername;
  GstPad *pad;
  GstPad *apad;
  GstPad *otherPad;
  GstPad *aotherPad;
  gint64 v_stoptime, a_stoptime;
  gint64 v_starttime, a_starttime;
  gint64 v_runningtime, a_runningtime;
  gint64 starttime, stoptime;

  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "foo");


  /* find the selector */
  select = gst_bin_get_by_name (GST_BIN (pipeline), "selector");
  aselect = gst_bin_get_by_name (GST_BIN (pipeline), "aselector");

  if (!select) {
     g_print("Input selector not found\n");
  }
  if (!aselect) {
     g_print("Audio input selector not found\n");
  }

  /* get the named pad */
//  name = g_strdup_printf ("sink%d", active_channel);
  othername = g_strdup_printf ("sink%d", other_channel);
 
  pad = gst_element_get_static_pad (select, name);
  otherPad = gst_element_get_static_pad (select, othername);
  apad = gst_element_get_static_pad (aselect, name);
  aotherPad = gst_element_get_static_pad (aselect, othername);

  if (!pad) {
     g_print("Input selector pad %s not found\n", name);
  }
  if (!apad) {
     g_print("Audio Input selector pad %s not found\n", name);
  }
  if (!otherPad) {
     g_print("Input selector pad %s not found\n", othername);
  }
  if (!aotherPad) {
     g_print("Input selector pad %s not found\n", othername);
  }

  /* set the active pad */

  g_signal_emit_by_name (select, "block", &v_stoptime);
  g_signal_emit_by_name (aselect, "block", &a_stoptime);

  if (v_stoptime > a_stoptime) {
     stoptime = v_stoptime;
  } else {
     stoptime = a_stoptime;
  }

//  Need to figure this out stil

//  gst_pad_get_property (otherPad, "running-time", &v_runningtime);
//  gst_pad_get_property (aotherPad, (const char *)"running-time", &a_runningtime);
  

//  if (v_runningtime > a_runningtime) {
//     stoptime = a_runningtime;
//  } else {
//     stoptime = v_runningtime;
//  } 

  g_signal_emit_by_name (select, "switch", pad, stoptime, -1);
  g_signal_emit_by_name (aselect, "switch", apad, stoptime,-1);

  g_free (name);

  return TRUE;
}



static void
cb_new_source_pad (GstElement *element,
	    GstPad     *pad,
	    gpointer    data)
{
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

  GstCaps *filtercaps;

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
  vbox         = gst_element_factory_make ("videobox",           "vbox");
  vmix         = gst_element_factory_make ("videomixer",         "vmix");
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
                    vq102, ffcolor1, vq103, vrate101, vcapsfil102, vq104, vq105,
                    vscale102, vcapsfil103, vq106, vrate102, vcapsfil104, vq107,
                    vqout1, vtextover, vqout2, ffcolorout, vqout3, imagesink, NULL);

  if (gst_element_link_many (vsource1, vq101, vscale101, vcapsfil101,
                             vq102, ffcolor1, vq103, vrate101, vcapsfil102, vq104, vq105,
                             vscale102, vcapsfil103, vq106, vrate102, vcapsfil104, vq107,
                             vqout1, vtextover, vqout2, ffcolorout, vqout3, imagesink, NULL) != TRUE )  {
     g_printerr ("Video output pipeD could not be linked.\n");
     gst_object_unref (pipeline);
     return -1;
  }

  /* Set up the various caps filters */

  filtercaps = gst_caps_new_simple ("video/x-raw-yuv",
//			   "format", G_TYPE_STRING, "YUV",
			   "width", G_TYPE_INT, 640,
			   "height", G_TYPE_INT, 360,
                           "framerate", GST_TYPE_FRACTION, 15, 1,
			   NULL);
  g_object_set (G_OBJECT (vcapsfil101), "caps", filtercaps, NULL);
  g_object_set (G_OBJECT (vcapsfil102), "caps", filtercaps, NULL);
  g_object_set (G_OBJECT (vcapsfil103), "caps", filtercaps, NULL);
  gst_caps_unref (filtercaps);

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


//  /* listen for newly created pads */
//  g_signal_connect (source, "pad-added", G_CALLBACK (cb_new_source_pad), NULL);

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
