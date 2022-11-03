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
#include <struct/metric.h>
#include <struct/distance_reading.h>
#include <struct/command.h>

using simulation::Simulation;
using simulation::MissionRequest;
using simulation::MissionReply;
using simulation::Telemetric;
using simulation::TelemetricsReply;
using simulation::DistanceObstacle;
using simulation::DistancesReply;
using simulation::LogData;
using simulation::LogReply;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class ServiceImplementation final : public Simulation::Service {
  public:
    ServiceImplementation(std::mutex& mutex, std::queue<Command>& queue, std::queue<Metric>& queue_metric, std::queue<DistanceReadings>& queueDistance);
    Status StartMission(ServerContext* context, const MissionRequest* request, MissionReply* reply) override;
    Status EndMission(ServerContext* context, const MissionRequest* request, MissionReply* reply) override;
    Status GetTelemetrics(ServerContext* context, const MissionRequest* request, TelemetricsReply* reply);
    Status GetDistances(ServerContext* context, const MissionRequest* request, DistancesReply* reply);
    Status GetDLogs(ServerContext* context, const MissionRequest* request, LogReply* reply);
  private:
    std::mutex& m_queueMutex;
    std::queue<Command>& m_command_queue;
    std::queue<Metric>& m_queueMetric;
    std::queue<DistanceReadings>& m_queueDistance;
};
