//
// Created by spaceon on 11/20/16.
//

#ifndef ESP8266SENSORBROADCASTER_BMP180_H
#define ESP8266SENSORBROADCASTER_BMP180_H

#endif //ESP8266SENSORBROADCASTER_BMP180_H

#define BMP_180_ADDRESS          0x77
#define BMP_180_OSS              0

typedef struct BMP180 BMP180;
struct BMP180 {
    sint64 P;
    sint16 address;
    sint16 AC1;
    sint16 AC2;
    sint16 AC3;
    uint16 AC4;
    uint16 AC5;
    uint16 AC6;
    sint16 B1;
    sint16 B2;
    sint64 B3;
    uint64 B4;
    sint64 B5;
    sint64 B6;
    uint64 B7;
    sint16 MB;
    sint16 MC;
    sint16 MD;
    sint64 UT;
    sint64 UP;
    sint64 X1;
    sint64 X2;
    sint64 X3;
    sint64 T;
    sint16 oss;
};
bool ICACHE_FLASH_ATTR bmp180_read_byte(uint8 addr, uint8 regAddr, uint8 *readData);
bool ICACHE_FLASH_ATTR bmp180_read_2bytes(uint8 addr, uint8 regAddr, uint16 *readData);
bool ICACHE_FLASH_ATTR bmp180_write_byte_to_register(uint8 addr, uint8 regAddr, uint8 writeValue);
bool ICACHE_FLASH_ATTR bmp180_read(BMP180 *bmp_struct);
bool ICACHE_FLASH_ATTR bmp180_init(BMP180 *bmp_struct);