// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "config_helper.h"
#include "example/usps_api/server.h"
#include "proto/usps_api/sfc.grpc.pb.h"
using namespace ConfigHelper;
using namespace ghost;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::_;
using ::testing::Return;
namespace {
grpc::Status CreateSfc(std::shared_ptr<usps_api_server::Config> config,
                       CreateSfcRequest request) {
  usps_api_server::GhostImpl service(config);
  grpc::ServerContext context;
  CreateSfcResponse response;
  return service.CreateSfc(&context, &request, &response);
}
grpc::Status DeleteSfc(std::shared_ptr<usps_api_server::Config> config) {
  usps_api_server::GhostImpl service(config);
  grpc::ServerContext context;
  const DeleteSfcRequest request;
  DeleteSfcResponse response;
  return service.DeleteSfc(&context, &request, &response);
}
grpc::Status Query(std::shared_ptr<usps_api_server::Config> config) {
  usps_api_server::GhostImpl service(config);
  grpc::ServerContext context;
  const QueryRequest request;
  QueryResponse response;
  return service.Query(&context, &request, &response);
}
GhostFilter* GetFilter(CreateSfcRequest &request) {
  SfcFilter* sfc_filter = request.mutable_sfc_filter();
  FilterLayer* layer = sfc_filter->add_filter_layers();
  GhostFilter* filter = layer->mutable_ghost_filter();
  return filter;
}
GhostTunnelIdentifier* CreateSfcTunnel(std::uint64_t terminal_value,
                                       std::uint64_t service_value,
                                       CreateSfcRequest &request) {
  GhostFilter* filter = GetFilter(request);
  GhostTunnelIdentifier* tunnel_id = filter->mutable_tunnel_id();
  GhostLabel* terminal_label = tunnel_id->mutable_terminal_label();
  GhostLabel* service_label = tunnel_id->mutable_service_label();
  terminal_label->set_value(terminal_value);
  service_label->set_value(service_value);
  return tunnel_id;
}
GhostRoutingIdentifier* CreateSfcRoute(std::uint64_t value,
                                       std::uint32_t prefix_len,
                                       CreateSfcRequest &request) {
  GhostFilter* filter = GetFilter(request);
  GhostRoutingIdentifier* routing_id = filter->mutable_routing_id();
  GhostLabelPrefix* dest_label_prefix = routing_id->mutable_destination_label_prefix();
  dest_label_prefix->set_value(value);
  dest_label_prefix->set_prefix_len(prefix_len);
  return routing_id;
}
} // namespace

TEST(ServerTest, CanCreate) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request;
  grpc::Status status = CreateSfc(config, request);
  EXPECT_TRUE(status.ok());
  config.get()->create_ = false;
  status = CreateSfc(config, request);
  EXPECT_FALSE(status.ok());
}
TEST(ServerTest, CanDelete) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  grpc::Status status = DeleteSfc(config);
  EXPECT_TRUE(status.ok());
  config.get()->del_ = false;
  status = DeleteSfc(config);
  EXPECT_FALSE(status.ok());
}
TEST(ServerTest, CanQuery) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  grpc::Status status = Query(config);
  EXPECT_TRUE(status.ok());
  config->query_ = false;
  status = Query(config);
  EXPECT_FALSE(status.ok());
}
TEST(ServerTest, DenyTunnelTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostTunnelIdentifier* tunnel_id1 = CreateSfcTunnel(100, 1, request1);
  GhostTunnelIdentifier* tunnel_id2 = CreateSfcTunnel(200, 1, request2);
  config.get()->deny_.tunnels.push_back(*tunnel_id1);
  config.get()->deny_.tunnels.push_back(*tunnel_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_FALSE(status1.ok());
  EXPECT_FALSE(status2.ok());
  config.get()->deny_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_FALSE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->deny_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}
TEST(ServerTest, DenyRoutingTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostRoutingIdentifier* routing_id1 = CreateSfcRoute(100, 1, request1);
  GhostRoutingIdentifier* routing_id2 = CreateSfcRoute(200, 1, request2);
  config.get()->deny_.routings.push_back(*routing_id1);
  config.get()->deny_.routings.push_back(*routing_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_FALSE(status1.ok());
  EXPECT_FALSE(status2.ok());
  config.get()->deny_.routings.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_FALSE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->deny_.routings.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}
TEST(ServerTest, AllowTunnelTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostTunnelIdentifier* tunnel_id1 = CreateSfcTunnel(100, 1, request1);
  GhostTunnelIdentifier* tunnel_id2 = CreateSfcTunnel(200, 1, request2);
  config.get()->allow_.tunnels.push_back(*tunnel_id1);
  config.get()->allow_.tunnels.push_back(*tunnel_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->allow_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_FALSE(status2.ok());
  config.get()->allow_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}
TEST(ServerTest, AllowRoutingTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostRoutingIdentifier* routing_id1 = CreateSfcRoute(100, 1, request1);
  GhostRoutingIdentifier* routing_id2 = CreateSfcRoute(200, 1, request2);
  config.get()->allow_.routings.push_back(*routing_id1);
  config.get()->allow_.routings.push_back(*routing_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->allow_.routings.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_FALSE(status2.ok());
  config.get()->allow_.routings.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}

TEST(ServerTest, DelayTunnelTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostTunnelIdentifier* tunnel_id1 = CreateSfcTunnel(100, 1, request1);
  GhostTunnelIdentifier* tunnel_id2 = CreateSfcTunnel(200, 1, request2);
  config.get()->delay_.tunnels.push_back(*tunnel_id1);
  config.get()->delay_.tunnels.push_back(*tunnel_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->delay_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->delay_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}
TEST(ServerTest, DelayRoutingTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostRoutingIdentifier* routing_id1 = CreateSfcRoute(100, 1, request1);
  GhostRoutingIdentifier* routing_id2 = CreateSfcRoute(200, 1, request2);
  config.get()->delay_.routings.push_back(*routing_id1);
  config.get()->delay_.routings.push_back(*routing_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->delay_.routings.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->delay_.routings.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}
TEST(ServerTest, DelayAndDenyTest) {
  std::shared_ptr<usps_api_server::Config> config = CreateSharedConfig();
  config.get()->Initialize();
  CreateSfcRequest request1;
  CreateSfcRequest request2;
  grpc::Status status1 = CreateSfc(config, request1);
  grpc::Status status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  GhostTunnelIdentifier* tunnel_id1 = CreateSfcTunnel(100, 1, request1);
  GhostTunnelIdentifier* tunnel_id2 = CreateSfcTunnel(200, 1, request2);
  config.get()->delay_.tunnels.push_back(*tunnel_id1);
  config.get()->deny_.tunnels.push_back(*tunnel_id2);
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_FALSE(status2.ok());
  config.get()->delay_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_FALSE(status2.ok());
  config.get()->delay_.tunnels.push_back(*tunnel_id1);
  config.get()->deny_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
  config.get()->delay_.tunnels.pop_back();
  status1 = CreateSfc(config, request1);
  status2 = CreateSfc(config, request2);
  EXPECT_TRUE(status1.ok());
  EXPECT_TRUE(status2.ok());
}
