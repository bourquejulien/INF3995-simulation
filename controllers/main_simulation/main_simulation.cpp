/* Include the controller definition */
#include "main_simulation.h"
/* Function definitions for XML parsing */
#include <argos3/core/utility/configuration/argos_configuration.h>
/* 2D vector definition */
#include <argos3/core/utility/math/vector2.h>
/* Logging */
#include <argos3/core/utility/logging/argos_log.h>

template<typename E>
constexpr auto toUnderlyingType(E e) 
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

/****************************************/
/****************************************/

/// @brief Constructor of the CMainSumulation
CMainSimulation::CMainSimulation()
    : m_pcDistance(NULL), m_pcPropellers(NULL), m_pcRNG(NULL), m_pcRABA(NULL),
      m_pcRABS(NULL), m_pcPos(NULL), m_pcBattery(NULL), m_uiCurrentStep(0),
      m_actionTime(0), m_currentAction(Action::None), m_server()
{
}

/// @brief Initialise the sensors and start the server
/// @param t_node 
void CMainSimulation::Init(TConfigurationNode& t_node)
{

    std::string id = GetId();
    unsigned int port = 9854 + stoi(id.substr(id.length() - 1));

    std::string address = "0.0.0.0:" + std::to_string(port);

    m_server.Run(address);

    try
    {
        /*
         * Initialize sensors/actuators
         */
        m_pcDistance = GetSensor<CCI_CrazyflieDistanceScannerSensor>(
            "crazyflie_distance_scanner");
        m_pcPropellers =
            GetActuator<CCI_QuadRotorPositionActuator>("quadrotor_position");
        /* Get pointers to devices */
        m_pcRABA =
            GetActuator<CCI_RangeAndBearingActuator>("range_and_bearing");
        m_pcRABS = GetSensor<CCI_RangeAndBearingSensor>("range_and_bearing");
        try
        {
            m_pcPos = GetSensor<CCI_PositioningSensor>("positioning");
        }
        catch (CARGoSException& ex)
        {
        }
        try
        {
            m_pcBattery = GetSensor<CCI_BatterySensor>("battery");
        }
        catch (CARGoSException& ex)
        {
        }
    }
    catch (CARGoSException& ex)
    {
        THROW_ARGOSEXCEPTION_NESTED(
            "Error initializing the crazyflie sensing controller for robot \""
                << GetId() << "\"",
            ex);
    }

    /* Create a random number generator. We use the 'argos' category so
       that creation, reset, seeding and cleanup are managed by ARGoS. */
    m_pcRNG = CRandom::CreateRNG("argos");
    
    // Resets the rng seed, as well as the drone's state
    Reset();
}

/// @brief Control the steps the drone has to follow
void CMainSimulation::ControlStep()
{

    HandleAction(); // Comment to test takeoff without backend

    m_server.UpdateTelemetrics(getCurrentMetric());

    // Takeoff
    if (m_currentAction == Action::Start)
    {
        bool result = TakeOff();
        m_cInitialPosition = m_pcPos->GetReading().Position;
        LOG << "ID = " << GetId() << " - " << "Taking off..." << std::endl;
    }

    GetDistanceReadings();
    Position positionDistance = getCurrentPosition();
    DistanceReadings distanceReading = DistanceReadings(m_distance.front, m_distance.back, m_distance.left, m_distance.right, positionDistance);
    m_server.UpdateDistances(distanceReading);

    if (m_currentAction == Action::ChooseAngle) {
        ChooseAngle();
    }

    if (m_currentAction == Action::Move) {
        Move();
    }
    
    if (m_currentAction == Action::Stop)
    {
        if (!Land())
        {
            m_currentAction = Action::None;
        }
        LOG << "ID = " << GetId() << " - " << "Landing..." << std::endl;
    }
    
    // Print current position.
    LOG << "ID = " << GetId() << " - "
        << "Position (x,y,z) = (" << m_pcPos->GetReading().Position.GetX()
        << "," << m_pcPos->GetReading().Position.GetY() << ","
        << m_pcPos->GetReading().Position.GetZ() << ")" << std::endl;

    // Print angle of movement
    LOG << "Current angle : " << m_moveAngle.GetValue() * CRadians::RADIANS_TO_DEGREES << std::endl;

    // Print current battery level
    const CCI_BatterySensor::SReading& sBatRead = m_pcBattery->GetReading();
    LOG << "Battery level: " << sBatRead.AvailableCharge << std::endl;
    LOG << "Current state: " << toUnderlyingType(m_currentAction) << std::endl;

    // Print distances
    LOG << "Front dist: " << m_distance.front << std::endl;
    LOG << "Left dist: " << m_distance.left << std::endl;
    LOG << "Back dist: " << m_distance.back << std::endl;
    LOG << "Right dist: " << m_distance.right << std::endl;

    // Increase step counter
    m_uiCurrentStep++;

    if (m_actionTime > 0)
    {
        --m_actionTime;
    }
    LOG << " " << std::endl;
}

/// @brief Handle the Take off action
/// @return True if action succeed, False if unsuccessful
bool CMainSimulation::TakeOff()
{
    // Drone height mysteriously does not go past 0.91
    float takeOffHeight = 0.7f;
    float takeoffPrecision = 0.01f;

    CVector3 cPos = m_pcPos->GetReading().Position;
    if (cPos.GetZ() >= takeOffHeight - takeoffPrecision) {
        m_currentAction = Action::ChooseAngle;
        return false;
    }
    cPos.SetZ(takeOffHeight);
    m_pcPropellers->SetAbsolutePosition(cPos);
    return true;
}

/// @brief Handle the Land action
/// @return True if action succeed, False if unsuccessful
bool CMainSimulation::Land()
{
    argos::Real landingPrecision = 0.05;
    CVector3 cPos = m_pcPos->GetReading().Position;
    if (cPos.GetZ() < landingPrecision)
        return false;
    cPos.SetZ(0.0f);
    m_pcPropellers->SetAbsolutePosition(cPos);
    return true;
}

/// @brief Handle the Choose angle action
void CMainSimulation::ChooseAngle() 
{
    int wallsClose = 0;
    float X = 0.0f;
    float Y = 0.0f;

    // If the wall is close enough, we add the inverse of the distance to a vector's coordinates. This vector then determines the range in which the new angle is chosen
    // The left is the positive X direction, and back the positive Y
    if (0.0f <= m_distance.front && m_distance.front <= m_distanceThreshold) {
        wallsClose++;
        Y += 1.0f/m_distance.front;
    }
    if (0.0f <= m_distance.left && m_distance.left <= m_distanceThreshold) {
        wallsClose++;
        X -= 1.0f/m_distance.left;
    }
    if (0.0f <= m_distance.back && m_distance.back <= m_distanceThreshold){
        wallsClose++;
        Y -= 1.0f/m_distance.back;
    }
    if (0.0f <= m_distance.right && m_distance.right <= m_distanceThreshold){
        wallsClose++;
        X += 1.0f/m_distance.right;
    }

    CRange<CRadians> range;
    if (wallsClose == 0) {
        // If no walls are close, i.e. the drone just took off, choose a completely random direction 
        range = CRange(CRadians::ZERO, CRadians::TWO_PI);
    } else {
        // Else, find the angle of the vector above
        CRadians angleRange = CRadians::PI_OVER_FOUR;
        CRadians rangeCenter = ATan2(Y, X);
        range = CRange(rangeCenter - angleRange, rangeCenter + angleRange);
    }

    m_server.AddLog("Updating position", "INFO");

    m_nextPosition = m_pcPos->GetReading().Position;

    m_currentAction = Action::Move;
    m_moveAngle = m_pcRNG->Uniform(range);
}

/// @brief Handle the Move action
/// @return True if action succeed, False if unsuccessful
bool CMainSimulation::Move() {
    float speed = 0.5f;
    CVector3 cPos = m_pcPos->GetReading().Position;

    // If drone is close enough to intended position, choose next position
    // Movement is done in steps like this so drone does not accelerate too much and clips into walls
    if ((cPos - m_nextPosition).Length() < 0.1f) { 
        m_nextPosition.SetX(cPos.GetX() + Cos(m_moveAngle) * speed);
        m_nextPosition.SetY(cPos.GetY() + Sin(m_moveAngle) * speed);
    }

    m_pcPropellers->SetAbsolutePosition(m_nextPosition);

    if (m_actionTime <= 0 && ShouldChangeDirection()) {
        m_currentAction = Action::ChooseAngle;
        m_actionTime = 10; // Timer is added to change direction check so a new angle isn't chosen every step when close to a wall
        return false;
    }
    return true;
}

/// @brief Get the distances
void CMainSimulation::GetDistanceReadings() {
    // Look here for documentation on the distance sensor:
    // https://github.com/MISTLab/argos3/blob/inf3995/src/plugins/robots/crazyflie/control_interface/ci_crazyflie_distance_scanner_sensor.h
    // Read distance sensor measurements
    CCI_CrazyflieDistanceScannerSensor::TReadingsMap sDistRead =
        m_pcDistance->GetReadingsMap(); //Do not use GetLongReadingsMap, doesn't work and reads nothing

    auto iterDistRead = sDistRead.begin();

    if (sDistRead.size() == 4)
    {
        m_distance.front = (iterDistRead++)->second;
        m_distance.left = (iterDistRead++)->second;
        m_distance.back = (iterDistRead++)->second;
        m_distance.right = (iterDistRead++)->second;
    } else {
        LOG << "There is a problem with the distance scanners" << "Size: " << sDistRead.size() << std::endl;
    }

}

/// @brief Determine if the drone should change direction
/// @return True if changing direction, False if not
bool CMainSimulation::ShouldChangeDirection() {
    if (0.0f <= m_distance.front && m_distance.front <= m_distanceThreshold)
        return true;
    if (0.0f <= m_distance.left && m_distance.left <= m_distanceThreshold)
        return true;
    if (0.0f <= m_distance.back && m_distance.back <= m_distanceThreshold)
        return true;
    if (0.0f <= m_distance.right && m_distance.right <= m_distanceThreshold)
        return true;

    return false;
}

/// @brief Reset the drone
void CMainSimulation::Reset() {

    //Reset rng seed
    m_pcRNG->SetSeed(std::time(0) + stoi(GetId().substr(GetId().length() - 1))); // Drone ID is taken into account so both drones have different rng
    m_pcRNG->Reset();

    m_uiCurrentStep = 0;
    //m_currentAction = Action::Start; // Comment when doing merge request
    m_currentAction = Action::None; // Comment when testing without backend
    m_actionTime    = 5;
    m_distance = SensorDistance();
    m_distanceThreshold = 30.0f;
}

/// @brief Stop the server
void CMainSimulation::Destroy() { m_server.Stop(); }

/// @brief Determine wich action should be done by the command
void CMainSimulation::HandleAction()
{
    Command command;

    if (m_actionTime <= 0 && m_server.GetNextCommand(&command))
    {
        m_currentAction = command.action;
        m_actionTime    = 5;
    }
}

/// @brief Get the current position of the drone
/// @return Position of the drone
Position CMainSimulation::getCurrentPosition()
{
    CVector3 cPos = m_pcPos->GetReading().Position;
    return Position(cPos.GetX(), cPos.GetY(), cPos.GetZ());
}

/// @brief Get the current status of the drone
/// @return Status of the drone (metric)
Metric CMainSimulation::getCurrentMetric()
{
    Position position = getCurrentPosition();
    return Metric(toUnderlyingType(m_currentAction), position);
}

/*
 * This statement notifies ARGoS of the existence of the controller.
 * It binds the class passed as first argument to the string passed as
 * second argument.
 * The string is then usable in the XML configuration file to refer to
 * this controller.
 * When ARGoS reads that string in the XML file, it knows which controller
 * class to instantiate.
 * See also the XML configuration files for an example of how this is used.
 */
REGISTER_CONTROLLER(CMainSimulation, "main_simulation_controller")
