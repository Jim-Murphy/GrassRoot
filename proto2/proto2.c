#include <gst/gst.h>

//#define SCREEN_SINK
//#define FILE_SINK
#define RTMP_SINK  

//#define INCLUDE_MONITOR
#define REMOTE_MONITOR


static GstElement *pipeline;

static int active_channel = 0;
static int num_channels = 3; 
static int auto_switch_interval = 0;
static int auto_switch_countdown = 0;

static void add_text (GstElement * pipeline, gchar *text)

{
   
  GstElement *overlay;

  overlay = gst_bin_get_by_name (GST_BIN(pipeline), "usrtextover");

  g_object_set (overlay, "text", text, NULL);
  g_object_set (overlay, "silent", FALSE, NULL);
 
}

static void clear_text (GstElement * pipeline)

{
   
  GstElement *overlay;

  overlay = gst_bin_get_by_name (GST_BIN(pipeline), "usrtextover");

  g_object_set (overlay, "silent", TRUE, NULL);
  g_object_set (overlay, "text", "", NULL);
 
}


static gboolean switch_channel (GstElement * pipeline, int new_channel)
{
  int other_channel;
  GstElement *select;
  GstStateChangeReturn ret;
  gchar *name;
  gchar *othername;
  GstPad *pad;
  gint64 v_stoptime;
  gint64 v_starttime;
  gint64 v_runningtime;
  gint64 starttime, stoptime;

  if ((new_channel > num_channels) ||
      (new_channel < 1 )) {
     g_print ("Illegal channel number %d\n", new_channel);
     return TRUE;
  }

  active_channel = new_channel - 1;   /*  Pad numbers are Channel_N -1 */

  /* find the selector */
  select = gst_bin_get_by_name (GST_BIN (pipeline), "selector");

  if (!select) {
     g_print("Input selector not found\n");
  }

  /* get the named pad */
  name = g_strdup_printf ("sink%d", active_channel);
  pad = gst_element_get_static_pad (select, name);
  if (!pad) {
     g_print("Input selector pad %s not found\n", name);
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


static gboolean check_cmd (GstElement * pipeline) {

   int next_channel;
   gboolean result;
   FILE *cmd_file;
   char line[80];
   char command[80];
   char argument[80];


  GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "foo");

   if (cmd_file = fopen("/tmp/grctl", "rt"))   //  Only jump in here if there's a command to process
   {

      if (fgets(line, 80, cmd_file) != NULL)
      {
	 /* get a line, up to 80 chars from cmd file.  done if NULL */
	 sscanf (line, "%s", &command[0]);
      }

      fclose(cmd_file);
      remove("/tmp/grctl");

      if (strcmp(command,"switch")==0) {
  
         auto_switch_interval = 0;   // turn off auto switching if we issue a
         auto_switch_countdown = 0;  // manual switch command

         sscanf (line, "%s %d", &command[0], &next_channel);
         result = switch_channel (pipeline, next_channel);  

      } else if (strcmp(command, "text")==0)  {

         sscanf (line, "%s %s", &command[0], &argument[0]);
       
         if (strcmp(argument, "off")==0) {   // if "off" clear it, otherwise display it
            clear_text(pipeline); 
         } else {
            add_text(pipeline, &line[5]);
         }

      } else if (strcmp(command, "auto-sw")==0)  {
         sscanf (line, "%s %s", &command[0], &argument[0]);
       
         if (strcmp(argument, "off")==0) {   // if "off" stop switching
            auto_switch_interval = 0;
            auto_switch_countdown = 0;
         } else {
            sscanf (line, "%s %d", &command[0], &auto_switch_interval);
            auto_switch_countdown = auto_switch_interval;
         }

      } else if (strcmp(command, "pip")==0)  {
         g_print("PIP command \n");
      } else {
         g_print("unknown command \n");
      }


   }

   if (auto_switch_interval) {
      auto_switch_countdown--;
      if (auto_switch_countdown <= 0) {
         auto_switch_countdown = auto_switch_interval;
        
         if (active_channel >= (num_channels - 1) ) {
            switch_channel (pipeline, 1);
         } else {
            switch_channel (pipeline, active_channel + 2);
         } 

      }
   }

   return TRUE;

}

int main(int argc, char *argv[]) {
  GstElement *source, *source2, *source3, *sink, *selector; 
  GstElement *monitor, *mtee, *mq, *mq2;
  GstElement *vdec1, *vdec2, *vdec3;
  GstElement *vq1, *vq2, *vq3, *voq;
  GstElement *txo1, *txo2, *txo3, *tolo;
  GstElement *usrtextover;
  GstElement *vconv, *venc, *vencq, *vmuxq, *mux, *rtmpq;
  GstElement *filesink, *rtmpsink;
  GstElement *monscale, *moncapsfil;
  GstElement *remmonscale, *remmoncapsfil;
  GstElement *remmontol, *remmonencq, *remmonenc, *remmonpayq, *remmonpay, *remmonsink;
  GstElement *vmixq, *vmix, *vpipq;
  GstElement *outscale, *outcapsfil;

  GstBus *bus;
  GstPad *srcpad, *sinkpad, *pad, *pad0, *pad1;
  GstMessage *msg;
  GstStateChangeReturn ret;
  GMainLoop *loop;

  GstCaps *pipCaps;
  GstCaps *monCaps;
  GstCaps *outCaps;
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
   
  /* Create the elements */
  source      = gst_element_factory_make ("souphttpsrc",       "source");
  source2     = gst_element_factory_make ("souphttpsrc",       "source2");
  source3     = gst_element_factory_make ("souphttpsrc",       "source3");
  selector    = gst_element_factory_make ("input-selector","selector");
  vdec1       = gst_element_factory_make ("jpegdec",       "vdec1");
  vdec2       = gst_element_factory_make ("jpegdec",       "vdec2");
  vdec3       = gst_element_factory_make ("jpegdec",       "vdec3");
  vq1         = gst_element_factory_make ("queue",         "vq1");
  vq2         = gst_element_factory_make ("queue",         "vq2");
  vq3         = gst_element_factory_make ("queue",         "vq3");
  voq         = gst_element_factory_make ("queue",         "voq");
  txo1        = gst_element_factory_make ("textoverlay",   "txo1");
  txo2        = gst_element_factory_make ("textoverlay",   "txo2");
  txo3        = gst_element_factory_make ("textoverlay",   "txo3");
  usrtextover = gst_element_factory_make ("textoverlay",   "usrtextover");
  tolo        = gst_element_factory_make ("timeoverlay",   "tolo");
  venc        = gst_element_factory_make ("ffenc_flv",     "venc");
  vconv       = gst_element_factory_make ("ffmpegcolorspace",  "vconv");
  vencq       = gst_element_factory_make ("queue",         "vencq");
  vmuxq       = gst_element_factory_make ("queue",    "vmuxq");
  mux         = gst_element_factory_make ("flvmux",        "mux");

  vmixq       = gst_element_factory_make ("queue",         "vmixq");
  vpipq       = gst_element_factory_make ("queue",         "vpipq");
  vmix        = gst_element_factory_make ("videomixer",    "vmix");

#ifdef FILE_SINK
  filesink    = gst_element_factory_make ("filesink",      "filesink");
#endif
#ifdef RTMP_SINK
  rtmpsink    = gst_element_factory_make ("rtmpsink",      "rtmpsink");
#endif
#ifdef SCREEN_SINK
  sink        = gst_element_factory_make ("xvimagesink", "sink");
#endif

#ifdef INCLUDE_MONITOR
  monitor     = gst_element_factory_make ("xvimagesink",   "monitor");
#else
  monitor     = gst_element_factory_make ("fakesink",   "monitor");
#endif

  outscale    = gst_element_factory_make ("videoscale",    "outscale");
  outcapsfil  = gst_element_factory_make ("capsfilter",    "outcapsfil");

  mtee        = gst_element_factory_make ("tee",           "mtee");
  mq          = gst_element_factory_make ("queue",         "mq");
  mq2         = gst_element_factory_make ("queue",         "mq2");
  monscale    = gst_element_factory_make ("videoscale",    "monscale");
  moncapsfil  = gst_element_factory_make ("capsfilter",    "moncapsfil");


  remmonscale   = gst_element_factory_make ("videoscale", "remmonscale");
  remmoncapsfil = gst_element_factory_make ("capsfilter", "remmoncapsfil");
  remmontol   = gst_element_factory_make ("timeoverlay",  "remmontol");
  remmonenc   = gst_element_factory_make ("ffenc_mjpeg",  "remmonenc");
  remmonencq  = gst_element_factory_make ("queue",        "remmonencq");
  remmonpay   = gst_element_factory_make ("rtpjpegpay",   "remmonpay");
  remmonpayq  = gst_element_factory_make ("queue",        "remmonpayq");
#ifdef REMOTE_MONITOR
  remmonsink = gst_element_factory_make ("udpsink",    "remmonsink");
#else
  remmonsink = gst_element_factory_make ("fakesink",   "remmonsink");
#endif

  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
   
  if (!pipeline || !source || !source2 || !source3 || !selector || 
      !vdec1 || !vdec2 || !vdec3 ||
      !vq1 || !vq2 || !vq3 || !voq || !txo1 || !txo2 || !usrtextover || !tolo || 
      !vconv || !venc || !vencq || !vmuxq || !mux || 
      !vmixq || !vpipq || !vmix || !outscale || !outcapsfil ||
      !monitor || !mtee || !mq || !mq2 || !monscale || !moncapsfil ||
      !remmonscale || !remmoncapsfil ||
      !remmontol || !remmonenc || !remmonpay || !remmonsink || !remmonencq || !remmonpayq ||

#ifdef SCREEN_SINK
      !sink
#endif
#ifdef FILE_SINK
      !filesink  
#endif
#ifdef RTMP_SINK
      !rtmpsink 
#endif
      ) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */

  gst_bin_add (GST_BIN (pipeline), source);
  gst_bin_add (GST_BIN (pipeline), source2);
  gst_bin_add (GST_BIN (pipeline), source3);
  gst_bin_add (GST_BIN (pipeline), selector);
  gst_bin_add (GST_BIN (pipeline), vdec1);
  gst_bin_add (GST_BIN (pipeline), vdec2);
  gst_bin_add (GST_BIN (pipeline), vdec3);
  gst_bin_add (GST_BIN (pipeline), vq1);
  gst_bin_add (GST_BIN (pipeline), vq2);
  gst_bin_add (GST_BIN (pipeline), vq3);
  gst_bin_add (GST_BIN (pipeline), voq);
  gst_bin_add (GST_BIN (pipeline), txo1);
  gst_bin_add (GST_BIN (pipeline), txo2);
  gst_bin_add (GST_BIN (pipeline), txo3);
  gst_bin_add (GST_BIN (pipeline), usrtextover);
  gst_bin_add (GST_BIN (pipeline), tolo);
  gst_bin_add (GST_BIN (pipeline), venc);
  gst_bin_add (GST_BIN (pipeline), vconv);
  gst_bin_add (GST_BIN (pipeline), vencq);
  gst_bin_add (GST_BIN (pipeline), vmuxq);
  gst_bin_add (GST_BIN (pipeline), mux);
  gst_bin_add (GST_BIN (pipeline), vmixq);
  gst_bin_add (GST_BIN (pipeline), vpipq);
  gst_bin_add (GST_BIN (pipeline), vmix);
  gst_bin_add (GST_BIN (pipeline), outscale);
  gst_bin_add (GST_BIN (pipeline), outcapsfil);

  gst_bin_add (GST_BIN (pipeline), monitor);
  gst_bin_add (GST_BIN (pipeline), mtee);
  gst_bin_add (GST_BIN (pipeline), mq);
  gst_bin_add (GST_BIN (pipeline), mq2);
  gst_bin_add (GST_BIN (pipeline), monscale);
  gst_bin_add (GST_BIN (pipeline), moncapsfil);

  gst_bin_add (GST_BIN (pipeline), remmonscale);
  gst_bin_add (GST_BIN (pipeline), remmoncapsfil);
  gst_bin_add (GST_BIN (pipeline), remmontol);
  gst_bin_add (GST_BIN (pipeline), remmonenc);
  gst_bin_add (GST_BIN (pipeline), remmonencq);
  gst_bin_add (GST_BIN (pipeline), remmonpay);
  gst_bin_add (GST_BIN (pipeline), remmonpayq);
  gst_bin_add (GST_BIN (pipeline), remmonsink);

#ifdef FILE_SINK
  gst_bin_add (GST_BIN (pipeline), filesink);
#endif
#ifdef RTMP_SINK
  gst_bin_add (GST_BIN (pipeline), rtmpsink);
#endif
#ifdef SCREEN_SINK
  gst_bin_add (GST_BIN (pipeline), sink);
#endif

/*****   VIDEO 1 INPUT SIDE  ***/

  if (gst_element_link_many (source, vq1, vdec1, txo1, selector, NULL) != TRUE) {
    g_printerr ("Video input 1  pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   VIDEO 2 INPUT SIDE  ***/

  if (gst_element_link_many (source2, vq2, vdec2, txo2, selector, NULL) != TRUE) {
    g_printerr ("Video input 2  pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   VIDEO 3 INPUT SIDE  ***/

  if (gst_element_link_many (source3, vq3, vdec3, txo3, selector, NULL) != TRUE) {
    g_printerr ("Video input 3  pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   VIDEO OUTPUT SIDE ****/


  {
     GstPad *srcPad, *sinkPad;

     srcPad=gst_element_get_static_pad ( selector, "src");
     sinkPad=gst_element_get_static_pad ( usrtextover, "video_sink");
    
     if (gst_pad_link(srcPad,sinkPad)) {
        g_print("link of selector to textoverlay failed,\n");
     }
  }
  
  if (gst_element_link_many (usrtextover, outscale, outcapsfil, mtee, tolo, vencq, NULL) != TRUE) {
    g_printerr ("Video output pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

#ifdef SCREEN_SINK
  if (gst_element_link_many (vencq, sink, NULL) != TRUE) {
#endif
#ifdef RTMP_SINK
  if (gst_element_link_many (vencq, venc, vmuxq, mux, voq, rtmpsink, NULL) != TRUE) {
#endif
#ifdef FILE_SINK
  if (gst_element_link_many (vencq, venc, vmuxq, mux, voq, filesink, NULL) != TRUE) {
#endif
    g_printerr ("Video output pipe2 could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   VIDEO MONITOR SIDE  ***/

  if (gst_element_link_many (mtee, mq2, monscale, moncapsfil, mq, monitor, NULL) != TRUE) {
    g_printerr ("Video monitor pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

/*****   REMOTE MONITOR SIDE  ***/

  if (gst_element_link_many (mtee, remmonscale, remmoncapsfil, remmontol, remmonencq, 
                             remmonenc, remmonpayq, remmonpay, remmonsink, NULL) != TRUE) {
    g_printerr ("Remotemonitor pipe could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  /* Modify the source's properties */

//Key West Swimming Pool:
  g_object_set (source, "location", "http://66.184.211.231/mjpg/video.mjpg", NULL);

//Colorado College, ColoSprings:
  g_object_set (source2, "location", "http://webcam1.coloradocollege.edu/mjpg/video.mjpg", NULL);

// Clarkson Univ Stu Center:
//  g_object_set (source3, "location", "http://128.153.6.47/mjpg/video.mjpg", NULL);

// Kestrel Nest, Lincoln, NE
//  g_object_set (source2, "location", "http://164.119.54.138/mjpg/video.mjpg", NULL);

// Dartmouth College:
  g_object_set (source3, "location", "http://wc2.dartmouth.edu/mjpg/video.mjpg", NULL);

// St. Joes Yacht Club, Michigan:
//  g_object_set (source3, "location", "http://sjryc.axiscam.net/axis-cgi/mjpg/video.cgi", NULL);

// Ocean City Boardwalk:
//  g_object_set (source3, "location", "http://74.94.55.182/axis-cgi/mjpg/video.cgi", NULL);

// Local Cam from Shadywood
//  g_object_set (source3, "location", "http://68.189.245.64:8088/videofeed", NULL);

  g_object_set (source, "do-timestamp", TRUE, NULL);
  g_object_set (source2, "do-timestamp", TRUE, NULL);
  g_object_set (source3, "do-timestamp", TRUE, NULL);
  g_object_set (source, "is-live", TRUE, NULL);
  g_object_set (source2, "is-live", TRUE, NULL);
  g_object_set (source3, "is-live", TRUE, NULL);

  g_object_set (selector, "sync-streams", TRUE, NULL);

  outCaps = gst_caps_new_simple ("video/x-raw-yuv",
                           "width", G_TYPE_INT, 640,
                           "height", G_TYPE_INT, 480,
                           NULL);
  
  monCaps = gst_caps_new_simple ("video/x-raw-yuv",
                           "width", G_TYPE_INT, 320,
                           "height", G_TYPE_INT, 240,
                           NULL);

  g_object_set (G_OBJECT (outcapsfil), "caps", outCaps, NULL);

  g_object_set (G_OBJECT (moncapsfil), "caps", monCaps, NULL);
  g_object_set (G_OBJECT (remmoncapsfil), "caps", monCaps, NULL);


#ifdef RTMP_SINK
  g_object_set (rtmpsink, "location",
  "rtmp://1.7669465.fme.ustream.tv/ustreamVideo/7669465/dX3r3M2m3mAfLwfA7hCa9YQXc3FntQum flashver=FME/2.5 (compatible; FMSc 1.0)",NULL);
  g_object_set (rtmpsink, "sync", FALSE, NULL);
#endif

#ifdef FILE_SINK
  g_object_set (filesink,   "location", "stuff.flv", NULL);
#endif

  g_object_set (txo1, "halign","left", NULL);
  g_object_set (txo1, "valign","top", NULL);
  g_object_set (txo1, "text","1", NULL);
  g_object_set (txo1, "shaded-background",FALSE, NULL);
  g_object_set (txo1, "font-desc","Sans Bold 12", NULL);
  
  g_object_set (txo2, "halign","left", NULL);
  g_object_set (txo2, "valign","top", NULL);
  g_object_set (txo2, "text","2", NULL);
  g_object_set (txo2, "shaded-background",FALSE, NULL);
  g_object_set (txo2, "font-desc","Sans Bold 12", NULL);

  g_object_set (txo3, "halign","left", NULL);
  g_object_set (txo3, "valign","top", NULL);
  g_object_set (txo3, "text","3", NULL);
  g_object_set (txo3, "shaded-background",FALSE, NULL);
  g_object_set (txo3, "font-desc","Sans Bold 12", NULL);

  g_object_set (tolo, "halign","right", NULL);
  g_object_set (tolo, "valign","top", NULL);
  g_object_set (tolo, "shaded-background",TRUE, NULL);
  g_object_set (tolo, "font-desc","Sans Bold 12", NULL);

  g_object_set (usrtextover, "halign","center", NULL);
  g_object_set (usrtextover, "valign","bottom", NULL);
  g_object_set (usrtextover, "text","", NULL);
  g_object_set (usrtextover, "font-desc","Sans Bold 20", NULL);
  g_object_set (usrtextover, "silent",FALSE, NULL);

  g_object_set (vq1, "max-size-bytes", 1000000000, NULL);
  g_object_set (vq2, "max-size-bytes", 1000000000, NULL);
  g_object_set (vmuxq, "max-size-bytes", 1000000000, NULL);
  g_object_set (vencq, "max-size-bytes", 1000000000, NULL);
  g_object_set (voq,   "max-size-bytes", 1000000000, NULL);
  g_object_set (mq,   "max-size-bytes", 1000000000, NULL);
  g_object_set (mq2,  "max-size-bytes", 1000000000, NULL);
  g_object_set (remmonencq,  "max-size-bytes", 1000000000, NULL);
  g_object_set (remmonpayq,  "max-size-bytes", 1000000000, NULL);
  g_object_set (vmixq,  "max-size-bytes", 1000000000, NULL);
  g_object_set (vpipq,  "max-size-bytes", 1000000000, NULL);

#ifdef REMOTE_MONITOR
  g_object_set (remmontol, "halign","right", NULL);
  g_object_set (remmontol, "valign","top", NULL);
  g_object_set (remmontol, "shaded-background",TRUE, NULL);
  g_object_set (remmontol, "font-desc","Sans Bold 20", NULL);
  g_object_set (remmonsink,   "host", "127.0.0.1" , NULL);
  g_object_set (remmonsink,   "port", 5000 , NULL);
#endif

  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  
  /* Wait until error or EOS */

  loop=g_main_loop_new (NULL, FALSE);
  g_timeout_add (1000, (GSourceFunc) check_cmd, pipeline);
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
