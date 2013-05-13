#!/bin/sh

gst-launch --eos-on-shutdown videotestsrc pattern=18 is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=160,height=120 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! clockoverlay halignment=1 valignment=1 shaded-background=true font-desc="Sans Bold 80" ypad=80 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=green left=-160 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videomixer name=mix ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! ximagesink sync=false \
\
videotestsrc is-live=true ! videoscale ! video/x-raw-yuv,width=160,height=120 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-320 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
videotestsrc is-live=true pattern=11 ! tee name=ch1 !videoscale ! video/x-raw-yuv, width=640, height=480 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=0 top=-252 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
videotestsrc is-live=true pattern=1 ! videoscale ! video/x-raw-yuv, width=160, height=120 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-480 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch1. ! videoscale ! video/x-raw-yuv, width=160, height=120 !\
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
