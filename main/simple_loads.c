#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "cJSON.h"

#include "pins.h"
#include "audio.h"
#include "simple_loads.h"

extern xQueueHandle xReceive_Info;

void simple_loads_task(void *pvParameters)
{
    char Loads_json[2048];
    cJSON *Load=NULL, *id=NULL, *Loads=NULL, *action=NULL;
    struct tm timeinfo;
    time_t now;
    char strftime_buf[64];
    uint8_t pwm;

    ledc_timer_config_t ledc_timer = {
      .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty 8bit of resolution
      .freq_hz = 5000,                      // frequency of PWM signal
      .speed_mode = LEDC_LS_MODE,
      .timer_num = LEDC_LS_TIMER
    };;

    ledc_timer_config(&ledc_timer);

    /*
    * Prepare individual configuration
    * for each channel of LED Controller
    * by selecting:
    * - controller's channel number
    * - output duty cycle, set initially to 0
    * - GPIO number where LED is connected to
    * - speed mode, either high or low
    * - timer servicing selected channel
    *   Note: if different channels use one timer,
    *         then frequency and bit_num of these channels
    *         will be the same
    */
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
      {
          .channel    = LEDC_LS_CH4_CHANNEL,
          .duty       = 0,
          .gpio_num   = XDC1,
          .speed_mode = LEDC_LS_MODE,
          .timer_sel  = LEDC_LS_TIMER
      },
      {
          .channel    = LEDC_LS_CH5_CHANNEL,
          .duty       = 0,
          .gpio_num   = XDC2,
          .speed_mode = LEDC_LS_MODE,
          .timer_sel  = LEDC_LS_TIMER
      },
    };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel[0]);
    ledc_channel_config(&ledc_channel[1]);

    while(1){
        xQueuePeek( xReceive_Info, &Loads_json, portMAX_DELAY );
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%F %T", &timeinfo);
        Loads = cJSON_Parse(Loads_json);
        cJSON_ArrayForEach(Load, Loads)
        {
        		id = cJSON_GetObjectItemCaseSensitive(Load, "id");
            action = cJSON_GetObjectItemCaseSensitive(Load, "action");
            if(id->valueint == 15 || id->valueint == 16 ||
              id->valueint == 13 || id->valueint == 14)
            {
                switch (id->valueint) {
                  case 15: //17:
                      if(!(cJSON_IsNull(action)))
                      {
                          if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->valuestring,strftime_buf)<=0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)>=0 &&
                             gpio_get_level(XAC1)!=1)
                          {
                              audio_on_start();
                              gpio_set_level(XAC1, 1);
                          }
                          else if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)<=0 &&
                             gpio_get_level(XAC1)!=0)
                          {
                              audio_off_start();
                              gpio_set_level(XAC1, 0);
                          }
                          else if(strcmp(action->valuestring,"off")==0 &&
                                  strcmp(action->next->valuestring,strftime_buf)<=0 &&
                                  gpio_get_level(XAC1)!=0)
                          {
                              audio_off_start();
                              gpio_set_level(XAC1, 0);
                          }
                      } else {
                          if(strcmp(id->next->valuestring,"on")==0 &&
                          gpio_get_level(XAC1)!=1){
                            gpio_set_level(XAC1, 1);
                          }
                          else if(strcmp(id->next->valuestring,"off")==0 &&
                          gpio_get_level(XAC1)!=0){
                            gpio_set_level(XAC1, 0);
                          }
                      }
                  break;

                  case 16: //18:
                      if(!(cJSON_IsNull(action)))
                      {
                          if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->valuestring,strftime_buf)<=0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)>=0 &&
                             gpio_get_level(XAC2)!=1)
                          {
                              audio_on_start();
                              gpio_set_level(XAC2, 1);
                          }
                          else if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)<=0 &&
                             gpio_get_level(XAC2)!=0)
                          {
                              audio_off_start();
                              gpio_set_level(XAC2, 0);
                          }
                          else if(strcmp(action->valuestring,"off")==0 &&
                                  strcmp(action->next->valuestring,strftime_buf)<=0 &&
                                  gpio_get_level(XAC1)!=0)
                          {
                              audio_off_start();
                              gpio_set_level(XAC2, 0);
                          }
                      } else {
                          if(strcmp(id->next->valuestring,"on")==0 &&
                          gpio_get_level(XAC2)!=1){
                            gpio_set_level(XAC2, 1);
                          }
                          else if(strcmp(id->next->valuestring,"off")==0 &&
                          gpio_get_level(XAC2)!=0){
                            gpio_set_level(XAC2, 0);
                          }
                      }
                  break;

                  case 13: //19
                      if(!(cJSON_IsNull(action)))
                      {
                          pwm = id->next->next->valueint * (float)2.55;
                          if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->valuestring,strftime_buf)<=0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)>=0 &&
                             pwm != ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel))
                          {
                              audio_on_start();
                              ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, pwm);
                              ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                          }
                          else if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)<=0 &&
                             ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel != 0))
                          {
                              audio_off_start();
                              ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, 0);
                              ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                          }
                          else if(strcmp(action->valuestring,"off")==0 &&
                                  strcmp(action->next->valuestring,strftime_buf)<=0 &&
                                  ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel != 0))
                          {
                              audio_off_start();
                              ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, 0);
                              ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                          }
                      } else {
                          if(strcmp(id->next->valuestring,"on")==0){
                            pwm = id->next->next->valueint * (float)2.55;
                            if(pwm != ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel)){
                              ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, pwm);
                              ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                            }
                          }
                          else if(strcmp(id->next->valuestring,"off")==0){
                            if(ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel) != 0){
                              ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, 0);
                              ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                            }
                          }
                      }
                  break;

                  case 14: //20
                      if(!(cJSON_IsNull(action)))
                      {
                          pwm = id->next->next->valueint * (float)2.55;
                          if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->valuestring,strftime_buf)<=0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)>=0 &&
                             pwm != ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel))
                          {
                              audio_on_start();
                              ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, pwm);
                              ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                          }
                          else if(strcmp(action->valuestring,"on")==0 &&
                             strcmp(action->next->next->valuestring,strftime_buf)<=0 &&
                             ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel)!=0)
                          {
                              audio_off_start();
                              ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, 0);
                              ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                          }
                          else if(strcmp(action->valuestring,"off")==0 &&
                                  strcmp(action->next->valuestring,strftime_buf)<=0 &&
                                  ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel)!=0)
                          {
                              audio_off_start();
                              ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, 0);
                              ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                          }
                      } else {
                          if(strcmp(id->next->valuestring,"on")==0){
                            pwm = id->next->next->valueint * (float)2.55;
                            if(pwm != ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel)){
                              ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, pwm);
                              ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                            }
                          }
                          else if(strcmp(id->next->valuestring,"off")==0){
                            if(ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel) != 0){
                              ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, 0);
                              ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                            }
                          }
                      }
                  break;

                  default:
                  break;
                }
            }
        }
        cJSON_Delete(Loads);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
