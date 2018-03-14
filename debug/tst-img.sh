#!/bin/bash
gst-launch-1.0 filesrc location=$1 ! pngdec ! videoconvert ! qreader ! videoconvert ! fakesink
