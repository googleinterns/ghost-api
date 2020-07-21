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
#include "proto/usps_api/ghost_label.pb.h"
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <google/protobuf/util/message_differencer.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <thread>
#include <sys/types.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN ((1024*( EVENT_SIZE + 16 )))

// Runs FileWatch on another thread
void usps_api_server::Config::MonitorConfig() {
  std::thread monitor (&usps_api_server::Config::FileWatch, this);
  monitor.detach();
}

// Watches the config file for updates and reloads it.
void usps_api_server::Config::FileWatch() {
  while(true) {
    char buffer[BUF_LEN];
    int fd = inotify_init();
    int wd = inotify_add_watch(fd, "example/usps_api/config/config.json",
                               IN_MODIFY | IN_CREATE);
    int i = 0;
    int length = read(fd, buffer, BUF_LEN);
    while(i < length) {
      struct inotify_event* event = ( struct inotify_event * ) &buffer[ i ];
      Initialize();
      i+= EVENT_SIZE + event->len;
    }
    inotify_rm_watch(fd, wd);
    close(fd);
  }
}

// Reads the configuration file or creates one if not present.
bool usps_api_server::Config::Initialize() {
  const std::string filename = "example/usps_api/config/config.json";
  std::ifstream file(filename, std::ifstream::binary);
  if (file.good()) {
    std::cout << "Reading from " << filename << std::endl;
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    bool valid_json = Json::parseFromStream(builder, file, &root, &errs);
    if (valid_json) {
      Config::ParseConfig(root);
      return true;
    } else {
      std::cout << "Invalid JSON in " << filename  << errs << std::endl;
      return false;
    }
  } else {
    std::cout << filename << " does not exist" << std::endl;
    return false;
  }
}

// A helper function to parse the values in the configuration file.
// TODO(sam) Implement functionality & gmock tests for this function.
void usps_api_server::Config::ParseConfig(Json::Value root) {
  const Json::Value address = root["address"];
  host = address.get("host", "").asString();
  port = address.get("port", 0).asInt();

  const Json::Value requests = root["requests"];
  create = requests.get("create", true).asBool();
  del = requests.get("delete", true).asBool();
  query = requests.get("query", true).asBool();

  const Json::Value sfcfilter = root["sfcfilter"];
  ParseIdentifiers(&deny, sfcfilter["deny"]);
  ParseIdentifiers(&allow, sfcfilter["allow"]);
  ParseIdentifiers(&delay, sfcfilter["delay"]);
  delay_time = sfcfilter["delay"].get("seconds", 0).asInt();

  const Json::Value ssl = root["ssl"];
  enable_ssl = ssl.get("enable", false).asBool();
  key = ssl.get("key", "").asString();
  cert = ssl.get("cert", "").asString();
  root = ssl.get("root", "").asString();
}

// Parses the configuration file for TunnelIdentifiers and RoutingIdentifiers.
void usps_api_server::Config::ParseIdentifiers(Filter* filter,
                                               Json::Value root) {
  const Json::Value tunnels = root["ghost_tunnel_identifier"]["ghostlabel"];
  const Json::Value routings = root["ghost_routing_identifier"]["destination_label_prefix"];
  filter->tunnels.clear();
  filter->routings.clear();
  // Adds given ghostlabel's in ghost_tunnel_identifier to filter's tunnel list.
  for (unsigned int index = 0; index < tunnels.size(); ++index) {
    ghost::GhostTunnelIdentifier tunnel_id;
    ghost::GhostLabel* terminal_label = tunnel_id.mutable_terminal_label();
    ghost::GhostLabel* service_label = tunnel_id.mutable_service_label();
    terminal_label->set_value(tunnels[index].get("terminal_label", 0).asInt());
    service_label->set_value(tunnels[index].get("service_label", 0).asInt());
    filter->tunnels.push_back(tunnel_id);
  }
  // Adds given destination_label_prefix's in ghost_routing_identifier to
  // filter's routing list.
  for (unsigned int index = 0; index < routings.size(); ++index) {
    ghost::GhostRoutingIdentifier routing_id;
    ghost::GhostLabelPrefix* dest_label = routing_id.mutable_destination_label_prefix();
    dest_label->set_value(routings[index].get("value", 0).asInt());
    dest_label->set_prefix_len(routings[index].get("prefix_len", 0).asInt());
    filter->routings.push_back(routing_id);
  }
}
// Will return true if the exact same filters in 'sfc_filter' are in 'filter'
bool usps_api_server::Config::FilterMatch(Filter* filter,
                                          ghost::SfcFilter sfc_filter) {
  // Iterates over all filter layers in sfc_filter.
  for (int i = 0; i < sfc_filter.filter_layers_size(); ++i) {
    ghost::FilterLayer filter_layer = sfc_filter.filter_layers(i);
    const ghost::GhostFilter ghost_filter = filter_layer.ghost_filter();
    // If layer contains a GhostTunnelIdentifier,
    // check if tunnels list contains same filters
    if (ghost_filter.has_tunnel_id()) {
      std::list<ghost::GhostTunnelIdentifier>::iterator it;
      std::list<ghost::GhostTunnelIdentifier> tunnels = filter->tunnels;
      for (it = tunnels.begin(); it != tunnels.end(); ++it) {
        ghost::GhostTunnelIdentifier filter_id = *it;
        bool matched = google::protobuf::util::MessageDifferencer::Equals(filter_id, ghost_filter.tunnel_id());
        // If one filter does not match, then its not a match.
        if (!matched) {
          return false;
        }
      }
    }
    if (ghost_filter.has_routing_id()) {
      std::list<ghost::GhostRoutingIdentifier>::iterator it;
      std::list<ghost::GhostRoutingIdentifier> routings = filter->routings;
      // If instead we have a GhostRoutingIdentifier, check the routings list.
      for (it = routings.begin(); it != routings.end(); ++it) {
        ghost::GhostRoutingIdentifier filter_id = *it;
        bool matched = google::protobuf::util::MessageDifferencer::Equals(filter_id, ghost_filter.routing_id());
        if (!matched) {
          return false;
        }
      }
    }
  }
  return true;
}

// Returns true if a given filter is active.
bool usps_api_server::Config::FilterActive(Filter* filter) {
  return (filter->tunnels.size() + filter->routings.size()) > 0;
}
