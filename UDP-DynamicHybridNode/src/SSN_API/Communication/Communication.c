#include "Communication.h"

uint8_t message_to_send[max_send_message_size];
uint8_t message_to_recv[max_recv_message_size];
uint8_t params[max_recv_message_size];

/** Data Node Specific Variables */
uint8_t SENDER_IP[4];
uint16_t SENDER_PORT;
/** Routing table */
MAC_IP_Dictionary routing_dictionary = { .count = 0 };


bool SendMessage(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* message_to_send, uint8_t ssn_message_to_send_size) {
    int32_t send_message_status = Send_Message_Over_UDP(SSN_Socket, message_to_send, ssn_message_to_send_size, SSN_SERVER_IP, SSN_SERVER_PORT);
    if (send_message_status==ssn_message_to_send_size) {
        printf("-> %d-Byte Message Sent to IP: %d:%d:%d:%d @ PORT:%d\n", send_message_status, SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3], SSN_SERVER_PORT);
        return true;
    }
    else {
        printf("-> Error : %d\n", send_message_status);
        return false;
    }
}

void Send_GETMAC_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_get_mac_message(message_to_send, NodeID);
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
}

void Send_GETCONFIG_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_get_configuration_message(message_to_send, NodeID);
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
}

void Send_ACKCONFIG_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* SSN_CONFIG) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_ack_configuration_message(message_to_send, NodeID, SSN_CONFIG);
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
}

void Send_GETTimeOfDay_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_get_timeofday_message(message_to_send, NodeID);
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
}

bool Send_STATUSUPDATE_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, 
        float* Machine_load_currents, uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, 
        uint32_t* Machine_status_timestamp, uint32_t ssn_uptime_in_seconds, uint8_t abnormal_activity) {
    /* Clear the message array but we can't because if we do, this will throw an error at the server end */
    //clear_array(message_to_send, max_send_message_size);
    // Finally, construct the full status update message structure
    uint8_t ssn_message_to_send_size = construct_status_update_message(message_to_send, NodeID, temperature_bytes, relative_humidity_bytes, Machine_load_currents, Machine_load_percentages, 
            Machine_status, Machine_status_flag, Machine_status_duration, Machine_status_timestamp, ssn_uptime_in_seconds, abnormal_activity);
    if(ssn_message_to_send_size!=STATUS_UPDATE_MESSAGE_Size) {
        // This is not possible but still..
        printf("(Message BAD) ");
    }
    return SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);    
}

bool SSN_I_AM_DESTINATION(uint8_t* SSN_Mac_Address, uint8_t* destination_of_message) {
	uint8_t i; for(i=0;i<6;i++) {
		if (destination_of_message[i] != SSN_Mac_Address[i]) {
			return false;
		}
	}
	return true;
}

int8_t find_in_dictionary(MAC_IP_Dictionary* dictionary, uint8_t* mac_address) {
	bool match_found;
	/** Loop through each dictionary key */
	uint8_t i; for (i=0;i<dictionary->count;i++) {
		match_found = true;
		/** Loop through each byte of MAC address */
		uint8_t j; for (j=0;j<MAC_LEN;j++) {
			/** if a single byte mismatch is found, break */
			if (dictionary->mac_addresses[MAC_LEN*i+j]!=mac_address[j]) {
				match_found = false;
				break;
			}
		}
		if (match_found) {
			/** match found, return index */
			return i;
		}
	}
	/** no match found, return sentinel */
	return No_Match_Found;
}

void Receive_MAC(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    
    /* Clear the message array */
    clear_array(message_to_recv, max_recv_message_size);
    
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t received_message_id, received_message_status;

    // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
    Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);

    // if there are more than one messages in buffer, we want to receive all of them
    while (Received_Message_Bytes_in_Buffer) {

        // read the message from buffer
        received_message_status = Recv_Message_Over_UDP(SSN_Socket, message_to_recv, max_recv_message_size, SENDER_IP, SENDER_PORT);

        // Parse and make sense of the message
        // 'params' array stores and organizes whatever data we have received in the message
        // this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
        received_message_id = decipher_received_message(message_to_recv, params);

        // based on which message was received (received_message_id), we extract and save the data
        switch (received_message_id) {
            case SET_MAC_MESSAGE_ID:
                printf("<- SET_MAC MESSAGE RECEIVED: %X:%X:%X:%X:%X:%X\n", params[0], params[1], params[2], params[3], params[4], params[5]);
                printf("Reseting Controller Now...\n");
                // write the new MAC addresses to designated location in EEPROM
                EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_MAC_LOC, params, EEPROM_MAC_SIZE);
                // reset the SSN from software
                SoftReset();
                while(1);
                break;

            // Only for debugging, will be removed
            // This message will clear the EEPROM of our SSN
            case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
                // stop the global timer
                stop_Global_Clock();
                printf("(DEBUG): Clearing EEPROM Now...\n");
                // Clear EEPROM and reset node
                EEPROM_Clear();
                // reset the SSN
                printf("(DEBUG): Reseting Controller Now...\n");
                SoftReset();
                while(1);
                break;

            // Only for debugging, will be removed
            // This message will reset our SSN
            case DEBUG_RESET_SSN_MESSAGE_ID:
                // stop the global timer
                stop_Global_Clock();
                // reset the SSN
                printf("(DEBUG): Reseting Controller Now...\n");
                sleep_for_microseconds(1000000);
                SoftReset();
                while(1);
                break;

            default:
                break;
        }
        // See if there is another message in the buffer so we can do this all over again
        Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);            
    }
}

uint8_t Receive_CONFIG(uint8_t SSN_Socket, uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* SSN_CONFIG, uint8_t* SSN_REPORT_INTERVAL, uint8_t* TEMPERATURE_MIN_THRESHOLD, 
	uint8_t* TEMPERATURE_MAX_THRESHOLD, uint8_t* HUMIDITY_MIN_THRESHOLD, uint8_t* HUMIDITY_MAX_THRESHOLD, uint8_t* SSN_CURRENT_SENSOR_RATINGS,  uint8_t* SSN_CURRENT_SENSOR_THRESHOLDS, 
	uint8_t* SSN_CURRENT_SENSOR_MAXLOADS, float* SSN_CURRENT_SENSOR_VOLTAGE_SCALARS, uint8_t* Machine_status) {
        
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t received_message_id, received_message_size;
    // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
    Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);
    // if there are more than one messages in buffer, we want to receive all of them
    while (Received_Message_Bytes_in_Buffer) {
		//printf("Received\n");
		/* Clear the message array */
	    clear_array(message_to_recv, max_recv_message_size);
        // read the message from buffer
        received_message_size = Recv_Message_Over_UDP(SSN_Socket, message_to_recv, max_recv_message_size, SENDER_IP, SENDER_PORT);
		/** check who is the destination of the message, if this node itself, then consume the message 
		 * otherwise, route it to the proper destination
		 */
		if (SSN_I_AM_DESTINATION(SSN_MAC_ADDRESS, &message_to_recv[0])) {
			// Parse and make sense of the message
			// 'params' array stores and organizes whatever data we have received in the message
			// this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
			received_message_id = decipher_received_message(message_to_recv, params);
			// based on which message was received (received_message_id), we extract and save the data
			switch (received_message_id) {
				case SET_CONFIG_MESSAGE_ID:
					// write the new config to designated location in EEPROM
					EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, params, EEPROM_CONFIG_SIZE);
					// Copy received configurations to the SSN_CONFIG array
					int i; for (i = 0; i < EEPROM_CONFIG_SIZE; i++) {
						SSN_CONFIG[i] = params[i];
					}
					// Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
					for (i = 0; i < NO_OF_MACHINES; i++) {
						/* Get the parameters from the Configurations */
						SSN_CURRENT_SENSOR_RATINGS[i]       = SSN_CONFIG[4*i+0];
						SSN_CURRENT_SENSOR_THRESHOLDS[i]    = SSN_CONFIG[4*i+1];
						SSN_CURRENT_SENSOR_MAXLOADS[i]      = SSN_CONFIG[4*i+2];
						if (SSN_CONFIG[4*i+3] == 0) {
							SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 1.0;
						} else {
							SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[i] = 0.333;
						}
					}
					*TEMPERATURE_MIN_THRESHOLD	= SSN_CONFIG[23];
					*TEMPERATURE_MAX_THRESHOLD	= SSN_CONFIG[24];
					*HUMIDITY_MIN_THRESHOLD		= SSN_CONFIG[25];
					*HUMIDITY_MAX_THRESHOLD		= SSN_CONFIG[26];
					// save new reporting interval
					*SSN_REPORT_INTERVAL = SSN_CONFIG[27];
					printf("LOG: Received New Current Sensor Configuration from SSN Server: \n"
						"     >> S1-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M1-Threshold: %03d Arms | M1-Maxload: %03d Arms |\n"
						"     >> S2-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M2-Threshold: %03d Arms | M2-Maxload: %03d Arms |\n"
						"     >> S3-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M3-Threshold: %03d Arms | M3-Maxload: %03d Arms |\n"
						"     >> S4-Rating: %03d Arms | S1-Scalar: %.3f Vrms | M4-Threshold: %03d Arms | M4-Maxload: %03d Arms |\n"
						"     >> MIN TEMP : %03d C    | MAX TEMP : %03d C    |\n"
						"     >> MIN RH   : %03d %    | MAX RH   : %03d %    |\n"
						"     >> Report   : %d seconds\n", 
						SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
						SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
						SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
						SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_VOLTAGE_SCALARS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], 
						*TEMPERATURE_MIN_THRESHOLD, *TEMPERATURE_MAX_THRESHOLD, 
						*HUMIDITY_MIN_THRESHOLD, *HUMIDITY_MAX_THRESHOLD,
						*SSN_REPORT_INTERVAL);
					// Reset Machine States 
					for (i = 0; i < NO_OF_MACHINES; i++) {
						Machine_status[i] = SENSOR_NOT_CONNECTED;
					}
					return 1;
					break;

				// Only for debugging, will be removed
				// This message will clear the EEPROM of our SSN
				case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
					// stop the global timer
					//stop_Global_Clock();
					DisableGlobalInterrupt();
					printf("(DEBUG): Clearing EEPROM Now...\n");
					// Clear EEPROM and reset node
					EEPROM_Clear();
					// reset the SSN
					printf("(DEBUG): Reseting Controller Now...\n");
					SoftReset();
					while(1);
					break;

				// Only for debugging, will be removed
				// This message will reset our SSN
				case DEBUG_RESET_SSN_MESSAGE_ID:
					// stop the global timer
					//stop_Global_Clock();
					DisableGlobalInterrupt();
					// reset the SSN
					printf("(DEBUG): Reseting Controller Now...\n");
					sleep_for_microseconds(1000000);
					SoftReset();
					while(1);
					break;

				default:
					break;
			}
		} else {
			int8_t this_index;
			// first six bytes are MAC address of destination, 7th byte contains message id
			received_message_id = message_to_recv[6];
			// based on which message was received (received_message_id), we extract and save the data
			switch (received_message_id) {
				/** Server to Node Messages */
				case SET_MAC_MESSAGE_ID:
				case SET_CONFIG_MESSAGE_ID:
				case SET_TIMEOFDAY_MESSAGE_ID:
					/** Search for the destination MAC address in routing dictionary */
					this_index = find_in_dictionary(&routing_dictionary, &message_to_recv[0]);
					if (this_index == No_Match_Found) {
						printf("(LOG) Can't Find %02X:%02X:%02X:%02X:%02X:%02X\n", message_to_recv[0], message_to_recv[1], message_to_recv[2], message_to_recv[3], message_to_recv[4], message_to_recv[5]);
					} else {
						printf("(LOG) Found %02X:%02X:%02X:%02X:%02X:%02X at IP Address: %d.%d.%d.%d\n", message_to_recv[0], message_to_recv[1], message_to_recv[2], message_to_recv[3], 
																		message_to_recv[4], message_to_recv[5],
																		routing_dictionary.ip_addresses[IP_LEN*this_index+0], routing_dictionary.ip_addresses[IP_LEN*this_index+1], 
																		routing_dictionary.ip_addresses[IP_LEN*this_index+2], routing_dictionary.ip_addresses[IP_LEN*this_index+3]);
					}
					SendMessage(SSN_Socket, &routing_dictionary.ip_addresses[IP_LEN*this_index], SSN_DEFAULT_PORT, message_to_recv, received_message_size);
					break;
				/** Node to Server Messages */
				case GET_MAC_MESSAGE_ID:
				case GET_CONFIG_MESSAGE_ID:
				case GET_TIMEOFDAY_MESSAGE_ID:
				case STATUS_UPDATE_MESSAGE_ID:
					/** Search for the destination MAC address in routing dictionary */
					this_index = find_in_dictionary(&routing_dictionary, &message_to_recv[0]);
					if (this_index == No_Match_Found) {
						printf("(DISCOVERY) New Node %02X:%02X:%02X:%02X:%02X:%02X Detected @ %d.%d.%d.%d\n", message_to_recv[0], message_to_recv[1], message_to_recv[2], message_to_recv[3], message_to_recv[4], 
																												message_to_recv[5], SENDER_IP[0], SENDER_IP[1], SENDER_IP[2], SENDER_IP[3]);
						/** Add a new MAC address against its IP in routing table */
						uint8_t current_count = routing_dictionary.count;
						// add MAC address as key
						uint8_t j; for (j=0;j<MAC_LEN;j++) {
							routing_dictionary.mac_addresses[current_count*MAC_LEN+j] = message_to_recv[j];
						}
						// add IP address as value
						uint8_t k; for (k=0;k<IP_LEN;k++) {
							routing_dictionary.ip_addresses[current_count*IP_LEN+k] = SENDER_IP[k];
						}
						// increment table entry count
						routing_dictionary.count += 1;
					}
					printf("(LOG) Sending Message from %d.%d.%d.%d to %d.%d.%d.%d\n", SENDER_IP[0], SENDER_IP[1], SENDER_IP[2], SENDER_IP[3], 
																						SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3]);
					SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_recv, received_message_size);                
					break;
				default:
					break;
			}
		}
		// See if there is another message in the buffer so we can do this all over again
		Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);
    }
    return 0;
}

uint8_t Receive_TimeOfDay(uint8_t SSN_Socket, uint8_t* SSN_MAC_ADDRESS, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {    
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t received_message_id, received_message_size;
    uint32_t TimeOFDayTick;
    // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
    Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);
    // if there are more than one messages in buffer, we want to receive all of them
    while (Received_Message_Bytes_in_Buffer) {
		//printf("Received\n");
		/* Clear the message array */
		clear_array(message_to_recv, max_recv_message_size);
        // read the message from buffer
        received_message_size = Recv_Message_Over_UDP(SSN_Socket, message_to_recv, max_recv_message_size, SENDER_IP, SENDER_PORT);
		/** check who is the destination of the message, if this node itself, then consume the message 
		 * otherwise, route it to the proper destination
		 */
		if (SSN_I_AM_DESTINATION(SSN_MAC_ADDRESS, &message_to_recv[0])) {
			// Parse and make sense of the message
			// 'params' array stores and organizes whatever data we have received in the message
			// this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
			received_message_id = decipher_received_message(message_to_recv, params);

			// based on which message was received (received_message_id), we extract and save the data
			switch (received_message_id) {
				case SET_TIMEOFDAY_MESSAGE_ID:
					TimeOFDayTick = get_uint32_from_bytes(params);
					printf("<- SET_TIMEOFDAY MESSAGE RECEIVED: %d\n", TimeOFDayTick);
					// assign incoming clock time to SSN Global Clock (Pseudo Clock because we don't have an RTCC)
					set_ssn_time(TimeOFDayTick);
					return 1;
					break;

				// Only for debugging, will be removed
				// This message will clear the EEPROM of our SSN
				case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
					// stop the global timer
					stop_Global_Clock();
					printf("(DEBUG): Clearing EEPROM Now...\n");
					// Clear EEPROM and reset node
					EEPROM_Clear();
					// reset the SSN
					printf("(DEBUG): Reseting Controller Now...\n");
					SoftReset();
					while(1);
					break;

				// Only for debugging, will be removed
				// This message will reset our SSN
				case DEBUG_RESET_SSN_MESSAGE_ID:
					// stop the global timer
					stop_Global_Clock();
					// reset the SSN
					printf("(DEBUG): Reseting Controller Now...\n");
					sleep_for_microseconds(1000000);
					SoftReset();
					while(1);
					break;

				default:
					break;
			}
		} else {
			int8_t this_index;
			// first six bytes are MAC address of destination, 7th byte contains message id
			received_message_id = message_to_recv[6];
			// based on which message was received (received_message_id), we extract and save the data
			switch (received_message_id) {
				/** Server to Node Messages */
				case SET_MAC_MESSAGE_ID:
				case SET_CONFIG_MESSAGE_ID:
				case SET_TIMEOFDAY_MESSAGE_ID:
					/** Search for the destination MAC address in routing dictionary */
					this_index = find_in_dictionary(&routing_dictionary, &message_to_recv[0]);
					if (this_index == No_Match_Found) {
						printf("(LOG) Can't Find %02X:%02X:%02X:%02X:%02X:%02X\n", message_to_recv[0], message_to_recv[1], message_to_recv[2], message_to_recv[3], message_to_recv[4], message_to_recv[5]);
					} else {
						printf("(LOG) Found %02X:%02X:%02X:%02X:%02X:%02X at IP Address: %d.%d.%d.%d\n", message_to_recv[0], message_to_recv[1], message_to_recv[2], message_to_recv[3], 
																		message_to_recv[4], message_to_recv[5],
																		routing_dictionary.ip_addresses[IP_LEN*this_index+0], routing_dictionary.ip_addresses[IP_LEN*this_index+1], 
																		routing_dictionary.ip_addresses[IP_LEN*this_index+2], routing_dictionary.ip_addresses[IP_LEN*this_index+3]);
					}
					SendMessage(SSN_Socket, &routing_dictionary.ip_addresses[IP_LEN*this_index], SSN_DEFAULT_PORT, message_to_recv, received_message_size);
					break;
				/** Node to Server Messages */
				case GET_MAC_MESSAGE_ID:
				case GET_CONFIG_MESSAGE_ID:
				case GET_TIMEOFDAY_MESSAGE_ID:
				case STATUS_UPDATE_MESSAGE_ID:
					/** Search for the destination MAC address in routing dictionary */
					this_index = find_in_dictionary(&routing_dictionary, &message_to_recv[0]);
					if (this_index == No_Match_Found) {
						printf("(DISCOVERY) New Node %02X:%02X:%02X:%02X:%02X:%02X Detected @ %d.%d.%d.%d\n", message_to_recv[0], message_to_recv[1], message_to_recv[2], message_to_recv[3], message_to_recv[4], 
																												message_to_recv[5], SENDER_IP[0], SENDER_IP[1], SENDER_IP[2], SENDER_IP[3]);
						/** Add a new MAC address against its IP in routing table */
						uint8_t current_count = routing_dictionary.count;
						// add MAC address as key
						uint8_t j; for (j=0;j<MAC_LEN;j++) {
							routing_dictionary.mac_addresses[current_count*MAC_LEN+j] = message_to_recv[j];
						}
						// add IP address as value
						uint8_t k; for (k=0;k<IP_LEN;k++) {
							routing_dictionary.ip_addresses[current_count*IP_LEN+k] = SENDER_IP[k];
						}
						// increment table entry count
						routing_dictionary.count += 1;
					}
					printf("(LOG) Sending Message from %d.%d.%d.%d to %d.%d.%d.%d\n", SENDER_IP[0], SENDER_IP[1], SENDER_IP[2], SENDER_IP[3], 
																						SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3]);
					SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_recv, received_message_size);                
					break;
				default:
					break;
			}
		}
        // See if there is another message in the buffer so we can do this all over again
        Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);            
    }
    return 0;
}


