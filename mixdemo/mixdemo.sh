#!/bin/sh

gst-launch --eos-on-shutdown \
\
videotestsrc pattern=18 is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=480 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! \
clockoverlay halignment=1 valignment=1 shaded-background=true font-desc="Sans Bold 80" ypad=160 ! \
tee name=ch1 \
\
ch1. ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=160,height=120 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=green left=-8 top=-8 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videomixer name=mix ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! ximagesink sync=false \
\
videotestsrc is-live=true pattern=11 ! videoscale ! video/x-raw-yuv, width=640, height=480 ! \
tee name=ch2 \
\
videotestsrc is-live=true pattern=0 ! videoscale ! video/x-raw-yuv, width=640, height=480 ! \
tee name=ch3 \
\
videotestsrc is-live=true pattern=15 ! videoscale ! video/x-raw-yuv, width=640, height=480 ! \
tee name=ch4 \
\
ch2. ! videoscale ! video/x-raw-yuv, width=160, height=120 !\
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-176 top=-8 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch3. ! queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=160,height=120 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-344 top=-8 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch4. ! queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=160,height=120 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-512 top=-8 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
queue name=standby max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=240 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-8 top=-136 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
queue name=live max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=240 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-344 top=-136 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch1. ! standby. \
ch3. ! live. \
