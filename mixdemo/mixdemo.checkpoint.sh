#!/bin/sh

gst-launch --eos-on-shutdown \
\
videotestsrc pattern=18 is-live=true ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=360 ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! \
clockoverlay halignment=1 valignment=1 shaded-background=true font-desc="Sans Bold 80" ypad=160 ! \
tee name=ch1 \
\
ch1. ! \
queue max-size-bytes=100000000 max-size-time=0 ! \
textoverlay text="Input1" valign="bottom" shaded-background=false font-desc="Sans Bold 20" ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=180 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=green left=0 top=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videomixer name=mix ! \
queue max-size-bytes=100000000 max-size-time=0 ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! ximagesink sync=false \
\
videotestsrc is-live=true pattern=11 ! videoscale ! video/x-raw-yuv, width=640, height=360 ! \
tee name=ch2 \
\
videotestsrc is-live=true pattern=0 ! videoscale ! video/x-raw-yuv, width=640, height=360 ! \
tee name=ch3 \
\
videotestsrc is-live=true pattern=15 ! videoscale ! video/x-raw-yuv, width=640, height=360 ! \
tee name=ch4 \
\
ch2. ! queue max-size-bytes=100000000 max-size-time=0 ! \
textoverlay text="Input2" valign="bottom" shaded-background=false font-desc="Sans Bold 20" ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=180 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-320 top=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch3. ! queue max-size-bytes=100000000 max-size-time=0 ! \
textoverlay text="Input3" valign="bottom" shaded-background=false font-desc="Sans Bold 20" ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=180 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-640 top=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch4. ! queue max-size-bytes=100000000 max-size-time=0 ! \
textoverlay text="Input4" valign="bottom" shaded-background=false font-desc="Sans Bold 20" ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=320,height=180 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-960 top=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
queue name=standby max-size-bytes=100000000 max-size-time=0 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=360 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=0 top=-180 ! \
textoverlay text="STANDBY" valign="bottom" shaded-background=false font-desc="Sans Bold 40" ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
queue name=live max-size-bytes=100000000 max-size-time=0 ! \
textoverlay text="LIVE" valign="bottom" shaded-background=false font-desc="Sans Bold 40" ! ffmpegcolorspace ! \
queue max-size-bytes=100000000 max-size-time=0 ! videoscale ! video/x-raw-yuv,width=640,height=360 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videorate ! video/x-raw-yuv,framerate=15/1 ! \
queue max-size-bytes=100000000 max-size-time=0 ! videobox border-alpha=0 fill=blue left=-640 top=-180 ! \
queue max-size-bytes=100000000 max-size-time=0 ! mix. \
\
ch1. ! standby. \
ch3. ! live. \
