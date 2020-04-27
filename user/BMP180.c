//
// Created by spaceon on 11/20/16.
//
#include "os_type.h"
#include "ets_sys.h"
#include "BMP180.h"
#include "i2c_master.h"
#include "osapi.h"
#include "fs_math.h"


#define BMP_180_ADDRESS          0x77
#define BMP_180_OSS              0

/*##class BMP180*/
double ICACHE_FLASH_ATTR bmp180_get_altitude(sint64 pressure, sint64 pressure_sea_level){
    return 44330*( 1- fs_pow( ( pressure/(1.0*pressure_sea_level) ), 0.190294957 ) );
}

bool ICACHE_FLASH_ATTR bmp180_read_byte(uint8 addr, uint8 regAddr, uint8 *readData) {
    uint8 ack;

//write address
    i2c_master_start();
    i2c_master_writeByte((addr << 1));
    ack = i2c_master_checkAck();
    if (!ack) {
//os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }
//register
    i2c_master_writeByte(regAddr);
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }
//restart
    i2c_master_wait(40000);
    i2c_master_stop();
    i2c_master_start();
//read address
    i2c_master_writeByte((addr << 1) + 1);
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }

//read msb
    *readData = i2c_master_readByte();
    i2c_master_send_nack();
    i2c_master_stop();
    return true;

}

bool ICACHE_FLASH_ATTR bmp180_read_2bytes(uint8 addr, uint8 regAddr, uint16 *readData) {
    uint8 ack;

//write address
    i2c_master_start();

    i2c_master_writeByte((addr
            << 1));
    ack = i2c_master_checkAck();
    if (!ack) {
//os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();

        return false;
    }
//register
    i2c_master_writeByte(regAddr);
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");

        i2c_master_stop();

        return false;
    }
//restart
    i2c_master_wait(40000);

    i2c_master_stop();

    i2c_master_start();

//read address
    i2c_master_writeByte((addr
            << 1) + 1);
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");

        i2c_master_stop();

        return false;
    }
//read msb
    *readData = i2c_master_readByte() << 8;

    i2c_master_send_ack();
    *readData |= (

            i2c_master_readByte()

            & 0xff);

    i2c_master_send_nack();
    i2c_master_stop();
    return true;
}

bool ICACHE_FLASH_ATTR bmp180_write_byte_to_register(uint8 addr,uint8 regAddr, uint8 writeValue) {
    uint8 ack;
    //write address
    i2c_master_start();
    i2c_master_writeByte((addr<<1));
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }

    //register
    i2c_master_writeByte(regAddr);
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");

        i2c_master_stop();

        return false;
    }

    i2c_master_writeByte(writeValue);
    ack = i2c_master_checkAck();
    if (!ack) {
        os_printf("addr not ack when tx write cmd \n");

        i2c_master_stop();

        return false;
    }

    i2c_master_stop();


}

bool ICACHE_FLASH_ATTR bmp180_read_uncompensated_temperature(BMP180 *bmp_struct){
    uint16 temp = 0;
    if (!bmp180_write_byte_to_register(bmp_struct->address, 0xF4, 0x2E))
        return false;
    else {
        os_delay_us(4500);
        bmp180_read_2bytes(bmp_struct->address, 0xF6, &temp);
        bmp_struct->UT = temp;
    }
    return true;
}

bool ICACHE_FLASH_ATTR bmp180_read_uncompensated_pressure(BMP180 *bmp_struct){
    uint16 mlsb = 0;
    uint8 xlsb = 0;
    if (!bmp180_write_byte_to_register(bmp_struct->address,
                                       0xF4,
                                       0x34 + ((bmp_struct->oss) << 6))
            )
        return false;
    else {
        switch (bmp_struct->oss) {
            case 0:
                os_delay_us(4500);
                break;
            case 1:
                os_delay_us(7500);
                break;
            case 2:
                os_delay_us(13500);
                break;
            case 3:
                os_delay_us(25500);
                break;
            default:
                os_delay_us(25500);
        }


        if (!(bmp180_read_2bytes(bmp_struct->address, 0xF6, &mlsb) &&
              bmp180_read_byte(bmp_struct->address, 0xF8, &xlsb)))
            return false;
        bmp_struct->UP = (mlsb << 8 + xlsb) >> (8 - bmp_struct->oss);
    }
    return true;
}

bool ICACHE_FLASH_ATTR bmp180_calculate_true_temperature(BMP180 *bmp_struct){
    bmp_struct->X1 = (bmp_struct->UT - bmp_struct->AC6) * bmp_struct->AC5 / 32768;
    bmp_struct->X2 = bmp_struct->MC * 2048 / (bmp_struct->X1 + bmp_struct->MD);
    bmp_struct->B5 = bmp_struct->X1 + bmp_struct->X2;
    bmp_struct->T = (bmp_struct->B5 + 8) / 16;
}

bool ICACHE_FLASH_ATTR bmp180_calculate_true_pressure(BMP180 *bmp_struct){
    bmp_struct->B6 = bmp_struct->B5 - 4000;
    bmp_struct->X1 = (bmp_struct->B2 * (bmp_struct->B6 * bmp_struct->B6 / 4096)) / 2048;
    bmp_struct->X2 = (bmp_struct->AC2 * bmp_struct->B6) / 2048;
    bmp_struct->X3 = bmp_struct->X1 + bmp_struct->X2;
    bmp_struct->B3 = (((bmp_struct->AC1 * 4 + bmp_struct->X3) << bmp_struct->oss) + 2) / 4;
    bmp_struct->X1 = bmp_struct->AC3 * bmp_struct->B6 / 8192;
    bmp_struct->X2 = (bmp_struct->B1 * (bmp_struct->B6 * bmp_struct->B6 / 4096)) / 65536;
    bmp_struct->X3 = ((bmp_struct->X1 + bmp_struct->X2) + 2) / 4;
    bmp_struct->B4 = (bmp_struct->AC4) * (uint64)(bmp_struct->X3 + 32768) / 32768;
    bmp_struct->B7 = ((uint64) bmp_struct->UP - bmp_struct->B3) * (50000 >> bmp_struct->oss);
    if (bmp_struct->B7 < 0x80000000) {
        bmp_struct->P = (bmp_struct->B7 * 2) / bmp_struct->B4;
    } else {
        bmp_struct->P = (bmp_struct->B7 / bmp_struct->B4) * 2;
    }
    bmp_struct->X1 = (bmp_struct->P / 256) * (bmp_struct->P / 256);
    bmp_struct->X1 = (bmp_struct->X1 * 3038) / 65536;
    bmp_struct->X2 = (-7357 * bmp_struct->P) / 65536;
    bmp_struct->P = bmp_struct->P + (bmp_struct->X1 + bmp_struct->X2 + 3791) / 16;
}

bool ICACHE_FLASH_ATTR bmp180_read(BMP180 *bmp_struct) {
    //read uncompensated temp value
    if(! bmp180_read_uncompensated_temperature(bmp_struct) )return false;
    //read uncompensated pressure value
    if(! bmp180_read_uncompensated_pressure(bmp_struct) )return false;
    //calculate true temperature
    bmp180_calculate_true_temperature(bmp_struct);
    //calculate true pressure
    bmp180_calculate_true_pressure(bmp_struct);

    return true;
}


bool ICACHE_FLASH_ATTR bmp180_init(BMP180 *bmp_struct) {

    bmp_struct->address = BMP_180_ADDRESS;
    bmp_struct->oss = BMP_180_OSS;
    if (!(
            bmp180_read_2bytes(bmp_struct->address, 0xaa, &bmp_struct->AC1) &&
            bmp180_read_2bytes(bmp_struct->address, 0xac, &bmp_struct->AC2) &&
            bmp180_read_2bytes(bmp_struct->address, 0xae, &bmp_struct->AC3) &&
            bmp180_read_2bytes(bmp_struct->address, 0xb0, &bmp_struct->AC4) &&
            bmp180_read_2bytes(bmp_struct->address, 0xb2, &bmp_struct->AC5) &&
            bmp180_read_2bytes(bmp_struct->address, 0xb4, &bmp_struct->AC6) &&
            bmp180_read_2bytes(bmp_struct->address, 0xb6, &bmp_struct->B1) &&
            bmp180_read_2bytes(bmp_struct->address, 0xb8, &bmp_struct->B2) &&
            bmp180_read_2bytes(bmp_struct->address, 0xba, &bmp_struct->MB) &&
            bmp180_read_2bytes(bmp_struct->address, 0xbc, &bmp_struct->MC) &&
            bmp180_read_2bytes(bmp_struct->address, 0xbe, &bmp_struct->MD)
    ))return false;

    return true;


}