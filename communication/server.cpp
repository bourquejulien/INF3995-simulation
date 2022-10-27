#include "server.h"

ServiceImplementation::ServiceImplementation(
    std::mutex& mutex, std::queue<Command>& queue)
    : m_queueMutex(mutex), m_queue(queue)
{
}

Status ServiceImplementation::StartMission(
    ServerContext* context, const MissionRequest* request, Reply* reply)
{
    Command command = {request->uri(), Action::Start};
    m_queueMutex.lock();

    m_queue.push(command);

    m_queueMutex.unlock();

    reply->set_message("Success");
    return Status::OK;
}

Status ServiceImplementation::EndMission(
    ServerContext* context, const MissionRequest* request, Reply* reply)
{
    Command command = {request->uri(), Action::Stop};

    m_queueMutex.lock();
    m_queue.push(command);
    m_queueMutex.unlock();

    reply->set_message("Success");
    return Status::OK;
}

Status ServiceImplementation::GetPosition(
    ServerContext* context, const MissionRequest* request, PositionReply* reply)
{
    reply->set_posx(dronePosition.posX);
    reply->set_posy(dronePosition.posY);
    reply->set_posz(dronePosition.posZ);
    return Status::OK;
}

void ServiceImplementation::SetPosition(Position position){
    dronePosition.posX = position.posX;
    dronePosition.posY = position.posY;
    dronePosition.posZ = position.posZ;
}

SimulationServer::SimulationServer() : m_service(m_queueMutex, m_queue)
{
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
}

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

void SimulationServer::SetPosition(Position position){
    m_service.SetPosition(position);
}

void SimulationServer::Stop() { m_server->Shutdown(); }
