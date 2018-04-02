#!/bin/bash

gst-launch-1.0 filesrc location=../img/tst.mpeg ! videoconvert ! qreader ! videoconvert ! ximagesink
