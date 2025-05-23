#include "Autonomous/autonPaths.h"


namespace {
using pas1_lib::planning::splines::SplineCurve;
using pas1_lib::planning::splines::CurveSampler;
using pas1_lib::planning::trajectories::TrajectoryPlanner;
using pas1_lib::planning::profiles::SplineProfile;
}


namespace autonpaths {
namespace pathbuild {
// Build paths

void storeNewSplineProfile(std::string profileName, SplineCurve spline, bool reverse, std::vector<TrajectoryConstraint *> constraints, double maxVel) {
	if (splineProfile_storage.hasKey(profileName)) {
		// printf("Profile '%s' already exists!\n", profileName.c_str());
		return;
	}

	CurveSampler curveSampler = CurveSampler(spline).calculateByResolution(spline.getTRange().second * 10);
	double totalDistance = curveSampler.getDistanceRange().second;
	double distanceStep = aespa_lib::genutil::clamp(totalDistance / 64, 0.077, 0.5);
	TrajectoryPlanner splineTrajectoryPlan = TrajectoryPlanner(totalDistance * (reverse ? -1 : 1), botInfo.trackWidth_tiles, distanceStep)
		.setCurvatureFunction(
			[&](double d) -> double {
		return spline.getCurvatureAt(curveSampler.distanceToParam(d));
	},
			curveSampler.integerParamsToDistances()
		)
		.setSpline(&curveSampler)
		.maxSmoothCurvature()
		.addCenterConstraint_maxMotion({ maxVel, botInfo.maxAccel_tilesPerSec2 })
		.addTrackConstraint_maxMotion({ maxVel, botInfo.maxAccel_tilesPerSec2 * 0.5 })
		.addCenterConstraint_maxCentripetalAcceleration(botInfo.maxAccel_tilesPerSec2 * 0.3)
		.addCenterTrajectoryConstraints(constraints)
		.calculateMotionProfile();
	splineProfile_storage.store(profileName, SplineProfile(spline, curveSampler, splineTrajectoryPlan, reverse));
}

void storeNewSplineProfile(std::string profileName, pas1_lib::planning::splines::SplineCurve spline, bool reverse, std::vector<TrajectoryConstraint *> constraints) {
	storeNewSplineProfile(profileName, spline, reverse, constraints, botInfo.maxVel_tilesPerSec);
}

void runFollowSpline(Differential &chassis, std::string profileName, bool turnFirst) {
	if (!splineProfile_storage.hasKey(profileName)) {
		follow::_isRamsetePathFollowCompleted = true;
		printf("Profile %s not stored!\n", profileName.c_str());
		return;
	}
	SplineProfile *splineProfile = splineProfile_storage.getStored(profileName).get();
	if (turnFirst) {
		aespa_lib::datas::Linegular startPose = splineProfile->spline.getLinegularAt(0, splineProfile->willReverse);
		local::turnToAngle(chassis, local::turnToAngle_params(startPose.getRotation()), true);
		waitUntil(local::_turnAngleError_degrees < 5);
	}
	follow::ramseteFollowPath(chassis, follow::ramseteFollowPath_params(splineProfile), true);
}

void runFollowSpline(std::string profileName, bool turnFirst) {
	runFollowSpline(robotChassis, profileName, turnFirst);
}


std::vector<std::vector<std::vector<aespa_lib::units::Length>>> linearPaths;
std::vector<double> linearMaxVelocity_pct;
std::vector<bool> linearWillReverse;

int linearIndex;

void clearLinear() {
	linearPaths.clear();
	linearMaxVelocity_pct.clear();
	linearWillReverse.clear();
	linearIndex = 0;
}

void pushNewLinear(std::vector<std::vector<aespa_lib::units::Length>> path, bool reverse, double maxVelocity_pct) {
	linearPaths.push_back(path);
	linearMaxVelocity_pct.push_back(maxVelocity_pct);
	linearWillReverse.push_back(reverse);
}

void runFollowLinearYield() {
	runLinearPIDPath(linearPaths[linearIndex], linearMaxVelocity_pct[linearIndex], linearWillReverse[linearIndex]);
	linearIndex++;
}
}
}
