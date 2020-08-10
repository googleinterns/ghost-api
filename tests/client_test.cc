#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "proto/usps_api/sfc_mock.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "example/usps_api/server.h"
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::_;
using ::testing::Return;

namespace {
  class FakeClient {
   public:
    explicit FakeClient(ghost::SfcService::StubInterface* stub) : stub_(stub) {}
    void DoCreateSfc() {
      grpc::ClientContext context;
      ghost::CreateSfcRequest request;
      ghost::CreateSfcResponse response;
      grpc::Status status = stub_->CreateSfc(&context, request, &response);
      EXPECT_TRUE(status.ok());
    }
    void DoDeleteSfc() {
      grpc::ClientContext context;
      ghost::DeleteSfcRequest request;
      ghost::DeleteSfcResponse response;
      grpc::Status status = stub_->DeleteSfc(&context, request, &response);
      EXPECT_TRUE(status.ok());
    }
    void DoQuery() {
      grpc::ClientContext context;
      ghost::QueryRequest request;
      ghost::QueryResponse response;
      grpc::Status status = stub_->Query(&context, request, &response);
      EXPECT_TRUE(status.ok());
    }
   private:
    ghost::SfcService::StubInterface* stub_;
  };
}

TEST(ClientTest, CanCreateSfc) {
  ghost::MockSfcServiceStub stub;
  EXPECT_CALL(stub, CreateSfc(_,_,_))
      .Times(AtLeast(1))
      .WillOnce(Return(grpc::Status::OK));
  FakeClient client(&stub);
  client.DoCreateSfc();
}
TEST(ClientTest, CanDeleteSfc) {
  ghost::MockSfcServiceStub stub;
  EXPECT_CALL(stub, DeleteSfc(_,_,_))
      .Times(AtLeast(1))
      .WillOnce(Return(grpc::Status::OK));
  FakeClient client(&stub);
  client.DoDeleteSfc();

}
TEST(ClientTest, CanQuery) {
  ghost::MockSfcServiceStub stub;
  EXPECT_CALL(stub, Query(_,_,_))
      .Times(AtLeast(1))
      .WillOnce(Return(grpc::Status::OK));
  FakeClient client(&stub);
  client.DoQuery();
}
