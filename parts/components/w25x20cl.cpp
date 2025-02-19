/*
	w25x20cl.cpp - An SPI flash emulator for the Einsy external language flash

	Copyright 2020 leptun <https://github.com/leptun/>

	Rewritten for C++ in 2020 by VintagePC <https://github.com/vintagepc/>

 	This file is part of MK404.

	MK404 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MK404 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with MK404.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "w25x20cl.h"
#include "TelemetryHost.h"
#include <algorithm>   // for min
#include <cstdlib>     // for exit, free, malloc
#include <cstring>     // for memset, memcpy, strncpy
#include <fstream>		// IWYU pragma: keep
#include <iostream>
#include <string>

//#define TRACE(_w) _w
#ifndef TRACE
#define TRACE(_w)
#endif


#define _MFRID             0xEF
#define _DEVID             0x11

#define _CMD_ENABLE_WR     0x06
// #define _CMD_ENABLE_WR_VSR 0x50
#define _CMD_DISABLE_WR    0x04
#define _CMD_RD_STATUS_REG 0x05
// #define _CMD_WR_STATUS_REG 0x01
#define _CMD_RD_DATA       0x03
// #define _CMD_RD_FAST       0x0b
// #define _CMD_RD_FAST_D_O   0x3b
// #define _CMD_RD_FAST_D_IO  0xbb
#define _CMD_PAGE_PROGRAM  0x02
#define _CMD_SECTOR_ERASE  0x20
#define _CMD_BLOCK32_ERASE 0x52
#define _CMD_BLOCK64_ERASE 0xd8
#define _CMD_CHIP_ERASE    0xc7
#define _CMD_CHIP_ERASE2   0x60
// #define _CMD_PWR_DOWN      0xb9
// #define _CMD_PWR_DOWN_REL  0xab
#define _CMD_MFRID_DEVID   0x90
// #define _CMD_MFRID_DEVID_D 0x92
// #define _CMD_JEDEC_ID      0x9f
#define _CMD_RD_UID        0x4b

w25x20cl::w25x20cl():Scriptable("SPIFlash")
{
	// Verify register packing/bounds/alignment.
	Expects(sizeof(m_status_register) == sizeof(m_status_register.byte));


	RegisterAction("Load","Reloads the last used file",ActLoad);
	RegisterAction("Save","Saves the file",ActSave);
	RegisterAction("Clear","Resets the flash memory to empty (0xFF)",ActClear);
	RegisterAction("Fill","Fills the flash memory with the given value",ActFill,{ArgType::Int});
};


w25x20cl::~w25x20cl() = default;

/*
 * called when a SPI byte is sent
 */
uint8_t w25x20cl::OnSPIIn(struct avr_irq_t *, uint32_t value)
{
	switch (m_state)
	{
		case STATE_LOADING:
		{
			if (m_rxCnt >= m_cmdIn.size())
			{
				std::cout << "w25x20cl_t: error: command too long: ";
				for (auto i : m_cmdIn)
				{
					std::cout << std::hex << i << " ";
				}
				std::cout << '\n';
				break;
			}
			m_cmdIn[m_rxCnt] = value;
			m_rxCnt++;


			// Check for a loaded instruction in the cmdIn buffer
			m_command = m_cmdIn[0];
			switch(m_command)
			{
				case _CMD_RD_DATA:
				case _CMD_MFRID_DEVID:
				case _CMD_SECTOR_ERASE:
				case _CMD_BLOCK32_ERASE:
				case _CMD_BLOCK64_ERASE:
				{
					if (m_rxCnt == 4)
					{
						m_address = 0;
						for (unsigned  i = 0; i < 3; i++)
						{
							m_address *= W25X20CL_PAGE_SIZE;
							m_address |= m_cmdIn[i + 1];
						}
						m_address %= W25X20CL_TOTAL_SIZE;
						m_state = STATE_RUNNING;
					}
				} break;

				case _CMD_PAGE_PROGRAM:
				{
					if (m_rxCnt == 4)
					{
						m_address = 0;
						for (unsigned  i = 0; i < 3; i++)
						{
							m_address *= W25X20CL_PAGE_SIZE;
							m_address |= m_cmdIn[i + 1];
						}
						m_address %= W25X20CL_TOTAL_SIZE;
						memcpy(m_pageBuffer.data(), m_flash.begin() + (m_address / W25X20CL_PAGE_SIZE) * W25X20CL_PAGE_SIZE, W25X20CL_PAGE_SIZE);
						m_state = STATE_RUNNING;
					}
				} break;

				case _CMD_ENABLE_WR:
				case _CMD_DISABLE_WR:
				case _CMD_RD_STATUS_REG:
				case _CMD_CHIP_ERASE:
				case _CMD_CHIP_ERASE2:
				{
					m_state = STATE_RUNNING;
				} break;

				case _CMD_RD_UID:
				{
					if (m_rxCnt == 5)
					{
						m_address = sizeof(m_UID);
						m_state = STATE_RUNNING;
					}
				} break;

				default:
				{
				std::cout  << "w25x20cl_t: error: unknown command: ";
				for (auto i = 0; i < m_rxCnt; i++)
				{
					std::cout << std::hex << m_cmdIn[i];
				}
				std::cout << '\n';
				} break;
			}

		} break;
		case STATE_RUNNING:
		{
			TRACE(printf("w25x20cl_t: command:%02x, address:%05x, value:%02x\n", m_command, m_address, value));
			switch (m_command)
			{
				case _CMD_MFRID_DEVID:
				{
					m_cmdOut = (m_address % 2)?_DEVID:_MFRID;
					m_address++;
					m_address %= W25X20CL_TOTAL_SIZE;
					SetSendReplyFlag();
				} break;
				case _CMD_RD_DATA:
				{
					m_cmdOut = m_flash[m_address];
					m_address++;
					m_address %= W25X20CL_TOTAL_SIZE;
					SetSendReplyFlag();
				} break;
				case _CMD_RD_STATUS_REG:
				{
					m_cmdOut = m_status_register.byte;
					SetSendReplyFlag();
				} break;
				case _CMD_RD_UID:
				{
					if (m_address)
					{
						m_cmdOut = (m_UID >> 8U*(m_address - 1)) & 0xFFU;
						SetSendReplyFlag();
						m_address--;
					}
				} break;
				case _CMD_PAGE_PROGRAM:
				{
					m_pageBuffer[m_address & 0xFFU] = value;
					m_address = ((m_address / W25X20CL_PAGE_SIZE) * W25X20CL_PAGE_SIZE) + (((m_address) + 1U) & 0xFFU);
				} break;
			}
			break;
		}

		case STATE_IDLE:
		default:
		{
			return 0;
		} break;
	}
	TRACE(printf("w25x20cl_t: byte received: %02x\n",value));
	return m_cmdOut;
}

// Called when CSEL changes.
void w25x20cl::OnCSELIn(struct avr_irq_t *, uint32_t value)
{
	TRACE(printf("w25x20cl_t: CSEL changed to %02x\n",value));
	if (value == 0)
	{
		m_state = STATE_LOADING;
		memset(m_cmdIn.data(), 0, m_cmdIn.size_bytes());
		m_rxCnt = 0;
		m_cmdOut = 0;
		m_command = 0;
		m_address = 0;
	}
	else
	{
		if (m_state == STATE_RUNNING)
		{
			switch (m_command)
			{
				case _CMD_ENABLE_WR:
				{
					m_status_register.bits.WEL = 1;
				} break;
				case _CMD_DISABLE_WR:
				{
					m_status_register.bits.WEL = 0;
				} break;
				case _CMD_PAGE_PROGRAM:
				{
					if(!m_status_register.bits.WEL) break;
					m_address /= W25X20CL_PAGE_SIZE;
					m_address *= W25X20CL_PAGE_SIZE;
					for (unsigned int i = 0; i < m_pageBuffer.size(); i++)
					{
						m_flash[m_address + i] &= m_pageBuffer[i];
					}
					m_status_register.bits.WEL = 0;
				} break;
				case _CMD_CHIP_ERASE:
				case _CMD_CHIP_ERASE2:
				{
					if(!m_status_register.bits.WEL) break;
					memset(m_flash.data(), 0xFF, m_flash.size_bytes());
					m_status_register.bits.WEL = 0;
				} break;
				case _CMD_SECTOR_ERASE:
				{
					if(!m_status_register.bits.WEL) break;
					m_address /= W25X20CL_SECTOR_SIZE;
					m_address *= W25X20CL_SECTOR_SIZE;
					memset(m_flash.begin() + m_address, 0xFF, W25X20CL_SECTOR_SIZE);
					m_status_register.bits.WEL = 0;
				} break;
				case _CMD_BLOCK32_ERASE:
				{
					if(!m_status_register.bits.WEL) break;
					m_address /= W25X20CL_BLOCK32_SIZE;
					m_address *= W25X20CL_BLOCK32_SIZE;
					memset(m_flash.begin() + m_address, 0xFF, W25X20CL_BLOCK32_SIZE);
					m_status_register.bits.WEL = 0;
				} break;
				case _CMD_BLOCK64_ERASE:
				{
					if(!m_status_register.bits.WEL) break;
					m_address /= W25X20CL_BLOCK64_SIZE;
					m_address *= W25X20CL_BLOCK64_SIZE;
					memset(m_flash.begin() + m_address, 0xFF, W25X20CL_BLOCK64_SIZE);
					m_status_register.bits.WEL = 0;
				} break;
			}
		}
		m_state = STATE_IDLE;
	}
}

void w25x20cl::Init(struct avr_t * avr, avr_irq_t* irqCS)
{
	_InitWithArgs(avr,this,nullptr, SPI_CSEL);
	ConnectFrom(irqCS, SPI_CSEL);

	auto &TH = TelemetryHost::GetHost();
	TH.AddTrace(this, SPI_BYTE_IN,{TC::SPI, TC::Storage},8);
	TH.AddTrace(this, SPI_BYTE_OUT,{TC::SPI, TC::Storage},8);
	TH.AddTrace(this, SPI_CSEL, {TC::SPI, TC::Storage, TC::OutputPin});

	m_status_register.byte = 0b00000000; //SREG default values}
};

Scriptable::LineStatus w25x20cl::ProcessAction(unsigned int iAct, const std::vector<std::string> &vArgs)
{
	switch (iAct)
	{
		case ActClear:
		{
			memset(m_flash.data(),0xFF,m_flash.size_bytes());
			return LineStatus::Finished;
		}
		case ActFill:
		{
			memset(m_flash.data(),gsl::narrow<uint8_t>(stoi(vArgs.at(0))) & 0xFFU,m_flash.size_bytes());
			return LineStatus::Finished;
		}
		case ActLoad:
		{
			if (!m_filepath.empty())
			{
				Load();
				return LineStatus::Finished;
			}
			else
			{
				return LineStatus::Error;
			}

		}
		case ActSave:
		{
			if (!m_filepath.empty())
			{
				Save();
				return LineStatus::Finished;
			}
			else
			{
				return LineStatus::Error;
			}
		}
		default:
			return LineStatus::Unhandled;
	}
}

void w25x20cl::Load(const std::string &path)
{
		m_filepath = path;
		Load();
}

void w25x20cl::Load(const gsl::span<uint8_t> data, uint32_t uiOffset)
{
	size_t bytes = std::min(m_flash.size_bytes() - uiOffset, data.size_bytes());
	memcpy(m_flash.begin() + uiOffset, data.begin(), bytes);
	std::cout << "Loaded " << std::to_string(bytes) << " bytes from hex file to xflash\n";
}

void w25x20cl::Load()
{
	auto *path = m_filepath.c_str();
	// Now deal with the external flash. Can't do this in special_init, it's not allocated yet then.
	std::ifstream fsIn(path, fsIn.binary | fsIn.ate);
	m_filepath = path;

	if (!fsIn.is_open() || fsIn.tellg() < W25X20CL_TOTAL_SIZE) {
		std::cerr << "ERROR: Could not open SPI flash file. Flash contents were NOT restored" << '\n';
	}
	else
	{
		std::cout << "Loading " <<  W25X20CL_TOTAL_SIZE  <<" bytes of xflash\n";
		fsIn.seekg(fsIn.beg);
		fsIn.read(reinterpret_cast<char*>(m_flash.data()), W25X20CL_TOTAL_SIZE + 1); // NOLINT no choice but to cast...
		if (fsIn.fail() || fsIn.gcount() != W25X20CL_TOTAL_SIZE + 1 ) {
			std::cerr << "Unable to load w25x20cl\n";
			exit(1);
		}
		bool bEmpty = true;
		for (auto &b : m_flash)
		{
			bEmpty &= b==0;
		}
		if (bEmpty)
		{
			for (auto &c : m_flash)
			{
				c = 0xFF;
			}
		}
	}
	fsIn.close();
}

void w25x20cl::Save()
{
	// Also write out the xflash contents. Note  you can save snapshots anytime you like.
		// Write out the EEPROM contents:
	std::ofstream fsOut(m_filepath, fsOut.binary);
	if (!fsOut.is_open())
	{
		std::cerr << "Failed to open xflash output file\n";
		return;
	}
	fsOut.write(reinterpret_cast<char*>(m_flash.data()),W25X20CL_TOTAL_SIZE+1); //NOLINT no choice but to cast...
	std::cout << "Wrote "<< fsOut.tellp() <<" bytes of xflash to " << m_filepath <<'\n';
	if (fsOut.tellp() != W25X20CL_TOTAL_SIZE + 1) {
		std::cerr << "Unable to write xflash memory\n";
	}
	fsOut.close();
}
