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
#include "config/config_parser.h"

#include <string>
#include <iostream>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "proto/usps_api/sfc.grpc.pb.h"
#include <arpa/inet.h>
#include <thread>
#include <chrono>

ABSL_FLAG(std::string, HOST, "", "The host of the ip to listen on");
// The port will be within a valid range due to the flag being uint16_t type.
ABSL_FLAG(std::uint16_t, PORT, 0, "The port of the ip to listen on");
namespace usps_api_server {
  // This is the implementation of the 3 rpc functions in the proto file.
  namespace {
    Config* config = new Config;
    class GhostImpl final : public ghost::SfcService::Service {
      public:
        grpc::Status CreateSfc(grpc::ServerContext* context,
                         const ghost::CreateSfcRequest* request,
                         ghost::CreateSfcResponse* response) override {
          // If creating is disabled, deny request.
          if (!(config->create)) {
            return grpc::Status::CANCELLED;
          }
          const ghost::SfcFilter sfc_filter = request->sfc_filter();
          // Delay list is active.
          if (config->FilterActive(&(config->delay))) {
            if (config->FilterMatch(&(config->delay), sfc_filter)) {
              int delay_time = config->delay_time * 1000;
              // TODO(sam) use multithreading to avoid blocking main thread
              std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
              return grpc::Status::OK;
            }
          }
          // Deny list is active.
          if (config->FilterActive(&(config->deny))) {
            if (config->FilterMatch(&(config->deny), sfc_filter)) {
              return grpc::Status::CANCELLED;
            }
            return grpc::Status::OK;
          } else if (config->FilterActive(&(config->allow))) {
            // Allow list is active.
            if (config->FilterMatch(&(config->allow), sfc_filter)) {
              return grpc::Status::OK;
            }
            return grpc::Status::CANCELLED;
          } else { // Neither allow or deny is active.
            return grpc::Status::OK;
          }
        }
        grpc::Status DeleteSfc(grpc::ServerContext* context,
                               const ghost::DeleteSfcRequest* request,
                               ghost::DeleteSfcResponse* response) override {
          // If deleting is disabled, deny request.
          if (!(config->del)) {
            return grpc::Status::CANCELLED;
          }
          return grpc::Status::OK;
        }
        grpc::Status Query(grpc::ServerContext* context,
                           const ghost::QueryRequest* request,
                           ghost::QueryResponse* response) override {
          // If querying is disabled, deny request.
          if (!(config->query)) {
            return grpc::Status::CANCELLED;
          }
          return grpc::Status::OK;
        }
    };
    // Runs the server using the grpc server builder.
    // See https://grpc.io/docs/languages/cpp/basics/ for more info
    // TODO(sam) use absl:status as return type & implement configuration.
    void Run(std::string host, uint16_t port) {
      std::string server_address = host + ":" + std::to_string(port);
      std::cout << "Server attempting to listen on " << server_address << std::endl;
      grpc::ServerBuilder builder;
      GhostImpl service;
      builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
      builder.RegisterService(&service);
      std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
      if (server == nullptr) {
        std::cout << "Server could not listen on " << server_address << std::endl;
        return;
      }
      std::cout << "Server listening on " << server_address << std::endl;
      server->Wait();
    }

    // Verifies a valid IPV4 address in the form A.B.C.D or localhost.
    bool IsValidAddress(std::string host) {
      if (host.compare("localhost") == 0) {
        return true;
      }
      // The inet_pton library handles ip verification.
      struct sockaddr_in sa;
      int result = inet_pton(AF_INET, host.c_str(), &(sa.sin_addr));
      return result == 1;
    }
  }
}
// This uses abseil flags to parse the host & port at runtime.
// See https://abseil.io/docs/cpp/guides/flags for more information on flags.
int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  usps_api_server::config->Initialize();  
  // Prioritize using address specified in flags.
  if (usps_api_server::IsValidAddress(absl::GetFlag(FLAGS_HOST))) {
    usps_api_server::Run(absl::GetFlag(FLAGS_HOST), absl::GetFlag(FLAGS_PORT));
  } else if (usps_api_server::config->host != "") {
    // Fall back to address in config if present.
    std::string host = usps_api_server::config->host;
    std::uint16_t port = usps_api_server::config->port;
    if (usps_api_server::IsValidAddress(host)) {
      usps_api_server::Run(host, port);
    } else {
      std::cout << "Invalid address in config." << std::endl;
    }
  } else {
    std::cout << "Invalid address specified." << std::endl;
  }
  return 0;
}
