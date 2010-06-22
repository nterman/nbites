from . import BrunswickSpeeds as speeds

#navstates.py
GOTO_FORWARD_SPEED = speeds.MAX_X_SPEED
GOTO_BACKWARD_SPEED = speeds.MIN_X_SPEED
GOTO_STRAFE_SPEED = speeds.MAX_Y_SPEED
GOTO_SPIN_SPEED = speeds.MAX_SPIN_SPEED
GOTO_STRAIGHT_SPIN_SPEED = speeds.MAX_SPIN_SPEED
GOTO_SPIN_STRAFE = 0
GOTO_SURE_THRESH = 3

WALK_TO_MAX_X_SPEED = speeds.MAX_X_SPEED
WALK_TO_MIN_X_SPEED = speeds.MIN_X_SPEED
WALK_TO_MAX_SPIN_SPEED = speeds.MAX_SPIN_WHILE_X_SPEED
WALK_TO_MIN_SPIN_SPEED = -WALK_TO_MAX_SPIN_SPEED

OMNI_GOTO_FORWARD_SPEED = 5
OMNI_GOTO_STRAFE_SPEED = 7
GOTO_FORWARD_GAIN = 0.05
GOTO_STRAFE_GAIN = 0.5
GOTO_SPIN_GAIN = 0.8
OMNI_GOTO_SPIN_GAIN = 1.0
OMNI_GOTO_X_GAIN = .2
OMNI_GOTO_Y_GAIN = .3

OMNI_MAX_X_SPEED = 10.0
OMNI_MIN_X_SPEED = -OMNI_MAX_X_SPEED
OMNI_MAX_Y_SPEED = 8.0
OMNI_MIN_Y_SPEED = -OMNI_MAX_Y_SPEED
OMNI_MAX_SPIN_SPEED = speeds.MAX_SPIN_SPEED
OMNI_MIN_SPIN_SPEED = -OMNI_MAX_SPIN_SPEED

OMNI_MIN_X_MAGNITUDE = speeds.MIN_X_MAGNITUDE
OMNI_MIN_Y_MAGNITUDE = speeds.MIN_Y_MAGNITUDE
OMNI_MIN_SPIN_MAGNITUDE = speeds.MIN_SPIN_MAGNITUDE

CHANGE_SPIN_DIR_THRESH = 6

# orbitPoint values
ORBIT_SPIN_SPEED = -20
ORBIT_STRAFE_SPEED = 8

ORBIT_LEFT = -1
ORBIT_RIGHT = 1
ORBIT_SMALL_ANGLE = 35
ORBIT_SMALL_GAIN = .5
ORBIT_MID_GAIN = 1
ORBIT_LARGE_ANGLE = 90
ORBIT_LARGE_GAIN = 1
MIN_ORBIT_ANGLE = 10

#navigator.py
LOC_IS_ACTIVE_H  = 720
CLOSE_ENOUGH_XY = 25.0
CLOSER_XY = 8.0
CLOSE_ENOUGH_H = 10.0
ALMOST_CLOSE_ENOUGH_H = 30.0

AT_HEADING_GOTO_DEG = 20

HEADING_NEAR_THRESH = 10.
HEADING_MEDIUM_THRESH = 30.

HEADING_NEAR_SCALE = 0.3
HEADING_MEDIUM_SCALE = 0.4
HEADING_FAR_SCALE = .6
SPIN_EPSILON = 2.0
FORWARD_EPSILON = 0.3
STRAFE_EPSILON = 0.5

POSITION_NEAR_THRESH = CLOSER_XY
POSITION_MEDIUM_THRESH = CLOSE_ENOUGH_XY
POSITION_NEAR_SCALE = 0
POSITION_MEDIUM_SCALE = 0.6
POSITION_FAR_SCALE = 1.0

FRAME_RATE = 30       # Rough estimate of fps

MIN_SPIN_SPEED = speeds.MIN_SPIN_MAGNITUDE
MIN_SPIN_MAGNITUDE_WALK = speeds.MIN_SPIN_WHILE_X_MAGNITUDE

#
FINAL_HEADING_DIST = 100
FINAL_HEADING_READY_DIST = 60
HEADING_THRESHOLD_TO_SPIN = 60
FRAMES_THRESHOLD_TO_SPIN = 30
FRAMES_THRESHOLD_TO_POSITION_OMNI = 30
FRAMES_THRESHOLD_TO_POSITION_PLAYBOOK = 30

# turnToBall
FIND_BALL_SPIN_SPEED = speeds.MAX_SPIN_SPEED
BALL_SPIN_SPEED = speeds.MAX_SPIN_SPEED
BALL_SPIN_GAIN = 0.9
MIN_BALL_SPIN_MAGNITUDE = speeds.MIN_SPIN_MAGNITUDE

# approachBall() values
APPROACH_X_GAIN = 0.1
APPROACH_SPIN_SPEED = speeds.MAX_SPIN_WHILE_X_SPEED
MIN_APPROACH_SPIN_MAGNITUDE = speeds.MIN_SPIN_MAGNITUDE
APPROACH_SPIN_GAIN = 1.1
MAX_APPROACH_X_SPEED = speeds.MAX_X_SPEED
MIN_APPROACH_X_SPEED = speeds.MIN_X_SPEED
APPROACH_WITH_GAIN_DIST = 50

# approachBallWithLoc() values
IN_FRONT_SLOPE = 5.6
APPROACH_DIST_TO_BALL = 25
APPROACH_NO_LOC_THRESH = 4
APPROACH_NO_MORE_LOC_DIST = 150
APPROACH_OMNI_DIST = 25
APPROACH_ACTIVE_LOC_DIST = 65
APPROACH_ACTIVE_LOC_BEARING = 40

# chaseAroundBox
GOALBOX_OFFSET = 10
STOP_CHASING_AROUND_BOX = 5
IGNORE_BALL_IN_BOX_DIST = 100

# Obstacle avoidance stuff
SHOULD_AVOID_OBSTACLE_APPROACH_DIST = 50.0
AVOID_OBSTACLE_FRONT_DIST = 40.0 #cm
AVOID_OBSTACLE_SIDE_DIST = 30.0 #cm
AVOID_OBSTACLE_FRAMES_THRESH = 2
DONE_AVOIDING_FRAMES_THRESH = 10
DODGE_BACK_SPEED = speeds.MIN_X_SPEED
DODGE_RIGHT_SPEED = speeds.MIN_Y_SPEED
DODGE_LEFT_SPEED = speeds.MAX_Y_SPEED
