# BLACKSTOMP
- Description: A quick development platform for ESP32-A1S digital audio effect processor!
- Version: 2.0

# Features
- Dual core 240MHz Tensilica Xtensa LX-6
- WiFi and Bluetooth connectivity
- 24-bit stereo codec with analog pass-through (for effect bypass)
- Six control inputs (for potentiometers, pedal, button, or selector switch)
- Guitar input
- Microphone input
- Stereo input/output
- MIDI input/output
- Rotary encoder interface
- I2C/OLED display interface

# Available Sketches Example
- Taptempo Stereo (Ping-Pong) Delay 
- Stereo Chorus
- BLE Pedal
- MIDI Pedal
- Mic Mixer
- Gain Doubler

# Latest Known Issues (V2.0)
- In V1.3 or lower, rapid control change might cause system restart by interrupt wdt reset failure
 -- This bug is not always reproduceable, it often hapens on a noisy boards.
 -- The condition when it happens: rapid control changes, direct access of Serial port from inside  Process, onButtonChange, and onControlChange functions
- Version 2.0 tried to fix the issues but don't know if it really solve it, however it is done by:
 -- Avoiding any direct access to Serial port from inside the Process, onButtonCahange, and onControlChange functions
 -- Emptying the main loop from monitoring codes, moving them to separate task on core 0 and provide the API
 
# TODO List
- Provide API to print debug message from inside the Process, onButtonChange, and onControlChange functions without direct Serial access
- Provide BLE terminal client app and implement debug monitoring API via BLE, so the Serial port can be dedicated for MIDI

# Change History
- Version 2.0
	-- All sketches in the example now left the main loop empty to dedicate core 1 for audio processing task
	-- Performance monitoring method runSystemMonitor has been added to the API, can be called after blackstompSetup when MIDI is not implemented
	-- All task's priorities has been reverted back to the same priority with audioprocessing task, except for the performance monitoring task which is the lowest (idle) priority
- Version 1.3 Fatal error fix at blackstomp.cpp at line 555 and 565
- Version 1.2 Changing the example sketches: change the serial to 9600 baud, deactivate the system info printing by commenting the code lines inside the loop
- Version 1.1 Fixed blackstomp.cpp at line 555 and 565
- Version 1.0 Initial release

# Documentation
- Blackstomp hardware and software programming manual at https://www.deeptronic.com/blackstomp/
- Blackstomp hardware circuit schematic diagram at https://www.deeptronic.com/blackstomp/schematic-diagram/
