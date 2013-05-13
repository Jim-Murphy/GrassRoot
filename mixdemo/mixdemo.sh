#!/bin/sh

gst-launch --eos-on-shutdown videotestsrc pattern=18 is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=240 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! clockoverlay halignment=1 valignment=1 shaded-background=true font-desc="Sans Bold 80" ypad=80 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=green left=-320 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videomixer name=mix ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! ximagesink sync=false \
\
videotestsrc is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=240 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-640 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
videotestsrc is-live=true pattern=11 ! video/x-raw-yuv, width=352, height=240 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
videotestsrc is-live=true pattern=1 ! video/x-raw-yuv, width=352, height=240 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-960 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
