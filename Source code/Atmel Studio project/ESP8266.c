/*	
	ESP8266 library made for UART (tested on ATmega328P)
	Made by Ewlbo https://github.com/Ewlbo/ESP8266-Firebase-AVR-ThingSpeak
	Date 29.10.2017
*/

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "ESP8266.h"

#define _OK "OK"
#define _CONNECT "CONNECT"
#define _CLOSED "CLOSED"
#define _COMMAND "COMMAND"

// Define tasks (has to be the in the same order as in Firebase)
// Keep the value of tasks simple e.g. "on", "true" etc.
#define _taskName "task"

char ssid[] = "Pretty Fly for a Wi-Fi";
char psk[] = "canttouchthis";					// If no password leave it like this = "";
char POST_api_key[] = "PMGUCG3B0QCLZ4CA";		// Api key from ThingSpeak ThingHTTP POST request
char GET_api_key[] = "2V8KK12EJ9FT11DW";		// Api key from ThingSpeak ThingHTTP GET request
char api_key_twitter[] = "M76RHV38FM1IM8WU";	// Api key from ThingSpeak ThingTweet app (optional)
int port = 80;									// Port for TCP connection
char buffer [128];								// Placeholder for handling data
int dataCount = 0;
// HTTP request array/HTML array placeholder (increase if more data will be sent - MAX:2048)
char Hypertext[256];

void ESPinit(void)
{
	// Reset ESP8266
	printf("AT+RST\r\n");
	_delay_ms(100);
	ping();					// Check if ready after reset
	
	// Connect to Wi-Fi
	printf("AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,psk);
	waitFor(_OK);
	getIP();				// After connecting, get IP of ESP8266
	
	// Client mode
	printf("AT+CWMODE=1\r\n");
	waitFor(_OK);
	
	// Enable multiple connections
	printf("AT+CIPMUX=1\r\n");
	waitFor(_OK);
	
	_delay_ms(500);
}

void reset(void)
{
	ping();
	memset(buffer,0,strlen(buffer));
	memset(Hypertext,0,strlen(Hypertext));
	ESPinit();
}

void waitFor(char* response)
{
	memset(buffer,0,strlen(buffer));		// Reset array
	scanf("\n%s", buffer);					// Get message from ESP8266
	if (strstr(buffer,response) == NULL)	// Finding specific ESP8266 response (returns NULL if not found)
	{
		waitFor(response);
	}
}

void addData(char* name, int data)
{
	if (dataCount == 0)
	{
		// Clear array
		memset(Hypertext,0,strlen(Hypertext));
		// HTTP GET request base
		strcat(Hypertext,"GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=");
		// Add api_key to HTTP request array
		strcat(Hypertext,POST_api_key);
	}
	// Add the data to be sent
	strcat(Hypertext,"&");
	strcat(Hypertext,name);
	strcat(Hypertext,"=");
	memset(buffer,0,strlen(buffer));
	itoa(data,buffer,10);				// Transform integer to string
	strcat(Hypertext,buffer);
	dataCount++;
}

void addString(char* name, char* string)
{
	if (dataCount == 0)
	{
		// Clear array
		memset(Hypertext,0,strlen(Hypertext));
		// HTTP GET request base
		strcat(Hypertext,"GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=");
		// Add api_key to HTTP request array
		strcat(Hypertext,POST_api_key);
	}
	// Add the data to be sent
	strcat(Hypertext,"&");
	strcat(Hypertext,name);
	strcat(Hypertext,"=");
	strcat(Hypertext,string);
	dataCount++;
}

void pushData(void)
{
	// Check if ESP8266 is ready to send
	ping();
	
	// Start TCP connection with ThingSpeak on port 80 channel 0 (ESP8266 can handle 4 channels)
	printf("AT+CIPSTART=0,\"TCP\",\"api.thingspeak.com\",%d\r\n",port);
	waitFor(_OK);
	
	// Use channel 0 and define length of message
	printf("AT+CIPSEND=0,%d\r\n",strlen(Hypertext)+34);	// 34 is the length of "HTTP/1.1 Host: api.thingspeak.com" added later on
	waitFor(_OK);
	
	// Send HTTP request with data
	printf("%s\nHTTP/1.1 Host: api.thingspeak.com\r\n",Hypertext);

	// Wait for connection to get closed
	waitFor(_CLOSED);
	printf("Data sent\n");
	
	// Reset HTTP array and close connection to be sure
	printf("AT+CIPCLOSE=0\r\n");
	memset(Hypertext,0,strlen(Hypertext));
	dataCount = 0;
}

void getData(void)
{
	// Check if ESP8266 is ready to send
	ping();
	
	// Reset array
	memset(Hypertext,0,strlen(Hypertext));
	// HTTP GET request base
	strcat(Hypertext,"GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=");
	// Add api_key to HTTP request array
	strcat(Hypertext,GET_api_key);
	
	// Start TCP connection with ThingSpeak on port 80 channel 0 (ESP8266 can handle 4 channels)
	printf("AT+CIPSTART=1,\"TCP\",\"api.thingspeak.com\",%d\r\n",port);
	waitFor(_OK);
	
	// Use channel 0 and define length of message
	printf("AT+CIPSEND=1,%d\r\n",strlen(Hypertext)+34);	// 34 is the length of "http/1.1 Host: api.thingspeak.com" added later on
	waitFor(_OK);
	
	// Send HTTP request with data
	printf("%s\nHTTP/1.1 Host: api.thingspeak.com\r\n",Hypertext);
	
	// Check the task_list in Firebase
	checkTask(_taskName);					// Check task
	printf("Task: \"%s\"\n",_taskStatus);
}

void checkTask(char* taskName)
{
	int position = 0;
	waitFor(taskName);
	// Find where JSON starts
	while(buffer[position] != 123)
	{
		position++;
	}
	
	position = position + 2 + strlen(taskName) + 3;
	memset(_taskStatus,0,strlen(_taskStatus));
	int pos = 0;
	while(buffer[position] != 34)	// Until it reaches quotation mark
	{
		_taskStatus[pos] = buffer[position];
		pos++;
		position++;
	}
}

void ping(void)
{
	// Check if ESP8266 is responding to AT with OK
	
	_delay_ms(100);							// Default interval: 100ms
	printf("AT\r\n");
	memset(buffer,0,strlen(buffer));		// Reset array
	scanf("\n%s", buffer);					// Get message from ESP8266
	memset(buffer,0,strlen(buffer));		// Reset array
	scanf("\n%s", buffer);					// Get message from ESP8266
	if (strstr(buffer,"OK") == NULL)		// Finding specific response (returns NULL if not found)
	{
		ping();
	}
}

void getIP(void)
{
	memset(ip,0,strlen(ip));
	memset(buffer,0,strlen(buffer));
	printf("AT+CIFSR\r\n");
	scanf("\n%s", buffer);
	memset(buffer,0,strlen(buffer));
	scanf("\n%s", buffer);
	int x = 0;
	for (int i = 0; i <= 64;i++)
	{
		if (buffer[i] == 34)	// Check for quotation mark
		{
			i++;
			while(buffer[i] != 34)
			{
				ip[x] = buffer[i];
				i++;
				x++;
			}
		}
	}
	waitFor(_OK);
}

void addTweet(char* msg)
{
	if (dataCount == 0)
	{
		// Reset HTTP array
		memset(Hypertext,0,strlen(Hypertext));
		// HTTP GET request base
		strcat(Hypertext,"GET https://api.thingspeak.com/apps/thingtweet/1/statuses/update?api_key=");
		// Add api_key_twitter to HTTP request array
		strcat(Hypertext,api_key_twitter);
		// Add the message to send
		strcat(Hypertext,"&status=");
	}
	strcat(Hypertext,msg);
	dataCount++;
}

void tweet(char* message)
{
	// Note - Twitter wont allow to tweet message with the same text
	
	// Check if ESP8266 is ready to send
	ping();
	
	// Start TCP connection with ThingSpeak on port 80 channel 1 (ESP8266 can handle 4 channels)
	printf("AT+CIPSTART=1,\"TCP\",\"api.thingspeak.com\",%d\r\n",port);
	waitFor(_OK);
	
	// Use channel 1 and define length of message
	printf("AT+CIPSEND=1,%d\r\n",strlen(Hypertext)+34);	// 34 is the length of "HTTP/1.1 Host: api.thingspeak.com" added later on
	waitFor(_OK);
	
	// Send HTTP request with data
	printf("%s\nHTTP/1.1 Host: api.thingspeak.com\r\n",Hypertext);

	// Wait for connection to get closed
	waitFor(_CLOSED);
	printf("Tweet sent\n");
	
	// Close connection to be sure
	printf("AT+CIPCLOSE=1\r\n");
	dataCount = 0;
}
