/*
 * SoftSPI.h
 *
 *  Created on: 15 sty 2023
 *      Author: admin
 */

#ifndef INC_SOFTSPI_H_
#define INC_SOFTSPI_H_

#include "stdint.h"
#include "stm32f4xx.h"

typedef enum
{
	LSBFIRST = 0,
	MSBFIRST = 1,
} ORDER_MODE;

typedef enum
{
	SPI_MODE0 = 0,
	SPI_MODE1 = 1,
	SPI_MODE2 = 2,
	SPI_MODE3 = 3,
} SPI_MODE;

typedef enum
{
	SPI_CLOCK_DIV2 = 2,
	SPI_CLOCK_DIV4 = 4,
	SPI_CLOCK_DIV8 = 8,
	SPI_CLOCK_DIV16 = 16,
	SPI_CLOCK_DIV32 = 32,
	SPI_CLOCK_DIV64 = 64,
	SPI_CLOCK_DIV128 = 128,
} SPI_CLK_DIV;

class SoftSPI {
    private:
        void wait(uint_fast8_t del);

    private:
        uint32_t _cke;
        uint32_t _ckp;
        uint32_t _delay;
        GPIO_TypeDef* _miso_port;
        GPIO_TypeDef* _mosi_port;
        GPIO_TypeDef* _sck_port;
        uint32_t _miso;
        uint32_t _mosi;
        uint32_t _sck;
        uint8_t _order;

        void configInput  (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
        void configOutput (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
        bool readIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
        void writeIO(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool state);

    public:
        SoftSPI(GPIO_TypeDef* mosi_port, uint32_t mosi, GPIO_TypeDef* miso_port, uint32_t miso, GPIO_TypeDef* sck_port, uint32_t sck);
        void begin();
        void end();
        void setBitOrder(uint8_t);
        void setDataMode(uint8_t);
        void setClockDivider(uint8_t);
        uint8_t transfer(uint8_t);
        void send(uint8_t data);
        void send16(uint16_t data);
        void sendBit(uint8_t bit, uint8_t data);
	uint16_t transfer16(uint16_t data);

};

#endif /* INC_SOFTSPI_H_ */
