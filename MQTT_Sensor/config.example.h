/* Este archivo contiene las opciones de configuración para el
 * cliente MQTT utilizando el ESP8266. Lea detenidamente las
 * opciones de configuración y modifique con los valores que
 * sean requeridos. Debe asegurarse de renombrar este archivo
 * a "config.h" antes de compilar.
 */

#ifndef MQTT_Sensor_Config_H
#define MQTT_Sensor_Config_H

/* Opciones de configuración WiFi */
// Nombre de la red WiFi
const char* wifi_ssid = "";
// Clave de la red WiFi
const char* wifi_password = "";

/* Configuración del Broker MQTT */
// Identificador de cliente MQTT
const char* mqtt_client_id = "";
// Dirección del servidor MQTT
const char* mqtt_server = "";
// Usuario para publicación
const char* mqtt_user = "";
// Clave para publicación
const char* mqtt_passwd = "";
// Nombre del tema para publicaciones
const char* mqtt_topic_pub = "";
// Nombre del tema para suscripciones
const char* mqtt_topic_sub = "";

#endif
