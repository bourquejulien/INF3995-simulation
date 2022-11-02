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
#include <struct/position.h>
#include <struct/distanceReadings.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using simulation::Simulation;
using simulation::MissionRequest;
using simulation::Reply;
using simulation::TelemetricsReply;
using simulation::Telemetric;
using simulation::DistancesReply;
using simulation::DistanceObstacle;

enum class Action {None, Identify, Start, Stop, ChooseAngle, Move};

struct Command {
  std::string uri;
  Action action;
};

class ServiceImplementation final : public Simulation::Service {
  public:
    ServiceImplementation(std::mutex& mutex, std::queue<Command>& queue, std::queue<Position>& position,std::queue<std::string>& status, std::queue<DistanceReadings>& m_queueDistance);
    Status StartMission(ServerContext* context, const MissionRequest* request, Reply* reply) override;
    Status EndMission(ServerContext* context, const MissionRequest* request, Reply* reply) override;
    Status GetTelemetrics(ServerContext* context, const MissionRequest* request, TelemetricsReply* reply);
    Status GetDistances(ServerContext* context, const MissionRequest* request, DistancesReply* reply);
    void UpdateTelemetrics(Position position, std::string status);
    void UpdateDistances(DistanceReadings distance);
  private:
    std::mutex& m_queueMutex;
    std::queue<Command>& m_queue;
    std::queue<Position>& m_queuePosition;
    std::queue<std::string>& m_queueStatus;
    std::queue<DistanceReadings>& m_queueDistance;
};

class SimulationServer final {
  public:
    SimulationServer();
    virtual ~SimulationServer() {}
    void Run(std::string address);
    void Stop();
    bool GetNextCommand(Command* command);
    void UpdateTelemetrics(Position position, std::string status);
    void UpdateDistances(DistanceReadings distance);
  private:
    std::mutex m_queueMutex;
    std::queue<Command> m_queue;
    std::queue<Position> m_queuePosition;
    std::queue<std::string> m_queueStatus;
    std::queue<DistanceReadings> m_queueDistance;
    std::unique_ptr<Server> m_server;
    ServiceImplementation m_service;
};


