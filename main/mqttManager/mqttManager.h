#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include "mqtt_client.h"


void mqtt_app_start(void);

void mqtt_publish(const char *topic, const char *data);

#endif // MQTTMANAGER_H
