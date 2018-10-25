#ifndef LOADS_H_INCLUDED
#define LOADS_H_INCLUDED

#define LEDC_LS_TIMER          LEDC_TIMER_0
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH4_CHANNEL    LEDC_CHANNEL_4
#define LEDC_LS_CH5_CHANNEL    LEDC_CHANNEL_5
#define LEDC_TEST_CH_NUM       (2)

void simple_loads_task(void *pvParameters);

#endif
