#pragma once


#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <queue>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "simulation.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using simulation::Simulation;
using simulation::MissionRequest;
using simulation::Reply;

enum class Action {None, Start, Stop, ChooseAngle, Move};

struct Command {
  std::string uri;
  Action action;
};


class ServiceImplementation final : public Simulation::Service {
  public:
    ServiceImplementation(std::mutex& mutex, std::queue<Command>& queue);
    Status StartMission(ServerContext* context, const MissionRequest* request, Reply* reply) override;
    Status EndMission(ServerContext* context, const MissionRequest* request, Reply* reply) override;

  private:
    std::mutex& m_queueMutex;
    std::queue<Command>& m_queue;
};

class SimulationServer final {
  public:
    SimulationServer();
    virtual ~SimulationServer() {}
    void Run(std::string address);
    void Stop();
    bool GetNextCommand(Command* command);
  private:
    std::mutex m_queueMutex;
    std::queue<Command> m_queue;
    std::unique_ptr<Server> m_server;
    ServiceImplementation m_service;
};


