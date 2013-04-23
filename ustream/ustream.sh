#!/bin/sh
gst-launch videotestsrc !  'video/x-raw-yuv, width=640, height=480' ! \
timeoverlay halign=left valign=bottom text="Stream time:" shaded-background=true !\
videorate ! 'video/x-raw-yuv, framerate=15/1' ! \
ffenc_flv bitrate=300000 ! \
queue max-size-bytes=100000000 max-size-time=0 name=vq ! mux. \
audiotestsrc ! \
queue max-size-bytes=100000000 max-size-time=0  name=aq ! mux. \
flvmux name=mux ! rtmpsink location="rtmp://1.7669465.fme.ustream.tv/ustreamVideo/7669465/dX3r3M2m3mAfLwfA7hCa9YQXc3FntQum flashver=FME/2.5\20(compatible;\20FMSc\201.0)"

