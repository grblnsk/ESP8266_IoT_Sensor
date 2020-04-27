#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "i2c_master.h"
#include "BMP180.h"
#include "os_type.h"

#define I2C_MASTER_SDA_GPIO 5
#define I2C_MASTER_SCL_GPIO 14
#define I2C_MASTER_SDA_FUNC FUNC_GPIO1
#define I2C_MASTER_SCL_FUNC FUNC_GPIO14
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
#define REMOTE_UDP_PORT          50001
#define SENSOR_ID                "3333"
#define UART_BAUD_RATE          115200
#define READ_PERIOD_IN_MS       1000

os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t udp_send_timer, read_bmp180_timer;


/*---------------------------------------------------------------------------*/
LOCAL struct espconn ptrespconn;
LOCAL BMP180 *bmp180;
LOCAL int frameCounter =0;
LOCAL char frameString[] = "%ld";
LOCAL char frameBuffer[4096];
LOCAL char frameBuffer1[1024];
LOCAL char frameBuffer2[1024];
LOCAL char frameBuffer3[3];


LOCAL void ICACHE_FLASH_ATTR
        user_udp_sent_callback(void *arg)
{
    struct espconn *pespconn = arg;
    //os_printf("user_udp_sent_cb\n");
}



LOCAL void ICACHE_FLASH_ATTR
        user_udp_sent(struct espconn *pespconn, char* frameBuffer)
{
    uint16 length;
    uint16 packet_size=1024;
    char *pbuf = (char *)os_zalloc(packet_size);
    length=os_strlen(frameBuffer);
    os_memcpy(pbuf,frameBuffer,length);
    espconn_sent(pespconn, pbuf, os_strlen(pbuf));
    os_free(pbuf);
}

LOCAL void ICACHE_FLASH_ATTR user_udp_init(void)
{
    const char dst_ip[4]={255,255,255,255}; // Server IP
    ptrespconn.type = ESPCONN_UDP;
    ptrespconn.proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
    ptrespconn.proto.udp->remote_port = REMOTE_UDP_PORT;// Remote Server Port
    os_memcpy(ptrespconn.proto.udp->remote_ip, dst_ip, 4); // Remote Server IP
    ptrespconn.proto.udp->local_port=espconn_port();// ESP8266 udp port
    espconn_regist_sentcb(&ptrespconn, user_udp_sent_callback); // register a udp packet sending callback
    espconn_create(&ptrespconn);   // create udp
    //user_udp_sent(&ptrespconn); // sent data
}


void send_udp_packet_timerfunc(void *arg)
{
    //os_printf("before udp send\n");
    //user_udp_sent(&ptrespconn);
    //os_printf("after udp send\n");

}

//Do nothing function
static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{

    os_delay_us(10);
}

void read_bmp180_timerfunc(void *arg)
{
    frameCounter++;
    if(frameCounter==100) frameCounter=0;

    os_printf("---START BMP180 TIMER FUNC\n");
    if(bmp180_read(bmp180)) {

        os_printf("bmp read OK\n");

        os_sprintf(frameBuffer3,"%d", frameCounter);
        os_sprintf(frameBuffer1,frameString, bmp180->P);
        os_sprintf(frameBuffer2,frameString, bmp180->T);

        strcat(frameBuffer, SENSOR_ID);
        strcat(frameBuffer, ",");
        strcat(frameBuffer, frameBuffer3);
        strcat(frameBuffer, ",");
        strcat(frameBuffer, frameBuffer1);
        strcat(frameBuffer, ",");
        strcat(frameBuffer, frameBuffer2);

        user_udp_sent(&ptrespconn, frameBuffer);
        os_printf(frameBuffer);
        frameBuffer[0]='\0';

    } else os_printf("bmp read FAILED\n");
    os_printf("---END BMP180 TIMER FUNC\n");
    os_printf("\n");


    os_printf("\n");

}
//
//Init function 
void ICACHE_FLASH_ATTR user_init()
{
    os_delay_us(1000);

    //SET UART
    uart_div_modify(0, UART_CLK_FREQ / UART_BAUD_RATE);

    //SET WIFI
    char ssid[32] = "wifiSSID";
    char password[64] = "wifipassword";
    struct station_config stationConf;
    wifi_set_opmode( STATION_MODE );
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);

    wifi_station_connect();
    os_printf("wifi connected\n");

    //SET UDP
    user_udp_init();
    os_printf("udp set\n");

    //SET I2C WITH BMP180
    i2c_master_gpio_init();
    i2c_master_init();

    bmp180 = (BMP180*)os_malloc(sizeof(BMP180));

    if(bmp180_init(bmp180)){
        //SET BMP180 READ TIMER TEST
        os_timer_disarm(&read_bmp180_timer);
        //Setup timer
        os_timer_setfn(&read_bmp180_timer, (os_timer_func_t *)read_bmp180_timerfunc, NULL);
        //Arm the timer
        os_timer_arm(&read_bmp180_timer, READ_PERIOD_IN_MS, 1);

        os_printf("timer armed\n");

    } else os_printf("bmp not set\n");

    //start os task
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    system_os_post(_);
}