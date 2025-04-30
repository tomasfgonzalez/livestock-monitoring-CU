/*
 * rylr998.c
 *
 *  Created on: Mar 14, 2025
 *      Author: Tomas Francisco Gonzalez
 */

#include "rylr998.h"
#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private variables ----------------------------------------------------- */
static const char *RYLR_TAG = "RYLR"; 

static char txBuffer[TX_BUFFER_SIZE];

RYLR_RX_data_t rx_packet;

UartPort_t configPort = UART_PORT_MAIN;

/* Private functions ----------------------------------------------------- */
void Error_Handler(void) {
    while (1) {
        // Do nothing
    }
}

//------------------------------
// 		 RX PROCESS
//------------------------------

const RYLR_CommandEntry commandTable[] = {
	{"+OK", RYLR_OK},
	{"+RCV", RYLR_RCV},
	{"+ERR", RYLR_ERR},
	{"ACK", RYLR_RCV_ACK},
	{NULL, RYLR_NOT_FOUND} // Sentinel value
};

RYLR_RX_command_t rylr998_ResponseFind(const char *rxBuffer) {
	for (int i = 0; commandTable[i].prefix != NULL; i++) {
		if (strncmp(rxBuffer, commandTable[i].prefix, strlen(commandTable[i].prefix)) == 0) {
			return commandTable[i].command;
		}
	}
	return RYLR_NOT_FOUND;
}

RYLR_RX_command_t rylr998_parse_received(uint8_t *pBuff, uint8_t pBuff_size) {
	static char aux_buff[64];  // Should match with RX_BUFFER_SIZE
	static uint8_t start_indx = 0;

	uint8_t i = 0;
	// Find the '+' start character
	while (i < pBuff_size && pBuff[(start_indx + i) % pBuff_size] != '+') {
		i++;
	}
	start_indx = (start_indx + i) % pBuff_size;

	// Copy the command into aux_buff
	for (i = 0; i < pBuff_size; i++) {
		aux_buff[i] = pBuff[(start_indx + i) % pBuff_size];
		if (aux_buff[i] == '\n') break;
	}
	aux_buff[i + 1] = '\0'; // Ensure null termination

	rylr998_SetInterruptFlag(0, configPort);
	start_indx = (start_indx + i + 1) % pBuff_size;

	volatile  RYLR_RX_command_t cmd = rylr998_ResponseFind(aux_buff);   //se define como volatil para que no sea optimizada
	if (cmd == RYLR_OK) {
		// Do nothing
	} else if (cmd == RYLR_RCV) {
		char *ptr = aux_buff;
		memset(rx_packet.data, 0, sizeof(rx_packet.data));  // Initialize data as empty string

		// Skip past "+RCV="
		while (*ptr && *ptr != '=') ptr++;
		if (*ptr) ptr++; // Skip '='

		// Parse ID (first number after '=')
		rx_packet.id = 0;
		while (*ptr >= '0' && *ptr <= '9') {
				rx_packet.id = rx_packet.id * 10 + (*ptr - '0');
				ptr++;
		}
		if (*ptr != ',') return cmd = RYLR_RCV_ERR; // Invalid format
		ptr++; // Skip ','

		// Parse byte count
		rx_packet.byte_count = 0;
		while (*ptr >= '0' && *ptr <= '9') {
				rx_packet.byte_count = rx_packet.byte_count * 10 + (*ptr - '0');
				ptr++;
		}
		if (*ptr != ',') return cmd = RYLR_RCV_ERR; // Invalid format
		ptr++; // Skip ','

		// Parse data (copy up to byte_count or buffer size)
		size_t i = 0;
		size_t max_copy = (rx_packet.byte_count < sizeof(rx_packet.data)) ?
											rx_packet.byte_count : sizeof(rx_packet.data) - 1;
		while (i < max_copy && *ptr && *ptr != ',') {
			rx_packet.data[i++] = *ptr++;
		}
		rx_packet.data[i] = '\0';

		// Skip remaining data if needed
		while (*ptr && *ptr != ',') ptr++;
		if (*ptr != ',') return cmd = RYLR_RCV_ERR; // Invalid format
		ptr++; // Skip ','
		ptr++; // Skip '-'

		// Parse RSSI
		rx_packet.rssi = 0;
		while (*ptr >= '0' && *ptr <= '9') {
			rx_packet.rssi = rx_packet.rssi * 10 + (*ptr - '0');
			ptr++;
		}

		if (*ptr != ',') return cmd = RYLR_RCV_ERR; // Invalid format
		ptr++; // Skip ','

		// Parse SNR
		rx_packet.snr = 0;
		while (*ptr >= '0' && *ptr <= '9') {
			rx_packet.snr = rx_packet.snr * 10 + (*ptr - '0');
			ptr++;
		}

		//------------------------------
		// 		 PROCESS RECEIVED DATA:
		//------------------------------
		if(rylr998_ResponseFind(rx_packet.data)==RYLR_RCV_ACK){
			cmd = RYLR_RCV_ACK;
		}

		//TODO proccess received SYNC DATA

	} else if (cmd == RYLR_ERR) {
		while (1) { Error_Handler(); } // Handle error
	}
	return cmd;
}

//----------------------------------
// 		 IRQ FLAG -> LOOK UART FILE
//----------------------------------

volatile uint8_t rylr998_interrupt_flag[2];

void rylr998_SetInterruptFlag(uint8_t val, UartPort_t port){
	rylr998_interrupt_flag[port] = val;
}

uint8_t rylr998_GetInterruptFlag(UartPort_t port){
	return rylr998_interrupt_flag[port];
}

//------------------------------
// 		Config Commands
//------------------------------
static void rylr998_setAddress(const uint8_t address) {
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, sizeof(txBuffer), AT "ADDRESS=%d" END, address);
	rylr998_sendCommand(txBuffer, configPort);
}

static void rylr998_networkId(const uint8_t networkId){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, TX_BUFFER_SIZE, AT "NETWORKID=%u" END, networkId);
	rylr998_sendCommand(txBuffer, configPort);
}

static void rylr998_setParameter(const uint8_t SF,const uint8_t BW,const uint8_t CR,const uint8_t ProgramedPreamble){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, TX_BUFFER_SIZE, AT "PARAMETER=%u,%u,%u,%u" END, SF, BW, CR, ProgramedPreamble);
	rylr998_sendCommand(txBuffer, configPort);
}
/*
void rylr998_reset(void){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, TX_BUFFER_SIZE, AT "RESET" END);
	rylr998_sendCommand(txBuffer, configPort);
}*/

static void rylr998_mode(const uint8_t mode,const uint32_t rxTime,const uint32_t LowSpeedTime){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	if (rxTime==0||LowSpeedTime==0){snprintf(txBuffer, TX_BUFFER_SIZE, AT"MODE=%u" END, mode);
	}else{snprintf(txBuffer, TX_BUFFER_SIZE, AT"MODE=2,%lu,%lu" END,rxTime,LowSpeedTime);}
	rylr998_sendCommand(txBuffer, configPort);
}

/*
void rylr998_setBaudRate(uint32_t baudRate){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, TX_BUFFER_SIZE,  AT "IPR=%lu" END, baudRate);
	rylr998_sendCommand(txBuffer, configPort);
}*/

static void rylr998_setBand(const uint32_t frequency,const uint8_t memory){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	if(memory){snprintf(txBuffer, TX_BUFFER_SIZE, AT"BAND=%lu,M"END,frequency);
	}else{snprintf(txBuffer, TX_BUFFER_SIZE, AT"BAND=%lu,M"END,frequency);}
	rylr998_sendCommand(txBuffer, configPort);
}

static void rylr998_setCRFOP(const uint8_t CRFOP){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, TX_BUFFER_SIZE,  AT"CRFOP=%u"END, CRFOP);
	rylr998_sendCommand(txBuffer, configPort);
}

/*void rylr998_FACTORY(void){
	memset(txBuffer, 0, sizeof(TX_BUFFER_SIZE));
	snprintf(txBuffer, TX_BUFFER_SIZE,  AT"FACTORY"END);
	rylr998_sendCommand(txBuffer, configPort);
}*/

//------------------------------
// 		 CONFIG ALL SETTINGS
//------------------------------
void rylr998_config(const RYLR_config_t *config_handler, UartPort_t port){
	configPort = port;
	//rylr998_FACTORY();
	//rylr998_getCommand(RYLR_FACTORY, configPort);
	//NETWORKID
	
	rylr998_networkId(config_handler->networkId);
	rylr998_getCommand(RYLR_OK, configPort);
	ESP_LOGI(RYLR_TAG, "Network ID set to %d\n", config_handler->networkId);
	//ADDRESS
	rylr998_setAddress(config_handler->address);
	rylr998_getCommand(RYLR_OK, configPort);
	ESP_LOGI(RYLR_TAG, "Address set to %d\n", config_handler->address);
	//PARAMETERS
	rylr998_setParameter(config_handler->SF, config_handler->BW, config_handler->CR, config_handler->ProgramedPreamble);
	rylr998_getCommand(RYLR_OK, configPort);
	ESP_LOGI(RYLR_TAG, "Parameters set\n");
	//MODE
	rylr998_mode(config_handler->mode,config_handler->rxTime,config_handler->LowSpeedTime);
	rylr998_getCommand(RYLR_OK, configPort);
	ESP_LOGI(RYLR_TAG, "Mode set to %d\n", config_handler->mode);
	//BaudRate
	//rylr998_setBaudRate(config_handler->baudRate);
	//rylr998_getCommand(RYLR_IPR); //ADD RYLR_IPR
	//FREQ Band
	rylr998_setBand(config_handler->frequency,config_handler->memory);
	rylr998_getCommand(RYLR_OK, configPort);
	//PASSWORD
	//rylr998_setCPIN(config_handler->password);
	//rylr998_getCommand(RYLR_OK);
	//RF Output
	rylr998_setCRFOP(config_handler->CRFOP);
	rylr998_getCommand(RYLR_OK, configPort);
	ESP_LOGI(RYLR_TAG, "CRFOP set to %d\n", config_handler->CRFOP);
}

//------------------------------
// 		 CHANNELS
//------------------------------
void rylr998_setChannel(uint8_t ch, uint8_t address, UartPort_t port) {
	RYLR_config_t config_handler;
	if (ch) {
		/* MAIN CHANNEL ------------------*/
		config_handler.networkId =18;
		config_handler.address =address;
		config_handler.SF=9;
		config_handler.BW=7;
		config_handler.CR=1;
		config_handler.ProgramedPreamble=12;
		config_handler.mode=0;
		config_handler.rxTime=0;
		config_handler.LowSpeedTime=0;
		//config_handler.baudRate=115200;
		config_handler.frequency=915000000;
		config_handler.memory=1;
		//strcpy(config_handler.password, "FFFFFFFF"); //we dont want the \0 terminator so we overflow, estan comentados para ver los msj
		config_handler.CRFOP=22;
	} else {
		/* AUX CHANNEL ------------------*/
		config_handler.networkId =18;
		config_handler.address =address;
		config_handler.SF=9;
		config_handler.BW=7;
		config_handler.CR=1;
		config_handler.ProgramedPreamble=12;
		config_handler.mode=0;
		config_handler.rxTime=0;
		config_handler.LowSpeedTime=0;
		//config_handler.baudRate=115200;
		config_handler.frequency=925000000;
		config_handler.memory=1;
		//strcpy(config_handler.password, "FFFFFFFF"); //we dont want the \0 terminator so we overflow, esta comentado para no tener que config password en ambos dispositivos
		config_handler.CRFOP=22;
	}
	rylr998_config(&config_handler, port);
}

//-----------------------------------------------
// 			SEND AND GET DATA FROM UART1
//------------------------------------------------
void rylr998_sendCommand(const char *cmd, UartPort_t port) {
	uint16_t len = strlen(cmd);
	uart_send(cmd, len, port);
}

RYLR_RX_data_t* rylr998_getCommand(RYLR_RX_command_t cmd, UartPort_t port){
	uint8_t* rx_buff = uart_get_rx_buff(port);

	vTaskDelay(pdMS_TO_TICKS(30));  //Sin un retardo, la bandera no llega a ponerse en 1, Esta parte del codigo resulta delicada
	while(!rylr998_interrupt_flag[port]);

	RYLR_RX_command_t received_cmd = rylr998_parse_received(rx_buff, UART_RX_BUFF_SIZE);
	if (received_cmd != cmd) { 
		ESP_LOGE(RYLR_TAG, "Wrong command. Expected %d, got %d", cmd, received_cmd);
		ESP_LOGE(RYLR_TAG, "RX buffer: %s", rx_buff);
		ESP_LOGE(RYLR_TAG, "Read at port %d", port);
	}

	return &rx_packet;
}
