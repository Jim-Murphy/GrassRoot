#include <gst/gst.h>
  
static GstElement *pipeline;

static int v_channel = 0;
static int a_channel = 0;

static active_channel = 0;

static gboolean
do_switch (GstElement * pipeline)
{
  GstElement *select;
  GstElement *aselect;
  GstStateChangeReturn ret;
  gchar *name;
  GstPad *pad;
  GstPad *apad;
  gint64 v_stoptime, a_stoptime;
  gint64 starttime, stoptime;

  active_channel = active_channel ? 0 : 1;

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
  name = g_strdup_printf ("sink%d", active_channel);
//  g_print ("switching to pad %s\n", name);
 
  pad = gst_element_get_static_pad (select, name);
  apad = gst_element_get_static_pad (aselect, name);

  if (!pad) {
     g_print("Input selector pad %s not found\n", name);
  }
  if (!apad) {
     g_print("Audio Input selector pad %s not found\n", name);
  }

  /* set the active pad */

  g_signal_emit_by_name (select, "block", &v_stoptime);
  g_signal_emit_by_name (aselect, "block", &a_stoptime);

  if (v_stoptime > a_stoptime) {
     stoptime = a_stoptime;
     starttime = v_stoptime;
  } else {
     stoptime = v_stoptime;
     starttime = a_stoptime;
  }
  
  g_signal_emit_by_name (select, "switch", pad, stoptime, starttime);
  g_signal_emit_by_name (aselect, "switch", apad, stoptime, starttime);

  g_free (name);



  return TRUE;
}


static void link_new_pad ( GstElement *source, GstPad *pad, char *queue_name, 
                           char *depayloader_name, char *decoder_name,
                           char *overlay_name,
                           char *selector_name, int channel, char media_type)
{
  GstElement *selector, *queue, *depayloader, *decoder, *overlay;
  gchar *name;
  gchar *media;
  GstPad *sinkpad;
  GstPad *srcpad, *tosrc;
  GstPad *decsrc, *qsink;
  int returnCode;


  if (media_type == 'v') {
     media = g_strdup_printf ("video");
  } else {
     media = g_strdup_printf ("audio");
  }

  selector = gst_bin_get_by_name (GST_BIN (pipeline), selector_name);
  if (!selector) {
     g_print("Unable to get selector element %s.\n", selector_name);
  }

  queue = gst_bin_get_by_name (GST_BIN (pipeline), queue_name);
  if (!queue) {
     g_print("Unable to get selector queue %s.\n", queue_name);
  }

  depayloader = gst_bin_get_by_name (GST_BIN (pipeline), depayloader_name);
  if (!depayloader) {
     g_print("Unable to get depayloader%s.\n", depayloader_name);
  }

  decoder = gst_bin_get_by_name (GST_BIN (pipeline), decoder_name);
  if (!decoder) {
     g_print("Unable to get decoder%s.\n", decoder_name);
  }
  
  overlay = gst_bin_get_by_name (GST_BIN (pipeline), overlay_name);
  // Don't check for error, overlay not present on Audio channels, so this can fail

  /* get all the pads */
  name = g_strdup_printf ("sink%d", channel);
  sinkpad = gst_element_get_request_pad (selector, name);
  if (!sinkpad) {
     g_print("Unable to create pad %s on %s selector.\n", name, media);
  }
  qsink = gst_element_get_static_pad (queue, "sink");
  if (!qsink) {
     g_print("Unable to get sink pad on %s queue %d.\n", media, channel);
  }

  /* Now, link it all up */

  if (returnCode = gst_pad_link (pad, qsink)) {
     g_print("Link of %s source%d pad to queue failed with Code %d\n", media, channel, returnCode);
  }

  if (gst_element_link_many (queue, depayloader, decoder, NULL) != TRUE) {
     g_print("Unable to link %s %s and %s.\n", queue_name, depayloader_name, decoder_name);
  }

  if (overlay) {
     if (gst_element_link (decoder, overlay) != TRUE) {
        g_print("Unable to link %s and %s.\n", decoder_name, overlay_name);
     }
     tosrc  = gst_element_get_static_pad (overlay, "src");
     if (!tosrc) {
        g_print("Unable to get %s overlay src pad on channel %d.\n", media, channel);
     }
     if (returnCode = gst_pad_link (tosrc, sinkpad)) {
        g_print("Link of %s timeoverlay src %d pad to selector failed with Code %d\n", media, channel, returnCode);
     }
     
  } else {
    decsrc  = gst_element_get_static_pad (decoder, "src");
    if (!decsrc) {
       g_print("Unable to get %s decoder src pad on channel %d.\n", media, channel);
    }
    if (returnCode = gst_pad_link (decsrc, sinkpad)) {
       g_print("Link of %s decoder src %d pad to selector failed with Code %d\n", media, channel, returnCode);
    }
  }


//  g_print("Linked %s source %d to %s %s and %s selector pad %s.\n", 
//           media, channel, depayloader_name, decoder_name, queue_name, name);

  gst_element_sync_state_with_parent(source);

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
     link_new_pad (element, pad, "vq1", "vdepay1", "vdec1", "tol1", "selector", v_channel++, 'v');
  } else {
     link_new_pad (element, pad, "aq1", "adepay1", "adec1", "", "aselector", a_channel++, 'a');
  }


}

static void
cb_new_source2_pad (GstElement *element,
	    GstPad     *pad,
	    gpointer    data)
{
  GstCaps *padCaps;
  const GstStructure *capStr;
 
  /* What kind of pad? */
  padCaps = gst_pad_get_caps(pad);
  if ( g_strrstr(gst_caps_to_string(padCaps),"video") ) {
     link_new_pad (element, pad, "vq2", "vdepay2", "vdec2", "tol2", "selector", v_channel++, 'v');
  } else {
     link_new_pad (element, pad, "aq2", "adepay2", "adec2", "", "aselector", a_channel++, 'a');
  }
}



int main(int argc, char *argv[]) {
  GstElement *source, *source2, *sink, *selector, *aselector, *asink; 
  GstElement *vdec1, *vdec2, *vdepay1, *vdepay2,  *adec1, *adec2, *adepay1, *adepay2;  
  GstElement *vq1, *vq2, *aq1, *aq2, *voq, *aoq;
  GstElement *tol1, *tol2, *tolo, *aresample;
  GstElement *vtee, *atee;
  GstElement *aenc, *aconv, *vconv, *venc, *aencq, *vencq, *vmuxq, *amuxq, *mux;
  GstElement *filesink;
  GstBus *bus;
  GstPad *srcpad, *sinkpad, *pad, *pad0, *pad1;
  GstMessage *msg;
  GstStateChangeReturn ret;
  GMainLoop *loop;
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
   
  /* Create the elements */
  source      = gst_element_factory_make ("rtspsrc",       "source");
  source2     = gst_element_factory_make ("rtspsrc",       "source2");
  selector    = gst_element_factory_make ("input-selector","selector");
  aselector   = gst_element_factory_make ("input-selector","aselector");
  sink        = gst_element_factory_make ("autovideosink", "sink");
  asink       = gst_element_factory_make ("autoaudiosink", "asink");
  vdepay1     = gst_element_factory_make ("rtph264depay",  "vdepay1");
  vdepay2     = gst_element_factory_make ("rtph264depay",  "vdepay2");
  vdec1       = gst_element_factory_make ("ffdec_h264",    "vdec1");
  vdec2       = gst_element_factory_make ("ffdec_h264",    "vdec2");
  adepay1     = gst_element_factory_make ("rtppcmadepay",  "adepay1");
  adepay2     = gst_element_factory_make ("rtppcmadepay",  "adepay2");
  adec1       = gst_element_factory_make ("alawdec",       "adec1");
  adec2       = gst_element_factory_make ("alawdec",       "adec2");
  vq1         = gst_element_factory_make ("queue",         "vq1");
  vq2         = gst_element_factory_make ("queue",         "vq2");
  voq         = gst_element_factory_make ("queue",         "voq");
  aq1         = gst_element_factory_make ("queue",         "aq1");
  aq2         = gst_element_factory_make ("queue",         "aq2");
  aoq         = gst_element_factory_make ("queue",         "aoq");
  tol1        = gst_element_factory_make ("timeoverlay",   "tol1");
  tol2        = gst_element_factory_make ("timeoverlay",   "tol2");
  tolo        = gst_element_factory_make ("timeoverlay",   "tolo");
  aresample   = gst_element_factory_make ("audioresample", "audioresample");
  vtee        = gst_element_factory_make ("tee",           "vtee");
  atee        = gst_element_factory_make ("tee",           "atee");
  aenc        = gst_element_factory_make ("ffenc_mp2",     "aenc");
  aconv       = gst_element_factory_make ("audioconvert",  "aconv");
  venc        = gst_element_factory_make ("ffenc_flv",     "venc");
  vconv       = gst_element_factory_make ("ffmpegcolorspace",  "vconv");
  vencq       = gst_element_factory_make ("queue",         "vencq");
  aencq       = gst_element_factory_make ("queue",         "aencq");
  vmuxq       = gst_element_factory_make ("multiqueue",         "vmuxq");
  amuxq       = gst_element_factory_make ("multiqueue",         "amuxq");
  mux         = gst_element_factory_make ("flvmux",        "mux");
  filesink    = gst_element_factory_make ("filesink",      "filesink");


  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
   
  if (!pipeline || !source || !source2 || !selector || !aselector || 
      !sink || !asink || !vdepay1 || !vdepay2 || !vdec1 || !vdec1 || !adepay1 || !adepay2 || !adec1 || !adec2 ||
      !vq1 || !vq2 || !aq1 || !aq2 || !aoq || !voq || !tol1 || !tol2 || !tolo || !aresample || !vtee || !atee ||
      !aenc || !aconv || !vconv || !venc || !aencq || !vencq || !amuxq || !vmuxq || !mux || !filesink
      ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */

  gst_bin_add (GST_BIN (pipeline), source);
  gst_bin_add (GST_BIN (pipeline), source2);
  gst_bin_add (GST_BIN (pipeline), selector);
  gst_bin_add (GST_BIN (pipeline), aselector);
  gst_bin_add (GST_BIN (pipeline), sink);
  gst_bin_add (GST_BIN (pipeline), asink);
  gst_bin_add (GST_BIN (pipeline), vdepay1);
  gst_bin_add (GST_BIN (pipeline), vdepay2);
  gst_bin_add (GST_BIN (pipeline), vdec1);
  gst_bin_add (GST_BIN (pipeline), vdec2);
  gst_bin_add (GST_BIN (pipeline), adepay1);
  gst_bin_add (GST_BIN (pipeline), adepay2);
  gst_bin_add (GST_BIN (pipeline), adec1);
  gst_bin_add (GST_BIN (pipeline), adec2);
  gst_bin_add (GST_BIN (pipeline), vq1);
  gst_bin_add (GST_BIN (pipeline), vq2);
  gst_bin_add (GST_BIN (pipeline), aq1);
  gst_bin_add (GST_BIN (pipeline), aq2);
  gst_bin_add (GST_BIN (pipeline), voq);
  gst_bin_add (GST_BIN (pipeline), aoq);
  gst_bin_add (GST_BIN (pipeline), tol1);
  gst_bin_add (GST_BIN (pipeline), tol2);
  gst_bin_add (GST_BIN (pipeline), tolo);
  gst_bin_add (GST_BIN (pipeline), aresample);
  gst_bin_add (GST_BIN (pipeline), vtee);
  gst_bin_add (GST_BIN (pipeline), atee);
  gst_bin_add (GST_BIN (pipeline), aenc);
  gst_bin_add (GST_BIN (pipeline), aconv);
  gst_bin_add (GST_BIN (pipeline), venc);
  gst_bin_add (GST_BIN (pipeline), vconv);
  gst_bin_add (GST_BIN (pipeline), aencq);
  gst_bin_add (GST_BIN (pipeline), vencq);
  gst_bin_add (GST_BIN (pipeline), amuxq);
  gst_bin_add (GST_BIN (pipeline), vmuxq);
  gst_bin_add (GST_BIN (pipeline), mux);
  gst_bin_add (GST_BIN (pipeline), filesink);

/*****   VIDEO OUTPUT SIDE ****/

  if (gst_element_link (selector, tolo)  != TRUE) {
    g_printerr ("Selector- output Timeoverlay could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (tolo, voq) != TRUE) {
    g_printerr ("Time overlay to Video output queue could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (voq,vtee) != TRUE) {
    g_printerr ("voq, vtee could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  if (gst_element_link (vtee,sink) != TRUE) {
    g_printerr ("vtee, vsink could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   AUDIO OUTPUT SIDE ****/

  if (gst_element_link (aselector, aresample) != TRUE) {
    g_printerr ("ASelector-resampler could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (aresample, aoq) != TRUE) {
    g_printerr ("Aresampler-A output queue could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (aoq, atee) != TRUE) {
    g_printerr ("Aoutput queue - atee could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (atee, asink) != TRUE) {
    g_printerr ("atee - Asink could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*** MULTIPLEXED FLV SIDE ***/
  

  if (gst_element_link_many (vtee,vencq,vconv,venc,vmuxq,mux,filesink, NULL) != TRUE) {
    g_printerr ("FLV video mux pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  if (gst_element_link_many (atee,amuxq, mux, NULL) != TRUE) {
    g_printerr ("FLV audio mux pipe could not be linked.\n");
   gst_object_unref (pipeline);
    return -1;
  }


  /* Modify the source's properties */

  g_object_set (source, "location", "rtsp://ec2-54-235-164-155.compute-1.amazonaws.com:8554/test", NULL);
  g_object_set (source2, "location", "rtsp://ec2-54-225-91-241.compute-1.amazonaws.com:8554/test", NULL);
 
  g_object_set (selector, "sync-streams", TRUE, NULL);
  g_object_set (aselector, "sync-streams", TRUE, NULL);

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
  g_object_set (aq1, "max-size-bytes", 1000000000, NULL);
  g_object_set (aq2, "max-size-bytes", 1000000000, NULL);
  g_object_set (vmuxq, "max-size-bytes", 1000000000, NULL);
  g_object_set (vencq, "max-size-bytes", 1000000000, NULL);
  g_object_set (voq,   "max-size-bytes", 1000000000, NULL);
  g_object_set (amuxq, "max-size-bytes", 1000000000, NULL);
  g_object_set (aencq, "max-size-bytes", 1000000000, NULL);
  g_object_set (aoq,   "max-size-bytes", 1000000000, NULL);
  g_object_set (vq1, "max-size-time", 0, NULL);
  g_object_set (vq2, "max-size-time", 0, NULL);
  g_object_set (aq1, "max-size-time", 0, NULL);
  g_object_set (aq2, "max-size-time", 0, NULL);
  g_object_set (vmuxq, "max-size-time", 0, NULL);
  g_object_set (vencq, "max-size-time", 0, NULL);
  g_object_set (voq,   "max-size-time", 0, NULL);
  g_object_set (amuxq, "max-size-time", 0, NULL);
  g_object_set (aencq, "max-size-time", 0, NULL);
  g_object_set (aoq,   "max-size-time", 0, NULL);

  g_object_set (filesink,   "location", "/home/murph/stuff.flv", NULL);

  /* listen for newly created pads */
  g_signal_connect (source, "pad-added", G_CALLBACK (cb_new_source_pad), NULL);
  g_signal_connect (source2, "pad-added", G_CALLBACK (cb_new_source2_pad), NULL);


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
