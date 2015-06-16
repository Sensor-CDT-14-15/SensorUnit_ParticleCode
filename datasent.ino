const int PIR_PIN         = A4;
const int NOISE_PIN       = A5;
const int PHOTOMETER_PIN  = A6;
const int TEMPERATURE_PIN = A7;

double temperature_celsius, temperature_voltage, light_voltage, raw_pir_reading, noise_voltage, maximum_noise;
int thresholded_pir_reading;
char publishString[40];

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
	publish_measurements();
}


void publish_measurements() {
	temperature_voltage = analogRead(TEMPERATURE_PIN);
	// Convert DAC reading to millivolts
	temperature_voltage = (temperature_voltage * 3.3 * 1000) / 4095;
	// Convert millivolts to Celsius using datasheet equation
	temperature_celsius = (2103 - temperature_voltage) / 10.9;
	if(DEBUG_MODE) {
		Serial.println(temperature_celsius);
	}

	light_voltage = analogRead(PHOTOMETER_PIN);

	sprintf(publishString,"%.1f, %.1f, %d, %.1f", temperature_celsius, light_voltage, thresholded_pir_reading, maximum_noise);
	Spark.publish("TL", publishString);
}


void measure_pir_and_noise() {
	thresholded_pir_reading = 0;
	noise_voltage = 0;
	maximum_noise = 0;
	for (int i = 0; i < 500; i++) {
		raw_pir_reading = analogRead(PIR_PIN);
		noise_voltage = analogRead(NOISE_PIN);
		if (raw_pir_reading > 3000) {
			thresholded_pir_reading = 1;
		}
		if (noise_voltage > maximum_noise) {
			maximum_noise = noise_voltage;
		}
		if (DEBUG_MODE) {
			Serial.println(raw_pir_reading);
			Serial.println(noise_voltage);
		}
		delay(10);
	}
}


void set_debug_mode(String debug) {
	// Convert debug string to lowercase
	for (int i = 0; debug[i]; i++){
		debug[i] = tolower(debug[i]);
	}

	if (debug == "1" || debug == "true") {
		DEBUG_MODE == true;
	} else {
		DEBUG_MODE = false;
	}
}
