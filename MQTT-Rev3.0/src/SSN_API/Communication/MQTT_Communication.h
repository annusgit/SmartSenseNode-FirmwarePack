#ifndef __MQTT_Communication_h__
#define __MQTT_Communication_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/NETWORK/network.h"

int SetupMQTTClientConnection(Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, uint8_t *MQTT_IP, char* cliendId, void* messageArrivedoverMQTT, uint8_t* SSN_MAC_ADDRESS);
void CloseMQTTClientConnectionAndSocket(MQTTClient* mqtt_client, uint8_t MQTT_Socket);
void SetupMQTTOptions(opts_struct* MQTTOptions, char* cliendId, enum QoS x, int showtopics, char* MQTT_IP);
void SetupMQTTData(MQTTPacket_connectData* MQTT_DataPacket);
void SetupMQTTMessage(MQTTMessage* Message_MQTT, uint8_t* payload, uint8_t payload_len, enum QoS x);
int SendMessageMQTT(char* topic, uint8_t* messagetosend, uint8_t len);
int getConnTimeout(int attemptNumber);
int MQTTallowedfailureCounts(uint8_t SSN_REPORT_INTERVAL);
#endif