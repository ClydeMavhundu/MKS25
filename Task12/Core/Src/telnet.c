/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */


#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

#define TELNET_THREAD_PRIO  (tskIDLE_PRIORITY + 4)
#define CMD_BUFFER_LEN 128 // Buffer size for command strings

// Function prototypes
static void telnet_thread(void *arg);
static void telnet_byte_available(uint8_t c, struct netconn *conn);
static void telnet_process_command(char *cmd, struct netconn *conn);

static void http_client(char *s, uint16_t size)
{
	struct netconn *client;
	struct netbuf *buf;
	ip_addr_t ip;
	uint16_t len = 0;
	IP_ADDR4(&ip, 147,229,144,124);
	const char *request = "GET /ip.php HTTP/1.1\r\n"
			"Host: www.urel.feec.vutbr.cz\r\n"
			"Connection: close\r\n"
			"\r\n\r\n";
	client = netconn_new(NETCONN_TCP);
	if (netconn_connect(client, &ip, 80) == ERR_OK) {
		netconn_write(client, request, strlen(request), NETCONN_COPY);
		// Receive the HTTP response
		s[0] = 0;
		while (len < size && netconn_recv(client, &buf) == ERR_OK) {
			len += netbuf_copy(buf, &s[len], size-len);
			s[len] = 0;
			netbuf_delete(buf);
		}
	} else {
		sprintf(s, "Connection error\n");
	}
	netconn_delete(client);
}



/*-----------------------------------------------------------------------------------*/
static void telnet_byte_available(uint8_t c, struct netconn *conn) {
	static uint16_t cnt = 0;
	static char data[CMD_BUFFER_LEN];

	if (cnt < CMD_BUFFER_LEN && c >= 32 && c <= 127) {
		data[cnt++] = c; // Append valid characters to the buffer
	}

	if (c == '\n' || c == '\r') {
		data[cnt] = '\0'; // Null-terminate the command
		if (strlen(data)<1) {
			cnt = 0; // Reset the buffer counter
		}else
		{telnet_process_command(data, conn); // Process the command
		cnt = 0;} // Reset the buffer counter

	}
}

/*-----------------------------------------------------------------------------------*/
static void telnet_process_command(char *cmd, struct netconn *conn) {
	char response[128]; // Buffer for responses

	// Command handling logic
	if (strcmp(cmd, "Hello") == 0) {
		sprintf(response, "Hello, Telnet user!\n");
	} else if (strcmp(cmd, "LED1 ON") == 0) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET); // Turn LED1 ON
		sprintf(response, "LED1 is ON\n");
	} else if (strcmp(cmd, "LED1 OFF") == 0) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET); // Turn LED1 OFF
		sprintf(response, "LED1 is OFF\n");

	} else if (strcmp(cmd, "LED2 ON") == 0) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET); // Turn LED2 ON
		sprintf(response, "LED2 is ON\n");
	} else if (strcmp(cmd, "LED2 OFF") == 0) {
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET); // Turn LED2 OFF
		sprintf(response, "LED2 is OFF\n");

	} else if (strcmp(cmd, "LED3 ON") == 0) {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET); // Turn LED3 ON
		sprintf(response, "LED3 is ON\n");
	} else if (strcmp(cmd, "LED3 OFF") == 0) {
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET); // Turn LED3 OFF
		sprintf(response, "LED3 is OFF\n");


	} else if (strcmp(cmd, "STATUS1") == 0) {
		sprintf(response, "STATUS: LED1=%s\n", HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin) ? "ON" : "OFF");
	} else if (strcmp(cmd, "STATUS2") == 0) {
		sprintf(response, "STATUS: LED2=%s\n", HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) ? "ON" : "OFF");
	} else if (strcmp(cmd, "STATUS3") == 0) {
		sprintf(response, "STATUS: LED3=%s\n", HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? "ON" : "OFF");
	} else if (strcmp(cmd, "STATUS_ALL") == 0) {
		// Combined status for all LEDs
		sprintf(response, "STATUS: LED1=%s, LED2=%s, LED3=%s\n",
				HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin) ? "ON" : "OFF",
						HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) ? "ON" : "OFF",
								HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? "ON" : "OFF");


	}else if (strcmp(cmd, "Client") == 0){

		char buff[1024]; // Buffer for responses
		http_client(buff, 1024);
		//sprintf(response, "Buff: %s\n", buff);

		netconn_write(conn, buff, strlen(buff), NETCONN_COPY);






	} else {
		sprintf(response, "Unknown command: %s\n", cmd);
	}

	// Send response to the Telnet client
	netconn_write(conn, response, strlen(response), NETCONN_COPY);
}

/*-----------------------------------------------------------------------------------*/
static void telnet_thread(void *arg) {
	struct netconn *conn, *newconn;
	err_t err, accept_err;
	struct netbuf *buf;
	uint8_t *data;
	uint16_t len;

	LWIP_UNUSED_ARG(arg);

	// Create a new connection identifier
	conn = netconn_new(NETCONN_TCP);
	if (conn != NULL) {
		// Bind the connection to port 23 (Telnet port)
		err = netconn_bind(conn, NULL, 23);

		if (err == ERR_OK) {
			// Put the connection in listening mode
			netconn_listen(conn);

			while (1) {
				// Accept a new connection
				accept_err = netconn_accept(conn, &newconn);
				if (accept_err == ERR_OK) {
					// Process incoming data
					while (netconn_recv(newconn, &buf) == ERR_OK) {
						netbuf_data(buf, (void **)&data, &len);
						while (len--) {
							telnet_byte_available(*data++, newconn);
						}
						netbuf_delete(buf);
					}

					// Close and delete the connection
					netconn_close(newconn);
					netconn_delete(newconn);
				}
			}
		} else {
			netconn_delete(conn);
		}
	}
}

/*-----------------------------------------------------------------------------------*/
void telnet_init(void) {
	sys_thread_new("telnet_thread", telnet_thread, NULL, DEFAULT_THREAD_STACKSIZE, TELNET_THREAD_PRIO);

}

/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
