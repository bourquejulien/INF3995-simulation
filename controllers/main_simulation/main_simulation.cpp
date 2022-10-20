/* Include the controller definition */
#include "main_simulation.h"
/* Function definitions for XML parsing */
#include <argos3/core/utility/configuration/argos_configuration.h>
/* 2D vector definition */
#include <argos3/core/utility/math/vector2.h>
/* Logging */
#include <argos3/core/utility/logging/argos_log.h>

/****************************************/
/****************************************/

CMainSimulation::CMainSimulation()
    : m_pcDistance(NULL), m_pcPropellers(NULL), m_pcRNG(NULL), m_pcRABA(NULL),
      m_pcRABS(NULL), m_pcPos(NULL), m_pcBattery(NULL), m_uiCurrentStep(0),
      m_actionTime(0), m_currentAction(Action::None), m_server()
{
}

/****************************************/
/****************************************/

void CMainSimulation::Init(TConfigurationNode& t_node)
{

    std::string id = GetId();
    //id.substr(id.length() - 1);
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
    /*
     * Initialize other stuff
     */

    /* Create a random number generator. We use the 'argos' category so
       that creation, reset, seeding and cleanup are managed by ARGoS. */
    m_pcRNG = CRandom::CreateRNG("argos");

    // Resets the rng seed, as well as the drone's state
    Reset();
}

/****************************************/
/****************************************/

void CMainSimulation::ControlStep()
{
    

    //HandleAction();
    
    
    // Takeoff
    if (m_currentAction == Action::Start)
    {
        bool result = TakeOff();
        m_cInitialPosition = m_pcPos->GetReading().Position;
        LOG << "ID = " << GetId() << " - " << "Taking off..." << std::endl;
    }

    GetDistanceReadings();


    if (m_currentAction == Action::ChooseAngle) {
        ChooseAngle();
    }

    if (m_currentAction == Action::Move) {
        Move();
    }
    
    if (m_currentAction == Action::Stop && m_actionTime > 0)
    {
        Land();
        LOG << "ID = " << GetId() << " - " << "Landing..." << std::endl;
    }

    // Print current action
    // if (m_currentAction == Action::Start) {
    //     LOG << "Current action: Start" << std::endl;
    // } else if (m_currentAction == Action::ChooseAngle){
    //     LOG << "Current action: Chose angle" << std::endl;
    // } else if (m_currentAction == Action::Move) {
    //     LOG << "Current action: Move" << std::endl;
    // }

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


    
    LOG << "Front dist: " << m_distanceReadings.front << std::endl;
    LOG << "Left dist: " << m_distanceReadings.left << std::endl;
    LOG << "Back dist: " << m_distanceReadings.back << std::endl;
    LOG << "Right dist: " << m_distanceReadings.right << std::endl;
    

    // Increase step counter
    m_uiCurrentStep++;

    if (m_actionTime > 0)
    {
        --m_actionTime;
    }
    LOG << " " << std::endl;
}

/****************************************/
/****************************************/

bool CMainSimulation::TakeOff()
{
    // La hauteur des drones mysterieusement ne depasse pas 0.91
    float takeOffHeight = 0.7f;
    CVector3 cPos = m_pcPos->GetReading().Position;
    if (cPos.GetZ() >= takeOffHeight - 0.01f) {
        m_currentAction = Action::ChooseAngle;
        return false;
    }
    cPos.SetZ(takeOffHeight);
    m_pcPropellers->SetAbsolutePosition(cPos);
    return true;
}

/****************************************/
/****************************************/

bool CMainSimulation::Land()
{
    CVector3 cPos = m_pcPos->GetReading().Position;
    if (Abs(cPos.GetZ()) < 0.01f)
        return false;
    cPos.SetZ(0.0f);
    m_pcPropellers->SetAbsolutePosition(cPos);
    return true;
}

/****************************************/
/****************************************/

// float CMainSimulation::ChooseAngle() 
// {
//     // Just return float between 0 and 360 for now
//     m_currentAction = Action::Move;
//     return (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 360.0f;
// }

void CMainSimulation::ChooseAngle() 
{
    int wallsClose = 0;
    float X = 0.0f;
    float Y = 0.0f;

    // If the wall is close enough, we add the inverse of the distance to a vector's coordinates. This vector then determines the range in which the new angle is chosen
    // The angle left is the positive X direction, and back the positive Y
    if (0.0f <= m_distanceReadings.front && m_distanceReadings.front <= m_distanceThreshold) {
        wallsClose++;
        Y += 1.0f/m_distanceReadings.front;
    }
    if (0.0f <= m_distanceReadings.left && m_distanceReadings.left <= m_distanceThreshold) {
        wallsClose++;
        X -= 1.0f/m_distanceReadings.left;
    }
    if (0.0f <= m_distanceReadings.back && m_distanceReadings.back <= m_distanceThreshold){
        wallsClose++;
        Y -= 1.0f/m_distanceReadings.back;
    }
    if (0.0f <= m_distanceReadings.right && m_distanceReadings.right <= m_distanceThreshold){
        wallsClose++;
        X += 1.0f/m_distanceReadings.right;
    }

    CRange<CRadians> range;
    if (wallsClose == 0) {
        // If no walls are close, i.e. the drone just took off, choose a completely random direction 
        range = CRange(CRadians::ZERO, CRadians::TWO_PI);
    } else {
        // Else, find the angle of the vector above
        CRadians angleRange = CRadians::PI_OVER_SIX;
        CRadians rangeCenter = ATan2(Y, X);
        range = CRange(rangeCenter - angleRange, rangeCenter + angleRange);
    }

    m_nextPosition = m_pcPos->GetReading().Position;

    m_currentAction = Action::Move;
    m_moveAngle = m_pcRNG->Uniform(range);
}

/****************************************/
/****************************************/

bool CMainSimulation::Move() {
    float speed = 0.6f;
    CVector3 cPos = m_pcPos->GetReading().Position;
    if ((cPos - m_nextPosition).Length() < 0.1f) {
        m_nextPosition.SetX(cPos.GetX() + Cos(m_moveAngle) * speed);
        m_nextPosition.SetY(cPos.GetY() + Sin(m_moveAngle) * speed);
    }

    m_pcPropellers->SetAbsolutePosition(m_nextPosition);

    if (m_actionTime <= 0 && ShouldChangeDirection()) {
        m_currentAction = Action::ChooseAngle;
        m_actionTime = 10;
        return false;
    } else {
        return true;
    }
}

void CMainSimulation::GetDistanceReadings() {
    // Look here for documentation on the distance sensor:
    // https://github.com/MISTLab/argos3/blob/inf3995/src/plugins/robots/crazyflie/control_interface/ci_crazyflie_distance_scanner_sensor.h
    // Read distance sensor measurements
    CCI_CrazyflieDistanceScannerSensor::TReadingsMap sDistRead =
        m_pcDistance->GetReadingsMap(); //Ne pas utiliser GetLongReadingsMap, ne lie rien

    auto iterDistRead = sDistRead.begin();

    if (sDistRead.size() == 4)
    {
        m_distanceReadings.front = (iterDistRead++)->second;
        m_distanceReadings.left = (iterDistRead++)->second;
        m_distanceReadings.back = (iterDistRead++)->second;
        m_distanceReadings.right = (iterDistRead++)->second;
    } else {
        LOG << "There is a problem with the distance scanners" << std::endl;
    }

}

bool CMainSimulation::ShouldChangeDirection() {
    if (0.0f <= m_distanceReadings.front && m_distanceReadings.front <= m_distanceThreshold)
        return true;
    if (0.0f <= m_distanceReadings.left && m_distanceReadings.left <= m_distanceThreshold)
        return true;
    if (0.0f <= m_distanceReadings.back && m_distanceReadings.back <= m_distanceThreshold)
        return true;
    if (0.0f <= m_distanceReadings.right && m_distanceReadings.right <= m_distanceThreshold)
        return true;

    return false;
}

void CMainSimulation::Reset() {

    //Reset rng seed
    m_pcRNG->SetSeed(std::time(0) + stoi(GetId().substr(GetId().length() - 1)));
    m_pcRNG->Reset();

    m_uiCurrentStep = 0;
    m_currentAction = Action::Start; // TODO : change to None in final version
    m_actionTime    = 5;
    m_distanceReadings = {-2.0f, -2.0f, -2.0f, -2.0f};
    m_distanceThreshold = 75.0f;
}

void CMainSimulation::Destroy() { m_server.Stop(); }

/****************************************/
/****************************************/

void CMainSimulation::HandleAction()
{
    Command command;

    if (m_actionTime <= 0 && m_server.GetNextCommand(&command))
    {
        m_currentAction = command.action;
        m_actionTime    = 5;
    }
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
