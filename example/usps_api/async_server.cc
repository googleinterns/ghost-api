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
#include "async_server.h"

#include <string>
#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "proto/usps_api/sfc.grpc.pb.h"
#include <thread>
#include <chrono>

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
                      std::shared_ptr<Config> config)
       : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), config_(config) {
     Proceed();
   }

   void Proceed() {
     if (status_ == CREATE) {
       status_ = PROCESS;
       service_->RequestCreateSfc(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
     } else if (status_ == PROCESS) {
       // Creates another CreateSfc to handle new requests.
       new CreateSfc(service_, cq_, config_);
       Config* config = config_.get();
       grpc::Status s;
       // If creating is disabled, deny request.
       if (!(config->create_)) {
         s = grpc::Status::CANCELLED;
       }
       const ghost::SfcFilter sfc_filter = request_.sfc_filter();
       // Delay list is active.
       if (config->FilterActive(&(config->delay_))) {
         if (config->FilterMatch(&(config->delay_), &sfc_filter)) {
           int delay_time = config->delay_time_ * 1000;
           // TODO(sam) use multithreading to avoid blocking main thread
           std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
           s = grpc::Status::OK;
         }
       }
       // Deny list is active.
       if (config->FilterActive(&(config->deny_))) {
         if (config->FilterMatch(&(config->deny_), &sfc_filter)) {
           s = grpc::Status::CANCELLED;
         }
         s = grpc::Status::OK;
       } else if (config->FilterActive(&(config->allow_))) {
         // Allow list is active.
         if (config->FilterMatch(&(config->allow_), &sfc_filter)) {
           s = grpc::Status::OK;
         }
         s = grpc::Status::CANCELLED;
       } else { // Neither allow or deny is active.
         s = grpc::Status::OK;
       }

       responder_.Finish(response_, s, this);
       status_ = FINISH;
     } else {
       delete this;
     }
   }
  private:
   ghost::SfcService::AsyncService* service_;
   grpc::ServerCompletionQueue* cq_;
   grpc::ServerContext ctx_;
   ghost::CreateSfcRequest request_;
   ghost::CreateSfcResponse response_;
   grpc::ServerAsyncResponseWriter<ghost::CreateSfcResponse> responder_;
   CallStatus status_;
   std::shared_ptr<Config> config_;
};

class DeleteSfc final : public Call {
  public:
   explicit DeleteSfc(ghost::SfcService::AsyncService* service,
                      grpc::ServerCompletionQueue* cq,
                      std::shared_ptr<Config> config)
       : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), config_(config) {
     Proceed();
   }

   void Proceed() {
     if (status_ == CREATE) {
       status_ = PROCESS;
       service_->RequestDeleteSfc(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
     } else if (status_ == PROCESS) {
       // Creates another DeleteSfc to handle new requests.
       new DeleteSfc(service_, cq_, config_);
       Config* config = config_.get();
       grpc::Status s = grpc::Status::OK;
       // If creating is disabled, deny request.
       if (!(config->del_)) {
         s = grpc::Status::CANCELLED;
       }
       responder_.Finish(response_, s, this);
       status_ = FINISH;
     } else {
       delete this;
     }
   }
  private:
   ghost::SfcService::AsyncService* service_;
   grpc::ServerCompletionQueue* cq_;
   grpc::ServerContext ctx_;
   ghost::DeleteSfcRequest request_;
   ghost::DeleteSfcResponse response_;
   grpc::ServerAsyncResponseWriter<ghost::DeleteSfcResponse> responder_;
   CallStatus status_;
   std::shared_ptr<Config> config_;
};
class Query final : public Call {
  public:
   explicit Query(ghost::SfcService::AsyncService* service,
                      grpc::ServerCompletionQueue* cq,
                      std::shared_ptr<Config> config)
       : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), config_(config) {
     Proceed();
   }

   void Proceed() {
     if (status_ == CREATE) {
       status_ = PROCESS;
       service_->RequestQuery(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
     } else if (status_ == PROCESS) {
       // Creates another Query to handle new requests.
       new Query(service_, cq_, config_);
       Config* config = config_.get();
       grpc::Status s = grpc::Status::OK;
       // If creating is disabled, deny request.
       if (!(config->query_)) {
         s = grpc::Status::CANCELLED;
       }
       responder_.Finish(response_, s, this);
       status_ = FINISH;
     } else {
       delete this;
     }
   }
  private:
   ghost::SfcService::AsyncService* service_;
   grpc::ServerCompletionQueue* cq_;
   grpc::ServerContext ctx_;
   ghost::QueryRequest request_;
   ghost::QueryResponse response_;
   grpc::ServerAsyncResponseWriter<ghost::QueryResponse> responder_;
   CallStatus status_;
   std::shared_ptr<Config> config_;
};
}

// Handles all calls asynchronously.
void usps_api_server::HandleRpcs(ghost::SfcService::AsyncService& service,
                                 grpc::ServerCompletionQueue* cq,
                                 std::shared_ptr<Config> config) {
  new CreateSfc(&service, cq, config);
  new DeleteSfc(&service, cq, config);
  new Query(&service, cq, config);
  void* tag;
  bool ok;
  while (true) {
    // Blocks waiting until next event from completion queue.
    if (!cq->Next(&tag, &ok)) {
      std::cout << "Shutting down..." << std::endl;
      break;
    }
    static_cast<Call*>(tag)->Proceed();
  }
}
