Arduino UNO and MEGA2560 USB drivers

This is a collection of USB firmware for the Atmega8U2 and Atmega16U2 processors
on the UNO and MEGA2560 boards.

By loading the appropriate firmware onto the 8U2 or 16U2 the Arduino can be turned
into a native USB device.

The following drivers are currently available here.

* USB Mouse
* USB Keyboard
* USB Joystick
* USB MIDI device

All drivers are based on examples from Dean Camera's <a href="http://www.fourwalledcubicle.com/LUFA.php"> LUFA project</a>.
LUFA version 100807 is used to build the drivers.

Example Arduino sketches are provided in the example directory.  The sketches send and receive simplified binary
USB reports using the hardware serial interface, which are in turn converted into real USB reports by the 8U2/16U2.

See <a href="http://hunt.net.nz/users/darran">Arduino Hacking</a> for details.

Sketch report formats
---------------------

Small joystick interface
* 2 axis, signed 8 bit values -100 to 100
* 2 buttons
```
    struct {  
        int8_t x;  
        int8_t y;  
        uint8_t buttons;  
        uint8_t rfu; 	/* reserved for future use */  
    } joyReport;  
```

Big joystick interface
* 8 axis, signed 16 bit values -32768 to 32767
* 40 buttons

```
    typedef struct joyReport_t {
        int16_t axis[8];
        uint8_t button[5]; // 8 buttons per byte
    } joyReport_t;
```

USB HID Keyboard 
* See <a href="http://www.usb.org/developers/devclass_docs/Hut1_11.pdf">http://www.usb.org/developers/devclass_docs/Hut1_11.pdf</a> for key codes.

```
    typedef {  
        uint8_t modifiers_keys;	// CTRL, Shift, ALT, and GUI modifier keys  
        uint8_t rfu;		// reserved  
        uint8_t key[6];		// HID active key usage codes. This represents up to 6 keys currently being pressed.  
    } keyboardReport_t;  
```

USB MIDI device
```
    typedef union {  
        uint8_t command;  
        uint8_t channel;  
        uint8_t data2;  
        uint8_t data3;  
    } t_midiMsg;  
```

Mouse
```
    struct {  
        uint8_t buttons;  
        int8_t x;  
        int8_t y;  
        int8_t wheel;	/* Not yet implemented */  
    } mouseReport;  
```

Building the firmware
--------------------

You will need the avr-gcc toolchain and avr-libc.

To install them on a debian linux system:
# sudo apt-get install avr-gcc avr-libc

To install them on OSX:
# Install macports from <a href="http://www.macports.org">http://www.macports.org</a>
# sudo port install avr-gcc
# sudo port install avr-libc

Build the firmware using the makefile in its local directory.  E.g.

    cd firmwares/arduino-keyboard  
    make  


You can also flash it to the UNO's 8U2 using the makefile after putting the UNO into DFU mode.

    make dfu  


If you have a revision 3 UNO (or MEGA2560), edit the makefile and set the following variables before building the firmware and flashing it:

    MCU = atmega16u2  
    MCU_AVRDUDE = atmega16u2  
    MCU_DFU = atmega16u2  


Loading the firmware
--------------------

For the Atmega8u2 (UNO R1, R2, MEGA2560 R1, R2):
Put the UNO into DFU mode.

    dfu-programmer at90usb82 erase  
    dfu-programmer at90usb82 flash --debug 1 Arduino-big-joystick.hex  
    dfu-programmer at90usb82 reset  

Unplug the UNO's USB cable for a few seconds and plug it back in.  

For the ATMEGA16U2 (UNO R3, MEGA2560 R3):
Put the UNO into DFU mode.

    dfu-programmer atmega16u2 erase  
    dfu-programmer atmega16u2 flash --debug 1 Arduino-big-joystick.hex  
    dfu-programmer atmega16u2 reset  

Unplug the UNO's USB cable for a few seconds and plug it back in

Note you will need dfu-programmer version 0.5.5 to flash the atmega16u2.

FAQ
---

1. New USB drivers are loaded to the Arduino using DFU mode and a loading progam
   like dfu-progammer (Linux, OSX) or flip (Windows).
2. Sketches can only be loaded to the Arduino when the 8U2/16U2 is running the
   arduino-usbserial.hex driver. You must flash this firmware back to the Arduino 
   using DFU mode before you can load a new sketch.
