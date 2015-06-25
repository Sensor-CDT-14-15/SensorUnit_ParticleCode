#include "math.h"

const int PIR_PIN         = A4;
const int NOISE_PIN       = A5;
const int PHOTOMETER_PIN  = A6;
const int TEMPERATURE_PIN = A7;
const int ARRAY_LENGTH    = 500;

double temperature_celsius, temperature_voltage, light_voltage, raw_pir_reading, noise_voltage, noise_maximum, noise_average, noise_total, noise_variance, noise_sd;
int thresholded_pir_reading;
char publishString[40];
double noise_array[ARRAY_LENGTH];

bool DEBUG_MODE = false;


void setup() {
	pinMode(TEMPERATURE_PIN, INPUT);
	pinMode(PHOTOMETER_PIN, INPUT);
	pinMode(PIR_PIN, INPUT);
	pinMode(NOISE_PIN, INPUT);
	Serial.begin(9600);
	Spark.function("debug", set_debug_mode);
}

void loop() {
	measure_pir_and_noise();
	noise_analysis();
	publish_measurements();
}


void publish_measurements() {
	temperature_voltage = analogRead(TEMPERATURE_PIN);
	// Convert DAC reading to millivolts
	temperature_voltage = (temperature_voltage * 3.3 * 1000) / 4095;
	// Convert millivolts to Celsius using datasheet equation
	temperature_celsius = (2103 - temperature_voltage) / 10.9;
	if(DEBUG_MODE) {
		Serial.println("Temperature (Celsius): " + String(temperature_celsius));
	}

	light_voltage = analogRead(PHOTOMETER_PIN);

	sprintf(publishString,"%.1f, %.1f, %d, %.1f, %.1f, %.1f", temperature_celsius, light_voltage, thresholded_pir_reading, noise_maximum, noise_average, noise_variance);
	Spark.publish("measurements", publishString);
}


void measure_pir_and_noise() {
	thresholded_pir_reading = 0;
	noise_voltage = 0;

	for (int i = 0; i < ARRAY_LENGTH; i++) {
		raw_pir_reading = analogRead(PIR_PIN);
		noise_voltage = analogRead(NOISE_PIN);
		noise_array[i] = noise_voltage;
		if (raw_pir_reading > 3000) {
			thresholded_pir_reading = 1;
		}
		if (DEBUG_MODE) {
			Serial.println("PIR: " + String(raw_pir_reading));
			Serial.println("Noise: " + String(noise_voltage));
		}
		delay(10);
	}
}


void noise_analysis() {
	double residuals[ARRAY_LENGTH];
	noise_maximum = 0;
	noise_average = 0;
	noise_variance = 0;
	noise_total = 0;

	for (int i = 0; i < ARRAY_LENGTH; i++) {
		if (noise_array[i] > noise_maximum) noise_maximum = noise_array[i];
		noise_total += noise_array[i];
	}
	noise_average = noise_total / (ARRAY_LENGTH * 1.0);

	for (int i = 0; i < ARRAY_LENGTH; i++) {
		residuals[i] = noise_array[i] - noise_average;
		noise_variance += residuals[i] * residuals[i];
	}
	noise_variance = noise_variance / (ARRAY_LENGTH * 1.0);
}


int set_debug_mode(String debug) {
	// Convert debug string to lowercase
	for (int i = 0; debug[i]; i++){
		debug[i] = tolower(debug[i]);
	}

	if (debug == "1" || debug == "true") {
		DEBUG_MODE = true;
		return 1;
		} else {
		DEBUG_MODE = false;
	}
	return 0;
}
