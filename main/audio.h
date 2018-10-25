#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

//i2s number
#define I2S_NUM           (0)
//i2s sample rate
#define I2S_SAMPLE_RATE   (16000)
//i2s data bits
#define I2S_SAMPLE_BITS   (16)
//I2S read buffer length
#define I2S_READ_LEN      (16 * 1024)
//I2S data format
#define I2S_FORMAT        (I2S_CHANNEL_FMT_RIGHT_LEFT)

void i2s_init();
void audio_task(void * arg);
void audio_on_start();
void audio_off_start();

#endif
