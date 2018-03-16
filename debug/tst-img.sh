#!/bin/bash
gst-launch-1.0 filesrc location=$1 ! pngdec ! videoconvert ! qreader ! videoconvert ! pngenc ! filesink location=tst_img_out.png
