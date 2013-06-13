#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main (int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMediaMapping *mapping;
  GstRTSPMediaFactory *factory;

  char factoryCmd[512];
  char patternStr[80];
  char toneStr[80];
  
  char *cmd = &factoryCmd[0];
  char *patt = &patternStr[0];
  char *tone = &toneStr[0];

  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);
  server = gst_rtsp_server_new ();
  mapping = gst_rtsp_server_get_media_mapping (server);
  factory = gst_rtsp_media_factory_new ();

  strcpy (cmd, "( udpsrc port=5000 ");
  strcat (cmd, " ! application/x-rtp, clock-rate=90000,payload=96 ");
  strcat (cmd, " ! rtpjpegdepay queue-delay=0 ");
  strcat (cmd, " ! queue ");
  strcat (cmd, " ! rtpjpegpay name=pay0 pt=96 )");

  g_print("Sending command\n%s\n", cmd);

  gst_rtsp_media_factory_set_launch (factory, cmd);

  gst_rtsp_media_factory_set_shared (factory, TRUE);
  gst_rtsp_media_mapping_add_factory (mapping, "/monitor", factory);
  g_object_unref (mapping);
  gst_rtsp_server_attach (server, NULL);
  g_main_loop_run (loop);

  return 0;
}

