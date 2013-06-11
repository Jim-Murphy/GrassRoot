gst-launch -v \
souphttpsrc location="http://66.184.211.231/mjpg/video.mjpg" do-timestamp=TRUE ! \
queue max-size-bytes=1000000000 ! \
jpegdec ! \
queue max-size-bytes=1000000000 ! \
timeoverlay ! \
queue max-size-bytes=1000000000 ! \
ffenc_flv ! \
flvmux ! \
rtmpsink location="rtmp://1.7669465.fme.ustream.tv/ustreamVideo/7669465/dX3r3M2m3mAfLwfA7hCa9YQXc3FntQum flashver=FME/2.5\20(compatible;\20FMSc\201.0)"

