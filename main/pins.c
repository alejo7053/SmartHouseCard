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
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "mdns.h"
#include "cJSON.h"

#include "http_request.h"
#include "dht11.h"
#include "i2c_sensors.h"
#include "pins.h"
#include "interruptions.h"
#include "isr_tasks.h"


void vInit_GPIO()
{
  	gpio_config_t io_conf;
  	//disable interrupt
  	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
  	//set as output mode
  	io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
  	//bit mask of the pins that you want to set,e.g.GPIO18/19
  	io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
  	//disable pull-down mode
  	io_conf.pull_down_en = 1;
  	//disable pull-up mode
  	io_conf.pull_up_en = 0;
  	//configure GPIO with the given settings
  	gpio_config(&io_conf);

  	//interrupt of rising edge
  	io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
  	//bit mask of the pins, use GPIO4/5 here
  	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  	//set as input mode
  	io_conf.mode = GPIO_MODE_INPUT;
  	//disable pull-up mode
  	io_conf.pull_up_en = 0;
  	//enable pull-down mode
  	io_conf.pull_down_en = 1;
  	gpio_config(&io_conf);

  	//change gpio intrrupt type for one pin and pull mode
    gpio_set_intr_type(PIN_RST_WIFI, GPIO_INTR_NEGEDGE);
  	gpio_set_pull_mode(PIN_RAIN_SENSOR, GPIO_PULLUP_ONLY);
    //install default gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIN_RST_WIFI, gpio_wifi_rst, NULL);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(PIN_MOTION_SENSOR, gpio_states_sensors, (void *) PIN_MOTION_SENSOR);
  	gpio_isr_handler_add(PIN_RAIN_SENSOR, gpio_states_sensors, (void*) PIN_RAIN_SENSOR);
}

/**
 * @brief i2c master initialization
 */
void i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}
