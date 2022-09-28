include_directories(${CMAKE_SOURCE_DIR}/build/communication)
add_library(main_simulation SHARED main_simulation.h main_simulation.cpp)
target_link_libraries(main_simulation
  simulation_server
  hw_grpc_proto
  argos3core_simulator
  argos3plugin_simulator_crazyflie
  argos3plugin_simulator_genericrobot
  hw_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)
