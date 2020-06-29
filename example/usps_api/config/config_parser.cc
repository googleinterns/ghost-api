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
#include "config_parser.h"
#include "json/json.h"
#include <iostream>
#include <fstream>

// Reads the configuration file or creates one if not present.
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

// A helper function to parse the values in the configuration file.
// TODO(sam) Implement functionality & gmock tests for this function.
void Config::ParseConfig(Json::Value root) {
  std::string enc = root.get("encoding", "").asString();
  std::cout << enc << std::endl;
}
