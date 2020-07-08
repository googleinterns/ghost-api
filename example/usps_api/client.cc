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
#include <string>
#include <iostream>
#include <cstdint>
#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "proto/usps_api/sfc.grpc.pb.h"

ABSL_FLAG(std::string, HOST, "localhost", "The host of the ip to listen on");
// port will be within range due to flag definition
ABSL_FLAG(std::uint16_t, PORT, 0, "The port of the ip to listen on");
namespace usps_api_client {
  namespace {
    class GhostClient {
      public:
       GhostClient(std::shared_ptr<grpc::Channel> channel)
           : stub_(ghost::SfcService::NewStub(channel)) {}

       // Send a CreateSfc Request to server.
       void CreateSfc(grpc::ClientContext* context,
                      ghost::CreateSfcRequest request,
                      ghost::CreateSfcResponse* response) {
         grpc::Status status = stub_->CreateSfc(context, request, response);
         if(!status.ok()) {
           std::cout << "Failed to create SFC" << std::endl;
         } else {
           std::cout << "Successfully created SFC" << std::endl;
         }
         return;
       }
       // Send a DeleteSfc Request to server.
       void DeleteSfc(grpc::ClientContext* context,
                      ghost::CreateSfcRequest request,
                      ghost::CreateSfcResponse* response) {
         return;
       }
       // Send a QuerySfc Request to server.
       void Query(grpc::ClientContext* context,
                      ghost::CreateSfcRequest request,
                      ghost::CreateSfcResponse* response) {
         return;
       }
      private:
        std::unique_ptr<ghost::SfcService::Stub> stub_;
    };
    // Creates a modifiable GhostFilter at a given request.
    ghost::GhostFilter* GetFilter(ghost::CreateSfcRequest* request) {
      ghost::SfcFilter* sfc_filter = request->mutable_sfc_filter();
      ghost::FilterLayer* layer = sfc_filter->add_filter_layers();
      ghost::GhostFilter* filter = layer->mutable_ghost_filter();
      return filter;
    }
    // Creates SfcRequest using GhostTunnelIdentifier as a filter
    void CreateSfcTunnel(GhostClient* client,
                         std::uint64_t terminal_value,
                         std::uint64_t service_value) {
      grpc::ClientContext context;
      ghost::CreateSfcRequest request;
      ghost::CreateSfcResponse response;
      ghost::GhostFilter* filter = GetFilter(&request);
      ghost::GhostTunnelIdentifier* tunnel_id = filter->mutable_tunnel_id();
      ghost::GhostLabel* terminal_label = tunnel_id->mutable_terminal_label();;
      ghost::GhostLabel* service_label = tunnel_id->mutable_service_label();
      terminal_label->set_value(terminal_value);
      service_label->set_value(service_value);
      client->CreateSfc(&context, request, &response);
    }
    // Creates Sfc using GhostRoutingIdentifier as a filter
    void CreateSfcRoute(GhostClient* client,
                         std::uint64_t value,
                         std::uint32_t prefix_len) {
      grpc::ClientContext context;
      ghost::CreateSfcRequest request;
      ghost::CreateSfcResponse response;
      ghost::GhostFilter* filter = GetFilter(&request);
      ghost::GhostRoutingIdentifier* routing_id = filter->mutable_routing_id();
      ghost::GhostLabelPrefix* dest_label_prefix = routing_id->mutable_destination_label_prefix();
      dest_label_prefix->set_value(value);
      dest_label_prefix->set_prefix_len(prefix_len);
      client->CreateSfc(&context, request, &response);
    }
  }
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::string server_address = absl::GetFlag(FLAGS_HOST) + ":" +
      std::to_string(absl::GetFlag(FLAGS_PORT));
  usps_api_client::GhostClient client(
      grpc::CreateChannel(server_address,
                          grpc::InsecureChannelCredentials()));
  return 0;
}
