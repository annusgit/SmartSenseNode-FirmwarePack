#include "Communication.h"

char ConfigurationsChannel[14]  = "Configurations";

uint8_t message_to_send[max_send_message_size] = {0};
uint8_t message_to_recv[max_recv_message_size] = {0};
uint8_t params[max_recv_message_size] = {0};


int Send_GETMAC_Message(uint8_t* NodeID) {
	/* Clear the message array */
	clear_array(message_to_send, max_send_message_size);
	uint8_t ssn_message_to_send_size = construct_get_mac_message(message_to_send, NodeID);
	return SendMessageMQTT(GettersChannel, message_to_send, ssn_message_to_send_size);
}

int Send_GETCONFIG_Message(uint8_t* NodeID) {
	/* Clear the message array */
	clear_array(message_to_send, max_send_message_size);
	uint8_t ssn_message_to_send_size = construct_get_configuration_message(message_to_send, NodeID);
	return SendMessageMQTT(GettersChannel, message_to_send, ssn_message_to_send_size);
}

int Send_ACKCONFIG_Message(uint8_t* NodeID, uint8_t* SSN_CONFIG) {
	/* Clear the message array */
	clear_array(message_to_send, max_send_message_size);
	uint8_t ssn_message_to_send_size = construct_ack_configuration_message(message_to_send, NodeID, SSN_CONFIG);
	return SendMessageMQTT(GettersChannel, message_to_send, ssn_message_to_send_size);
}

int Send_GETTimeOfDay_Message(uint8_t* NodeID) {
	/* Clear the message array */
	clear_array(message_to_send, max_send_message_size);
	uint8_t ssn_message_to_send_size = construct_get_timeofday_message(message_to_send, NodeID);
	return SendMessageMQTT(GettersChannel, message_to_send, ssn_message_to_send_size);
}

int Send_STATUSUPDATE_Message(uint8_t* NodeID, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, float* Machine_load_currents, uint8_t* Machine_load_percentages, 
        uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, uint32_t* Machine_status_timestamp, uint32_t ssn_uptime_in_seconds, 
        uint8_t abnormal_activity) {
	/* Clear the message array but we can't because if we do, this will throw an error at the server end */
	//clear_array(message_to_send, max_send_message_size);
	// Finally, construct the full status update message structure
	uint8_t ssn_message_to_send_size = construct_status_update_message(message_to_send, NodeID, temperature_bytes, relative_humidity_bytes, Machine_load_currents, 
            Machine_load_percentages, Machine_status, Machine_status_flag, Machine_status_duration,
		Machine_status_timestamp, ssn_uptime_in_seconds, abnormal_activity);
	if (ssn_message_to_send_size != STATUS_UPDATE_MESSAGE_Size) {
		// This is not possible but still..
		printf("[ERROR] Message BAD due to INCORRECT BYTE SIZE\n");
	}
	return SendMessageMQTT(StatusUpdatesChannel, message_to_send, ssn_message_to_send_size);
}

int Send_RETRIEVECONFIG_Message(uint8_t* NodeID, uint8_t* SSN_CONFIG) {
	/* Clear the message array */
	clear_array(message_to_send, max_send_message_size);
	uint8_t ssn_message_to_send_size = construct_retrieve_configuration_message(message_to_send, NodeID, SSN_CONFIG);
	return SendMessageMQTT(ConfigurationsChannel, message_to_send, ssn_message_to_send_size);
}