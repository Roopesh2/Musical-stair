/**
 * piano-sketch.ino
 * Author: Bonnie Eisenman
 * twitter.com/brindelle
 *
 * Arduino-side code for taking in inputs from several sensors,
 * and if they pass the thresholds, send information to
 * Serial for processing.
 *
 * This is meant to run on a Teensy, with 8 analog inputs.
 * Make sure to adjust analogPins[] and numAnalogPins below
 * if you want to run this on e.g. an Arduino Uno,
 * which has only 6 analog inputs.
 **/

int LED_PIN = 13;
float analogPins[] = {A0, A1, A2, A3, A4, A5};
int numAnalogPins = 6;
int thresholds[6];

// This value can be tuned depending on sensor installation.
// It will likely vary based on: ambient light conditions;
// positioning of the sensors; resistor values used; etc.
int absoluteThresh = 20;

// Keep track of the last N x H sensor readings,
// where N = number of inputs
// and H = history length.
// This allows us to do smoothing.
int sensorHistory[6][10];

// This is used to cycle through the sensorHistory array
// along the H dimension.
int bufferIndex = 0;


// Keep track of the previous values so that we can do smoothing
int prevInputs[] = {
	0,
	0,
	0,
	0,
	0,
	0,
};

// note files
char *notes[] = {
	"samples/c1.wav",
	"samples/d.wav",
	"samples/e.wav",
	"samples/f.wav",
	"samples/g.wav",
	"samples/a.wav",
	"samples/b.wav",
	"samples/c.wav"
};

void setup() {
	Serial.begin(9600);
	calibrate();
	pinMode(LED_PIN, OUTPUT);
}

void calibrate() {
	// How many steps' worth of data we use for calibration purposes
	int maxSteps = 10;

	// Begin by accumulating several steps' worth of baseline data
	for (int curStep = 0; curStep < maxSteps; curStep++) {
		for (int pin = 0; pin < numAnalogPins; pin++) {
			 thresholds[pin] += analogRead(analogPins[pin]);
		}
	}

	// Average out the baseline readings based on how many input readings we did
	for (int pin = 0; pin < numAnalogPins; pin++) {
		thresholds[pin] = thresholds[pin] / maxSteps;
	}
	
	// Fill the entire sensorHistory array with baseline data
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 10; j++) {
			 sensorHistory[i][j] = thresholds[i];
		}
	} 
}

void loop() {
	int activation[6];

	// copy previous sensor activity
	for (int i = 0; i < numAnalogPins; i++) {
		activation[i] = prevInputs[i];
	}

	// read sensor status
	for (int i = 0; i < numAnalogPins; i++) {
		int val = analogRead(analogPins[i]);

		// get average of previous sensor data
		int oldAvg = 0;
		for (int j = 0; j < 10; j++) {
			oldAvg += sensorHistory[i][j];
		}
		oldAvg = oldAvg / 10;

		// trigger input if value exceeds threshold
		int triggered = val > oldAvg + absoluteThresh;
		if (triggered) {
			digitalWrite(LED_PIN, HIGH);

			// play tones only if it is not being played
			if (!prevInputs[i]) {
				playTune(i);
			}
		} else {
			digitalWrite(LED_PIN, LOW);
		}

		// update sensor data
		prevInputs[i] = triggered;
		sensorHistory[i][bufferIndex] = val;
	}
	
	bufferIndex = (bufferIndex + 1) % 10;
	
	// print output only if there is change in any sensor values
	int printToSerial = 0;
	for (int i = 0; i < numAnalogPins; i++) {
		if (activation[i] != prevInputs[i]) {
			printToSerial = 1;
			break;
		}
	}
	if (0) {
		for (int i = 0; i < numAnalogPins; i++)
			Serial.print(prevInputs[i]);
		Serial.println();
	}


	delay(20);
}

void playTune(int sensorIndex) {
	Serial.println("-------");
	Serial.print("Playing: ");
	Serial.println(notes[sensorIndex]);
}