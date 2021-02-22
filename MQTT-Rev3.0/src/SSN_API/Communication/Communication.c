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
    uint8_t status;
    clear_array(stringtosend, stringtosendsize);
    switch(Error_code) {
        printf("%d",Error_code);
       case MQTT_Publication_Failed:
          errorstring="MQTT_Publication_Failed";
          break;
       case TCP_Socket_Conn_Failed:
          errorstring="TCP_Socket_Conn_Failed";
          break;          
       case Ethernet_just_recovered:
          errorstring="Ethernet_just_recovered";
          break;   
       case DHCP_IP_Time_Received:
          errorstring="DHCP_IP_Time_Received";
          break;          
       case SSN_just_Restarted:
          errorstring="SSN_just_Restarted";
          break;        
       case MQTT_Client_Reconnected:
          errorstring="MQTT_Reconnected";
          break;        
       case TimeofDay_Received_for_syncing:
          errorstring="TimeofDay_Received_for_syncing";
          break;
       case MQTT_Connection_failed_Restarting:
          errorstring="MQTT_Connection_failed_Restarting";
          break;    
       default : /* Optional */
          printf("here\n");
     }   
	sprintf(stringtosend, "%02X:%02X:%02X:%02X:%02X:%02X, %d, %s", SSN_MAC_ADDRESS[0], SSN_MAC_ADDRESS[1], SSN_MAC_ADDRESS[2], SSN_MAC_ADDRESS[3], SSN_MAC_ADDRESS[4], SSN_MAC_ADDRESS[5], ssn_time, errorstring);
    uint8_t stringtosend_size = strlen(stringtosend);
//    printf("String is %s Length of string stringtosend is = %d \n",stringtosend, stringtosend_size);
    status = sendto(SSN_UDP_Socket, stringtosend, stringtosend_size, UDP_SERVER_IP, UDP_SERVER_PORT);
    printf("%d byte string sent on UDP %s \n",status, stringtosend);
    return status;

}
void initUDPDebug(struct UDP_Debug_message *UDP_Debug_msg, uint8_t* ssn_mac,uint16_t udp_debug_server_port, uint8_t* udp_debug_server_ip, uint32_t ssn_current_time, SOCKET udp_debug_socket){
//    printf("setting structure variables\n");
//    strcpy(UDP_Debug_msg->SSN_MAC, ssn_mac);
    UDP_Debug_msg->SSN_MAC[0] = ssn_mac[0];
    UDP_Debug_msg->SSN_MAC[1] = ssn_mac[1];
    UDP_Debug_msg->SSN_MAC[2] = ssn_mac[2];
    UDP_Debug_msg->SSN_MAC[3] = ssn_mac[3];    
    UDP_Debug_msg->SSN_MAC[4] = ssn_mac[4];
    UDP_Debug_msg->SSN_MAC[5] = ssn_mac[5];   
//    printf("%02X:%02X:%02X:%02X:%02X:%02X\n",ssn_mac[0], ssn_mac[1], ssn_mac[2], ssn_mac[3], ssn_mac[4], ssn_mac[5]);
    UDP_Debug_msg->SSN_UDP_DEBUG_SERVER_IP[0] = udp_debug_server_ip[0];
    UDP_Debug_msg->SSN_UDP_DEBUG_SERVER_IP[1] = udp_debug_server_ip[1];
    UDP_Debug_msg->SSN_UDP_DEBUG_SERVER_IP[2] = udp_debug_server_ip[2];
    UDP_Debug_msg->SSN_UDP_DEBUG_SERVER_IP[3] = udp_debug_server_ip[3];
//    printf("%d.%d.%d.%d\n",udp_debug_server_ip[0], udp_debug_server_ip[1], udp_debug_server_ip[2], udp_debug_server_ip[3]);
    UDP_Debug_msg->SSN_UDP_DEBUG_SERVER_PORT = udp_debug_server_port;
//    printf("%d \n",udp_debug_server_port);
    UDP_Debug_msg->ssn_curent_time = ssn_current_time;
//    printf("%d \n",ssn_current_time);
    UDP_Debug_msg->SSN_UDP_DEBUG_SOCKET = udp_debug_socket;
//    printf("done structure variables\n");
}

int32_t sendDebugmessageUDP(UDP_Debug_message* UDP_message, uint32_t ssn_time, uint8_t Error_code){  
    uint8_t status;
    clear_array(stringtosend, stringtosendsize);
     switch(Error_code) {
        printf("%d",Error_code);

       case MQTT_Publication_Failed:
          errorstring="MQTT_Publication_Failed";
          break;
       case TCP_Socket_Conn_Failed:
          errorstring="TCP_Socket_Conn_Failed";
          break;          
       case Ethernet_just_recovered:
          errorstring="Ethernet_just_recovered";
          break;   
       case DHCP_IP_Time_Received:
          errorstring="DHCP_IP_Time_Received";
          break;          
       case SSN_just_Restarted:
          errorstring="SSN_just_Restarted";
          break;        
       case MQTT_Client_Reconnected:
          errorstring="MQTT_Reconnected";
          break;        
       case TimeofDay_Received_for_syncing:
          errorstring="TimeofDay_Received_for_syncing";
          break;        
       case MQTT_Connection_failed_Restarting:
          errorstring="MQTT_Connection_failed_Restarting";
          break;        
       default : /* Optional */
          printf("here\n");
     }   
//    printf("%02X:%02X:%02X:%02X:%02X:%02X\n",UDP_message->SSN_MAC[0], UDP_message->SSN_MAC[1], UDP_message->SSN_MAC[2], UDP_message->SSN_MAC[3], UDP_message->SSN_MAC[4], UDP_message->SSN_MAC[5]);
    UDP_message->ssn_curent_time=ssn_time;
	sprintf(stringtosend, "%02X:%02X:%02X:%02X:%02X:%02X, %d, %s", UDP_message->SSN_MAC[0], UDP_message->SSN_MAC[1], UDP_message->SSN_MAC[2], UDP_message->SSN_MAC[3], UDP_message->SSN_MAC[4], UDP_message->SSN_MAC[5], UDP_message->ssn_curent_time, errorstring);
//    sprintf(stringtosend, "hi, %d, %s", UDP_message->ssn_curent_time, errorstring);
    uint8_t stringtosend_size = strlen(stringtosend);
//    printf("%d\n",stringtosend_size);
//    printf("String is %s Length of string stringtosend is = %d \n",stringtosend, stringtosend_size);
    status = sendto(UDP_message->SSN_UDP_DEBUG_SOCKET, stringtosend, stringtosend_size, UDP_message->SSN_UDP_DEBUG_SERVER_IP, UDP_message->SSN_UDP_DEBUG_SERVER_PORT);
    printf("[UDP] %d byte message string sent on UDP = %s \n",status, stringtosend);
    return status;

}
