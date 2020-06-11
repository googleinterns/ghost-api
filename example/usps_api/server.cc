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
  std::cout << "Server attempting to listen on " << server_address << std::endl;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
  // TODO(Sam) must register service here or else won't be polled
  if(server!=nullptr){
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
  } else {
    std::cout << "Server could not listen on " << server_address << std::endl;
  }
}

/*
 * main takes in host & port and runs a server
 * argv[1]: host
 * argv[2]: port
 */ 
int main(int argc, char *argv[]) {
  if(argc!=3){
    std::cout << "Usage: ./run-server <host> <port>" << std::endl;
    return 1;
  } else {
    std::string host = argv[1];
    std::string port = argv[2];
    RunServer(host, port);
    return 0;
  }
}
