#include "server.h"

#include <string>
#include <iostream>

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>


/* https://grpc.io/docs/languages/cpp/basics/
 * Runs server using server builder
 */ 
void RunServer(std::string host, std::string port) {
  std::string server_address = host + ":" + port;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
  // TODO(Sam) must register service here or else won't be polled
  if(server!=NULL){
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
  }
}
