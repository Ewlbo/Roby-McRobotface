#ifndef ESP8266_H_
#define ESP8266_H_

// Placeholder for ESP8266 IP on network
char ip[32];		

// Placeholder for task name
char _taskStatus[32];

// Function prototypes
void ESPinit(void);
void reset(void);
void waitFor(char* response);
void addData(char* name, int data);
void addString(char* name, char* string);
void pushData(void);
void getData(void);
void checkTask(char* taskName);
void ping(void);
void getIP(void);
void makePage(int id);
void addTweet(char* msg);
void tweet();

#endif /* ESP8266_H_ */
