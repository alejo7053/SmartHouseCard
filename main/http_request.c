#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/apps/sntp.h"
#include "cJSON.h"

#include "http_request.h"
#include "wifi_manager.h"
#include "pins.h"
#include "dht11.h"
#include "air_sensor.h"
#include "i2c_sensors.h"
#include "simple_loads.h"
#include "audio.h"
#include "dc_control.h"
#include "interruptions.h"
#include "isr_tasks.h"

const char *REQUEST_FORMAT =
				"GET "WEB_URL"%s HTTP/1.0\r\n"
				"Host: "WEB_SERVER"\r\n"
				"User-Agent: esp-idf/1.0 esp32\r\n\r\n";

static const char *TAG = "HTTP_Rqst";
extern xQueueHandle xSend_Info, xReceive_Info;

EventGroupHandle_t http_request_event_group;
EventBits_t uxBits;

void http_request_set_event_start(){
	xEventGroupSetBits(http_request_event_group, HTTP_REQUEST_START_BIT_0 );
}

void http_request_set_event_stop(){
	xEventGroupClearBits(http_request_event_group, HTTP_REQUEST_START_BIT_0 );
}

void obtain_time(void)
{
    initialize_sntp();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
		sntp_stop();
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void http_get_task(void *pvParameters){

    http_request_event_group = xEventGroupCreate();

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    // struct in_addr *addr;
    int s, r;
		char Rx[30];
    char recv_buf[2048];
		char *line=NULL;
		char *recv_json=NULL;

		char *REQUEST = NULL;

		struct tm timeinfo;
		time_t now;
		time(&now);
		localtime_r(&now, &timeinfo);
		// Is time set? If not, tm_year will be (1970 - 1900).

		#if WIFI_MANAGER_DEBUG
		printf("http_request: waiting for start bit\n");
		#endif

		/*Espera el bit de activación y luego crea las tareas y comienza a enviar datos*/
		uxBits = xEventGroupWaitBits(http_request_event_group, HTTP_REQUEST_START_BIT_0, pdFALSE, pdTRUE, portMAX_DELAY );

		if (timeinfo.tm_year < (2016 - 1900)) {
				obtain_time();
		}

		xTaskCreate(&air_sensor_read, "air_sensor_read", 3072, NULL, 3, NULL);
		xTaskCreate(&simple_loads_task, "Loads ON/OFF Task", 6144, NULL, 4, NULL);
		xTaskCreate(&PWM_task, "Loads DC Control", 4098, NULL, 5, NULL);
		xTaskCreate(&ControlLoad_Task, "Ac Control", 4098, NULL, 7, NULL);
		xTaskCreate(&i2c_task, "i2c_test_task_0", 2048, NULL, 2, NULL);
		xTaskCreate(&gpio_task_example, "gpio_task_example", 2048, NULL, 7, NULL);
		// xTaskCreate(&audio_task, "Audio Task", 2048, NULL, 3, NULL);
		xTaskCreate(&dht_task, "dht_task", 3072, NULL, 3, NULL);
		gpio_isr_handler_add(PIN_DET_X, detector, NULL);

		vTaskDelay( pdMS_TO_TICKS(100) );

    while(1) {

				if(xQueueReceive(xSend_Info,&Rx,10000/portTICK_RATE_MS)==pdTRUE) {//10s --> Tiempo max. que la tarea está bloqueada si la cola está vacía
					asprintf(&REQUEST, REQUEST_FORMAT, Rx);
					// printf("%s\n", REQUEST);
				} else{
					asprintf(&REQUEST, REQUEST_FORMAT, "[]");
				}

        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);
        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }


        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        // addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        // ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        // ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

				free(REQUEST);

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
				recv_json = (char *)malloc(2048);
        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
						if(r != 0)
						{
							line=strstr(recv_buf,"[{");
							line=strtok(line,"\n");
							if(line!=NULL)
							{
								strcpy( recv_json, line );
							}
						}
        } while(r > 0);

        // ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
				xQueueOverwrite(xReceive_Info, recv_json);
				free(recv_json);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
