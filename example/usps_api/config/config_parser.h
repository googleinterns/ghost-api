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
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "proto/usps_api/sfc_filter.pb.h"
#include "json/json.h"
#include <string>
#include <list>


namespace usps_api_server {
class Config {
  public:
    struct Filter {
      std::list<ghost::GhostTunnelIdentifier> tunnels;
      std::list<ghost::GhostRoutingIdentifier> routings;
    };
    const std::string filename = "example/usps_api/config/config.json";
    std::string host;
    std::uint16_t port;
    bool enable_ssl;
    std::string key;
    std::string cert;
    std::string root;
    bool create;
    bool del;
    bool query;
    int delay_time;
    Filter deny, allow, delay;
    bool Initialize();
    void MonitorConfig();
    void FileWatch();
    void ParseConfig(Json::Value root);
    void ParseIdentifiers(Filter* filter, Json::Value root);
    bool FilterMatch(Filter* filter, const ghost::SfcFilter* sfc_filter);
    bool FilterActive(Filter* filter);
};
}

#endif
