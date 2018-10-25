#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "cJSON.h"

#include "wifi_manager.h"
#include "pins.h"
#include "interruptions.h"
#include "isr_tasks.h"

extern SemaphoreHandle_t ResetHW_Semaphore, xSemaphore;
extern xQueueHandle gpio_evt_queue, xReceive_Info;

esp_timer_handle_t periodic_timer;
esp_timer_handle_t oneshot_timer;
int cnt = 0, id_p1 = 0, id_p2 = 0, id_p3 = 0, id_p4 = 0;
bool id_c1 = 0, id_c2 = 0, id_c3 = 0, id_c4 = 0;

void periodic_timer_callback(void* arg)
{
		cnt++;
		if(id_c1!=0)
		{
			if(cnt == id_p1){
				gpio_set_level(CAC1, 1);}
		}
		if(id_c2!=0)
		{
			if(cnt == id_p2){
				gpio_set_level(CAC2, 1);}
		}
		if(id_c3!=0)
		{
			if(cnt == id_p3){
				gpio_set_level(CAC3, 1);}
		}
		if(id_c4!=0)
		{
			if(cnt == id_p4){
				gpio_set_level(CAC4, 1);}
		}
}

void oneshot_timer_callback(void* arg)
{
    esp_timer_handle_t periodic_timer_handle = (esp_timer_handle_t) arg;
    /* To start the timer which is running, need to stop it first */
    esp_timer_stop(periodic_timer_handle);
		cnt = 0;
}

void IRAM_ATTR detector(void *arg) {
	//da el semaforo para que quede libre para la terea pulsador
	// xSemaphoreGiveFromISR(xSemaphore, NULL);
	gpio_set_level(CAC1, 0);
	gpio_set_level(CAC2, 0);
	gpio_set_level(CAC3, 0);
	gpio_set_level(CAC4, 0);
	esp_timer_start_periodic(periodic_timer, 1500);
	esp_timer_start_once(oneshot_timer, 7000);
	if(id_c2!=0)
	{
		if(id_p2 == 0){
			gpio_set_level(CAC2, 1);}
	}
	if(id_c3!=0)
	{
		if(id_p3 == 0){
			gpio_set_level(CAC3, 1);}
	}
	if(id_c4!=0)
	{
		if(id_p4 == 0){
			gpio_set_level(CAC4, 1);}
	}
	if(id_c1!=0)
	{
		if(id_p1 == 0){
			gpio_set_level(CAC1, 1);}
		}
}

void IRAM_ATTR gpio_wifi_rst(void *arg) {
	//da el semaforo para que quede libre para la terea pulsador
	xSemaphoreGiveFromISR(ResetHW_Semaphore, NULL);
}

void IRAM_ATTR gpio_states_sensors(void* arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendToBackFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void ControlLoad_Task(void *arg){
	char Loads_json[2048];
	cJSON *Load=NULL, *id=NULL, *Loads=NULL;

	while(1){
			xQueuePeek( xReceive_Info, &Loads_json, portMAX_DELAY );
			Loads = cJSON_Parse(Loads_json);
			cJSON_ArrayForEach(Load, Loads)
			{
					id = cJSON_GetObjectItemCaseSensitive(Load, "id");
					switch (id->valueint) {
						case 2: //6:
								if(strcmp(id->next->valuestring,"on")==0){
										switch (id->next->next->valueint) {
											case 100:
													if(id_c1 != 1 || id_p1 != 0){
														id_c1=1;
														id_p1=0;}
											break;
											case 80:
													if(id_c1 != 1 || id_p1 != 1){
														id_c1=1;
														id_p1=1;}
											break;
											case 60:
													if(id_c1 != 1 || id_p1 != 2){
														id_c1=1;
														id_p1=2;}
											break;
											case 40:
													if(id_c1 != 1 || id_p1 != 3){
														id_c1=1;
														id_p1=3;}
											break;
											case 20:
													if(id_c1 != 1 || id_p1 != 4){
														id_c1=1;
														id_p1=4;}
											break;
											case 0:
													if(id_c1 != 0){
														id_c1=0;}
													gpio_set_level(CAC1, 0);
											break;
										}
								}
								else if(strcmp(id->next->valuestring,"off")==0){
											if(id_c1 != 0){
												id_c1=0;}
										gpio_set_level(CAC1, 0);
								}
						break;

						case 3: //14:
								if(strcmp(id->next->valuestring,"on")==0){
										switch (id->next->next->valueint) {
											case 100:
													printf("3,100\n" );
													if(id_c2 != 1 || id_p2 != 0){
														id_c2=1;
														id_p2=0;}
											break;
											case 80:
													if(id_c2 != 1 || id_p2 != 1){
														id_c2=1;
														id_p2=1;}
											break;
											case 60:
													if(id_c2 != 1 || id_p2 != 2){
														id_c2=1;
														id_p2=2;}
											break;
											case 40:
													if(id_c2 != 1 || id_p2 != 3){
														id_c2=1;
														id_p2=3;}
											break;
											case 20:
													if(id_c2 != 1 || id_p2 != 4){
														id_c2=1;
														id_p2=4;}
											break;
											case 0:
													if(id_c2 != 0){
														id_c2=0;}
													gpio_set_level(CAC2, 0);
											break;
										}
								}
								else if(strcmp(id->next->valuestring,"off")==0){
											if(id_c2 != 0){
												id_c2=0;}
										gpio_set_level(CAC2, 0);
								}
						break;

						case 9: //15:
								if(strcmp(id->next->valuestring,"on")==0){
										switch (id->next->next->valueint) {
											case 100:
													if(id_c3 != 1 || id_p3 != 0){
														id_c3=1;
														id_p3=0;}
											break;
											case 80:
													if(id_c3 != 1 || id_p3 != 1){
														id_c3=1;
														id_p3=1;}
											break;
											case 60:
													if(id_c3 != 1 || id_p3 != 2){
														id_c3=1;
														id_p3=2;}
											break;
											case 40:
													if(id_c3 != 1 || id_p3 != 3){
														id_c3=1;
														id_p3=3;}
											break;
											case 20:
													if(id_c2 != 1 || id_p3 != 4){
														id_c3=1;
														id_p3=4;}
											break;
											case 0:
													if(id_c3 != 0){
														id_c3=0;}
													gpio_set_level(CAC3, 0);
											break;
										}
								}
								else if(strcmp(id->next->valuestring,"off")==0){
											if(id_c3 != 0){
												id_c3=0;}
										gpio_set_level(CAC3, 0);
								}
						break;

						case 10: //16:
								if(strcmp(id->next->valuestring,"on")==0){
										switch (id->next->next->valueint) {
											case 100:
													if(id_c4 != 1 || id_p4 != 0){
														id_c4=1;
														id_p4=0;}
											break;
											case 80:
													if(id_c4 != 1 || id_p4 != 1){
														id_c4=1;
														id_p4=1;}
											break;
											case 60:
													if(id_c4 != 1 || id_p4 != 2){
														id_c4=1;
														id_p4=2;}
											break;
											case 40:
													if(id_c4 != 1 || id_p4 != 3){
														id_c4=1;
														id_p4=3;}
											break;
											case 20:
													if(id_c4 != 1 || id_p4 != 4){
														id_c4=1;
														id_p4=4;}
											break;
											case 0:
													if(id_c4 != 0){
														id_c4=0;}
													gpio_set_level(CAC4, 0);
											break;
										}
								}
								else if(strcmp(id->next->valuestring,"off")==0){
											if(id_c4 != 0){
												id_c4=0;}
										gpio_set_level(CAC4, 0);
								}
						break;

						default:
						break;
					}
			}
			cJSON_Delete(Loads);
			vTaskDelay(pdMS_TO_TICKS(1500));
	}
}
