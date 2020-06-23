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

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "proto/usps_api/sfc.grpc.pb.h"

ABSL_FLAG(std::string, HOST, "0.0.0.0", "The host of the ip to listen on");
// port will be within range due to flag definition
ABSL_FLAG(std::uint16_t, PORT, 0, "The port of the ip to listen on");

// implementation of 3 rpc functions
class GhostImpl final : public ghost::SfcService::Service {
  public:
    grpc::Status CreateSfc(grpc::ServerContext* context,
                     const ghost::CreateSfcRequest* request,
                     ghost::CreateSfcResponse* response) override {
      return grpc::Status::OK;
    }
    grpc::Status DeleteSfc(grpc::ServerContext* context,
                           const ghost::DeleteSfcRequest* request,
                           ghost::DeleteSfcResponse* response) override {
      return grpc::Status::OK;
    }
    grpc::Status Query(grpc::ServerContext* context,
                       const ghost::QueryRequest* request,
                       ghost::QueryResponse* response) override {
      return grpc::Status::OK;
    }
};

namespace servercore {
  // Runs server using server builder
  // see https://grpc.io/docs/languages/cpp/basics/
  // TODO(sam) use absl:status as return type
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

  // verifies a valid IPV4 address in the form A.B.C.D
  // port is already valid due to flag parser
  bool IsValidAddress(std::string host) {
    int A, B, C, D;
    // temp variable to catch longer input ip address
    char terms [1];
    int matched = sscanf(host.c_str(), "%d.%d.%d.%d%s", &A, &B, &C, &D, terms);
    if (matched != 4) {
      return false;
    } else if (A < 0 || B < 0 || C < 0 || D < 0) {
      return false;
    } else if (A > 255 || B > 255 || C > 255 || D > 255) {
      return false;
    }
    return true;
  }
}

// using abseil flags to parse host & port at runtime
// see https://abseil.io/docs/cpp/guides/flags
int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  if (servercore::IsValidAddress(absl::GetFlag(FLAGS_HOST))) {
    servercore::Run(absl::GetFlag(FLAGS_HOST), absl::GetFlag(FLAGS_PORT));
  } else {
    std::cout << "Invalid host or port" << std::endl;
  }
  return 0;
}
