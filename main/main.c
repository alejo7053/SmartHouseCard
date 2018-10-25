/*
Copyright (c) 2017 Tony Pottier

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

@file main.c
@author Tony Pottier
@brief Entry point for the ESP32 application.
@see https://idyl.io
@see https://github.com/tonyp7/esp32-wifi-manager
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/i2s.h"

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/apps/sntp.h"

#include "nvs_flash.h"
#include "mdns.h"
#include "cJSON.h"

#include "http_server.h"
#include "wifi_manager.h"

#include "isr_tasks.h"
#include "interruptions.h"
#include "http_request.h"
#include "dht11.h"
#include "i2c_sensors.h"
#include "air_sensor.h"
#include "simple_loads.h"
#include "pins.h"
#include "audio.h"
#include "dc_control.h"


static TaskHandle_t task_wifi_manager = NULL;

extern void periodic_timer_callback(void* arg);
extern void oneshot_timer_callback(void* arg);
extern esp_timer_handle_t periodic_timer;
extern esp_timer_handle_t oneshot_timer;

/**
 * @brief RTOS task that periodically prints the heap memory available.
 * @note Pure debug information, should not be ever started on production code!
 */
#if WIFI_MANAGER_DEBUG
void monitoring_task(void *pvParameter)
{
	#if WIFI_MANAGER_DEBUG
	struct tm timeinfo;
	time_t now;
	char strftime_buf[64];
	#endif
	for(;;){
		printf("\n\nfree heap: %d\n",esp_get_free_heap_size());
		#if WIFI_MANAGER_DEBUG
				time(&now);
				localtime_r(&now, &timeinfo);
				strftime(strftime_buf, sizeof(strftime_buf), "%F %T", &timeinfo);
				ESP_LOGI("HOUR", "The current date/time in Colombia is: %s", strftime_buf);
				const size_t bytes_per_task = 40; /* see vTaskList description */
				char* task_list_buffer = malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
				if (task_list_buffer == NULL) {
						ESP_LOGE(__func__, "failed to allocate buffer for vTaskList output");
				}
				fputs("Task Name\tStatus\tPrio\tHWM\tTask Number\n", stdout);
				vTaskList(task_list_buffer);
				fputs(task_list_buffer, stdout);
				free(task_list_buffer);
		#endif
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
#endif

SemaphoreHandle_t ResetHW_Semaphore = NULL;
xQueueHandle xSend_Info = NULL, xReceive_Info = NULL, gpio_evt_queue = NULL;

void app_main()
{
	/* disable the default wifi logging */
	esp_log_level_set("wifi", ESP_LOG_NONE);
	/* initialize flash memory */
	nvs_flash_init();

	vInit_GPIO(); //Inicia GPIO's
	i2c_master_init();//Inicia I2C Como maestro

	const esp_timer_create_args_t periodic_timer_args = {
					.callback = &periodic_timer_callback,
					/* name is optional, but may help identify the timer when debugging */
					.name = "periodic"
	};

	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	/* The timer has been created but is not running yet */

	const esp_timer_create_args_t oneshot_timer_args = {
					.callback = &oneshot_timer_callback,
					/* argument specified here will be passed to timer callback function */
					.arg = (void*) periodic_timer,
					.name = "one-shot"
	};
	ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));


	xSend_Info= xQueueCreate(15, sizeof( char ) * 30); //Queue to send info to the http_request
	xReceive_Info= xQueueCreate(1, sizeof( char ) * 2048); //Queue to receive info from the http_request
	ResetHW_Semaphore = xSemaphoreCreateBinary();
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	struct tm timeinfo;
  time_t now;
  time(&now);
  localtime_r(&now, &timeinfo);
	// Set timezone to Eastern Standard Time and print local time
	setenv("TZ", "EST5", 1);
	tzset();

	//For monitoring the free heap
	#if WIFI_MANAGER_DEBUG
	xTaskCreatePinnedToCore(&monitoring_task, "monitoring_task", 2048, NULL, 1, NULL, 1);
	#endif

/*** TAREAS ***/
	/*																						 | APSTA MODE
	 *													 |──Not connected──| Create HTTP_ServerTask
	 * WiFi_ManagerTask ─────────|
	 *													 |──Connected──────| Create HTTP_RequestTask──|
	 *																						 | STA MODE									|──Create Sensors and actuators Tasks
	 */

	/* The HTTP Server task start in the Wifi_Manager Task*/
	/* start the wifi manager task */
	i2s_init();
	xTaskCreate(&wifi_manager, "wifi_manager", 4096, NULL, 4, &task_wifi_manager);

	/* your code should go here. In debug mode we create a simple task on core 2 that monitors free heap memory */
	xTaskCreate(&ResetWifi, "ResetWifi", 2048, NULL, 5, NULL); //Task for reset the WiFi credentials.
	xTaskCreate(&audio_task, "Audio Task", 2048, NULL, 3, NULL);


	/* TAREAS PARA LECTURA DE SENSORES */
	/* Lectura Sensor DHT11 (Temperatura y Humedad) Esta tarea se crea en http_request *
	 * Lectura Sensores I2C (BH1750) Esta tarea se crea en http_request *
	 * Lectura Sensor MQ-135 (Calidad del Aire) Esta tarea se crea en http_request */

	/* TAREAS PARA CONTROL DE CARGAS */
	/* http_request */

/**
	**/
}
