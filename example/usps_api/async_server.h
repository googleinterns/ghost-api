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
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "proto/usps_api/sfc.grpc.pb.h"

namespace usps_api_server {
class Call {
  public:
   virtual void Proceed() = 0;
   enum CallStatus { CREATE, PROCESS, FINISH };
};

class CreateSfc final : public Call {
 public:
   explicit CreateSfc(ghost::SfcService::AsyncService* service,
                        grpc::ServerCompletionQueue* cq,
                        std::shared_ptr<Config> config);
   void Proceed();
 private:
   ghost::SfcService::AsyncService* service_;
   grpc::ServerCompletionQueue* cq_;
   grpc::ServerAsyncResponseWriter<ghost::CreateSfcResponse> responder_;
   grpc::ServerContext ctx_;
   CallStatus status_;
   std::shared_ptr<Config> config_;
   ghost::CreateSfcRequest request_;
   ghost::CreateSfcResponse response_;
};
class DeleteSfc final : public Call {
 public:
  explicit DeleteSfc(ghost::SfcService::AsyncService* service,
                     grpc::ServerCompletionQueue* cq,
                     std::shared_ptr<Config> config);
  void Proceed();
 private:
  ghost::SfcService::AsyncService* service_;
  grpc::ServerCompletionQueue* cq_;
  grpc::ServerContext ctx_;
  grpc::ServerAsyncResponseWriter<ghost::DeleteSfcResponse> responder_;
  CallStatus status_;
  std::shared_ptr<Config> config_;
  ghost::DeleteSfcRequest request_;
  ghost::DeleteSfcResponse response_;
};
class Query final : public Call {
 public:
  explicit Query(ghost::SfcService::AsyncService* service,
                     grpc::ServerCompletionQueue* cq,
                     std::shared_ptr<Config> config);
  void Proceed();
 private:
  ghost::SfcService::AsyncService* service_;
  grpc::ServerCompletionQueue* cq_;
  grpc::ServerContext ctx_;
  grpc::ServerAsyncResponseWriter<ghost::QueryResponse> responder_;
  CallStatus status_;
  std::shared_ptr<Config> config_;
  ghost::QueryRequest request_;
  ghost::QueryResponse response_;
};

void HandleRpcs(ghost::SfcService::AsyncService& service,
                grpc::ServerCompletionQueue* cq,
                std::shared_ptr<Config> config);
} //namespace
