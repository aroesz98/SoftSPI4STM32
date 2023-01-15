/*
 * SoftSPI.cpp
 *
 *  Created on: 15 sty 2023
 *      Author: admin
 */

#include "SoftSPI.h"

SoftSPI::SoftSPI(GPIO_TypeDef* mosi_port, uint32_t mosi, GPIO_TypeDef* miso_port, uint32_t miso, GPIO_TypeDef* sck_port, uint32_t sck) {
	_mosi_port = mosi_port;
	_miso_port = miso_port;
    _sck_port = sck_port;
    _mosi = mosi;
    _miso = miso;
    _sck = sck;
    _delay = SPI_CLOCK_DIV128;
    _cke = 0;
    _ckp = 0;
    _order = MSBFIRST;
}

void SoftSPI::configInput (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void SoftSPI::configOutput (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void SoftSPI::writeIO (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool state)
{
	if(state)
	{
		GPIOx->BSRR = GPIO_Pin;
	}
	else
	{
		GPIOx->BSRR = GPIO_Pin << 16u;
	}
}

bool SoftSPI::readIO (GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	return (GPIOx->IDR & GPIO_Pin);
}

void SoftSPI::begin() {
	configOutput(_mosi_port, _mosi);
	configInput(_miso_port, _miso);
	configOutput(_sck_port, _sck);
}

void SoftSPI::end() {
    configInput(_mosi_port, _mosi);
	configInput(_miso_port, _miso);
	configInput(_sck_port, _sck);
}

void SoftSPI::setBitOrder(uint8_t order) {
    _order = order & 1;
}

void SoftSPI::setDataMode(uint8_t mode) {
    switch (mode) {
        case SPI_MODE0:
            _ckp = 0;
            _cke = 0;
            break;
        case SPI_MODE1:
            _ckp = 0;
            _cke = 1;
            break;
        case SPI_MODE2:
            _ckp = 1;
            _cke = 0;
            break;
        case SPI_MODE3:
            _ckp = 1;
            _cke = 1;
            break;
    }

    writeIO(_sck_port, _sck, _ckp ? 1 : 0);
}

void SoftSPI::setClockDivider(uint8_t div) {
    switch (div) {
        case SPI_CLOCK_DIV2:
            _delay = 2;
            break;
        case SPI_CLOCK_DIV4:
            _delay = 4;
            break;
        case SPI_CLOCK_DIV8:
            _delay = 8;
            break;
        case SPI_CLOCK_DIV16:
            _delay = 16;
            break;
        case SPI_CLOCK_DIV32:
            _delay = 32;
            break;
        case SPI_CLOCK_DIV64:
            _delay = 64;
            break;
        case SPI_CLOCK_DIV128:
            _delay = 128;
            break;
        default:
            _delay = 128;
            break;
    }
}

void SoftSPI::wait(uint_fast8_t del) {
    for (uint_fast8_t i = 0; i < del; i++) {
        asm volatile("nop");
    }
}

inline __attribute__((always_inline)) void SoftSPI::sendBit(uint8_t bit, uint8_t data)
{
	uint8_t del = _delay >> 1;

	writeIO(_mosi_port, _mosi, data & (1 << bit));
	wait(del);
	writeIO(_sck_port, _sck, 1);
	wait(del);
	writeIO(_sck_port, _sck, 0);
	wait(del);
}

inline __attribute__((always_inline)) void SoftSPI::send(uint8_t data)
{
	sendBit(7, data);
	sendBit(6, data);
	sendBit(5, data);
	sendBit(4, data);
	sendBit(3, data);
	sendBit(2, data);
	sendBit(1, data);
	sendBit(0, data);
}

void SoftSPI::send16(uint16_t data)
{
	union {
		uint16_t val;
		struct {
			uint8_t lsb;
			uint8_t msb;
		};
	} in;

	in.val = data;

	if ( _order == MSBFIRST )
	{
		send(in.msb);
		send(in.lsb);
	}

	else
	{
		send(in.lsb);
		send(in.msb);
	}
}

uint8_t SoftSPI::transfer(uint8_t val) {
    uint8_t out = 0;
    if (_order == MSBFIRST) {
        uint8_t v2 =
            ((val & 0x01) << 7) |
            ((val & 0x02) << 5) |
            ((val & 0x04) << 3) |
            ((val & 0x08) << 1) |
            ((val & 0x10) >> 1) |
            ((val & 0x20) >> 3) |
            ((val & 0x40) >> 5) |
            ((val & 0x80) >> 7);
        val = v2;
    }

    uint8_t del = _delay >> 1;

    uint8_t bval = 0;
    int sck = (_ckp) ? 1 : 0;

    for (uint8_t bit = 0u; bit < 8u; bit++)
    {
        if (_cke) {
            sck ^= 1;
            writeIO(_sck_port, _sck, sck);
            wait(del);
        }

        /* ... Write bit */
        writeIO(_mosi_port, _mosi, (val & (1<<bit)) ? 1 : 0);

        wait(del);

        sck ^= 1u; writeIO(_sck_port, _sck, sck);

        /* ... Read bit */
        {
            bval = readIO(_miso_port, _miso);

            if (_order == MSBFIRST) {
                out <<= 1;
                out |= bval;
            } else {
                out >>= 1;
                out |= bval << 7;
            }
        }

        wait(del);

        if (!_cke) {
            sck ^= 1u;
            writeIO(_sck_port, _sck, sck);
        }
    }

    return out;
}

uint16_t SoftSPI::transfer16(uint16_t data)
{
	union {
		uint16_t val;
		struct {
			uint8_t lsb;
			uint8_t msb;
		};
	} in, out;

	in.val = data;

	if ( _order == MSBFIRST ) {
		out.msb = transfer(in.msb);
		out.lsb = transfer(in.lsb);
	} else {
		out.lsb = transfer(in.lsb);
		out.msb = transfer(in.msb);
	}

	return out.val;
}
