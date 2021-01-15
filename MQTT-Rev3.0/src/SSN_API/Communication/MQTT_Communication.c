#include "MQTT_Communication.h"

void setup_MQTT_Millisecond_Interrupt(uint32_t PERIPH_CLOCK) {
    // enable timer-5 interrupt
    IEC0bits.T5IE = 0x00;			// disable timer 1 interrupt, IEC0<4>
    IFS0CLR = 0x0010;				// clear timer 1 int flag, IFS0<4>
    IPC1CLR = 0x001f;				// clear timer 1 priority/subpriority fields 
    IPC1SET = 0x0010;				// set timer 1 int priority = 4, IPC1<4:2>
    IEC0bits.T5IE = 0x01;			// enable timer 1 int, IEC0<4>
    T5CON = 0x8030;					// this prescaler reduces the input clock frequency by 256
    PR5 = (0.001*PERIPH_CLOCK/256);	// millsecond timer interrupt period
}

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

void SetupMQTTClientConnection(uint32_t clock_frequency, Network* net, MQTTClient* mqtt_client, opts_struct* MQTTOptions, uint8_t *MQTT_IP, char* cliendId, void* messageArrivedoverMQTT) {
	int rc = FAILURE;
	unsigned char tempBuffer[MQTT_BUFFER_SIZE] = {};
	printf("(MQTT): Creating MQTT Network Variables\n");
	NewNetwork(net, MQTT_TCP_SOCKET);
	//printf("%d\n",Client_MQTT.command_timeout_ms);
	printf("(MQTT): Initiating MQTT Client\n");
	MQTTClientInit(mqtt_client, net, 100, MQTT_buf, 100, tempBuffer, 2048);
	//printf("%d\n",Client_MQTT.command_timeout_ms);
	printf("(MQTT): Setting Up MQTT Communication Options\n");
	SetupMQTTOptions(MQTTOptions, cliendId, QOS1, 1, MQTT_IP);
	printf("(MQTT): Setting Up MQTT Data Variables\n");
	SetupMQTTData(&MQTT_DataPacket);
	while (rc == FAILURE) {
		ServiceWatchdog();
		printf("(MQTT): Connecting to MQTT Broker\n");
		ConnectNetwork(clock_frequency, net, MQTT_IP, MQTT_Port);
		printf("(MQTT): Finalizing MQTT Connection\n");
		rc = MQTTConnect(mqtt_client, &MQTT_DataPacket);
		if (rc == SUCCESSS) {
			printf("(MQTT): Successfully Connected to Broker at %d:%d:%d:%d as MQTT Client\n", MQTT_IP[0], MQTT_IP[1], MQTT_IP[2], MQTT_IP[3]);	
			// Now do some subscriptions
			printf("(MQTT): Subscribing to Node Exclusive Channel: %s\n", NodeExclusiveChannel);
			rc = MQTTSubscribe(mqtt_client, NodeExclusiveChannel, MQTTOptions->qos, messageArrivedoverMQTT);
			if (rc == SUCCESSS) {
				printf("(MQTT): Subscription Successful\n");
				return;
			} else {
				printf("(MQTT): Subscription Attempt Failed. Retrying...\n");
			}
		} else {
			printf("(MQTT): Connection to Broker Failed. Retrying in 2 seconds...\n");	
		}
		sleep_for_microseconds(2000000);
	}
}

void SetupMQTTMessage(MQTTMessage* Message_MQTT, uint8_t* payload, uint8_t payload_len, enum QoS x) {
	// printf("%d\n",Message_MQTT->payloadlen);
	Message_MQTT->qos = x;
	Message_MQTT->retained = 0;
	Message_MQTT->dup = 0;
	Message_MQTT->id = 1;
	Message_MQTT->payload = payload;
	// printf("In SetupMQTTMessage=%d \n",payload);
	Message_MQTT->payloadlen = payload_len;
	// printf("SetupMQTTMessage %d\n",Message_MQTT->payloadlen);
}

int Send_Message_Over_MQTT(char* topic, uint8_t* messagetosend, uint8_t len) {
	int rc = 0;
	SetupMQTTMessage(&Message_MQTT, messagetosend, len, QOS1);
	rc = MQTTPublish(&Client_MQTT, topic, &Message_MQTT);
	// printf("Published %d\r\n", rc);
	return rc;
}

int SendMessageMQTT(char* topic, uint8_t* messagetosend, uint8_t ssn_message_to_send_size) {
	int rc = Send_Message_Over_MQTT(topic, messagetosend, ssn_message_to_send_size);
	if (rc == SUCCESSS) {
		printf("(LOG): %d-Byte Message Sent to MQTT Broker\n", ssn_message_to_send_size);
	}
	return rc;
}