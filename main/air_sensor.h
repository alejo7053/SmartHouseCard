#ifndef AIR_SENSOR_H_INCLUDED
#define AIR_SENSOR_H_INCLUDED

#define DEFAULT_VREF    1100        //Maximum V_REF
#define NO_OF_SAMPLES   64          //Multisampling

void air_sensor_read(void *pvParameters);

#endif
