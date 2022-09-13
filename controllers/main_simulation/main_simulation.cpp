/* Include the controller definition */
#include "main_simulation.h"
/* Function definitions for XML parsing */
#include <argos3/core/utility/configuration/argos_configuration.h>
/* 2D vector definition */
#include <argos3/core/utility/math/vector2.h>

/****************************************/
/****************************************/

const int max_length = 1024;

CMainSim::CMainSim() :
   m_pcWheels(NULL),
   m_pcProximity(NULL),
   m_cAlpha(10.0f),
   m_fDelta(0.5f),
   m_fWheelVelocity(2.5f),
   m_is_init(false),
   m_cGoStraightAngleRange(-ToRadians(m_cAlpha),
                           ToRadians(m_cAlpha)) {}

/****************************************/
/****************************************/

void CMainSim::Init(TConfigurationNode& t_node) {
   m_io_context = new boost::asio::io_context();
   m_acceptor = new tcp::acceptor(*m_io_context, tcp::endpoint(tcp::v4(), 9854));

   m_acceptor -> non_blocking(true);
   
   /*
    * Get sensor/actuator handles
    *
    * The passed string (ex. "differential_steering") corresponds to the
    * XML tag of the device whose handle we want to have. For a list of
    * allowed values, type at the command prompt:
    *
    * $ argos3 -q actuators
    *
    * to have a list of all the possible actuators, or
    *
    * $ argos3 -q sensors
    *
    * to have a list of all the possible sensors.
    *
    * NOTE: ARGoS creates and initializes actuators and sensors
    * internally, on the basis of the lists provided the configuration
    * file at the <controllers><footbot_diffusion><actuators> and
    * <controllers><footbot_diffusion><sensors> sections. If you forgot to
    * list a device in the XML and then you request it here, an error
    * occurs.
    */
   m_pcWheels    = GetActuator<CCI_DifferentialSteeringActuator>("differential_steering");
   m_pcProximity = GetSensor  <CCI_FootBotProximitySensor      >("footbot_proximity"    );
   /*
    * Parse the configuration file
    *
    * The user defines this part. Here, the algorithm accepts three
    * parameters and it's nice to put them in the config file so we don't
    * have to recompile if we want to try other settings.
    */
   GetNodeAttributeOrDefault(t_node, "alpha", m_cAlpha, m_cAlpha);
   m_cGoStraightAngleRange.Set(-ToRadians(m_cAlpha), ToRadians(m_cAlpha));
   GetNodeAttributeOrDefault(t_node, "delta", m_fDelta, m_fDelta);
   GetNodeAttributeOrDefault(t_node, "velocity", m_fWheelVelocity, m_fWheelVelocity);
}

/****************************************/
/****************************************/

void CMainSim::ControlStep() {
   boost::system::error_code error;
   tcp::socket socket = m_acceptor->accept(error);

   if (!error)
   {
      std::thread(&CMainSim::TcpSession, this, std::move(socket)).detach();
   }


   this->ProcessCommands();
   
   /* Get readings from proximity sensor */
   const CCI_FootBotProximitySensor::TReadings& tProxReads = m_pcProximity->GetReadings();
   /* Sum them together */
   CVector2 cAccumulator;
   for(size_t i = 0; i < tProxReads.size(); ++i) {
      cAccumulator += CVector2(tProxReads[i].Value, tProxReads[i].Angle);
   }
   cAccumulator /= tProxReads.size();
   /* If the angle of the vector is small enough and the closest obstacle
    * is far enough, continue going straight, otherwise curve a little
    */
   CRadians cAngle = cAccumulator.Angle();

   if (!m_is_init)
   {
      return;
   }

   if(m_cGoStraightAngleRange.WithinMinBoundIncludedMaxBoundIncluded(cAngle) &&
      cAccumulator.Length() < m_fDelta ) {
      /* Go straight */
      m_pcWheels->SetLinearVelocity(m_fWheelVelocity, m_fWheelVelocity);
   }
   else {
      /* Turn, depending on the sign of the angle */
      if(cAngle.GetValue() > 0.0f) {
         m_pcWheels->SetLinearVelocity(m_fWheelVelocity, 0.0f);
      }
      else {
         m_pcWheels->SetLinearVelocity(0.0f, m_fWheelVelocity);
      }
   }
   
}

void CMainSim::TcpSession(tcp::socket sock)
{
   boost::system::error_code error;
  try
  {
    while(!error)
    {
      char data[max_length];

      if (error == boost::asio::error::eof)
        break;
      else if (error)
        throw boost::system::system_error(error);

      size_t length = sock.read_some(boost::asio::buffer(data), error);

      char code = data[0];
      
      std::string response = "Succeded from Argos";

      Message message;

      switch (code)
      {
      case 'i':
         message = {Action::Init, ""};
         break;
      
      case 't':
         message = {Action::Takeoff, ""};
         break;
      
      default:
         response = "Failed from Argos";
         break;
      }

      m_history_mutex.lock();
      m_command_history.push(message);
      m_history_mutex.unlock();

      boost::asio::write(sock, boost::asio::buffer(response));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception in thread: " << e.what() << "\n";
  }
}

void CMainSim::ProcessCommands()
{
   Message command;

   m_history_mutex.lock();

   if(m_command_history.empty())
   {
      m_history_mutex.unlock();
      return;
   }

   command = m_command_history.front();
   m_command_history.pop();
   m_history_mutex.unlock();

   if(command.action == Action::Init)
   {
      this->m_is_init = true;
   }
}

void CMainSim::Reset()
{
   m_is_init = false;
}

void CMainSim::Destroy()
{
   delete m_io_context;
   m_io_context = nullptr;
   delete m_acceptor;
   m_acceptor = nullptr;
}

/****************************************/
/****************************************/

/*
 * This statement notifies ARGoS of the existence of the controller.
 * It binds the class passed as first argument to the string passed as
 * second argument.
 * The string is then usable in the configuration file to refer to this
 * controller.
 * When ARGoS reads that string in the configuration file, it knows which
 * controller class to instantiate.
 * See also the configuration files for an example of how this is used.
 */
REGISTER_CONTROLLER(CMainSim, "main_sim_controller")
