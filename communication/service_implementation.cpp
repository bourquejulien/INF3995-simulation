#include "service_implementation.h"

/// @brief Constructor of the ServiceImplementation class
/// @param mutex mutex
/// @param command_queue Commands queue
/// @param queue_metric Metric queue
/// @param distance_queue Distances queue
/// @param queue_log Logs queue
ServiceImplementation::ServiceImplementation(std::mutex& mutex, std::queue<Command>& command_queue, std::queue<bool>& done_queue, std::queue<Metric>& queue_metric,  std::queue<DistanceReadings>& queue_distance, std::queue<LogData>& queue_log)
    : m_queue_mutex(mutex), m_queue_command(command_queue), m_queue_done(done_queue), m_queue_metric(queue_metric), m_queue_distance(queue_distance), m_queue_log(queue_log)
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
    
    m_queue_mutex.lock();
    m_queue_command.push(command);
    m_queue_mutex.unlock();

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

    m_queue_mutex.lock();
    m_queue_command.push(command);
    m_queue_mutex.unlock();

    reply->set_message("Success");
    return Status::OK;
}

/// @brief Put the return command in the command queue
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::ReturnToBase(
    ServerContext* context, const MissionRequest* request, MissionReply* reply)
{
    std::cout << "starting" << std::endl;
    Command command = {request->uri(), Action::Return};

    m_queue_mutex.lock();
    m_queue_command.push(command);
    m_queue_mutex.unlock();

    bool inProgress = true;
    const int WAIT_INTERVAL = 1000;
    while(inProgress)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_INTERVAL));
        m_queue_mutex.lock();
        
        if(!m_queue_done.empty())
        {
            m_queue_done.pop();
            inProgress = false;
        }

        m_queue_mutex.unlock();
    }

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
    if (m_queue_metric.empty()){
        return Status::OK;
    }

    m_queue_mutex.lock();

    while (!m_queue_metric.empty())
    {
        Metric metric = m_queue_metric.front();
        Position position = metric.position;
        int status = metric.status;

        
        Telemetric* telemetric = reply->add_telemetric();
        simulation::Position* rpc_position = new simulation::Position();

        rpc_position->set_x(position.posX);
        rpc_position->set_y(position.posY);
        rpc_position->set_z(position.posZ);

        telemetric->set_status(status);
        telemetric->set_allocated_position(rpc_position);

        m_queue_metric.pop();
    }

    m_queue_mutex.unlock();
    
    return Status::OK;
}

/// @brief Set the reply to send distances to server
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::GetDistances(ServerContext* context, const MissionRequest* request, DistancesReply* reply)
{
    if (m_queue_distance.empty()){
        return Status::OK;
    }
    
    m_queue_mutex.lock();

    while (!m_queue_distance.empty())
    {
        DistanceReadings distance = m_queue_distance.front();
        Position position = distance.position;

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

        m_queue_distance.pop();
    }

    m_queue_mutex.unlock();
    
    return Status::OK;
}

/// @brief Set the reply to send log to server
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::GetLogs(ServerContext* context, const MissionRequest* request, LogReply* reply)
{
    if (m_queue_log.empty()){
        return Status::OK;
    }
    
    m_queue_mutex.lock();

    while (!m_queue_log.empty())
    {
        LogData log = m_queue_log.front();

        simulation::LogData* logData = reply->add_logs();

        logData->set_level(log.level);
        logData->set_message(log.message);

        m_queue_log.pop();
    }

    m_queue_mutex.unlock();
    
    return Status::OK;
}
