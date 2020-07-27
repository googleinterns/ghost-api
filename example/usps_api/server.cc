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

#include <string>
#include <iostream>
#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "proto/usps_api/sfc.grpc.pb.h"
#include <thread>
#include <chrono>

// This is the implementation of the 3 rpc functions in the proto file.
grpc::Status usps_api_server::GhostImpl::CreateSfc(grpc::ServerContext* context,
                 const ghost::CreateSfcRequest* request,
                 ghost::CreateSfcResponse* response) {
  Config* config = config_.get();
  // If creating is disabled, deny request.
  if (!(config->create_)) {
    return grpc::Status::CANCELLED;
  }
  const ghost::SfcFilter sfc_filter = request->sfc_filter();
  // Delay list is active.
  if (config->FilterActive(&(config->delay_))) {
    if (config->FilterMatch(&(config->delay_), &sfc_filter)) {
      int delay_time = config->delay_time_ * 1000;
      // TODO(sam) use multithreading to avoid blocking main thread
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
      return grpc::Status::OK;
    }
  }
  // Deny list is active.
  if (config->FilterActive(&(config->deny_))) {
    if (config->FilterMatch(&(config->deny_), &sfc_filter)) {
      return grpc::Status::CANCELLED;
    }
    return grpc::Status::OK;
  } else if (config->FilterActive(&(config->allow_))) {
    // Allow list is active.
    if (config->FilterMatch(&(config->allow_), &sfc_filter)) {
      return grpc::Status::OK;
    }
    return grpc::Status::CANCELLED;
  } else { // Neither allow or deny is active.
    return grpc::Status::OK;
  }
}
grpc::Status usps_api_server::GhostImpl::DeleteSfc(grpc::ServerContext* context,
                       const ghost::DeleteSfcRequest* request,
                       ghost::DeleteSfcResponse* response) {
  Config* config = config_.get();
  // If deleting is disabled, deny request.
  if (!(config->del_)) {
    return grpc::Status::CANCELLED;
  }
  return grpc::Status::OK;
}
grpc::Status usps_api_server::GhostImpl::Query(grpc::ServerContext* context,
                   const ghost::QueryRequest* request,
                   ghost::QueryResponse* response){
  Config* config = config_.get();
  // If querying is disabled, deny request.
  if (!(config->query_)) {
    return grpc::Status::CANCELLED;
  }
  return grpc::Status::OK;
}
