#include "server.h"

/// @brief Constructor of the SimulationServer
SimulationServer::SimulationServer() : m_service(m_queueMutex, m_command_queue, m_queueMetric, m_queueDistance)
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
    if (m_command_queue.empty())
    {
        return false;
    }

    m_queueMutex.lock();
    *command = m_command_queue.front();
    m_command_queue.pop();
    m_queueMutex.unlock();

    return true;
}

/// @brief Update the telemetrics in the ServiceImplementation
/// @param position position to add to the position queue
/// @param status status to add to the status queue
void SimulationServer::UpdateTelemetrics(Metric metric)
{
    m_queueMutex.lock();
    m_queueMetric.push(metric);
    m_queueMutex.unlock();
}

/// @brief Update the distances in the ServiceImplementation
/// @param distance Distance to add to the queue
void SimulationServer::UpdateDistances(DistanceReadings distance)
{
    m_queueMutex.lock();
    m_queueDistance.push(distance);
    m_queueMutex.unlock();
}

/// @brief Shut down the simulation server
void SimulationServer::Stop() { m_server->Shutdown(); }
