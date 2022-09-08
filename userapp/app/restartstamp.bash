#!/bin/bash

# Description: Stops the automated timestamping by killing the event_cap
#	       Process.
#		Then restarts it. This is so that we can do intervals using cron
# and miss less pulses when we restart the timestamping program.

# get process id of event_cap
CHILD_PID=`ps aux | grep event_cap | awk '$11 !~ /grep/ {print $2}'`

echo "Killing PID = $CHILD_PID"

while [ "${CHILD_PID}" != "" ]
do
	kill -9 $CHILD_PID
	CHILD_PID=`ps aux | grep event_cap | awk '$11 !~ /grep/ {print $2}'`
done

# Now restart the sym560 command line program
cd /home/dds/epop_timestamps 
sym560_cmdline auto >> /home/dds/logging/sym560cmdlineautolog.txt 2>&1

