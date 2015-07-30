#include "math.h"

const int PIR_PIN         = A4;
const int NOISE_PIN       = A5;
const int PHOTOMETER_PIN  = A6;
const int TEMPERATURE_PIN = A7;
const int ARRAY_LENGTH    = 300;

float temperature_celsius, temperature_voltage, light_voltage, raw_pir_reading, noise_voltage, noise_maximum, noise_average, noise_total, noise_variance, noise_sd, presence_percentage, num_consecutive_runs;
char publishString[255];
float noise_array[ARRAY_LENGTH];
int pir_array[ARRAY_LENGTH];

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
	pir_analysis();
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

	sprintf(publishString,"temperature: %.1f, light: %.1f, noise-max: %.1f, noise-avg: %.1f, noise-var: %.1f, pir-percentage: %.1f, num-consecutive-runs: %.1f", temperature_celsius, light_voltage, noise_maximum, noise_average, noise_variance, presence_percentage, num_consecutive_runs);
	Spark.publish("measurements", publishString);
}


void measure_pir_and_noise() {
	noise_voltage = 0;

	for (int i = 0; i < ARRAY_LENGTH; i++) {
		raw_pir_reading = analogRead(PIR_PIN);
		noise_voltage = analogRead(NOISE_PIN);
		noise_array[i] = noise_voltage;
		if (raw_pir_reading > 3000) {
			pir_array[i] = 1;
		} else {
			pir_array[i] = 0;
		}
		if (DEBUG_MODE) {
			Serial.println("PIR: " + String(raw_pir_reading));
			Serial.println("Noise: " + String(noise_voltage));
		}
		delay(1000);
	}
}


void noise_analysis() {
	float residuals[ARRAY_LENGTH];
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


void pir_analysis() {
	int counts_over_1s = 0;
	int longest_consecutive_run = 0;
	int curr_num_consecutive = 0;

	counts_over_1s = pir_array[0] == 1 ? 1 : 0;
	for (int i = 1; i < ARRAY_LENGTH; i++) {
		if (pir_array[i] == 1) {
			++counts_over_1s;
		}
		if (pir_array[i] == 1 && pir_array[i-1] == 1) {
			++curr_num_consecutive;
		} else {
			if (curr_num_consecutive > longest_consecutive_run) {
				longest_consecutive_run = curr_num_consecutive;
				curr_num_consecutive = 0;
			}
		}
	}

	presence_percentage = 100 * counts_over_1s * 1.0 / ARRAY_LENGTH;
	num_consecutive_runs = longest_consecutive_run;
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
