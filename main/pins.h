#ifndef PINS_H_INCLUDED
#define PINS_H_INCLUDED

/* INPUT PINS */
#define PIN_RAIN_SENSOR            GPIO_NUM_36
#define PIN_MOTION_SENSOR          GPIO_NUM_39
#define PIN_RST_WIFI               GPIO_NUM_35
#define PIN_DET_X                  GPIO_NUM_26
#define GPIO_INPUT_PIN_SEL  ((1ULL<<PIN_RST_WIFI) | (1ULL<<PIN_RAIN_SENSOR) | (1ULL<<PIN_MOTION_SENSOR) | (1ULL<<PIN_DET_X))

#define PIN_DHT11                  GPIO_NUM_14
#define I2C_MASTER_SCL_IO          GPIO_NUM_33               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO          GPIO_NUM_32               /*!< gpio number for I2C master data  */

/* OUTPUT PINS */
#define XAC1                       GPIO_NUM_5
#define XAC2                       GPIO_NUM_18
#define XDC1                       GPIO_NUM_17
#define XDC2                       GPIO_NUM_16
#define CAC1                       GPIO_NUM_23
#define CAC2                       GPIO_NUM_22
#define CAC3                       GPIO_NUM_21
#define CAC4                       GPIO_NUM_19
#define CDC1                       GPIO_NUM_4
#define CDC2                       GPIO_NUM_0
#define CDC3                       GPIO_NUM_15
#define CDC4                       GPIO_NUM_2
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<XAC1) | (1ULL<<XAC2) | (1ULL<<CAC1) | \
                              (1ULL<<CAC2) | (1ULL<<CAC3) | (1ULL<<CAC4))

void vInit_GPIO();
void i2c_master_init();

#endif
