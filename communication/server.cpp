#include "server.h"

/// @brief Constructor of the SimulationServer
SimulationServer::SimulationServer() : m_service(m_queue_mutex, m_queue_command, m_queue_metric, m_queue_distance, m_queue_log)
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
    if (m_queue_command.empty() || m_queue_command.front().action == Action::Done)
    {
        return false;
    }

    m_queue_mutex.lock();
    *command = m_queue_command.front();
    m_queue_command.pop();
    m_queue_mutex.unlock();

    return true;
}

/// @brief Mark that a given uri is done with its command execution
/// @param uri Uri that is done
void SimulationServer::SendDoneCommand(std::string uri)
{
    Command doneCommand = {uri, Action::Done};
    m_queue_mutex.lock();
    m_queue_command.push(doneCommand);
    m_queue_mutex.unlock();
}

/// @brief Update the telemetrics in the ServiceImplementation
/// @param metric metric to add to the position queue
void SimulationServer::UpdateTelemetrics(Metric metric)
{
    m_queue_mutex.lock();
    m_queue_metric.push(metric);
    m_queue_mutex.unlock();
}

/// @brief Update the distances in the ServiceImplementation
/// @param distance Distance to add to the queue
void SimulationServer::UpdateDistances(DistanceReadings distance)
{
    m_queue_mutex.lock();
    m_queue_distance.push(distance);
    m_queue_mutex.unlock();
}

/// @brief Update the status in the ServiceImplementation
/// @param message Log message
/// @param level Log level
void SimulationServer::AddLog(std::string message, std::string level)
{
    m_queue_mutex.lock();
    m_queue_log.push(LogData(message, level));
    m_queue_mutex.unlock();
}

/// @brief Shut down the simulation server
void SimulationServer::Stop() { m_server->Shutdown(); }
