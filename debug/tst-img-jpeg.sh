#!/bin/bash
gst-launch-1.0 filesrc location=$1 ! jpegdec ! videoconvert ! qreader ! videoconvert ! pngenc ! filesink location=tst_img_out.png
