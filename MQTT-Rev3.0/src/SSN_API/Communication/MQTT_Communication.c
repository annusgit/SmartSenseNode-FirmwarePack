#include "MQTT_Communication.h"

void SetupMQTTData(MQTTPacket_connectData* MQTT_DataPacket) {
	MQTT_DataPacket->willFlag = 0;
	MQTT_DataPacket->MQTTVersion = 3;
	MQTT_DataPacket->clientID.cstring = MQTTOptions.clientid;
	MQTT_DataPacket->username.cstring = MQTTOptions.username;
	MQTT_DataPacket->password.cstring = MQTTOptions.password;
	MQTT_DataPacket->keepAliveInterval = 60;
	MQTT_DataPacket->cleansession = 0;
}

void SetupMQTTOptions(opts_struct* MQTTOptions, char* cliendId, enum QoS x, int showtopics, char* MQTT_IP) {
	strcpy(MQTTOptions->clientid, cliendId);
	MQTTOptions->nodelimiter = 0;
	strcpy(MQTTOptions->delimiter, "\n");
	MQTTOptions->qos = x;
	strcpy(MQTTOptions->username, "NULL");
	strcpy(MQTTOptions->password, "NULL");
	strcpy(MQTTOptions->host, MQTT_IP);
	MQTTOptions->port = MQTT_Port;
	MQTTOptions->showtopics = showtopics;
}

struct MQTTClient SetupMQTTClientConnection(Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, uint8_t *MQTT_IP, char* cliendId, void* messageArrivedoverMQTT) {
	int rc = 0;
	unsigned char tempBuffer[MQTT_BUFFER_SIZE] = {};
	printf("Waiting for connection 1...\n");
	NewNetwork(net, MQTT_TCP_SOCKET);
	printf("Waiting for connection 2...\n");
	ConnectNetwork(net, MQTT_IP, MQTT_Port);
	//printf("%d\n",Client_MQTT.command_timeout_ms);
	printf("Waiting for connection 3...\n");
	MQTTClientInit(mqtt_client, net, 1000, MQTT_buf, 100, tempBuffer, 2048);
	//printf("%d\n",Client_MQTT.command_timeout_ms);
	printf("Waiting for connection 4...\n");
	SetupMQTTOptions(MQTTOptions, cliendId, QOS1, 1, MQTT_IP);
	printf("Waiting for connection 5...\n");
	SetupMQTTData(&MQTT_DataPacket);
	printf("Waiting for connection 6...\n");
	rc = MQTTConnect(mqtt_client, &MQTT_DataPacket);
	printf(">>I feel Connected!!!\r\n");
	// Now do some subscriptions
	printf("Subscribing to %s\r\n", NodeExclusiveChannel);
	rc = MQTTSubscribe(mqtt_client, NodeExclusiveChannel, MQTTOptions->qos, messageArrivedoverMQTT);
	printf("Subscribed %d\r\n", rc);
}

void SetupMQTTMessage(MQTTMessage* Message_MQTT, uint8_t* payload, uint8_t payload_len, enum QoS x) {
	//    printf("%d\n",Message_MQTT->payloadlen);
	Message_MQTT->qos = x;
	Message_MQTT->retained = 0;
	Message_MQTT->dup = 0;
	Message_MQTT->id = 1;
	Message_MQTT->payload = payload;
	//    printf("In SetupMQTTMessage=%d \n",payload);
	Message_MQTT->payloadlen = payload_len;
	//    printf("SetupMQTTMessage %d\n",Message_MQTT->payloadlen);
}

void Send_Message_Over_MQTT(char* topic, uint8_t* messagetosend, uint8_t len) {
	int rc = 0;
	SetupMQTTMessage(&Message_MQTT, messagetosend, len, QOS1);
	rc = MQTTPublish(&Client_MQTT, topic, &Message_MQTT);
	// printf("Published %d\r\n", rc);
	// return rc;
}

bool SendMessageMQTT(char* topic, uint8_t* messagetosend, uint8_t ssn_message_to_send_size) {
	//    printf("In sendmessageMQTT %d\n",ssn_message_to_send_size);
	Send_Message_Over_MQTT(topic, messagetosend, ssn_message_to_send_size);
	printf("-> %d-Byte Message Sent to MQTT Broker\n", ssn_message_to_send_size);
	return true;
}