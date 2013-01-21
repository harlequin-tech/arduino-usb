/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this 
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in 
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting 
  documentation, and that the name of the author not be used in 
  advertising or publicity pertaining to distribution of the 
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/*-
 * Copyright (c) 2011 Darran Hunt (darran [at] hunt dot net dot nz)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file
 *
 *  Main source file for the Arduino-keyboard project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "Arduino-usbmidi.h"

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface = {
    .Config = {
	.StreamingInterfaceNumber = 1,

	.DataINEndpointNumber      = MIDI_STREAM_IN_EPNUM,
	.DataINEndpointSize        = MIDI_STREAM_EPSIZE,
	.DataINEndpointDoubleBank  = false,

	.DataOUTEndpointNumber     = MIDI_STREAM_OUT_EPNUM,
	.DataOUTEndpointSize       = MIDI_STREAM_EPSIZE,
	.DataOUTEndpointDoubleBank = false,
    },
};

/** Circular buffer to hold data from the host before it is sent to the device via the serial port. */
RingBuff_t USBtoUSART_Buffer;

/** Circular buffer to hold data from the serial port before it is sent to the host. */
RingBuff_t USARTtoUSB_Buffer;

#define LED_ON_TICKS 10000	/* Number of ticks to leave LEDs on */

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    MIDI_EventPacket_t midiEvent;
    struct {
	uint8_t command;
	uint8_t channel;
	uint8_t data2;
	uint8_t data3;
    } midiMsg;
    int ind;

    int led1_ticks = 0;
    int led2_ticks = 0;

    SetupHardware();

    RingBuffer_InitBuffer(&USBtoUSART_Buffer);
    RingBuffer_InitBuffer(&USARTtoUSB_Buffer);

    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
    sei();

    for (;;) {
	RingBuff_Count_t BufferCount = RingBuffer_GetCount(&USARTtoUSB_Buffer);

	/* See if we have a message yet */
	if (BufferCount >= 4) {
	    /* Read in the message from the serial buffer */
	    for (ind=0; ind<4; ind++) {
		((uint8_t *)&midiMsg)[ind] = RingBuffer_Remove(&USARTtoUSB_Buffer);
	    }

	    /* Build a midi event to send via USB */
	    midiEvent.CableNumber = 0;
	    midiEvent.Command = midiMsg.command >> 4;
	    midiEvent.Data1 = (midiMsg.command & 0xF0) | ((midiMsg.channel-1) & 0x0F);
	    midiEvent.Data2 = midiMsg.data2;
	    midiEvent.Data3 = midiMsg.data3;

	    MIDI_Device_SendEventPacket(&Keyboard_MIDI_Interface, &midiEvent);
	    MIDI_Device_Flush(&Keyboard_MIDI_Interface);

	    /* Turn on the TX led and starts its timer */
	    LEDs_TurnOnLEDs(LEDS_LED1);
	    led1_ticks = LED_ON_TICKS;
	}
	
	/* Turn off the Tx LED when the tick count reaches zero */
	if (led1_ticks) {
	    led1_ticks--;
	    if (led1_ticks == 0) {
		LEDs_TurnOffLEDs(LEDS_LED1);
	    }
	}

	if (MIDI_Device_ReceiveEventPacket(&Keyboard_MIDI_Interface, &midiEvent)) {
	    RingBuff_Count_t count = RingBuffer_GetCount(&USBtoUSART_Buffer);
	    /* Room to send a message? */
	    if ((BUFFER_SIZE - count) >= sizeof(midiMsg)) {
		midiMsg.command = midiEvent.Command << 4;
		midiMsg.channel = (midiEvent.Data1 & 0x0F) + 1;
		midiMsg.data2 = midiEvent.Data2;
		midiMsg.data3 = midiEvent.Data3;

		for (ind=0; ind<sizeof(midiMsg); ind++) {
		    RingBuffer_Insert(&USBtoUSART_Buffer, ((uint8_t *)&midiMsg)[ind]);
		}

		/* Turn on the RX led and start its timer */
		LEDs_TurnOnLEDs(LEDS_LED2);
		led2_ticks = LED_ON_TICKS;
	    } else {
		/* Turn on the RX led and leave it on to indicate the
		 * buffer is full and the sketch is not reading it 
		 * fast enough.
		 */
		LEDs_TurnOnLEDs(LEDS_LED2);
	    }

	    /* if there's no room in the serial buffer the message gets dropped */
	}

	/* Turn off the RX LED when the tick count reaches zero */
	if (led2_ticks) {
	    led2_ticks--;
	    if (led2_ticks == 0) {
		LEDs_TurnOffLEDs(LEDS_LED2);
	    }
	}

	/* any data to send to main processor? */
	if (!(RingBuffer_IsEmpty(&USBtoUSART_Buffer))) {
	    Serial_TxByte(RingBuffer_Remove(&USBtoUSART_Buffer));
	}

	MIDI_Device_USBTask(&Keyboard_MIDI_Interface);
	USB_USBTask();
    }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Hardware Initialization */
	Serial_Init(115200, false);
	LEDs_Init();
	USB_Init();

	/* Start the flush timer so that overflows occur rapidly to push received bytes to the USB interface */
	TCCR0B = (1 << CS02);
	
	/* Must turn off USART before reconfiguring it, otherwise incorrect operation may occur */
	UCSR1B = 0;
	UCSR1A = 0;
	UCSR1C = 0;

	UBRR1  = SERIAL_2X_UBBRVAL(115200);

	UCSR1C = ((1 << UCSZ11) | (1 << UCSZ10));
	UCSR1A = (1 << U2X1);
	UCSR1B = ((1 << RXCIE1) | (1 << TXEN1) | (1 << RXEN1));
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_READY);
    if (!MIDI_Device_ConfigureEndpoints(&Keyboard_MIDI_Interface))
	LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Unhandled Control Request event. */
void EVENT_USB_Device_UnhandledControlRequest(void)
{
	MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
}

/** ISR to manage the reception of data from the serial port, placing received bytes into a circular buffer
 *  for later transmission to the host.
 */
ISR(USART1_RX_vect, ISR_BLOCK)
{
    uint8_t ReceivedByte = UDR1;

    if ((USB_DeviceState == DEVICE_STATE_Configured) &&
	    !RingBuffer_IsFull(&USARTtoUSB_Buffer)) {
	RingBuffer_Insert(&USARTtoUSB_Buffer, ReceivedByte);
    }
}
