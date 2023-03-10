#pragma once

#include <chrono>
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

using simulation::DistanceObstacle;
using simulation::DistancesReply;
using simulation::LogReply;
using simulation::MissionReply;
using simulation::MissionRequest;
using simulation::Simulation;
using simulation::Telemetric;
using simulation::TelemetricsReply;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class ServiceImplementation final : public Simulation::Service
{
public:
    ServiceImplementation(
        std::mutex& mutex, std::queue<Command>& command_queue,
        std::queue<bool>& done_queue, std::queue<Metric>& queue_metric,
        std::queue<DistanceReadings>& queue_distance,
        std::queue<LogData>& queue_log);
    Status StartMission(
        ServerContext* context, const MissionRequest* request,
        MissionReply* reply) override;
    Status EndMission(
        ServerContext* context, const MissionRequest* request,
        MissionReply* reply) override;
    Status ReturnToBase(
        ServerContext* context, const MissionRequest* request,
        MissionReply* reply) override;
    Status GetTelemetrics(
        ServerContext* context, const MissionRequest* request,
        TelemetricsReply* reply);
    Status GetDistances(
        ServerContext* context, const MissionRequest* request,
        DistancesReply* reply);
    Status GetLogs(
        ServerContext* context, const MissionRequest* request, LogReply* reply);

private:
    std::mutex& m_queue_mutex;
    std::queue<Command>& m_queue_command;
    std::queue<bool>& m_queue_done;
    std::queue<Metric>& m_queue_metric;
    std::queue<DistanceReadings>& m_queue_distance;
    std::queue<LogData>& m_queue_log;
};
