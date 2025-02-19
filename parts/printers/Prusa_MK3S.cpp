/*
	Prusa_MK3S.cpp - Printer definition for the Prusa MK3S
	Copyright 2020 VintagePC <https://github.com/vintagepc/>

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

#include "Prusa_MK3S.h"
#include "Beeper.h"           // for Beeper
#include "Fan.h"              // for Fan
#include "GLHelper.h"
#include "HD44780GL.h"        // for HD44780GL
#include "Heater.h"           // for Heater
#include "IRSensor.h"         // for IRSensor
#include "LED.h"              // for LED
#include "PINDA.h"            // for PINDA
#include "PinNames.h"         // for Pin::IR_SENSOR_PIN, Pin::VOLT_IR_PIN
#include "RotaryEncoder.h"    // for RotaryEncoder, RotaryEncoder::::CCW_CLICK
#include "SDCard.h"           // for SDCard
#include "TMC2130.h"          // for TMC2130
#include "sim_io.h"           // for avr_register_io_write
#include "uart_pty.h"         // for uart_pty
#include <GL/glew.h>		// NOLINT must come before freeglut
#include <GL/freeglut_std.h>  // for GLUT_DOWN, GLUT_LEFT_BUTTON, GLUT_RIGHT...
#include <cstdint>
#include <iostream>            // for printf

Prusa_MK3S::Prusa_MK3S():EinsyRambo(),Printer()
{
}

void Prusa_MK3S::Draw()
{
		glPushMatrix();
		glLoadIdentity(); // Start with an identity matrix
			glScalef(4, 4, 1);
			lcd.Draw();
		glPopMatrix();

		// Do something for the motors...
		float fX = (5 + lcd.GetWidth()* 6)*4;
		float fY = (5 + lcd.GetHeight() * 9);
		glLoadIdentity();
		glScalef(fX/350,4,1);
		glPushMatrix();
			glTranslatef(0, fY,0);
			X.Draw();
			glTranslatef(0, 10,0);
			Y.Draw();
			glTranslatef(0, 10,0);
			Z.Draw();
			glTranslatef(0, 10,0);
			E.Draw();
			glTranslatef(190,0,0);
			m_buzzer.Draw();
			glTranslatef(20,0,0);
			fPrint.Draw();
			glTranslatef(20,0,0);
			fExtruder.Draw();
			glTranslatef(20,0,0);
			hExtruder.Draw();
			glTranslatef(20,0,0);
			hBed.Draw();
			glTranslatef(20,0,0);
			lSD.Draw();
			glTranslatef(20,0,0);
			lPINDA.Draw();
			glTranslatef(20,0,0);
			lIR.Draw();
		glPopMatrix();

		if(!GetHasMMU())
		{
			m_gl.OnDraw();
		}

	if ((GetVisualType()!="none") && m_pVis)
	{
		m_pVis->FlagForRedraw();
	}
}

std::pair<int,int> Prusa_MK3S::GetWindowSize(){
	std::pair<int,int> prSize = { 5 + lcd.GetWidth() * 6,  40 + 5 + lcd.GetHeight() * 9};
	return prSize;
}

void Prusa_MK3S::OnVisualTypeSet(const std::string &type)
{
	if (type != "none")
	{
		m_pVis.reset(new MK3SGL(type,GetHasMMU(),this)); //NOLINT - suggestion is c++14.

		AddHardware(*m_pVis);

		m_pVis->ConnectFrom(X.GetIRQ(TMC2130::POSITION_OUT),MK3SGL::X_IN);
		m_pVis->ConnectFrom(Y.GetIRQ(TMC2130::POSITION_OUT),MK3SGL::Y_IN);
		m_pVis->ConnectFrom(Z.GetIRQ(TMC2130::POSITION_OUT),MK3SGL::Z_IN);
		m_pVis->ConnectFrom(E.GetIRQ(TMC2130::POSITION_OUT),MK3SGL::E_IN);
		m_pVis->ConnectFrom(X.GetIRQ(TMC2130::STEP_POS_OUT),MK3SGL::X_STEP_IN);
		m_pVis->ConnectFrom(Y.GetIRQ(TMC2130::STEP_POS_OUT),MK3SGL::Y_STEP_IN);
		m_pVis->ConnectFrom(Z.GetIRQ(TMC2130::STEP_POS_OUT),MK3SGL::Z_STEP_IN);
		m_pVis->ConnectFrom(E.GetIRQ(TMC2130::STEP_POS_OUT),MK3SGL::E_STEP_IN);
		m_pVis->ConnectFrom(pinda.GetIRQ(PINDA::SHEET_OUT), MK3SGL::SHEET_IN);
		m_pVis->ConnectFrom(fExtruder.GetIRQ(Fan::ROTATION_OUT), MK3SGL::EFAN_IN);
		m_pVis->ConnectFrom(fPrint.GetIRQ(Fan::ROTATION_OUT), MK3SGL::PFAN_IN);
		m_pVis->ConnectFrom(hBed.GetIRQ(Heater::ON_OUT), MK3SGL::BED_IN);
		m_pVis->ConnectFrom(sd_card.GetIRQ(SDCard::CARD_PRESENT), MK3SGL::SD_IN);
		m_pVis->ConnectFrom(pinda.GetIRQ(PINDA::TRIGGER_OUT), MK3SGL::PINDA_IN);
		m_pVis->SetLCD(&lcd);

		m_pVis->SetStepsPerMM(
			X.GetConfig().uiFullStepsPerMM,
			Y.GetConfig().uiFullStepsPerMM,
			Z.GetConfig().uiFullStepsPerMM,
			E.GetConfig().uiFullStepsPerMM
		);
	}
}

void Prusa_MK3S::FixSerial(avr_t * avr, avr_io_addr_t addr, uint8_t v)
{
	if (v==0x02)// Marlin is done setting up UCSRA0...
	{
		v|=(1U<<5U); // leave the UDRE0 alone
		std::cout << "Reset UDRE0 after serial config changed\n";
	}
	avr_core_watch_write(avr,addr,v);
}

void Prusa_MK3S::SetupIR()
{
	// Setup the 3S IR sensor.
	std::cout << "MK3S - adding IR sensor.\n";
	AddHardware(IR, GetPinNumber(VOLT_IR_PIN));
	TryConnect(&IR,IRSensor::DIGITAL_OUT, IR_SENSOR_PIN);
	TryConnect(IR_SENSOR_PIN, &lIR, LED::LED_IN);
	IR.Set(IRSensor::IR_v4_FILAMENT_PRESENT);
}

void Prusa_MK3S::SetupHardware()
{
	EinsyRambo::SetupHardware();

	SetupIR();

	if (GetConnectSerial())
	{
		UART0.Connect('0');
		UART1.Connect('1');
	}

	auto fcnSerial = [](avr_t *avr, avr_io_addr_t addr, uint8_t v, void * param)
	{auto *p = static_cast<Prusa_MK3S*>(param); p->FixSerial(avr, addr,v);};

	avr_register_io_write(m_pAVR, 0xC0, fcnSerial, this);

}

void Prusa_MK3S::OnAVRCycle()
{
	int mouseBtn = m_mouseBtn;                  // copy atomic to local
	if (mouseBtn)
	{
		switch (mouseBtn){
			case 1:
				encoder.MousePush();
				break;
			case 2:
				encoder.Release();
				break;
			case 3:
				encoder.Twist(RotaryEncoder::CCW_CLICK);
				if (m_pVis) m_pVis->TwistKnob(true);
				break;
			case 4:
				encoder.Twist(RotaryEncoder::CW_CLICK);
				if (m_pVis) m_pVis->TwistKnob(false);
				break;
		}
		m_mouseBtn = 0;
	}
}

// pragma: LCOV_EXCL_START
void Prusa_MK3S::OnMouseMove(int,int)
{
	// Passthrough? May not be necessary...
}
// pragma: LCOV_EXCL_STOP

// void Prusa_MK3S::OnKeyPress(unsigned char key, int, int)
// {
// 	switch (key) {
// 		case '1':
// 			m_iScheme ^=1;
// 			break;
//
// 		default:
// 			m_key = key;
// 	}
// }

void Prusa_MK3S::OnMousePress(int button, int action, int, int)
{
	if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
		if (action == GLUT_DOWN) {
			m_mouseBtn = 1;
		} else if (action == GLUT_UP) {
			m_mouseBtn = 2;
		}
	}
	if ((button==3 || button==4) && action == GLUT_DOWN) // wheel
	{
		m_mouseBtn = button;
	}
}
