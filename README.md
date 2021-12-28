# BLACKSTOMP
* Description: A quick development platform for ESP32-A1S digital audio effect processor!
* Version: 3.4

# Features
- Dual core 240MHz Tensilica Xtensa LX-6 (32-bit processor with floating point unit)
- WiFi and Bluetooth connectivity
- 24-bit stereo codec with analog pass-through bypass
- Six control inputs (for potentiometers, expression pedal, button, or selector switch)
- 3 buttons inputs (for foot switches)
- Guitar input
- Microphone input
- Stereo input/output
- MIDI input/output

# Available Sketches Example
- Taptempo Stereo (Ping-Pong) Delay 
- Stereo Chorus
- BLE Pedal
- MIDI Pedal
- Mic Mixer
- Gain Doubler
 
# TODO List
- Provide BLE terminal client app and implement debug monitoring API via BLE, so the Serial port can be dedicated for MIDI
- Provide implementation of multi-button mode of the control input

# Change History
* Version 3.4
  + Added tap multiplier control and re-arrange the previous controls in taptempodelay scketch
  + Added noise gate control in micmixer sketch example
  + Added conversion optimization control in gaindoubler sketch example
  + Fixed optimizeConversion function to be callable inside setup
  + Added commented (inactive) optimizeConversion() code inside setup() section of all example sketches
* Version 3.3
  + Added optimizeConversion(range) API for ES8388-version module to produce much less noise at default range 250mVrms/707mVpp
  + Enabled ALC to reduce the clipping effect for limited range of ES8388 version module
  + Enabled ALC to boost the microphoe gain with minimal compression and limiting effect
  + Added setMicNoiseGate() API to use the noise gate feature of ES8388-version module
  + Added setPhase method and setWaveTable method for oscillator object to accomodate non-sinusoidal waveforms
  + Added runScope function for sending real-time waveform data to Arduino IDE's serial plotter
* Version 3.2
  + Added slowSpeed property in control object to suppress the internal noise of ES8388 version in CM_POT control mode
  + Fixed some error in example scketches
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
