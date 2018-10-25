/* DHT11 temperature sensor library
   Usage:
   		Set DHT PIN using  setDHTPin(pin) command
   		getFtemp(); this returns temperature in F
*/
#ifndef DHT11_H_INCLUDED
#define DHT11_H_INCLUDED

#define DHT_TIMEOUT_ERROR -2
#define DHT_CHECKSUM_ERROR -1
#define DHT_OKAY  0
#define DHT_START_BIT_0	( 1 << 0 )
// function prototypes

//Start by using this function
void setDHTPin(int PIN);
//Do not need to touch these three
void sendStart();
void errorHandle(int response);

//To get all 3 measurements in an array use
int getData();
//call each function for live temperature updates
//if you only need one measurements use these functions
int getTemp();
int getHumidity();

/* Main Task */
void dht_task(void *pvParameter);

#endif
