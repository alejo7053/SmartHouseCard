#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "cJSON.h"

#include "pins.h"
#include "dc_control.h"

extern xQueueHandle xReceive_Info;

void PWM_task(void *arg)
{
    char Loads_json[2048];
    cJSON *Load=NULL, *id=NULL, *Loads=NULL;
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
    ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM_C] = {
      {
          .channel    = LEDC_LS_CH2_CHANNEL,
          .duty       = 0,
          .gpio_num   = LEDC_LS_CH2_GPIO,
          .speed_mode = LEDC_LS_MODE,
          .timer_sel  = LEDC_LS_TIMER
      },
      {
          .channel    = LEDC_LS_CH3_CHANNEL,
          .duty       = 0,
          .gpio_num   = LEDC_LS_CH3_GPIO,
          .speed_mode = LEDC_LS_MODE,
          .timer_sel  = LEDC_LS_TIMER
      },
      {
          .channel    = LEDC_LS_CH6_CHANNEL,
          .duty       = 0,
          .gpio_num   = LEDC_LS_CH6_GPIO,
          .speed_mode = LEDC_LS_MODE,
          .timer_sel  = LEDC_LS_TIMER
      },
      {
          .channel    = LEDC_LS_CH7_CHANNEL,
          .duty       = 0,
          .gpio_num   = LEDC_LS_CH7_GPIO,
          .speed_mode = LEDC_LS_MODE,
          .timer_sel  = LEDC_LS_TIMER
      },
    };

    // Set LED Controller with previously prepared configuration
    ledc_channel_config(&ledc_channel[0]);
    ledc_channel_config(&ledc_channel[1]);
    ledc_channel_config(&ledc_channel[2]);
    ledc_channel_config(&ledc_channel[3]);

    while (1) {

      xQueuePeek( xReceive_Info, &Loads_json, portMAX_DELAY );
      Loads = cJSON_Parse(Loads_json);
      cJSON_ArrayForEach(Load, Loads)
      {
          id = cJSON_GetObjectItemCaseSensitive(Load, "id");
          if(id->valueint == 11 || id->valueint == 12)
          {
              switch (id->valueint) {
                case 11: //21:
                    if(strcmp(id->next->valuestring,"on")==0){
                        if(id->next->next->valueint>=0){
                          pwm = id->next->next->valueint * (float)2.55;
                          if(pwm != ledc_get_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel)){
                              ledc_set_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel, 0);
                              ledc_update_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel);
                              ets_delay_us(500);
                              ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, pwm);
                              ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
                          }
                        } else {
                            pwm = id->next->next->valueint * (float)2.55 * -1;
                            if(pwm != ledc_get_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel)){
                              ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, 0);
                              ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
                              ets_delay_us(500);
                              ledc_set_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel, pwm);
                              ledc_update_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel);
                            }
                          }
                    } else {
                      if(ledc_get_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel)!=0 ||
                         ledc_get_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel)!=0){
                            ledc_set_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel, 0);
                            ledc_update_duty(ledc_channel[2].speed_mode, ledc_channel[2].channel);
                            ledc_set_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel, 0);
                            ledc_update_duty(ledc_channel[3].speed_mode, ledc_channel[3].channel);
                        }
                    }
                break;

                case 12: //22:
                    if(strcmp(id->next->valuestring,"on")==0){
                        if(id->next->next->valueint>=0){
                            pwm = id->next->next->valueint * (float)2.55;
                            if(pwm != ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel)){
                              ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, 0);
                              ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                              ets_delay_us(500);
                              ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, pwm);
                              ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                            }
                        } else {
                            pwm = id->next->next->valueint * (float)2.55 * -1;
                            if(pwm != ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel)){
                                ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, 0);
                                ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
                                ets_delay_us(500);
                                ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, pwm);
                                ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                              }
                        }
                    } else {
                      if(ledc_get_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel)!=0 ||
                         ledc_get_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel)!=0){
                            ledc_set_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel, 0);
                            ledc_update_duty(ledc_channel[1].speed_mode, ledc_channel[1].channel);
                            ledc_set_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel, 0);
                            ledc_update_duty(ledc_channel[0].speed_mode, ledc_channel[0].channel);
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
