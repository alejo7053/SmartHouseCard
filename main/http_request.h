#ifndef HTTP_REQUEST_H_INCLUDED
#define HTTP_REQUEST_H_INCLUDED


#define WEB_SERVER "smarthouseuq.herokuapp.com"
#define WEB_URL "/api/2/ecMfS8Dz21/"
#define WEB_PORT 80
#define HTTP_REQUEST_START_BIT_0	( 1 << 0 )

// #define WEB_SERVER "192.168.0.135"
// #define WEB_URL "/proyectoSmartHouse/public/api/5/S6LlR7EoTY/"

void http_get_task(void *pvParameters);
void http_request_set_event_start();
void http_request_set_event_stop();
void obtain_time(void);
void initialize_sntp(void);

#endif
