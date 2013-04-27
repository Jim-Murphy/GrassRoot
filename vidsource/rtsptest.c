#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaMapping *mapping;
  GstRTSPMediaFactory *factory;

  int  reqPattern = 0;
  int  reqTone    = 440;

  char factoryCmd[512];
  char patternStr[80];
  char toneStr[80];
  
  char *cmd = &factoryCmd[0];
  char *patt = &patternStr[0];
  char *tone = &toneStr[0];

  if (argc >=2) {
     reqPattern=atoi(argv[1]);
     if (argc >=3) {
        reqTone=atoi(argv[2]);
     }
  }

  printf("Generating GST Video Test Patten %d\nGenerating GST Audio Test Tone of %d Hz\n", reqPattern, reqTone);

  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);
  server = gst_rtsp_server_new ();
  mapping = gst_rtsp_server_get_media_mapping (server);
  factory = gst_rtsp_media_factory_new ();

  strcpy (cmd, "( videotestsrc pattern=");
  sprintf(patt,"%d", reqPattern);
  strcat (cmd, patt);
  strcat (cmd, " ! video/x-raw-yuv,width=320,height=240,framerate=10/1 ! x264enc ! queue ! rtph264pay name=pay0 pt=96  audiotestsrc freq=");
  sprintf(tone,"%d", reqTone);
  strcat (cmd, tone);
  strcat (cmd, " ! audio/x-raw-int,rate=8000 ! alawenc ! rtppcmapay name=pay1 pt=97 "")");

  gst_rtsp_media_factory_set_launch (factory, cmd);

  gst_rtsp_media_factory_set_shared (factory, TRUE);
  gst_rtsp_media_mapping_add_factory (mapping, "/test", factory);
  g_object_unref (mapping);
  gst_rtsp_server_attach (server, NULL);
  g_main_loop_run (loop);

  return 0;
}

