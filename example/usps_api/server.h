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
#include "proto/usps_api/sfc.grpc.pb.h"
#include <string>
#include <grpcpp/security/server_credentials.h>

namespace usps_api_server {
class GhostImpl final : public ghost::SfcService::Service {
 public:
   explicit GhostImpl(std::shared_ptr<Config> c) {
    config_ = c;
   }

   grpc::Status CreateSfc(grpc::ServerContext* context,
                       const ghost::CreateSfcRequest* request,
                       ghost::CreateSfcResponse* response) override;
   grpc::Status DeleteSfc(grpc::ServerContext* context,
                             const ghost::DeleteSfcRequest* request,
                             ghost::DeleteSfcResponse* response) override;
   grpc::Status Query(grpc::ServerContext* context,
                         const ghost::QueryRequest* request,
                         ghost::QueryResponse* response) override;

 private:
   std::shared_ptr<Config> config_;
};
} //namespace
