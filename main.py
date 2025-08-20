from flask import Flask, render_template
from flask_socketio import SocketIO
import paho.mqtt.client as mqtt
import json
import threading

app = Flask(__name__)
socketio = SocketIO(app)

MQTT_BROKER = "172.20.10.3"  # same as ESP broker
MQTT_TOPIC = "hive/data"
MQTT_COMMAND = "hive/command"

latest_data = {"temperature": 0, "beeCount": 0}

# MQTT Callbacks
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT with result code", rc)
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    global latest_data
    payload = json.loads(msg.payload.decode())
    latest_data = payload

    # Emit to dashboard
    socketio.emit('updateData', latest_data)

    # Check condition
    if payload["beeCount"] > 10:
        client.publish(MQTT_COMMAND, "expand the Hive")
        socketio.emit('alert', {"message": "Expand the Hive!"})

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, 1883, 60)

# Run MQTT loop in background
threading.Thread(target=mqtt_client.loop_forever).start()

@app.route('/')
def index():
    return render_template("dashboard.html")

if __name__ == "__main__":
    socketio.run(app, host="0.0.0.0", port=5000)
