#!/bin/bash

gst-launch-1.0 filesrc location=../img/tst.mpeg ! decodebin ! queue ! videoconvert ! qreader ! videoconvert ! ximagesink
