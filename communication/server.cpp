#include "server.h"

/// @brief Constructor of the ServiceImplementation class
/// @param mutex mutex
/// @param queue Commands queue
/// @param position Positions queue
/// @param status Status queue
/// @param distance Distances queue
ServiceImplementation::ServiceImplementation(std::mutex& mutex, std::queue<Command>& queue, std::queue<Position>& position,std::queue<std::string>& status, std::queue<DistanceReadings>& queueDistance, std::queue<Position>& queuePositionDistance)
    : m_queueMutex(mutex), m_queue(queue), m_queuePosition(position), m_queueStatus(status), m_queueDistance(queueDistance), m_queuePositionDistance(queuePositionDistance)
{
}

/// @brief Put the start mission command in the commands queue
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::StartMission(
    ServerContext* context, const MissionRequest* request, MissionReply* reply)
{
    Command command = {request->uri(), Action::Start};
    
    m_queueMutex.lock();
    m_queue.push(command);
    m_queueMutex.unlock();

    reply->set_message("Success");
    return Status::OK;
}

/// @brief Put the end mission command in the command queue
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::EndMission(
    ServerContext* context, const MissionRequest* request, MissionReply* reply)
{
    Command command = {request->uri(), Action::Stop};

    m_queueMutex.lock();
    m_queue.push(command);
    m_queueMutex.unlock();

    reply->set_message("Success");
    return Status::OK;
}

/// @brief Set the reply to send telemetrics to server
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::GetTelemetrics(ServerContext* context, const MissionRequest* request, TelemetricsReply* reply)
{
    if (!m_queuePosition.empty() && !m_queueStatus.empty()){
        m_queueMutex.lock();
        for (int i = 0; i < m_queuePosition.size(); i++)
        {
            Position position = m_queuePosition.front();
            std::string status = m_queueStatus.front();

            
            Telemetric* telemetric = reply->add_telemetric();
            simulation::Position* rpc_position = new simulation::Position();

            rpc_position->set_x(position.posX);
            rpc_position->set_y(position.posY);
            rpc_position->set_z(position.posZ);

            telemetric->set_status(status);
            telemetric->set_allocated_position(rpc_position);

            m_queuePosition.pop();
            m_queueStatus.pop();
        }
        m_queueMutex.unlock();
    }
    while(!m_queuePosition.empty()) m_queuePosition.pop();
    while(!m_queueStatus.empty()) m_queueStatus.pop();
    
    return Status::OK;
}

/// @brief Set the reply to send distances to server
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::GetDistances(ServerContext* context, const MissionRequest* request, DistancesReply* reply)
{
    if (!m_queueDistance.empty()){
        m_queueMutex.lock();
        for (int i = 0; i < m_queueDistance.size(); i++)
        {
            DistanceReadings distance = m_queueDistance.front();
            Position position = m_queuePositionDistance.front();

            DistanceObstacle* newDistance = reply->add_distanceobstacle();
            simulation::Position* rpc_position = new simulation::Position();

            rpc_position->set_x(position.posX);
            rpc_position->set_y(position.posY);
            rpc_position->set_z(position.posZ);

            newDistance->set_front(distance.front);
            newDistance->set_back(distance.back);
            newDistance->set_left(distance.left);
            newDistance->set_right(distance.right);

            newDistance->set_allocated_position(rpc_position);

            m_queueDistance.pop();
            m_queuePositionDistance.pop();
        }
        m_queueMutex.unlock();
    }
    while(!m_queueDistance.empty()) m_queueDistance.pop();
    
    return Status::OK;
}

/// @brief Set the reply to send distances to server
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::GetLogs(ServerContext* context, const MissionRequest* request, LogReply* reply)
{
    if (!m_queueDistance.empty()){
        m_queueMutex.lock();
        for (int i = 0; i < m_queueDistance.size(); i++)
        {
            DistanceReadings distance = m_queueDistance.front();
            Position position = m_queuePositionDistance.front();

            DistanceObstacle* newDistance = reply->add_distanceobstacle();
            simulation::Position* rpc_position = new simulation::Position();

            rpc_position->set_x(position.posX);
            rpc_position->set_y(position.posY);
            rpc_position->set_z(position.posZ);

            newDistance->set_front(distance.front);
            newDistance->set_back(distance.back);
            newDistance->set_left(distance.left);
            newDistance->set_right(distance.right);

            newDistance->set_allocated_position(rpc_position);

            m_queueDistance.pop();
            m_queuePositionDistance.pop();
        }
        m_queueMutex.unlock();
    }
    while(!m_queueDistance.empty()) m_queueDistance.pop();
    
    return Status::OK;
}

/// @brief Update the telemetric queue (position and status)
/// @param position Position to add to the position queue
/// @param status Status to add to the status queue
void ServiceImplementation::UpdateTelemetrics(Position position, std::string status)
{
    m_queueMutex.lock();
    m_queuePosition.push(position);
    m_queueStatus.push(status);
    m_queueMutex.unlock();
}

/// @brief Update the distances queue 
/// @param distance Distance to add to the distance queue
/// @param position Position du drone
void ServiceImplementation::UpdateDistances(DistanceReadings distance, Position position)
{
    m_queueMutex.lock();
    m_queueDistance.push(distance);
    m_queuePositionDistance.push(position);
    m_queueMutex.unlock();
}

/// @brief Constructor of the SimulationServer
SimulationServer::SimulationServer() : m_service(m_queueMutex, m_queue, m_queuePosition, m_queueStatus, m_queueDistance, m_queuePositionDistance)
{
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
}

/// @brief Run the simulation server
/// @param address adress to run the server
void SimulationServer::Run(std::string address)
{
    ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&m_service);

    // Assemble the server.
    m_server = std::unique_ptr<Server>(builder.BuildAndStart());

    std::cout << "Server listening on " << address << std::endl;
}

/// @brief Get the next command in the commands queue
/// @param command Command that is next in queue
/// @return True if could find next command, False if no command next
bool SimulationServer::GetNextCommand(Command* command)
{
    if (m_queue.empty())
    {
        return false;
    }

    m_queueMutex.lock();
    *command = m_queue.front();
    m_queue.pop();
    m_queueMutex.unlock();

    return true;
}

/// @brief Update the telemetrics in the ServiceImplementation
/// @param position position to add to the position queue
/// @param status status to add to the status queue
void SimulationServer::UpdateTelemetrics(Position position, std::string status){
    m_service.UpdateTelemetrics(position, status);
}

/// @brief Update the distances in the ServiceImplementation
/// @param distance Distance to add to the queue
void SimulationServer::UpdateDistances(DistanceReadings distance, Position position){
    m_service.UpdateDistances(distance, position);
}

/// @brief Shut down the simulation server
void SimulationServer::Stop() { m_server->Shutdown(); }
