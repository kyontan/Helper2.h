#include <Helper2.h>
#include <Wire.h>

void setup() {
	Serial.begin(9600);
	initialize();
	accel.debugPrintThreshold();

	led1.on();
	led2.color(0);
	led2.off();
	led2.color(0.5);
}

void loop() {
	if (accel.tap()) {
		led1.randomColor();
	}

	if (accel.doubletap()) {
		led2.flip();
	}

	accel.debugInputThreshold();

	wait(200);
};
