gst-launch videotestsrc ! ffenc_mpeg4 ! rtpmp4vpay send-config=true ! udpsink host=68.189.245.64 port=5000
