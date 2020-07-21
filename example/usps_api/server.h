#include "config/config_parser.h"
#include "proto/usps_api/sfc.grpc.pb.h"
#include <string>
#include <grpcpp/security/server_credentials.h>

namespace usps_api_server {
  class GhostImpl final : public ghost::SfcService::Service {
   public:
     explicit GhostImpl(Config*& c) {
      config = c;
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
     Config* config;
  };
}
