#/bin/sh

gst-launch --eos-on-shutdown \
\
videotestsrc pattern=0 is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=360 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 name=vch1 \
\
videotestsrc pattern=18 is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=360 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
tee name=vch2tap ! \
queue max-size-bytes=100000000 max-size-time=0 name=vch2 \
\
input-selector name=vsel ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=360 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
vch2tap. ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=180 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-320 top=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
vch1. ! vsel. \
vch2. ! vsel. \
\
videomixer name=mix sink_0::alpha=1.0 sink_1::alpha=1.0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! textoverlay text="GrassRoot Media" valign="bottom" shaded-background=false font-desc="Sans Bold 20" ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! ximagesink sync=false \
