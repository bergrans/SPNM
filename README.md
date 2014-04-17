Steam Pop Noise Machine
====

The Steam Pop Noise Machine (SPNM) is part of the [Steam Pop Machine](http://www.steampopmachine.com/) project. The Steam Pop Machine is a large mobile popcorn  machine build in [Steampunk style](http://nl.wikipedia.org/wiki/Steampunk). This machine is used for selling popcorn on huge festivals/events in an atractive/interactive way. To make the machine more realistic some background-, random- and trigged noises are used to spice things up. That's what the SPNM is for.

A 4 channel relay board is added to control some more powerfull stuff like lights or motors. 

![picture alt](https://raw.githubusercontent.com/bergrans/SPNM/master/images/spnm-intern.jpg "Internal view")

The hardware:
----
 - Arduino UNO controller http://arduino.cc/en/Main/ArduinoBoardUno
 - MP3 Player Shield by Sparkfun https://www.sparkfun.com/products/10628
 - 4 channel relay board https://iprototype.nl/products/components/buttons-switches/relay-board-4-channels-5v
 - Power Supply 12/3A

I/O
----
 - `A0`, `A1`, `A2`, `A3`, `A4` inputs for triggered samples
 - `A5` Potentiometer input used for volume setting
 - `10`, `3`, `4`, `5` relay channel 1...4

####Remarks
Pin `10` seems to be used by the SPI library during startup. Causing the relay on this channel to switch shortly. Since the pin is not used by the MP3 shield there is no problem in using it for general IO.

Used software libraries
----
 - [SFEMP3Shield](https://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library) 
 - [TimerOne](https://code.google.com/p/arduino-timerone/downloads/list) creating a 100ms timmer interrupt.

MP3 samples
----
The SPNM uses three types of samples:
 1. Background noises (played continious in random order)
 2. Random noises (played in random order at random intervals)
 3. Trigged noises (played when trigged by an external switch)

The samples are stored, in the root, on a SD card with the following naming convention:
```
trackXYY.mp3
```
Where:
 - `X` = the sample type. 1 = background, 2 = random, 3 = trigged
 - `YY` = the sample number, starting at 01. Don't leave gaps in the numbering otherwise not all files are found at startup.

For the triggered samples inputs:
 - `A0` triggers `track301.mp3`
 - `A1` triggers `track302.mp3`
 - `A2` triggers `track303.mp3`
 - `A3` triggers `track304.mp3`
 - `A4` triggers `track305.mp3`

Relay control
----
The relays can simply turned on and of by using:
```c++
// turn relay on
digitalWrite(relay_ch1, HIGH);

// turn relay off
digitalWrite(relay_ch1, LOW);
```
To use the relay to blink (e.g. a lamp) use
```c++
//turn on a relay in blink mode
blinkrate_ch1 = N;
digitalWrite(relay_ch1, HIGH);

// turn off a relay in blink mode
blinkrate_ch1 = 0;
digitalWrite(relay_ch1, LOW);
```
 - N = blink rate in 0.1s resolution.

