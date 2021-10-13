#ifndef __MQTT_Communication_h__
#define __MQTT_Communication_h__

#include "../global.h"
#include "../Drivers/UART/uart.h"
#include "../Drivers/NETWORK/network.h"
#include "../Drivers/NETWORK/Internet/MQTT/MQTTClient.h"

/**************************************************************************//**
 * @MQTT variables
 *****************************************************************************/
extern Network MQTT_Network;
extern MQTTClient Client_MQTT;    
extern MQTTMessage Message_MQTT;
extern MQTTPacket_connectData MQTT_DataPacket;
extern char StatusUpdatesChannel[13];
extern char GettersChannel[7];
extern char NodeExclusiveChannel[17];
extern unsigned char MQTT_buf[100];

typedef struct opts_struct {
	char clientid[MQTT_MAX_LEN];
	int nodelimiter;
	char delimiter[MQTT_MAX_LEN];
	enum QoS qos;
	char username[MQTT_MAX_LEN];
	char password[MQTT_MAX_LEN];
	char host[4]; // this is an ip
	int port;
	int showtopics;
} opts_struct;
extern opts_struct MQTTOptions;


int SetupMQTTClientConnection(Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, MQTTPacket_connectData* mqtt_datapacket, uint8_t *MQTT_IP, char* cliendId_node_exclusive_channel, 
        void* messageArrivedoverMQTT);
void CloseMQTTClientConnectionAndSocket(MQTTClient* mqtt_client, uint8_t MQTT_Socket);
void SetupMQTTOptions(opts_struct* MQTTOptions, char* cliendId, enum QoS x, int showtopics, char* MQTT_IP);
void SetupMQTTData(MQTTPacket_connectData* MQTT_DataPacket);
void SetupMQTTMessage(MQTTMessage* Message_MQTT, uint8_t* payload, uint8_t payload_len, enum QoS x);
int SendMessageMQTT(char* topic, uint8_t* messagetosend, uint8_t len);
int getConnTimeout(int attemptNumber);
int MQTTallowedfailureCounts(uint8_t SSN_REPORT_INTERVAL);

#endif