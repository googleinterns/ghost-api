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
#include "server.h"
#include "utils/file_reader.h"

#include <string>
#include <iostream>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>
#include <arpa/inet.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"


ABSL_FLAG(std::string, HOST, "", "The host of the ip to listen on");
// The port will be within a valid range due to the flag being uint16_t type.
ABSL_FLAG(std::uint16_t, PORT, 0, "The port of the ip to listen on");

// Gets server credentials if ssl is enabled
std::shared_ptr<grpc::ServerCredentials> GetCreds(usps_api_server::Config* config) {
  if (config->enable_ssl) {
    std::string key = FileReader::ReadString(config->key);
    std::string cert = FileReader::ReadString(config->cert);
    std::string root = FileReader::ReadString(config->root);
    grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = {key, cert};
    grpc::SslServerCredentialsOptions ssl_opts;
    ssl_opts.pem_root_certs = root;
    ssl_opts.pem_key_cert_pairs.push_back(pkcp);
    return grpc::SslServerCredentials(ssl_opts);
  } else {
    return grpc::InsecureServerCredentials();
  }
}

// Runs the server using the grpc server builder.
// See https://grpc.io/docs/languages/cpp/basics/ for more info
// TODO(sam) use absl:status as return type & implement configuration.
void Run(std::string host, uint16_t port, usps_api_server::Config* config) {
  std::string server_address = host + ":" + std::to_string(port);
  std::cout << "Server attempting to listen on " << server_address << std::endl;
  grpc::ServerBuilder builder;
  usps_api_server::GhostImpl service(config);
  std::shared_ptr<grpc::ServerCredentials> creds = GetCreds(config);
  builder.AddListeningPort(server_address, creds);
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

// This uses abseil flags to parse the host & port at runtime.
// See https://abseil.io/docs/cpp/guides/flags for more information on flags.
int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  usps_api_server::Config* config = new usps_api_server::Config();
  config->Initialize();
  config->MonitorConfig();
  // Prioritize using address specified in flags.
  if (IsValidAddress(absl::GetFlag(FLAGS_HOST))) {
    Run(absl::GetFlag(FLAGS_HOST), absl::GetFlag(FLAGS_PORT), config);
  } else if (config->host != "") {
    // Fall back to address in config if present.
    std::string host = config->host;
    std::uint16_t port = config->port;
    if (IsValidAddress(host)) {
      Run(host, port, config);
    } else {
      std::cout << "Invalid address in config." << std::endl;
    }
  } else {
    std::cout << "Invalid address specified." << std::endl;
  }
  return 0;
}
