#include "config_parser.h"
#include "json/json.h"
#include <iostream>
#include <fstream>

void Config::Initialize() {
  char filename[] = "config/config.json";
  std::ifstream file(filename, std::ifstream::binary);
  if (file.good()) {
    std::cout << "Reading from config.json" << std::endl;
    Json::Value root;
    file >> root;
    Config::ParseConfig(root);
  } else {
    std::cout << "Created config.json" << std::endl;
    std::ofstream outfile(filename);
  }
}

void Config::ParseConfig(Json::Value root) {
  std::string enc = root.get("encoding", "").asString();
  std::cout << enc << std::endl;
}
