/* Air Quality Sensor recieve an analog voltage */

#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "air_sensor.h"

extern xQueueHandle xSend_Info;

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //ADC1 => GPIO34
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

void air_sensor_read(void *pvParameters)
{
    // const char *sensor = "[{\"id\":10,\"value\":%d}]";
    const char *sensor = "[{\"id\":6,\"value\":%d}]";
    char *pcSend = NULL;
    //Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12); //12 bits de resolucion
    adc1_config_channel_atten(channel, atten); //configurando atenuacion, 0dB para el canal 6

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    //Continuously sample ADC1
    while (1) {
        uint32_t adc_reading = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
        adc_reading /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        asprintf(&pcSend, sensor, voltage);
        if (xQueueSendToBack(xSend_Info, pcSend,10000/portTICK_RATE_MS)!=pdTRUE){//2seg--> Tiempo max. que la tarea está bloqueada si la cola está llena
          printf("error S_AIR\n");
        }
        free(pcSend);
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
