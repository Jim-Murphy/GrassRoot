gst-launch  udpsrc  port=5000 ! application/x-rtp, clock-rate=90000,payload=96 ! rtpmp4vdepay queue-delay=0 ! ffdec_mpeg4 ! autovideosink
