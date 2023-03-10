/*
 * AUTHORS: Carlo Pinciroli <cpinciro@ulb.ac.be>
 *          Pierre-Yves Lajoie <lajoie.py@gmail.com>
 *
 * An example crazyflie drones sensing.
 *
 * This controller is meant to be used with the XML file:
 *    experiments/foraging.argos
 */

#ifndef MAIN_SIMULATION_H
#define MAIN_SIMULATION_H

/*
 * Include some necessary headers.
 */
/* Definition of the CCI_Controller class. */
#include <argos3/core/control_interface/ci_controller.h>
/* Definition of the crazyflie distance sensor */
#include <argos3/plugins/robots/crazyflie/control_interface/ci_crazyflie_distance_scanner_sensor.h>
/* Definition of the crazyflie position actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_quadrotor_position_actuator.h>
/* Definition of the crazyflie position sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_positioning_sensor.h>
/* Definition of the crazyflie range and bearing actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
/* Definition of the crazyflie range and bearing sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
/* Definition of the crazyflie battery sensor */
#include <argos3/plugins/robots/generic/control_interface/ci_battery_sensor.h>
/* Definitions for random number generation */
#include <argos3/core/utility/math/rng.h>

#include <communication/server.h>
#include <struct/distance_reading.h>
#include <struct/position.h>

struct SensorDistance
{
    float front;
    float back;
    float left;
    float right;

    SensorDistance() : front(-1.0), back(-1.0), left(-1.0), right(-1.0) {}

    SensorDistance(float front, float back, float left, float right)
        : front(front), back(back), left(left), right(right)
    {
    }
};

/*
 * All the ARGoS stuff in the 'argos' namespace.
 * With this statement, you save typing argos:: every time.
 */
using namespace argos;

/*
 * A controller is simply an implementation of the CCI_Controller class.
 */
class CMainSimulation : public CCI_Controller
{
public:
    /* Class constructor. */
    CMainSimulation();
    /* Class destructor. */
    virtual ~CMainSimulation() {}

    /*
     * This function initializes the controller.
     * The 't_node' variable points to the <parameters> section in the XML
     * file in the <controllers><footbot_foraging_controller> section.
     */
    virtual void Init(TConfigurationNode& t_node);

    /*
     * This function is called once every time step.
     * The length of the time step is set in the XML file.
     */
    virtual void ControlStep();

    /*
     * This function resets the controller to its state right after the
     * Init().
     * It is called when you press the reset button in the GUI.
     */
    virtual void Reset();

    /*
     * Called to cleanup what done by Init() when the experiment finishes.
     * In this example controller there is no need for clean anything up,
     * so the function could have been omitted. It's here just for
     * completeness.
     */
    virtual void Destroy();

    /*
     * This function starts the mission
     */
    bool Start();

    /*
     * This function returns the drone to the base
     */
    bool Return();

    /*
     * This function lifts the drone from the ground
     */
    bool TakeOff();

    /**
     * This function set the position in server
     */
    void SendPosition(Position& position);

    /*
     * This function returns the drone to the ground
     */
    bool Land();

    /*
     * This function determines the next random angle the drone will travel in
     */
    void ChooseRandomAngle();

    /*
     * This function moves the drone until it meets a wall or other drone
     */
    bool Move();

    /*
     * This function gets the distance readings and saves them in a struct
     */
    void GetDistanceReadings();

    /*
     * This function determines wether the direction should be changed when
     * close to walls
     */
    bool ShouldChangeDirection();

    void HandleAction();

    Position getCurrentPosition();

    Metric getCurrentMetric(float batteryLevel);

private:
    int m_actionTime;

    Action m_currentAction;

    /* Pointer to the crazyflie distance sensor */
    CCI_CrazyflieDistanceScannerSensor* m_pcDistance;

    /* Pointer to the position actuator */
    CCI_QuadRotorPositionActuator* m_pcPropellers;

    /* Pointer to the range and bearing actuator */
    CCI_RangeAndBearingActuator* m_pcRABA;

    /* Pointer to the range and bearing sensor */
    CCI_RangeAndBearingSensor* m_pcRABS;

    /* Pointer to the positioning sensor */
    CCI_PositioningSensor* m_pcPos;

    /* Pointer to the battery sensor */
    CCI_BatterySensor* m_pcBattery;

    /* The random number generator */
    CRandom::CRNG* m_pcRNG;

    /* Current step */
    uint m_uiCurrentStep;

    /* Initial Position */
    CVector3 m_cInitialPosition;

    /* Next Postion*/
    CVector3 m_nextPosition;

    /* Angle drone is currently moving at in random walk */
    CRadians m_moveAngle;

    /* Readings of distance scanner */
    SensorDistance m_distance;

    /* How close the drone should get to the walls before changing direction */
    float m_distanceThreshold;

    SimulationServer m_server;
};

#endif
