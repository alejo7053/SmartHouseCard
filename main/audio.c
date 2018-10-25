#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2s.h"

#include "audio_files/on_file.h"
#include "audio_files/off_file.h"
#include "audio.h"

EventGroupHandle_t audio_event_group;

const int ON_BIT = BIT0;
const int OFF_BIT = BIT1;

void audio_on_start(){
	xEventGroupSetBits(audio_event_group, ON_BIT );
}

void audio_off_start(){
	xEventGroupSetBits(audio_event_group, OFF_BIT );
}

void i2s_init()
{
  int i2s_num = I2S_NUM;
  i2s_config_t i2s_config = {
    .mode = I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
    .sample_rate =  I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_SAMPLE_BITS,
	  .communication_format = I2S_COMM_FORMAT_I2S_MSB,
	  .channel_format = I2S_FORMAT,
	  .intr_alloc_flags = 0,
	  .dma_buf_count = 2,
	  .dma_buf_len = 1024
	};
	//install and start i2s driver
	i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
	//init DAC pad
	i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN); //I2S_DAC_CHANNEL_RIGHT_EN => GPIO25
	i2s_zero_dma_buffer(I2S_NUM);
}

/**
 * @brief Scale data to 16bit/32bit for I2S DMA output.
 *        DAC can only output 8bit data value.
 *        I2S DMA will still send 16 bit or 32bit data, the highest 8bit contains DAC data.
 */
int i2s_dac_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
#if (I2S_SAMPLE_BITS == 16)
    for (int i = 0; i < len; i++) {
        d_buff[j++] = 0;
        d_buff[j++] = s_buff[i];
    }
    return (len * 2);
#else
    for (int i = 0; i < len; i++) {
        d_buff[j++] = 0;
        d_buff[j++] = 0;
        d_buff[j++] = 0;
        d_buff[j++] = s_buff[i];
    }
    return (len * 4);
#endif
}

void audio_task(void *ard)
{
    int i2s_read_len = I2S_READ_LEN;
		size_t bytes_written;
		uint8_t* i2s_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));
    i2s_set_clk(I2S_NUM, 16000, I2S_SAMPLE_BITS, 1);

		int offset = 0;
		int tot_size = 0;

		i2s_zero_dma_buffer(I2S_NUM);
		audio_event_group = xEventGroupCreate();
    EventBits_t uxBits;

    while (1) {
        /* actions that can trigger: on/off a load */
				i2s_zero_dma_buffer(I2S_NUM);
        uxBits = xEventGroupWaitBits(audio_event_group, ON_BIT | OFF_BIT , pdFALSE, pdFALSE, portMAX_DELAY );
        if(uxBits & ON_BIT)
        {
						offset = 0;
		        tot_size = sizeof(on_table);
		        while (offset < tot_size) {
		            int play_len = ((tot_size - offset) > (4 * 1024)) ? (4 * 1024) : (tot_size - offset);
		            int i2s_wr_len = i2s_dac_data_scale(i2s_write_buff, (uint8_t*)(on_table + offset), play_len);
		            i2s_write(I2S_NUM, i2s_write_buff, i2s_wr_len, &bytes_written, portMAX_DELAY);
		            offset += play_len;
		        }
						i2s_zero_dma_buffer(I2S_NUM);
            /* finally: release the scan request bit */
    			  xEventGroupClearBits(audio_event_group, ON_BIT);
        }
        if(uxBits & OFF_BIT)
        {
						offset = 0;
						tot_size = sizeof(off_table);
						while (offset < tot_size) {
								int play_len = ((tot_size - offset) > (4 * 1024)) ? (4 * 1024) : (tot_size - offset);
								int i2s_wr_len = i2s_dac_data_scale(i2s_write_buff, (uint8_t*)(off_table + offset), play_len);
								i2s_write(I2S_NUM, i2s_write_buff, i2s_wr_len, &bytes_written, portMAX_DELAY);
								offset += play_len;
						}
						i2s_zero_dma_buffer(I2S_NUM);
            xEventGroupClearBits(audio_event_group, OFF_BIT);
        }
    }
    free(i2s_write_buff);
    vTaskDelete(NULL);
}
