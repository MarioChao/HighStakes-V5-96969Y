#include "Autonomous/autonPaths.h"

namespace {
using namespace autonpaths;
using namespace autonpaths::pathbuild;

void doAuton();
}

/// @brief Run the rush test.
void autonpaths::runRushTest() {
	/* Pre auton */

	// Timer
	_autonTimer.reset();

	// Set config
	setRobotPosition(0.00, 0.00);
	setRobotRotation(0.00);
	setDifferentialUseRelativeRotation(true);

	// Wait for arm reset
	waitUntil(isArmResetted());


	/* Auton */
	doAuton();
}

namespace {

void doAuton() {
	// Drive forward 2 tiles
	local::driveAndTurn(robotChassis, local::driveAndTurn_params(2_tiles, robotChassis.getLookRotation()), false);
}

}
