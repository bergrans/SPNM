Steam Pop Noice Machine
====

The Steam Pop Noice Machine (SPNM) is part of the Steam Pop Machine project. The Steam Pop Machine is a large mobile popcorn  machine build in [Steampunk style](http://nl.wikipedia.org/wiki/Steampunk). This machine is used for selling poopcorn on huge festivals in an atractive/interactive way. To make the machine look more background-, random- and trigged noises are generated.

The SPNM contains the following parts:
----
 - Arduino UNO controller http://arduino.cc/en/Main/ArduinoBoardUno
 - MP3 Player Shield by Sparkfun https://www.sparkfun.com/products/10628
 - 4 channel relay board https://iprototype.nl/products/components/buttons-switches/relay-board-4-channels-5v
 - Power Supply 12/3A

I/O
----
 - A0...A4 inputs for triggered samples
 - A5 Potentiometer input used for volume setting
 - 10, 3, 4, 5 relay channel 1...4

####Remarks
Pin 10 seems to be used by the SPI library during startup. Causing the relay on this channel to switch shortly. Since the pin is not used by the MP3 shield there is no problem in using it for general IO.

Used software libraries
----
 - [SFEMP3Shield](https://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library) 
 - [TimerOne](https://code.google.com/p/arduino-timerone/downloads/list)

MP3 samples
----
The SPNM uses three types of samples:
 1. Background noises (played continious in random order)
 2. Random noises (played at random intervals)
 3. Trigged noise (played when trigged by an external switch)

The samples are stored on a SD card with the following naming convention:
```
track_XYY.mp3
```
Where:
 - X = the sample type. 1 = background, 2 = random, 3 = trigged
 - YY = the sample number, starting at 01. Don't leave gaps in the numbering otherwise not all files are found at startup.

For the triggered samples inputs:
 - A0 triggers track_301.mp3
 - A1 triggers track_302.mp3
 - A2 triggers track_303.mp3
 - A3 triggers track_304.mp3
 - A4 triggers track_305.mp3

Relay control
----
The relays can simply turned on and of by using:
```
// turn relay on
digitalWrite(relay_ch1, HIGH);

// turn relay off
digitalWrite(relay_ch1, LOW);
```
To use the relay to blink (e.g. a lamp) use
```
//turn on a relay in blink mode
blinkrate_ch1 = N;
digitalWrite(relay_ch1, HIGH);

// turn off a relay in blink mode
blinkrate_ch1 = 0;
digitalWrite(relay_ch1, LOW);
```
 - N = blink rate in 0.1s resolution.

