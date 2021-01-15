
#ifndef __MQTT_Communication_h__
#define __MQTT_Communication_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/NETWORK/network.h"


void setup_MQTT_Millisecond_Interrupt(uint32_t PERIPH_CLOCK);
void SetupMQTTClientConnection(uint32_t clock_frequency, Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, uint8_t *MQTT_IP, char* cliendId, void* messageArrivedoverMQTT );
int SendMessageMQTT(char* topic, uint8_t* messagetosend, uint8_t ssn_message_to_send_size);

#endif