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
#include "config_helper.h"
#include "example/usps_api/config/config_parser.h"
#include <fstream>
#include <iostream>
#include "json/json.h"
// Helper function that writes to the configuration file.
void ConfigHelper::WriteToConfig(usps_api_server::Config* config, Json::Value root) {
  std::ofstream outfile(config->kFilename);
  Json::StreamWriterBuilder builder;
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(root, &outfile);
  outfile.close();
}
// Creates new config.json
void ConfigHelper::CreateNewFile(usps_api_server::Config* config) {
  std::ifstream file(config->kFilename, std::ifstream::binary);
  if(file.good()) {
    std::remove((config->kFilename).c_str());
  }
  Json::Value root;
  WriteToConfig(config, root);
}
// Helper function that creates a config instance.
usps_api_server::Config* ConfigHelper::CreateConfig() {
  usps_api_server::Config *config = new usps_api_server::Config();
  CreateNewFile(config);
  return config;
}
std::shared_ptr<usps_api_server::Config> ConfigHelper::CreateSharedConfig() {
  std::shared_ptr<usps_api_server::Config> config =
      std::make_shared<usps_api_server::Config>();
  CreateNewFile(config.get());
  return config;
}
