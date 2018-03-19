#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// Update these with values suitable for your network.

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
char message[64];

SoftwareSerial particleSensor(12, 14); // RX, TX

float pm25; //2.5um particles detected in ug/m3
float pm10; //10um particles detected in ug/m3

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Repetir hasta lograr una conexión
  while (!client.connected()) {
    Serial.print("Intentando conexión a MQTT");
    if (client.connect("SensorCalidadAire", "", "")) {
      Serial.println("conectado");
    } else {
      Serial.print("error, rc=");
      Serial.print(client.state());
      Serial.println(" intentando nuevamente en 5 segundos.");
      delay(5000);
    }
  }
}

// Actualiza las mediciones de nivel de partículas
// para 2.5 y 10 partes por millón.
boolean dataAvailable(void)
{
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1)
  {
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    if (particleSensor.read() == 0xAA) break; //We have the message header
  }

  //Read the next 9 bytes
  byte sensorValue[10];
  for (byte spot = 1 ; spot < 10 ; spot++)
  {
    startTime = millis();
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    sensorValue[spot] = particleSensor.read();
  }

  //Check CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
    crc += sensorValue[x];
  if (crc != sensorValue[8])
    return (false); //CRC error

  //Update the global variables
  pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
  pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;

  return (true); //We've got a good reading!
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (dataAvailable()) {
    sprintf(message, "{'pm25':%f,'pm10':%f}", pm25, pm10);
    Serial.print("Publish message: ");
    Serial.println(message);
    client.publish("sensors", message);
  }
}
