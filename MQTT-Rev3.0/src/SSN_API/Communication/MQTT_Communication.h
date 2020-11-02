
#ifndef __MQTT_Communication_h__
#define __MQTT_Communication_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/NETWORK/network.h"

struct MQTTClient SetupMQTTClientConnection(Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, uint8_t *MQTT_IP, char* cliendId, void* messageArrivedoverMQTT );
bool SendMessageMQTT(uint8_t* messagetosend, uint8_t ssn_message_to_send_size);

#endif