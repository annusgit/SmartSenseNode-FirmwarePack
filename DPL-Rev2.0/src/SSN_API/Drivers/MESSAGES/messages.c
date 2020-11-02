
#include "messages.h"

uint8_t is_Valid_MAC(uint8_t* mac_address) {
        // just compare first four bytes of our MAC with the series of MAC addresses that we have for our SSN
        // First four bytes of our SSN MAC addresses will always be constant
        uint8_t i, SSN_VALID_MAC_BYTES[] = {0x70, 0xB3, 0xD5, 0xFE};
        for (i = 0; i < 4; i++)
                if (mac_address[i] != SSN_VALID_MAC_BYTES[i])
                        return 0;
        return 1;
}

uint8_t is_Valid_CONFIG(uint8_t* config_array) {
        uint8_t i, valid = EEPROM_CONFIG_SIZE;
        for (i = 0; i < EEPROM_CONFIG_SIZE; i++)
                if (config_array[i] == 0xFF) // 0xFF is EEPROM_CLEAR_VALUE 
                        valid--;
        return valid > 0 ? 1 : valid;
}

void clear_array(uint8_t* this_array, uint32_t this_size) {
        uint8_t i = 0;
        for (i = 0; i < this_size; i++) {
                this_array[i] = 0;
        }
}

uint8_t construct_get_mac_message(uint8_t* message_array, uint8_t* node_id) {
        uint8_t count = 0;

        /* Send the NODE ID */
        message_array[count++] = node_id[0];
        message_array[count++] = node_id[1];

        /* Send the MESSAGE ID */
        message_array[count++] = GET_MAC_MESSAGE_ID;

        // return how many bytes the message is
        return count;
}

uint8_t construct_get_configuration_message(uint8_t* message_array, uint8_t* node_id) {
        uint8_t count = 0;

        /* Send the NODE ID */
        message_array[count++] = node_id[0];
        message_array[count++] = node_id[1];

        /* Send the MESSAGE ID */
        message_array[count++] = GET_CONFIG_MESSAGE_ID;

        // return how many bytes the message is
        return count;
}

uint8_t construct_ack_configuration_message(uint8_t* message_array, uint8_t* node_id, uint8_t* received_configs) {
        uint8_t count = 0;

        /* Send the NODE ID */
        message_array[count++] = node_id[0];
        message_array[count++] = node_id[1];

        /* Send the MESSAGE ID */
        message_array[count++] = ACK_CONFIG_MESSAGE_ID;

        /* Attach the received configurations with this message */
        for (count; count < EEPROM_CONFIG_SIZE + 3; count++) {
                message_array[count] = received_configs[count - 3];
        }

        // return how many bytes the message is
        return count;
}

uint8_t construct_get_timeofday_message(uint8_t* message_array, uint8_t* node_id) {
        uint8_t count = 0;

        /* Send the NODE ID */
        message_array[count++] = node_id[0];
        message_array[count++] = node_id[1];

        /* Send the MESSAGE ID */
        message_array[count++] = GET_TIMEOFDAY_MESSAGE_ID;

        // return how many bytes the message is
        return count;
}

uint8_t construct_status_update_message(uint8_t* message_array, uint8_t* node_id, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, float* Machine_load_currents,
        uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, uint32_t* Machine_status_timestamp,
        uint32_t node_uptime_in_seconds, uint8_t abnormal_activity) {

        uint8_t count = 0;
        uint8_t temp_array[4];

        /* Send the NODE ID */
        message_array[count++] = node_id[0];
        message_array[count++] = node_id[1];
        message_array[count++] = node_id[2];
        message_array[count++] = node_id[3];
        message_array[count++] = node_id[4];
        message_array[count++] = node_id[5];

        /* Send the MESSAGE ID */
        message_array[count++] = STATUS_UPDATE_MESSAGE_ID;

        /* Send the temperature in two bytes */
        message_array[count++] = temperature_bytes[0];
        message_array[count++] = temperature_bytes[1];

        /* Send the humidity in two bytes */
        message_array[count++] = relative_humidity_bytes[0];
        message_array[count++] = relative_humidity_bytes[1];

        /* Send the status change flags */
        message_array[count++] = Machine_status_flag; // a single bytes whose bit values indicate which machines had their states changed

        ////////////////////// M1 //////////////////////////
        /* Send the load current, percentage load and on/off state of current sensor-i */
        get_bytes_from_uint16((uint16_t) (Machine_load_currents[0]*100), temp_array); // multiply the current with 100 before reporting, decipher on server side
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = Machine_load_percentages[0];
        message_array[count++] = (Machine_status[0] % 3);
        /* Send the time since on/off of this machine on current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_timestamp[0], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];
        /* Send the time duration for how long this machine has been in current state @ current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_duration[0], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];

        ////////////////////// M2 //////////////////////////
        /* Send the load current, percentage load and on/off state of current sensor-i */
        get_bytes_from_uint16((uint16_t) (Machine_load_currents[1]*100), temp_array); // multiply the current with 100 before reporting, decipher on server side
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = Machine_load_percentages[1];
        message_array[count++] = (Machine_status[1] % 3);
        /* Send the time since on/off of this machine on current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_timestamp[1], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];
        /* Send the time duration for how long this machine has been in current state @ current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_duration[1], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];

        ////////////////////// M3 //////////////////////////
        /* Send the load current, percentage load and on/off state of current sensor-i */
        get_bytes_from_uint16((uint16_t) (Machine_load_currents[2]*100), temp_array); // multiply the current with 100 before reporting, decipher on server side
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = Machine_load_percentages[2];
        message_array[count++] = (Machine_status[2] % 3);
        /* Send the time since on/off of this machine on current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_timestamp[2], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];
        /* Send the time duration for how long this machine has been in current state @ current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_duration[2], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];

        ////////////////////// M4 //////////////////////////
        /* Send the load current, percentage load and on/off state of current sensor-i */
        get_bytes_from_uint16((uint16_t) (Machine_load_currents[3]*100), temp_array); // multiply the current with 100 before reporting, decipher on server side
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = Machine_load_percentages[3];
        message_array[count++] = (Machine_status[3] % 3);
        /* Send the time since on/off of this machine on current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_timestamp[3], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];
        /* Send the time duration for how long this machine has been in current state @ current sensor-i (4 bytes) */
        get_bytes_from_uint32(Machine_status_duration[3], temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];

        /* Send the time for how long the node has been on */
        get_bytes_from_uint32(node_uptime_in_seconds, temp_array);
        message_array[count++] = temp_array[0];
        message_array[count++] = temp_array[1];
        message_array[count++] = temp_array[2];
        message_array[count++] = temp_array[3];

        /* Send abnormal activity status */
        message_array[count++] = abnormal_activity;

        // return how many bytes the message is
        return count;
}

uint8_t decipher_received_message(uint8_t* message, uint8_t* params) {
        // first byte will always be the message ID
        uint8_t message_id = message[0], i;
        switch (message_id) {
                        // Set MAC received?
                case SET_MAC_MESSAGE_ID:
                        for (i = 0; i < EEPROM_MAC_SIZE; i++)
                                params[i] = message[i + 1];
                        break;

                        // Set CONFIG received?
                case SET_CONFIG_MESSAGE_ID:
                        for (i = 0; i < EEPROM_CONFIG_SIZE; i++)
                                params[i] = message[i + 1];
                        break;

                        // Set Time of Day received?
                case SET_TIMEOFDAY_MESSAGE_ID:
                        for (i = 0; i < TIME_Of_DAY_SIZE; i++)
                                params[i] = message[i + 1];
                        break;

                        /* Only for debugging, will be removed */
                        // Clear EEPROM received?
                case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
                        // Do nothing, message ID will do
                        break;

                        /* Only for debugging, will be removed */
                        // Reset Controller received?
                case DEBUG_RESET_SSN_MESSAGE_ID:
                        // Do nothing, message ID will do
                        break;

                default:
                        break;
        }
        return message_id;
}







