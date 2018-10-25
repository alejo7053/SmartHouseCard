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

#include "wifi_manager.h"
#include "pins.h"
#include "interruptions.h"
#include "isr_tasks.h"


extern SemaphoreHandle_t ResetHW_Semaphore, xSemaphore;
extern xQueueHandle xSend_Info, gpio_evt_queue;

void ResetWifi(void* arg) {
	while(1) {
		// Espero por la notificacion de la ISR
		if(xSemaphoreTake(ResetHW_Semaphore,portMAX_DELAY) == pdTRUE) {
				wifi_manager_reset();
				vTaskDelay(3000 / portTICK_PERIOD_MS);
				esp_restart();
			}
	}
}

void gpio_task_example(void* arg)
{
    uint32_t sensor_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &sensor_num, portMAX_DELAY)) {
					if(sensor_num==PIN_MOTION_SENSOR){
						switch (gpio_get_level(sensor_num)) {
							case 0:
									xQueueSendToBack(xSend_Info, "[{\"id\":7,\"value\":0}]", pdMS_TO_TICKS(500));
							break;

							case 1:
									xQueueSendToBack(xSend_Info, "[{\"id\":7,\"value\":1}]", pdMS_TO_TICKS(500));
							break;

							default:
								printf("Error GPIO");
							break;
						}
					}
					else if(sensor_num==PIN_RAIN_SENSOR){
						switch (gpio_get_level(sensor_num)) {
							case 0:
									xQueueSendToBack(xSend_Info, "[{\"id\":8,\"value\":1}]", pdMS_TO_TICKS(500));
							break;

							case 1:
									xQueueSendToBack(xSend_Info, "[{\"id\":8,\"value\":0}]", pdMS_TO_TICKS(500));
							break;

							default:
								printf("Error GPIO");
							break;
						}
					}
        }
    }
}
