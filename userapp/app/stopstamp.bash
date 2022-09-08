#!/bin/bash

# Description: Stops the automated timestamping by killing the event_cap
#	       Process.

# get process id of event_cap
CHILD_PID=`ps aux | grep event_cap | awk '$11 !~ /grep/ {print $2}'`

echo "Killing PID = $CHILD_PID"

# kill event_cap
kill -9 $CHILD_PID
