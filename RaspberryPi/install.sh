#!/bin/bash
UDEV_RULE='KERNEL==\"ttyS0\", RUN+=\"/bin/systemctl --no-block start air_quality\"'
UDEV_FILE="99-com.rules"

echo "Installing required software"
sudo apt-get install vim python-pip python-sqlite sqlite3 python-serial git minicom
sudo pip install paho-mqtt
echo "Setting up directories"
mkdir log data
touch log/monitor.log
echo "Adding Systemd service"
sudo cp system_files/air_quality.service /etc/systemd/system/
sudo systemctl daemon-reload
echo "Adding udev rules"
grep -q -F 'KERNEL=="ttyS0", RUN+="/bin/systemctl --no-block start air_quality"' /etc/udev/rules.d/$UDEV_FILE || sudo su -c "echo $UDEV_RULE >> /etc/udev/rules.d/$UDEV_FILE"
sudo udevadm control --reload
sudo udevadm trigger
