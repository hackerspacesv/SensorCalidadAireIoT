#!/usr/bin/python
import time
import signal
import sys
import json
import sqlite3
import logging
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
from AirQualityMonitor import Monitor
import config as Config

# Set from config.py
log = Config.log
database = Config.database
period = Config.period

# Program start
run = True

# MQTT connection procedure
def on_connect(client, userdata, flags, rc):
    client.subscribe("sensors");

# Try to connect to mqtt server
def try_mqtt_connect(client):
    try:
        client.connect(Config.mqtt_host, port=Config.mqtt_port)
        client.loop_start()
        logging.info("Connection to MQTT sucessfull")
    except:
        logging.error("Cannot connect to MQTT broker")

# Sqlite DB initialization
def init_db(datafile):
    conn = sqlite3.connect(datafile)
    c = conn.cursor()
    c.execute("CREATE TABLE IF NOT EXISTS measurements(id INTEGER PRIMARY KEY, timestamp INTEGER, pm25 REAL, pm10 REAL)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_timestamps ON measurements(timestamp)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_pm25 ON measurements(pm25)")
    c.execute("CREATE INDEX IF NOT EXISTS idx_pm10 ON measurements(pm10)")
    conn.commit()
    return conn

def init_logging(logfile):
    logging.basicConfig(filename=logfile, level=logging.DEBUG)

# Handler for interrupt signals
def signal_handler(signal, frame):
    global run
    print("Interrupt received.")
    run = False

# Set signal handlers
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGUSR1, signal_handler)

# Application entry point
if __name__ == "__main__":
    init_logging(log)
    logging.info("Starting up...")

    logging.info("Setting up database")
    conn = init_db(database)
    c = conn.cursor();

    monitor = Monitor()

    # Set up client configuration
    client = mqtt.Client()
    client.username_pw_set(Config.mqtt_user, password=Config.mqtt_pass)

    # Currently we are not listening for messages
    # but we expect to subscribe to configuration
    # messages.
    client.on_connect = on_connect

    try_mqtt_reconnect = False

    while run:
        monitor.update()

        if try_mqtt_reconnect:
            try_mqtt_connect(client)
            try_mqtt_reconnect = False

        if monitor.has_new_data():
            # Prepare data
            timestamp = int(time.time())
            pm25 = monitor.get_pm25()
            pm10 = monitor.get_pm10()
            logging.info("PM2.5 reading: "+str(pm25))
            logging.info("PM10 reading: "+str(pm10))
            c.execute("INSERT INTO measurements(timestamp, pm25, pm10) VALUES(?,?,?)",(timestamp,pm25,pm10))
            conn.commit()

            # Prepare MQTT message
            payload = {'pm25':pm25,'pm10':pm10}
            auth_info =  {'username': Config.mqtt_user, 'password': Config.mqtt_pass}
            result = client.publish(
                    Config.mqtt_topic,
                    payload=json.dumps(payload),
                    qos=0,
                    retain=False,
                )
            # Try to reconnect on next read
            if result.rc == mqtt.MQTT_ERR_NO_CONN:
                try_mqtt_reconnect = True
        
        # Wait before try again
        time.sleep(period)

    client.loop_stop()
    monitor.cleanup()
    sys.exit(0)

