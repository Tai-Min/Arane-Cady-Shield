#!/bin/bash

# USE "/usr/bin/python /storage/cady/mcu_daemon/mcu_daemon.py" in crontab every minute or so as Lakka doesn't seem to trigger @reboot
# If your system supports @reboot then just use it

if pgrep -f "/usr/bin/python /storage/cady/mcu_daemon/mcu_daemon.py" &> /dev/null; then
	exit
else
	systemctl start retroarch.target && /usr/bin/python /storage/cady/mcu_daemon/mcu_daemon.py
fi
