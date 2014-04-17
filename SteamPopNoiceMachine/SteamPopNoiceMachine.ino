#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h> 
#include <SFEMP3Shield.h>
#include <Logging.h>
#include <TimerOne.h>

SdFat sd;
SFEMP3Shield NoiceMachine;

#define LOG_TRACE	0
#define LOG_DEBUG	1
#define LOG_INFO	2
#define LOG_WARN	3
#define LOG_ERROR	4

#define SAMPLE_BASE_TRIGGERED	100
#define SAMPLE_BASE_RANDOM		200
#define SAMPLE_BASE_BACKGROUND	300

#define VOLUME_SAMPLE_TRIGGERED		0		//   0dB
#define VOLUME_SAMPLE_RANDOM		20		// -10dB
#define VOLUME_SAMPLE_BACKGROUND	30		// -15dB

#define SAMPLE_TIME_MIN		5
#define SAMPLE_TIME_MAX		20

#define LOGLEVEL LOG_LEVEL_DEBUG

int numberOfTriggeredSamples = 0;
int numberOfRandomSamples = 0;
int numberOfBackGroundSamples = 0;

int sampleNumber;
int previousTrigger = 1;
long lastCheck;
long randSampleTime;
long lastRandomSampleTime = 0;
uint32_t resumeAtPosition = 0;
uint8_t sampleVolume = 0xFE; // silence

int trigger0 = A0;
int trigger1 = A1;
int trigger2 = A2;
int trigger3 = A3;
int trigger4 = A4;
int potmeter = A5;

int relay_ch1 = 10;
int relay_ch2 = 5;
int relay_ch3 = 4;
int relay_ch4 = 3;

int blink_rate_ch1 = 0;
int blink_rate_ch2 = 0;
int blink_rate_ch3 = 0;
int blink_rate_ch4 = 0;


void setup() {
	randomSeed(analogRead(0));

	Log.Init(LOGLEVEL, 38400L); // Set log level and baud-rate
	Log.Info("Steam Pop Noice Machine started"CR);

	sd.begin(SD_SEL, SPI_HALF_SPEED);
	NoiceMachine.begin();

	//Setup the triggers as inputs
	pinMode(trigger0, INPUT);
	pinMode(trigger1, INPUT);
	pinMode(trigger2, INPUT);
	pinMode(trigger3, INPUT);
	pinMode(trigger4, INPUT);

	//Setup the potmeter as inputs
	pinMode(potmeter, INPUT);

	//Setup the relay channels as outputs
	pinMode(relay_ch1, OUTPUT);
	pinMode(relay_ch2, OUTPUT);
	pinMode(relay_ch3, OUTPUT);
	pinMode(relay_ch4, OUTPUT);
	
	//Enable pullups on triggers
	digitalWrite(trigger0, HIGH);
	digitalWrite(trigger1, HIGH);
	digitalWrite(trigger2, HIGH);
	digitalWrite(trigger3, HIGH);
	digitalWrite(trigger4, HIGH);

	//Turn off all relay channels
	digitalWrite(relay_ch1, LOW);
	digitalWrite(relay_ch2, LOW);
	digitalWrite(relay_ch3, LOW);
	digitalWrite(relay_ch4, LOW);

	scanMedia();

	sampleNumber = 1;
	previousTrigger = 255;
	resetRandomSampleTime();

	// initialize timer1, and set a 100ms period
	Timer1.initialize(100000);
	Timer1.attachInterrupt(timerInterrupt);
}


void loop(){
	int triggerNumber = 255;
	
	while(triggerNumber == 255) {
		if (lastRandomSampleTime + randSampleTime * 1000 < millis()) {
			playRandomSample();
			resetRandomSampleTime();
		}

		if(!NoiceMachine.isPlaying()) {
			playBackgroundSample();
		}
		triggerNumber = checkTriggers();	
	}
	
	playTriggedSample(triggerNumber);
}


void playBackgroundSample() {
	playSample(SAMPLE_BASE_BACKGROUND + 2, VOLUME_SAMPLE_BACKGROUND);
	Log.Info("Playing background Sample"CR);
}


void playRandomSample() {
	int sampleNumber = SAMPLE_BASE_RANDOM + random(1, numberOfRandomSamples+1);
	playSample(sampleNumber, VOLUME_SAMPLE_RANDOM);
	Log.Info("Playing random Sample #%d"CR, sampleNumber);
}


void playTriggedSample(int triggerNumber){
	static int pause = 0;

	switch (triggerNumber) {
    case 0:
		sampleNumber = SAMPLE_BASE_TRIGGERED + 1;
		pause = 10;
		break;
    case 1:
		sampleNumber = SAMPLE_BASE_TRIGGERED + 2;
		pause = 0;
		break;
    case 2:
		sampleNumber = SAMPLE_BASE_TRIGGERED + 3;
		pause = 0;
		break;
	case 3:
		sampleNumber = SAMPLE_BASE_TRIGGERED + 4;
		pause = 0;
		break;
	case 4:
		sampleNumber = SAMPLE_BASE_TRIGGERED + 5;
		pause = 0;
		break;		
    default: 
		pause = 0;
	}

	playSample(sampleNumber, VOLUME_SAMPLE_TRIGGERED);
	Log.Info("Playing triggered Sample #%d"CR, triggerNumber);

	blink_rate_ch1 = 10;
	digitalWrite(relay_ch1, HIGH);

	while (NoiceMachine.isPlaying()) {}
	Log.Debug("Paused for %ds"CR, pause);

	delay(pause * 1000);

	blink_rate_ch1 = 0;
	digitalWrite(relay_ch1, LOW);	

	resetRandomSampleTime();
}


void timerInterrupt()
{
	static int chan_1 = 0;
	static int chan_2 = 0;
	static int chan_3 = 0;
	static int chan_4 = 0;

	if (blink_rate_ch1 != 0 && ++chan_1 > blink_rate_ch1) {
		digitalWrite(relay_ch1, digitalRead(relay_ch1) ^ 1);
		chan_1 = 0;
	}
	if (blink_rate_ch2 != 0 && ++chan_2 > blink_rate_ch2) {
		digitalWrite(relay_ch2, digitalRead(relay_ch2) ^ 1);
		chan_2 = 0;
	}
	if (blink_rate_ch3 != 0 && ++chan_3 > blink_rate_ch3) {
		digitalWrite(relay_ch3, digitalRead(relay_ch3) ^ 1);
		chan_3 = 0;
	}
	if (blink_rate_ch4 != 0 && ++chan_4 > blink_rate_ch4) {
		digitalWrite(relay_ch4, digitalRead(relay_ch4) ^ 1);
		chan_4 = 0;
	}

	if (blink_rate_ch1 == 0) { chan_1 = 0; }
	if (blink_rate_ch2 == 0) { chan_2 = 0; }
	if (blink_rate_ch3 == 0) { chan_3 = 0; }
	if (blink_rate_ch4 == 0) { chan_4 = 0; }
}


void fadeOut() {
	uint8_t initialVolume = NoiceMachine.getVolume();
	uint8_t vol = initialVolume;
	while (vol < 100) {
		vol += 2;
		NoiceMachine.setVolume(vol);
		delay(5);
	}
}


void fadeIn(uint8_t volume) {
	uint8_t vol = NoiceMachine.getVolume();
	while (vol > volume) {
		vol -= 2;
		NoiceMachine.setVolume(vol);
		delay(5);
	}
}


void playSample(int sampleNumber, uint8_t volume) {
	char fileName[12];
	setFileName(fileName, sampleNumber);

	if(NoiceMachine.isPlaying()) {
		NoiceMachine.stopTrack();
		Log.Verbose("Forced running sample to stop"CR);
	}
	
	sampleVolume = volume;
	setOutputVolume();
	NoiceMachine.playMP3(fileName);
	Log.Debug("Started sample \"%s\""CR, fileName);
	
}

void setOutputVolume() {
	uint8_t pot = 127-(analogRead(potmeter)/8);
	uint8_t vol;
	if (pot + sampleVolume < 255) {
		vol = pot + sampleVolume;
	} else {
		vol = 0xFE;
	}
	NoiceMachine.setVolume(vol);
	Log.Debug("Volume: -%ddB"CR, vol/2);
}

void resetRandomSampleTime() {
	randSampleTime = random(SAMPLE_TIME_MIN, SAMPLE_TIME_MAX);
	lastRandomSampleTime = millis();
	Log.Debug("Next random sample in %ds"CR, randSampleTime);
}


void scanMedia() {
	Log.Info("Scanning media"CR);
	uint8_t initialVolume = NoiceMachine.getVolume();
	NoiceMachine.setVolume(0xFEFE); // mute
	
	numberOfTriggeredSamples = scanMediaRange(SAMPLE_BASE_TRIGGERED);
	Log.Debug("Found %d triggered samples"CR, numberOfTriggeredSamples);

	numberOfRandomSamples = scanMediaRange(SAMPLE_BASE_RANDOM);
	Log.Debug("Found %d random samples"CR, numberOfRandomSamples);

	numberOfBackGroundSamples = scanMediaRange(SAMPLE_BASE_BACKGROUND);
	Log.Debug("Found %d background samples"CR, numberOfBackGroundSamples);
		
	NoiceMachine.setVolume(initialVolume);
}


int scanMediaRange(int sampleNumber) {
	int fileCount = 0;
	bool found = true;
	char fileName[12];
	do {
		setFileName(fileName, ++sampleNumber);
		found = !NoiceMachine.playMP3(fileName);
		if(found) {
			NoiceMachine.stopTrack();
			fileCount++;
			Log.Verbose("Found %s"CR, fileName);
		}
	} while (found);
	return fileCount;
}


void setFileName(char *sampleName ,int sampleNumber) {
	sprintf(sampleName, "track%03d.mp3", sampleNumber);
}


int checkTriggers(void) {
	#define DEBOUNCE  50
	#define TRIGGER_LOCK 10000

	int foundTrigger = 255;

	if( (previousTrigger != 255) && (millis() - lastCheck) > TRIGGER_LOCK) {
		lastCheck = millis();
		previousTrigger = 255;
		Log.Debug("Triggerlock released"CR);
	}

	if(digitalRead(trigger0) == LOW){ 
		delay(DEBOUNCE); 
		if(digitalRead(trigger0) == LOW) foundTrigger = 0;
	}
	else if(digitalRead(trigger1) == LOW ){ 
		delay(DEBOUNCE); 
		if(digitalRead(trigger1) == LOW) foundTrigger = 1;
	}
	else if(digitalRead(trigger2) == LOW){ 
		delay(DEBOUNCE); 
		if(digitalRead(trigger2) == LOW) foundTrigger = 2;
	}
	else if(digitalRead(trigger3) == LOW){ 
		delay(DEBOUNCE); 
		if(digitalRead(trigger3) == LOW) foundTrigger = 3;
	}
	else if(digitalRead(trigger4) == LOW){ 
		delay(DEBOUNCE); 
		if(digitalRead(trigger4) == LOW) foundTrigger = 4;
	}
	//else if(digitalRead(trigger5) == LOW){ 
		//delay(DEBOUNCE); 
		//if(digitalRead(trigger5) == LOW) foundTrigger = 5;
	//}
	
	if(foundTrigger != previousTrigger) {
		previousTrigger = foundTrigger;
		Log.Debug("Trigger #%d detected"CR, foundTrigger);
		return(foundTrigger);
	}
	else {
		return(255);
	}
}
