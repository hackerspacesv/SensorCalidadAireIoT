#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "config.h"

#ifndef MQTT_Sensor_Config_H
#error Debe de renombrar el archivo "config.example.h" a "config.h" antes de continuar.
#endif

// Buffer para mensajes de salida
char message[64];

// En el ESP8266 el puerto serie. Asegurese de conectar los pines 12 y 14
SoftwareSerial particleSensor(12, 14);

// Variables para almacenar las mediciones de pm
float pm25;
float pm10;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;

/*
 * setup_wifi(): Intenta conectar a la WiFi especificada
 * en la configuración. Este intentará conectar de forma continua.
 * Si toma demasiado tiempo en conectar revise las credenciales
 * y la orientación del receptor respecto al router.
 */
void setup_wifi() {
  delay(10);
  
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}


/*
 * callback(): Esta función es llamada cada vez que se recibe un
 * nuevo mensaje en el tema correspondiente al que el terminal esta
 * suscrito. Pueden enviarse mensajes de control al dispositivo.
 * Este ejemplo no se suscribe a ningún tema por defecto. Revise
 * el código en reconnect() para encontrar un ejemplo de como
 * suscribirse a un tema en particular.
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect() {
  // Repetir hasta lograr una conexión
  while (!client.connected()) {
    Serial.print("Intentando conexión a MQTT");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_passwd)) {
      Serial.println("conectado");
      // Si desea suscribirse a un tema descomente las siguientes líneas
      //client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("error, rc=");
      Serial.print(client.state());
      Serial.println(" intentando nuevamente en 5 segundos.");
      delay(5000);
    }
  }
}

/*
 * Actualiza las mediciones de partes por millon.
 * Nota: Esta implementación es bloqueante. Implementar versión no bloqueante.
 */
boolean dataAvailable(void)
{
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1)
  {
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Error de timeout
    }

    if (particleSensor.read() == 0xAA) break; //Encabezado del mensaje obtenido
  }

  //Read the next 9 bytes
  byte sensorValue[10];
  for (byte spot = 1 ; spot < 10 ; spot++)
  {
    startTime = millis();
    while (!particleSensor.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); // tiempo de espera agotado
    }

    sensorValue[spot] = particleSensor.read();
  }

  // Cálculo de CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++)
    crc += sensorValue[x];
  if (crc != sensorValue[8])
    return (false); // CRC inválido

  // Actualiza las variables locales con los valores de medición
  pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
  pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;

  return (true); // Si llegamos aquí tenemos una lectura válida
}

/*
 * Mantiene la conexión al broker MQTT y envía mensajes
 * con el valor del sensor cada vez que hay datos nuevos
 */
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Envía una publicación cada vez que se recibe un mensaje nuevo 
  if (dataAvailable()) {
    sprintf(message, "{'pm25':%f,'pm10':%f}", pm25, pm10);
    Serial.print("Publicando mensaje: ");
    Serial.println(message);
    client.publish(mqtt_topic_pub, message);
  }
}
