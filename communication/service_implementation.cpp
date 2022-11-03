#include "service_implementation.h"

/// @brief Constructor of the ServiceImplementation class
/// @param mutex mutex
/// @param queue Commands queue
/// @param position Positions queue
/// @param status Status queue
/// @param distance Distances queue
ServiceImplementation::ServiceImplementation(std::mutex& mutex, std::queue<Command>& command_queue, std::queue<Metric>& queue_metric,  std::queue<DistanceReadings>& queueDistance)
    : m_queueMutex(mutex), m_command_queue(command_queue), m_queueMetric(queue_metric), m_queueDistance(queueDistance)
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
    m_command_queue.push(command);
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
    m_command_queue.push(command);
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
    if (m_queueMetric.empty()){
        return Status::OK;
    }

    m_queueMutex.lock();

    while (!m_queueMetric.empty())
    {
        Metric metric = m_queueMetric.front();
        Position position = metric.position;
        std::string status = metric.status;

        
        Telemetric* telemetric = reply->add_telemetric();
        simulation::Position* rpc_position = new simulation::Position();

        rpc_position->set_x(position.posX);
        rpc_position->set_y(position.posY);
        rpc_position->set_z(position.posZ);

        telemetric->set_status(status);
        telemetric->set_allocated_position(rpc_position);

        m_queueMetric.pop();
    }

    m_queueMutex.unlock();
    
    return Status::OK;
}

/// @brief Set the reply to send distances to server
/// @param context Server context
/// @param request Request from the server
/// @param reply Reply to the server
/// @return Status of the request
Status ServiceImplementation::GetDistances(ServerContext* context, const MissionRequest* request, DistancesReply* reply)
{
    if (m_queueDistance.empty()){
        return Status::OK;
    }
    
    m_queueMutex.lock();

    while (!m_queueDistance.empty())
    {
        DistanceReadings distance = m_queueDistance.front();
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

        m_queueDistance.pop();
    }

    m_queueMutex.unlock();
    
    return Status::OK;
}

// /// @brief Set the reply to send distances to server
// /// @param context Server context
// /// @param request Request from the server
// /// @param reply Reply to the server
// /// @return Status of the request
// Status ServiceImplementation::GetLogs(ServerContext* context, const MissionRequest* request, LogReply* reply)
// {
//     if (!m_queueDistance.empty()){
//         m_queueMutex.lock();
//         for (int i = 0; i < m_queueDistance.size(); i++)
//         {
//             DistanceReadings distance = m_queueDistance.front();
//             Position position = m_queuePositionDistance.front();

//             DistanceObstacle* newDistance = reply->add_distanceobstacle();
//             simulation::Position* rpc_position = new simulation::Position();

//             rpc_position->set_x(position.posX);
//             rpc_position->set_y(position.posY);
//             rpc_position->set_z(position.posZ);

//             newDistance->set_front(distance.front);
//             newDistance->set_back(distance.back);
//             newDistance->set_left(distance.left);
//             newDistance->set_right(distance.right);

//             newDistance->set_allocated_position(rpc_position);

//             m_queueDistance.pop();
//             m_queuePositionDistance.pop();
//         }
//         m_queueMutex.unlock();
//     }
//     while(!m_queueDistance.empty()) m_queueDistance.pop();
    
//     return Status::OK;
// }
