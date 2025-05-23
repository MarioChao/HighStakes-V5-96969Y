#include "MatchSequence/preauton.h"

#include "Cosmetics/Graphics/BrainScreen.h"
#include "Utilities/debugFunctions.h"
#include "Cosmetics/Videos/video-main.h"
#include "global-vars.h"
#include "main.h"


namespace {
void initComponents();
void bufferScreen();

bool initComponentFinished = false;
bool bufferFinished = false;
}


namespace preauton {
void controllerThread() {
	debug::printOnController("No auton selected!");
}

void run() {
	// Buffer screen task
	task bufferTask([]() -> int {
		bufferScreen();
		return 1;
	});

	// Initialize components
	initComponents();
}

void calibrateIMU() {
	// Calibrate inertial sensor
	InertialSensor.startCalibration();
	wait(30, msec);
	while (InertialSensor.isCalibrating()) {
		wait(30, msec);
	}
	printf("Calibrated!\n");
}

bool isFinished() {
	return initComponentFinished;
}

bool isBufferFinished() {
	return bufferFinished;
}
}


namespace {
void initComponents() {
	initComponentFinished = false;

	// Calibrate inertial sensor
	preauton::calibrateIMU();

	// Increase optical sensor brightness
	RingOpticalSensor.setLight(ledState::on);
	RingOpticalSensor.setLightPower(100);
	printf("Optical on\n");

	// Configure sensor
	RingOpticalSensor.integrationTime(10);

	initComponentFinished = true;
}

void bufferScreen() {
	bufferFinished = false;

	// White / buffer screen
	Brain.Screen.clearScreen(color::white);
	Brain.Screen.setPenColor(color::black);
	Brain.Screen.setFillColor(color::white);
	timer bufferTimer;
	bufferTimer.reset();
	// printf("Competition: %d, Enabled: %d\n", Competition.isAutonomous(), Competition.isEnabled());
	while (!Competition.isAutonomous() && !Competition.isEnabled() && bufferTimer.value() <= 1) {
		Brain.Screen.printAt(240, 120, "%5.2f sec", bufferTimer.value());
		task::sleep(20);
	}

	// Shrinking circle
	Brain.Screen.setFillColor(color::transparent);
	Brain.Screen.setPenWidth(1);
	Brain.Screen.setPenColor(color::black);
	double prevR, newR;
	prevR = -1;
	for (double i = 0; ; i += 0.03) {
		if (i > 1) i = 1;
		newR = 275 - pow(i, 2) * 275;
		if (prevR == -1) prevR = newR + 1;
		// Draw ring
		Brain.Screen.setPenWidth(2 * fabs(newR - prevR));
		Brain.Screen.drawCircle(240, 120, newR);
		prevR = newR;
		task::sleep(20);
		// Exit when i is 1
		if (fabs(i - 1) <= 1e-8) break;
	}

	// Brain Screen
	video::startThread();
	task brainScreen([]() -> int {
		brainscreen::brainScreenThread();
		return 1;
	});

	bufferFinished = true;
}
}
