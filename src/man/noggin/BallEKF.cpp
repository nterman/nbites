#include "BallEKF.h"
#include "FieldConstants.h"
using namespace boost::numeric;
using namespace boost;
using namespace NBMath;
using namespace std;
using namespace ekf;

// #define DEBUG_BALL_EKF

// How much uncertainty naturally grows per update
const float BallEKF::BETA_BALL              = 5.0f;
const float BallEKF::BETA_BALL_VEL          = 50.0f;
// How much ball velocity should effect uncertainty
const float BallEKF::GAMMA_BALL             = 0.4f;
const float BallEKF::GAMMA_BALL_VEL         = 1.0f;
const float BallEKF::CARPET_FRICTION        = -15; // 25 cm/s^2

// Default initialization values
const float BallEKF::INIT_BALL_X            = 100.0f;
const float BallEKF::INIT_BALL_Y            = 0.0f;
const float BallEKF::INIT_BALL_X_VEL        = 0.0f;
const float BallEKF::INIT_BALL_Y_VEL        = 0.0f;

const float BallEKF::INIT_X_UNCERT          = FIELD_WIDTH / 4.0f;
const float BallEKF::INIT_Y_UNCERT          = FIELD_HEIGHT / 4.0f;
const float BallEKF::INIT_X_VEL_UNCERT      = 300.0f;
const float BallEKF::INIT_Y_VEL_UNCERT      = 300.0f;
const float BallEKF::INIT_X_ACC_UNCERT      = 300.0f;
const float BallEKF::INIT_Y_ACC_UNCERT      = 300.0f;

const float BallEKF::VELOCITY_EST_MAX       = 70.0f;
const float BallEKF::VELOCITY_EST_MIN       = -VELOCITY_EST_MAX;
const float BallEKF::ACC_EST_MAX            = 150.0f;
const float BallEKF::ACC_EST_MIN            = -ACC_EST_MAX;

const float BallEKF::VELOCITY_EST_MIN_SPEED = 0.01f;
const int BallEKF::ERROR_RESET_THRESH = 3;

BallEKF::BallEKF()
    : EKF<RangeBearingMeasurement, MotionModel, ball_ekf_dimension,
          ball_ekf_meas_dim>(BETA_BALL,GAMMA_BALL), robotPose(),
      lastUpdateTime(0), dt(0.0f)
{

    initMatrices();
}


/**
 * Constructor for the BallEKF class
 *
 * @param _initX An initial value for the x estimate
 * @param _initY An initial value for the y estimate
 * @param _initXUncert An initial value for the x uncertainty
 * @param _initYUncert An initial value for the y uncertainty
 * @param _initVelX An initial value for the x velocity estimate
 * @param _initVelY An initial value for the y velocity estimate
 * @param _initVelXUncert An initial value for the x velocity uncertainty
 * @param _initVelYUncert An initial value for the y velocity uncertainty
 */
BallEKF::BallEKF(float initX, float initY,
                 float initVelX, float initVelY,
                 float initXUncert,float initYUncert,
                 float initVelXUncert, float initVelYUncert)
    : EKF<RangeBearingMeasurement, MotionModel, ball_ekf_dimension,
          ball_ekf_meas_dim>(BETA_BALL,GAMMA_BALL)
{

}

/**
 * Initialize the values of certain matrices that are set once and
 * then used repeatedly.
 */
void BallEKF::initMatrices()
{
    /////////////////////
    // Init constant time update Jacobian values
    A_k(x_index     , x_index     ) = 1.0f;
    A_k(x_index     , y_index     ) = 0.0f;
    A_k(x_index     , vel_y_index ) = 0.0f;
    A_k(x_index     , acc_y_index ) = 0.0f;

    A_k(y_index     , x_index     ) = 0.0f;
    A_k(y_index     , y_index     ) = 1.0f;
    A_k(y_index     , vel_x_index ) = 0.0f;
    A_k(y_index     , acc_x_index ) = 0.0f;

    A_k(vel_x_index , x_index     ) = 0.0f;
    A_k(vel_x_index , y_index     ) = 0.0f;
    A_k(vel_y_index , x_index     ) = 0.0f;
    A_k(vel_y_index , y_index     ) = 0.0f;
    A_k(acc_x_index , x_index     ) = 0.0f;
    A_k(acc_x_index , y_index     ) = 0.0f;

    A_k(acc_x_index , vel_x_index ) = 0.0f;
    A_k(acc_x_index , vel_y_index ) = 0.0f;
    A_k(acc_x_index , acc_x_index ) = 1.0f;
    A_k(acc_x_index , acc_y_index ) = 0.0f;

    A_k(acc_y_index , x_index     ) = 0.0f;
    A_k(acc_y_index , y_index     ) = 0.0f;
    A_k(acc_y_index , vel_x_index ) = 0.0f;
    A_k(acc_y_index , vel_y_index ) = 0.0f;
    A_k(acc_y_index , acc_x_index ) = 0.0f;
    A_k(acc_y_index , acc_y_index ) = 1.0f;
    /////////////////////


    /////////////////////
    // Init constant correction step matrices

    // The derivatives of each state are differentiated with respect
    // to themselves only and undergo no further operations, H_k
    // (the Jacobian) is simply the identity matrix.
    H_k = boost::numeric::ublas::identity_matrix<float>(ball_ekf_meas_dim);

    // Constant values of measurement covariance calculations
    R_k(x_index, y_index)         = 0.0f;
    R_k(x_index, vel_x_index)     = 0.0f;
    R_k(x_index, vel_y_index)     = 0.0f;
    R_k(x_index, acc_x_index)     = 0.0f;
    R_k(x_index, acc_y_index)     = 0.0f;

    R_k(y_index, x_index)         = 0.0f;
    R_k(y_index, vel_x_index)     = 0.0f;
    R_k(y_index, vel_y_index)     = 0.0f;
    R_k(y_index, acc_x_index)     = 0.0f;
    R_k(y_index, acc_y_index)     = 0.0f;

    R_k(vel_x_index, x_index)     = 0.0f;
    R_k(vel_x_index, y_index)     = 0.0f;
    R_k(vel_x_index, vel_y_index) = 0.0f;
    R_k(vel_x_index, acc_x_index) = 0.0f;
    R_k(vel_x_index, acc_y_index) = 0.0f;

    R_k(vel_y_index, x_index)     = 0.0f;
    R_k(vel_y_index, y_index)     = 0.0f;
    R_k(vel_y_index, vel_x_index) = 0.0f;
    R_k(vel_y_index, acc_x_index) = 0.0f;
    R_k(vel_y_index, acc_y_index) = 0.0f;

    R_k(acc_x_index, x_index)     = 0.0f;
    R_k(acc_x_index, y_index)     = 0.0f;
    R_k(acc_x_index, vel_x_index) = 0.0f;
    R_k(acc_x_index, vel_y_index) = 0.0f;
    R_k(acc_x_index, acc_y_index) = 0.0f;

    R_k(acc_y_index, x_index)     = 0.0f;
    R_k(acc_y_index, y_index)     = 0.0f;
    R_k(acc_y_index, vel_x_index) = 0.0f;
    R_k(acc_y_index, vel_y_index) = 0.0f;
    R_k(acc_y_index, acc_x_index) = 0.0f;
    /////////////////////


    /////////////////////
    // Set velocity uncertainty parameters
    betas(2) = BETA_BALL_VEL;
    betas(3) = BETA_BALL_VEL;
    gammas(2) = GAMMA_BALL_VEL;
    gammas(3) = GAMMA_BALL_VEL;
    /////////////////////
}

void BallEKF::reset()
{
    // Reset all of the matrices to zeros
    for(unsigned i = 0; i < numStates; ++i) {

        xhat_k(i) = xhat_k_bar(i) = 0.0f;

        for(unsigned j = 0; j < numStates; ++j) {
            A_k(i,j) = Q_k(i,j) = P_k(i,j) = P_k_bar(i,j) = 0.0f;
        }
    }
    // Set the initial values
    P_k(x_index,x_index) = P_k_bar(x_index,x_index) = INIT_X_UNCERT;
    P_k(y_index,y_index) = P_k_bar(y_index,y_index) = INIT_Y_UNCERT;
    P_k(vel_x_index,vel_x_index) =
        P_k_bar(vel_x_index,vel_x_index) = INIT_X_VEL_UNCERT;
    P_k(vel_y_index,vel_y_index) =
        P_k_bar(vel_y_index,vel_y_index) = INIT_Y_VEL_UNCERT;
    P_k(acc_x_index,acc_x_index) =
        P_k_bar(acc_x_index,acc_x_index) = INIT_X_ACC_UNCERT;
    P_k(acc_y_index,acc_y_index) =
        P_k_bar(acc_y_index,acc_y_index) = INIT_Y_ACC_UNCERT;
}

void BallEKF::resetModelTo(const RangeBearingMeasurement& ball)
{
    if (ball.distance > 0.0){
        xhat_k(x_index) = ball.distance * cos(ball.bearing);
        xhat_k(y_index) = ball.distance * sin(ball.bearing);

        xhat_k(vel_y_index) = xhat_k(vel_y_index) = 0.0f;
        xhat_k(acc_y_index) = xhat_k(acc_y_index) = 0.0f;

        // Set the initial values
        P_k(x_index,x_index) = ball.distanceSD * ball.distanceSD;
        P_k(y_index,y_index) = ball.distanceSD * ball.distanceSD;
        P_k(vel_x_index,vel_x_index) = INIT_X_VEL_UNCERT;
        P_k(vel_y_index,vel_y_index) = INIT_Y_VEL_UNCERT;
        P_k(acc_x_index,acc_x_index) = INIT_X_ACC_UNCERT;
        P_k(acc_y_index,acc_y_index) = INIT_Y_ACC_UNCERT;

        errorCounter = 0;
    }
}


/**
 * Method to deal with updating the entire ball model
 *
 * @param ball The ball seen this frame.
 * @param odo  The robot's odometry from the last frame.
 * @param p    The robot's current estimate of its position on the field.
 */
void BallEKF::updateModel(const MotionModel& curOdometry,
                          const RangeBearingMeasurement& ball,
                          const PoseEst& p)
{
    robotPose = p;
    updateFrameLength();

    if (errorCounter > ERROR_RESET_THRESH){
        resetModelTo(ball);
    }

    curDeltaOdometry = curOdometry - lastOdometry;

    // Update expected ball movement
    timeUpdate(curDeltaOdometry);

    // We've seen a ball
    if (ball.distance > 0.0) {
        vector<RangeBearingMeasurement> z;
        z.push_back(ball);
        correctionStep(z);

    } else { // No ball seen
        errorCounter = 0;
        noCorrectionStep();
    }
    limitPosteriorEst();
    limitPosteriorUncert();

    if (testForNaNReset()) {
        cout << "\tBallEKF reset to " << *this << endl;
        cout << "\tObservation was: " << ball << endl;
    }

    lastOdometry = curOdometry;
}

/**
 * Method incorporate the expected change in ball position from the last
 * frame.  Updates the values of the covariance matrix Q_k and the jacobian
 * A_k.
 *
 * @param u The odometry calculations from the last frame
 * @return The expected change in ball position (x,y, xVelocity, yVelocity)
 */
EKF<RangeBearingMeasurement,
    MotionModel, ball_ekf_dimension,
    dist_bearing_meas_dim>::StateVector
BallEKF::associateTimeUpdate(MotionModel deltaOdometry)
{

    StateVector deltaBall(ball_ekf_dimension, 0.0f);

    // Calculate expected ball deltas
    updatePosition              ( deltaOdometry, deltaBall);
    updateVelocity              ( deltaOdometry, deltaBall);
    updateAcceleration          ( deltaOdometry, deltaBall);
    calculateTimeUpdateJacobian ( deltaOdometry, deltaBall);

#ifdef DEBUG_BALL_EKF
    cout << "deltaBall: " << deltaBall << endl
         << "dt: "        << dt        << endl;
#endif

    return deltaBall;
}

/**
 * Update the length of the last frame.
 */
void BallEKF::updateFrameLength()
{
    // Get time since last update
    const long long int time = monotonic_micro_time();
    dt = static_cast<float>(time - lastUpdateTime)/
        1000000.0f; // u_s to sec

    // Guard against a zero dt (maybe possible?)
    if (dt == 0.0){
        dt = 0.0001f;
    }
    lastUpdateTime = time;
}

/**
 * Update the x,y components based on the odometry and the previous state.
 */
void BallEKF::updatePosition(const DeltaMotionModel& deltaOdometry, StateVector& deltaBall)
{
    // Calculate change in position from velocity and acceleration
    // ds = vt + .5*a*t^2 + .5*(friction deceleration)*t^2

    // Also, ensure that the change in direction is in the same
    // direction as the velocity
    if (xhat_k(vel_x_index) > VELOCITY_EST_MIN_SPEED){
        deltaBall(x_index) = (xhat_k(vel_x_index)*dt +
                              .5f * (xhat_k(acc_x_index) +
                                     copysignf(1.0f, vel_x_index) *
                                     CARPET_FRICTION )
                              *dt*dt ); // Friction needs correct sign

        deltaBall(x_index) = xhat_k(vel_x_index) > 0 ?
            max(0.0f, deltaBall(x_index)) :
            deltaBall(x_index) = min(0.0f, deltaBall(x_index));
    }

    if (xhat_k(vel_y_index) > VELOCITY_EST_MIN_SPEED){

        deltaBall(y_index) = (xhat_k(vel_y_index)*dt +
                              .5f * (xhat_k(acc_y_index) +
                                     copysignf(1.0f, vel_y_index) *
                                     CARPET_FRICTION )
                              *dt*dt ); // Friction needs correct sign

        deltaBall(y_index) = xhat_k(vel_y_index) > 0 ?
            max(0.0f, deltaBall(y_index)) :
            deltaBall(y_index) = min(0.0f, deltaBall(y_index));
    }

    // Rotate the position according to odometry
    deltaBall(x_index) += -sin(deltaOdometry.theta) * deltaOdometry.y -
        cos(deltaOdometry.theta) * deltaOdometry.x;
    deltaBall(y_index) += sin(deltaOdometry.theta) * deltaOdometry.x -
        cos(deltaOdometry.theta) * deltaOdometry.y;
}

/**
 * Update the x,y velocity components based on the odometry and the
v * previous state.
 */
void BallEKF::updateVelocity(const DeltaMotionModel& deltaOdometry, StateVector& deltaBall)
{
    // Velocities
    float velX = xhat_k(vel_x_index) +
        xhat_k(acc_x_index) * dt;
    float velY = xhat_k(vel_y_index) +
        xhat_k(acc_y_index) * dt;

    velX = applyFriction(velX);
    velY = applyFriction(velY);

    // Rotate the velocity components into the new coordinate frame
    float estVelX = cos(deltaOdometry.theta) * velX + sin(deltaOdometry.theta) * velY;
    float estVelY = cos(deltaOdometry.theta) * velY - sin(deltaOdometry.theta) * velX;

    // Change in velocity
    deltaBall(vel_x_index) = estVelX - xhat_k(vel_x_index);
    deltaBall(vel_y_index) = estVelY - xhat_k(vel_y_index);
}

/**
 * Deceleration of ball due to friction (physics! sort of).  This is
 * not the proper method of slowing an object. The magnitude should be
 * slowed by the friction amount, not the components individually.
 */
float BallEKF::applyFriction(float vel)
{
    const float friction_decel = CARPET_FRICTION * dt;

    if ( abs(friction_decel) < abs(vel) ) {
        return vel + copysignf(1.0f, vel) * friction_decel;
    } else {
        return 0.0f;
    }
}

void BallEKF::updateAcceleration(const DeltaMotionModel& odo, StateVector& deltaBall)
{
    deltaBall(acc_x_index) = -xhat_k(acc_x_index);
    deltaBall(acc_y_index) = -xhat_k(acc_y_index);
}

/**
 * Update the Jacobian A_k for the time update step used in the EKF
 * class timeUpdate()
 */
void BallEKF::calculateTimeUpdateJacobian(const DeltaMotionModel& deltaOdometry,
                                          StateVector& deltaBall)
{
    const float dt2 = dt*dt;
    ///////////////////////
    // Derivatives of ball position update re:x,y,vel_x,vel_y
    //
    // NOTE: These are the values which change every frame. The
    //       constant values are all set once in initMatrices()
    ///////////////////////

    // derivatives of x
    A_k(x_index,vel_x_index)     = dt;
    A_k(x_index,acc_x_index)     = .5f * dt2;

    // derivatives of y
    A_k(y_index,vel_y_index)     = dt;
    A_k(y_index,acc_y_index)     = .5f * dt2;

    // derivatives of vel_x
    A_k(vel_x_index,vel_x_index) = cos(deltaOdometry.theta);
    A_k(vel_x_index,vel_y_index) = sin(deltaOdometry.theta);
    A_k(vel_x_index,acc_x_index) = cos(deltaOdometry.theta) * dt;
    A_k(vel_x_index,acc_y_index) = sin(deltaOdometry.theta) * dt;

    // derivatives of vel_y
    A_k(vel_y_index,vel_x_index) = -sin(deltaOdometry.theta);
    A_k(vel_y_index,vel_y_index) =  cos(deltaOdometry.theta);
    A_k(vel_y_index,acc_x_index) = -sin(deltaOdometry.theta) * dt;
    A_k(vel_y_index,acc_y_index) =  cos(deltaOdometry.theta) * dt;
}

/**
 * Method to deal with incorporating a ball measurement into the EKF
 *
 * @param z the measurement to be incorporated
 * @param H_k the jacobian associated with the measurement, to be filled out
 * @param R_k the covariance matrix of the measurement, to be filled out
 * @param V_k the measurement invariance
 *
 * @return the measurement invariance
 */
void BallEKF::incorporateMeasurement(const RangeBearingMeasurement& z,
                                     StateMeasurementMatrix &H_k,
                                     MeasurementMatrix &R_k,
                                     MeasurementVector &V_k)
{
    // We need to translate/rotate the old values into our new
    // coordinate frame based on the odometry update
    StateVector xhat_k_prev = transformStateWithOdometry(xhat_k, curDeltaOdometry);

    // Calculate new velocities and accelerations
    MeasurementVector z_x = calculateObservedState(z, xhat_k_prev);

    // Calculate invariance
    V_k = z_x - xhat_k_bar;

    const float sd_dist_sq = z.distanceSD * z.distanceSD;

    // Update the measurement covariance matrix
    R_k(x_index,x_index)         = sd_dist_sq;
    R_k(y_index,y_index)         = sd_dist_sq;
    R_k(vel_x_index,vel_x_index) = sd_dist_sq * 2 / (dt*dt);
    R_k(vel_y_index,vel_y_index) = sd_dist_sq * 2 / (dt*dt);
    R_k(acc_x_index,acc_x_index) = sd_dist_sq * 2 / (dt*dt);
    R_k(acc_y_index,acc_y_index) = sd_dist_sq * 2 / (dt*dt);

    // Calculate the standard error of the measurement
    const StateMeasurementMatrix newP = prod(P_k, trans(H_k));
    MeasurementMatrix se = prod(H_k, newP) + R_k;

    // se is actually the variance (square of standard deviation)
    // se(0,0) = sqrt(se(0,0));
    // se(1,1) = sqrt(se(1,1));

    // Square of the position error (standard deviation)
    float errorDistSq = se(0,0) + se(1,1);
    float measurementErrorSq = (V_k(x_index)*V_k(x_index) +
                                V_k(y_index)*V_k(y_index));

    // Ignore observations based on standard error
    if ( errorDistSq*36.0f < measurementErrorSq) {
#ifdef DEBUG_STANDARD_ERROR
        cout << "\t Ignoring measurement " << endl;
        cout << "\t Standard error is " << se << endl;
        cout << "\t Invariance (V_k) is " << V_k(0) << endl;
#endif
        R_k(0,0) = DONT_PROCESS_KEY;
    }

    // If we've seen to many erroneous frames, let's reset to be safe
    (R_k(0,0) == DONT_PROCESS_KEY) ? errorCounter++ : errorCounter = 0;

#ifdef DEBUG_BALL_EKF
    cout << "Odometry: " << curOdo << endl
         << "xhat_k_prev: " << xhat_k_prev << endl
         << "z_x: " << z_x << endl
         << "V_k: " << V_k << endl
         << "R_k: " << R_k << endl
         << endl;
#endif
}

/**
 * Using odometry, give the location of the last state vector in the
 * current coordinate frame.
 */
BallEKF::StateVector BallEKF::transformStateWithOdometry(const StateVector& x,
                                                         const DeltaMotionModel& deltaOdometry)
{
    StateVector prev(ball_ekf_dimension);
    prev(x_index)     = (x(x_index) -
                         cos(deltaOdometry.theta) * deltaOdometry.x -
                         sin(deltaOdometry.theta) * deltaOdometry.y);
    prev(y_index)     = (x(y_index) +
                         sin(deltaOdometry.theta) * deltaOdometry.x -
                         cos(deltaOdometry.theta) * deltaOdometry.y);

    prev(vel_x_index) = (cos(deltaOdometry.theta) * x(vel_x_index) +
                         sin(deltaOdometry.theta) * x(vel_y_index));
    prev(vel_y_index) = (cos(deltaOdometry.theta) * x(vel_y_index) -
                         sin(deltaOdometry.theta) * x(vel_x_index));

    prev(acc_x_index) = (cos(deltaOdometry.theta) * x(acc_x_index) +
                         sin(deltaOdometry.theta) * x(acc_y_index));
    prev(acc_y_index) = (cos(deltaOdometry.theta) * x(acc_y_index) -
                         sin(deltaOdometry.theta) * x(acc_x_index));
    return prev;
}

/**
 * Use the given range and bearing, plus the previous state of the
 * ball, to calculate the current state as was observed.
 */
BallEKF::MeasurementVector
BallEKF::calculateObservedState(const RangeBearingMeasurement& z,
                                const StateVector& xhat_k_prev)
{
    MeasurementVector z_x(ball_ekf_meas_dim);

    z_x(x_index)     = z.distance * cos(z.bearing);
    z_x(y_index)     = z.distance * sin(z.bearing);

    // v = ds/dt
    z_x(vel_x_index) = (z_x(x_index) - xhat_k_prev(x_index)) / dt;
    z_x(vel_y_index) = (z_x(y_index) - xhat_k_prev(y_index)) / dt;

    // a = dv/dt
    z_x(acc_x_index) = (z_x(vel_x_index) -
                        xhat_k_prev(vel_x_index)) / dt;
    z_x(acc_y_index) = (z_x(vel_y_index) -
                        xhat_k_prev(vel_y_index)) / dt;
    return z_x;
}

void BallEKF::beforeCorrectionFinish()
{
}

/**
 * Method to ensure that the ball estimate does have any unrealistic values
 */
void BallEKF::limitPosteriorEst()
{
    xhat_k_bar(vel_x_index) =
        xhat_k(vel_x_index) = NBMath::clip(xhat_k(vel_x_index),
                                           VELOCITY_EST_MIN,
                                           VELOCITY_EST_MAX);
    xhat_k_bar(vel_y_index) =
        xhat_k(vel_y_index) = NBMath::clip(xhat_k(vel_y_index),
                                           VELOCITY_EST_MIN,
                                           VELOCITY_EST_MAX);
    xhat_k_bar(acc_x_index) =
        xhat_k(acc_x_index) = NBMath::clip(xhat_k(acc_x_index),
                                           ACC_EST_MIN,
                                           ACC_EST_MAX);
    xhat_k_bar(acc_y_index) =
        xhat_k(acc_y_index) = NBMath::clip(xhat_k(acc_y_index),
                                           ACC_EST_MIN,
                                           ACC_EST_MAX);

    if (abs(xhat_k(vel_x_index)) < .02){
        xhat_k(vel_x_index) = xhat_k_bar(vel_x_index) = 0.0;
    }
    if (abs(xhat_k(vel_y_index)) < .02){
        xhat_k(vel_y_index) = xhat_k_bar(vel_y_index) = 0.0;
    }

    if (abs(xhat_k(acc_x_index)) < .02){
        xhat_k(acc_x_index) = xhat_k_bar(acc_x_index) = 0.0;
    }
    if (abs(xhat_k(acc_y_index)) < .02){
        xhat_k(acc_y_index) = xhat_k_bar(acc_y_index) = 0.0;
    }
}

/**
 * Limit the uncertainty, mostly to prevent overflow.
 */
void BallEKF::limitPosteriorUncert()
{
    for (int i=0; i < ball_ekf_meas_dim; ++i){
        for (int j=0; j < ball_ekf_meas_dim; ++j) {
            P_k_bar(i,j) = P_k(i,j) = clip(P_k(i,j),
                                           0.0f,
                                           10000000.0f);
        }
    }
}


/**
 * Transform relative positions, velocities, and acceleration to
 * global for outside use
 */
const float BallEKF::getGlobalX() const
{
    return robotPose.x + transformToGlobalX(xhat_k(x_index),
                                            xhat_k(y_index),
                                            robotPose.h);
}
const float BallEKF::getGlobalY() const
{
    return robotPose.y + transformToGlobalY(xhat_k(x_index),
                                            xhat_k(y_index),
                                            robotPose.h);
}
const float BallEKF::getGlobalXVelocity() const
{
    return transformToGlobalX(xhat_k(vel_x_index),
                              xhat_k(vel_y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalYVelocity() const
{
    return transformToGlobalY(xhat_k(vel_x_index),
                              xhat_k(vel_y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalXAcceleration() const
{
    return transformToGlobalX(xhat_k(acc_x_index),
                              xhat_k(acc_y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalYAcceleration() const
{
    return transformToGlobalY(xhat_k(acc_x_index),
                              xhat_k(acc_y_index),
                              robotPose.h);
}

/**
 * Global uncertainties
 */
const float BallEKF::getGlobalXUncert() const
{
    return transformToGlobalX(P_k(x_index,x_index),
                              P_k(y_index,y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalYUncert() const
{
    return transformToGlobalY(P_k(x_index,x_index),
                              P_k(y_index,y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalXVelocityUncert() const
{
    return transformToGlobalX(P_k(vel_x_index,vel_x_index),
                              P_k(vel_y_index,vel_y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalYVelocityUncert() const
{
    return transformToGlobalY(P_k(vel_x_index,vel_x_index),
                              P_k(vel_y_index,vel_y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalXAccelerationUncert() const
{
    return transformToGlobalX(P_k(acc_x_index,acc_x_index),
                              P_k(acc_y_index,acc_y_index),
                              robotPose.h);
}
const float BallEKF::getGlobalYAccelerationUncert() const
{
    return transformToGlobalY(P_k(acc_x_index,acc_x_index),
                              P_k(acc_y_index,acc_y_index),
                              robotPose.h);
}

float BallEKF::transformToGlobalX(float x, float y, float theta)
{
    return x * cos(theta) - y * sin(theta);
}
float BallEKF::transformToGlobalY(float x, float y, float theta)
{
    return x * sin(theta) + y * cos(theta);
}


// /**
//  * Transform relative positions and velocities to global
//  */
// const float BallEKF::setGlobalX(float v)
// {
//     return xhat_k(x_index) * cos(robotPose.h) +
//         xhat_k(y_index) * sin(robotPose.h) + robotPose.x;
// }
// const float BallEKF::setGlobalY(float v)
// {
//     return -xhat_k(x_index) * sin(robotPose.h) +
//         xhat_k(y_index) * cos(robotPose.h) + robotPose.y;
// }
// const float BallEKF::setGlobalXVelocity(float v)
// {
//     return xhat_k(vel_x_index) * cos(robotPose.h) +
//         xhat_k(vel_y_index) * sin(robotPose.h);
// }
// const float BallEKF::setGlobalYVelocity(float v)
// {
//     return -xhat_k(vel_x_index) * sin(robotPose.h) +
//         xhat_k(vel_y_index) * cos(robotPose.h);
// }

// /**
//  * Global uncertainties
//  */
// const float BallEKF::setGlobalXUncert(float v)
// {
//     return P_k(x_index,x_index) * cos(robotPose.h) +
//         P_k(y_index,y_index) * sin(robotPose.h);
// }
// const float BallEKF::setGlobalYUncert(float v)
// {
//     return -P_k(x_index,x_index) * sin(robotPose.h) +
//         P_k(y_index,y_index) * cos(robotPose.h);
// }
// const float BallEKF::setGlobalXVelocityUncert(float v)
// {
//     return P_k(vel_x_index,vel_x_index) * cos(robotPose.h) +
//         P_k(vel_y_index,vel_y_index) * sin(robotPose.h);
// }
// const float BallEKF::setGlobalYVelocityUncert(float v)
// {
//     return -P_k(vel_x_index,vel_x_index) * sin(robotPose.h) +
//         P_k(vel_y_index,vel_y_index) * cos(robotPose.h);
// }
