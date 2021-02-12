#include "Communication.h"


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
		printf("(ERROR): Message BAD due to INCORRECT BYTE SIZE\n");
	}
	return SendMessageMQTT(StatusUpdatesChannel, message_to_send, ssn_message_to_send_size);
}

int32_t sendmessageUDP(uint8_t* SSN_MAC_ADDRESS,uint32_t ssn_time, uint8_t SSN_UDP_Socket, uint8_t* UDP_SERVER_IP, uint16_t UDP_SERVER_PORT,uint8_t Error_code){  
//    uint8_t errorstringsize;
    clear_array(stringtosend, stringtosendsize);
//    memcpy(stringtosend, NodeExclusiveChannel, strlen(NodeExclusiveChannel));
//    printf("%s\n\n",stringtosend);
    
//    if (Error_code == MQTT_Publication_Failed){
//          errorstring="MQTT_Publication_Failed";
//    }
//    else if (Error_code == TCP_Socket_Error){
//          errorstring="TCP_Socket_Error";
//    }

     switch(Error_code) {
        printf("%d",Error_code);

       case MQTT_Publication_Failed:
          errorstring="MQTT_Publication_Failed";
          break;
       case TCP_Socket_Error:
          errorstring="TCP_Socket_Error";
          break;          
       case No_Ethernet_Connection:
          errorstring="No_Ethernet_Connection";
          break;   
       case DHCP_IP_Time_Received:
          errorstring="DHCP_IP_Time_Received";
          break;          
//       case Retrieve_Current_Configurations:
//           
//           errorstring="Retrieve_Current_Configurations";
//          break;
       default : /* Optional */
     printf("here\n");
     }   
////    errorstring = "hello world";
////    printf("%s",errorstring);
////    printf("%d",sizeof(errorstring));
//
//    
//    memcpy(stringtosend+ strlen(NodeExclusiveChannel), errorstring, strlen(errorstring));
//    printf("%s\n\n",stringtosend);
//
////    printf("%s")
////    stringtosend[sizeof bytes] = '\0';
////    stringtosend = "%d:%d:%d:%d:%d:%d HelloThere",NodeID[1];
////    sprintf(stringtosend,"%d:%d:%d:%d:%d:%d HelloThere",&NodeID);
	sprintf(stringtosend, "%02X:%02X:%02X:%02X:%02X:%02X, %d= %s", SSN_MAC_ADDRESS[0], SSN_MAC_ADDRESS[1], SSN_MAC_ADDRESS[2], SSN_MAC_ADDRESS[3], SSN_MAC_ADDRESS[4], SSN_MAC_ADDRESS[5], ssn_time, errorstring);
    uint8_t stringtosend_size = strlen(stringtosend);
    printf("String is %s Length of string stringtosend is = %d \n",stringtosend, stringtosend_size);

    return sendto(SSN_UDP_Socket, stringtosend, stringtosend_size, UDP_SERVER_IP, UDP_SERVER_PORT);
    
}
