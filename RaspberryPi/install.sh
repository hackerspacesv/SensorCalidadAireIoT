#!/bin/bash
echo Installing required software
sudo apt-get install vim python-pip python-sqlite sqlite3 python-serial git
sudo pip install paho-mqtt
echo Setting up directories
mkdir log data
