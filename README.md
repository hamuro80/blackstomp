# BLACKSTOMP
* Description: A quick development platform for ESP32-A1S digital audio effect processor!
* Version: 3.1

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
 
# TODO List
- Provide BLE terminal client app and implement debug monitoring API via BLE, so the Serial port can be dedicated for MIDI
- Provide implementation of multi-button mode of the encoder input

# Change History
* Version 3.1
  + Fixed uninitialized button pins for ES8388 version module
* Version 3.0 
  + Adding setDeviceType() function to support for both AC101 version and the new ES8388 version of the ESP32-A1S module
  + Removing setOutMix() API function
  + Adding new bypass mode parameter for analogBypass() and analogSoftBypass() API function
* Version 2.1 Edited Gain Doubler sketch example to synchronize with the manual document.
* Version 2.0
  + All sketches in the example now left the main loop empty to dedicate core 1 for audio processing task
  + Performance monitoring method runSystemMonitor has been added to the API, can be called after blackstompSetup when MIDI is not implemented
  + All task's priorities has been reverted back to the same priority with audioprocessing task, except for the performance monitoring task which is the lowest (idle) priority
* Version 1.3 Fatal error fix at blackstomp.cpp at line 555 and 565
* Version 1.2 Changing the example sketches: change the serial to 9600 baud, deactivate the system info printing by commenting the code lines inside the loop
* Version 1.1 Fixed blackstomp.cpp at line 555 and 565
* Version 1.0 Initial release

# Documentation
- Blackstomp hardware and software programming manual at https://www.deeptronic.com/blackstomp/
- Blackstomp hardware circuit schematic diagram at https://www.deeptronic.com/blackstomp/schematic-diagram/
