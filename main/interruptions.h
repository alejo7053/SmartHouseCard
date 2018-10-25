#ifndef INTERRUPTIONS_H_INCLUDED
#define INTERRUPTIONS_H_INCLUDED

void IRAM_ATTR detector(void *arg);
void IRAM_ATTR gpio_wifi_rst(void *arg);
void IRAM_ATTR gpio_states_sensors(void* arg);
void ControlLoad_Task(void *arg);

#endif
