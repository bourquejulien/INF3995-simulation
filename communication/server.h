#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "simulation.grpc.pb.h"
#include <struct/command.h>
#include <struct/distance_reading.h>
#include <struct/log.h>
#include <struct/metric.h>
#include <struct/position.h>

#include "service_implementation.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class SimulationServer final
{
public:
    SimulationServer();
    virtual ~SimulationServer() {}
    void Run(std::string address);
    void Stop();
    bool GetNextCommand(Command* command);
    void SendDone();
    void UpdateTelemetrics(Metric metric);
    void UpdateDistances(DistanceReadings distance);
    void AddLog(std::string message, std::string level);

private:
    std::mutex m_queue_mutex;
    std::queue<Command> m_queue_command;
    std::queue<bool> m_queue_done;
    std::queue<Metric> m_queue_metric;
    std::queue<DistanceReadings> m_queue_distance;
    std::queue<LogData> m_queue_log;
    std::unique_ptr<Server> m_server;
    ServiceImplementation m_service;
};
