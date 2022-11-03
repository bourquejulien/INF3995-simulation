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
#include <struct/log.h>
#include <struct/command.h>
#include <struct/metric.h>
#include <struct/distance_reading.h>

#include "service_implementation.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class SimulationServer final {
  public:
    SimulationServer();
    virtual ~SimulationServer() {}
    void Run(std::string address);
    void Stop();
    bool GetNextCommand(Command* command);
    void UpdateTelemetrics(Metric metric);
    void UpdateDistances(DistanceReadings distance);
    void AddLog(std::string message, std::string level);
  private:
    std::mutex m_queueMutex;
    std::queue<Command> m_command_queue;
    std::queue<Metric> m_queueMetric;
    std::queue<DistanceReadings> m_queueDistance;
    std::queue<LogData> m_log_queue;
    std::unique_ptr<Server> m_server;
    ServiceImplementation m_service;
};
