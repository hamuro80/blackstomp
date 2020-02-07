# BLACKSTOMP
ESP32-A1S DSP board: one board for all effect pedals

# Features
- 6 Control Input (for potentiometers, pedal, button, or selector switch)
- Guitar input
- Microphone input
- Stereo output
- MIDI input/output
- Rotary encoder interface
- OLED display interface

# Schematic Diagram
- There are 2 version of board: standard and simple
- Standard version support MIDI interface and decoupling capacitor for balanced audio I/O, but no support for headphone output
- Simple version rout all IO pins from ESP32-A1S module, including the headphone output and the serial port, but need external optical isolation circuit for MIDI port and need some decoupling capacitors for balanced audio I/O
- The standard version is suitable for design with MIDI I/O and balanced audio option, while the simple version is suitable for design without MIDI and balanced audio option

# Program Sketch
- The provided software contains only the basic core that initialize the codec, i2s interface, and running the signal processing block
- The signal processing block contains nothing more than copying the audio signal data from input buffer into the output buffer
- You can write your own signal processing algorithm by editing the signal processing block, and it is recommended to code the signal processing module as separate class and call the object's processing method in this block

