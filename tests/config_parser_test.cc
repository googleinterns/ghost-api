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
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "example/usps_api/config/config_parser.h"
#include "proto/usps_api/ghost_label.pb.h"
#include <string>
#include <fstream>
#include <list>
#include <iostream>
#include <cstdio>
#include "json/json.h"

namespace {
// Helper function that writes to the configuration file.
void WriteToConfig(usps_api_server::Config* config, Json::Value root) {
  std::ofstream outfile(config->kFilename);
  Json::StreamWriterBuilder builder;
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(root, &outfile);
  outfile.close();
}
// Helper function that creates a config instance.
usps_api_server::Config* CreateConfig() {
  usps_api_server::Config *config = new usps_api_server::Config();
  std::ifstream file(config->kFilename, std::ifstream::binary);
  if(file.good()) {
    std::remove((config->kFilename).c_str());
  }
  Json::Value root;
  WriteToConfig(config, root);
  return config;
}
} // namespace

// Tests if config file does not exist
TEST(ConfigTest, FileDoesNotExist) {
  usps_api_server::Config *config = CreateConfig();
  std::remove((config->kFilename).c_str());
  EXPECT_FALSE(config->Initialize());
  delete config;
}
// Tests behavior of bad config file
TEST(ConfigTest, MissingBrackets) {
  usps_api_server::Config *config = new usps_api_server::Config();
  std::ifstream file(config->kFilename, std::ifstream::binary);
  if(file.good()) {
    std::remove((config->kFilename).c_str());
  }
  std::ofstream outfile(config->kFilename);
  outfile << "{";
  outfile.close();
  EXPECT_FALSE(config->Initialize());
  delete config;
}
TEST(ConfigTest, ImproperName) {
  usps_api_server::Config *config = CreateConfig();
  Json::Value root;
  root["addresss"]["host"] = "2.2.2.2";
  root["address"]["port"] = 123;
  WriteToConfig(config, root);
  config->Initialize();
  EXPECT_NE(config->host_, "2.2.2.2");
  EXPECT_EQ(config->port_, 123);
  delete config;
}

// Tests if config behaves correctly if json is valid.
TEST(ConfigTest, CanInitialize) {
  usps_api_server::Config *config = CreateConfig();
  EXPECT_TRUE(config->Initialize());
  delete config;
}
// Tests if configuration can read address.
TEST(ConfigTest, CanParseAddress) {
  usps_api_server::Config *config = CreateConfig();
  Json::Value root;
  config->Initialize();
  EXPECT_EQ(config->host_, "");
  EXPECT_EQ(config->port_, 0);
  root["address"]["host"] = "1.1.1.1";
  root["address"]["port"] = 123;
  WriteToConfig(config, root);
  config->Initialize();
  EXPECT_EQ(config->host_, "1.1.1.1");
  EXPECT_EQ(config->port_, 123);
  delete config;
}

// Tests if configuration can parse the allowed filters.
TEST(ConfigTest, CanParseAllow) {
  usps_api_server::Config *config = CreateConfig();
  Json::Value root;
  Json::Value labels;
  Json::Value label;
  label["value"] = 1234;
  label["prefix_len"] = 2;
  labels.append(label);
  root["sfcfilter"]["allow"]["ghost_routing_identifier"]["destination_label_prefix"] = labels;
  WriteToConfig(config, root);
  config->Initialize();
  std::list<ghost::GhostRoutingIdentifier> tunnels = config->allow_.routings;
  EXPECT_GT(tunnels.size(), 0);
  ghost::GhostRoutingIdentifier id = tunnels.front();
  EXPECT_EQ(id.destination_label_prefix().value(), 1234);
  EXPECT_EQ(id.destination_label_prefix().prefix_len(), 2);
  delete config;
}
// Tests if configuration can parse the denied filters.
TEST(ConfigTest, CanParseDeny) {
  usps_api_server::Config *config = CreateConfig();
  Json::Value root;
  Json::Value labels;
  Json::Value label;
  label["terminal_label"] = 1234;
  label["service_label"] = 230;
  labels.append(label);
  root["sfcfilter"]["deny"]["ghost_tunnel_identifier"]["ghostlabel"] = labels;
  WriteToConfig(config, root);
  config->Initialize();
  std::list<ghost::GhostTunnelIdentifier> tunnels = config->deny_.tunnels;
  EXPECT_GT(tunnels.size(), 0);
  ghost::GhostTunnelIdentifier id = tunnels.front();
  EXPECT_EQ(id.terminal_label().value(), 1234);
  EXPECT_EQ(id.service_label().value(), 230);
  delete config;
}
// Tests if configuration can parse the delay filters.
TEST(ConfigTest, CanParseDelay) {
  usps_api_server::Config *config = CreateConfig();
  Json::Value root;
  Json::Value labels;
  Json::Value label;
  label["terminal_label"] = 1234;
  label["service_label"] = 230;
  labels.append(label);
  root["sfcfilter"]["delay"]["ghost_tunnel_identifier"]["ghostlabel"] = labels;
  WriteToConfig(config, root);
  config->Initialize();
  std::list<ghost::GhostTunnelIdentifier> tunnels = config->delay_.tunnels;
  EXPECT_GT(tunnels.size(), 0);
  ghost::GhostTunnelIdentifier id = tunnels.front();
  EXPECT_EQ(id.terminal_label().value(), 1234);
  EXPECT_EQ(id.service_label().value(), 230);
  EXPECT_EQ(config->delay_time_, 0);
  root["sfcfilter"]["delay"]["seconds"] = 5;
  WriteToConfig(config, root);
  config->Initialize();
  EXPECT_EQ(config->delay_time_, 5);
  delete config;
}
// Tests if configuration parses request enabling.
TEST(ConfigTest, CanParseRequests) {
  usps_api_server::Config *config = CreateConfig();
  config->Initialize();
  EXPECT_EQ(config->create_, true);
  EXPECT_EQ(config->del_, true);
  EXPECT_EQ(config->query_, true);
  Json::Value root;
  root["requests"]["create"] = false;
  root["requests"]["delete"] = false;
  root["requests"]["query"] = true;
  WriteToConfig(config, root);
  config->Initialize();
  EXPECT_EQ(config->create_, false);
  EXPECT_EQ(config->del_, false);
  EXPECT_EQ(config->query_, true);
  delete config;
}
// Tests if configuration can match two tunnel filters.
TEST(ConfigTest, CanMatchFilterTunnel) {
  usps_api_server::Config *config = CreateConfig();
  config->Initialize();
  EXPECT_FALSE(config->FilterActive(&(config->allow_)));
  Json::Value root;
  Json::Value labels;
  Json::Value label1;
  Json::Value label2;
  label1["terminal_label"] = 100;
  label1["service_label"] = 1;
  labels.append(label1);
  label2["terminal_label"] = 999;
  label2["service_label"] = 2;
  labels.append(label2);
  root["sfcfilter"]["allow"]["ghost_tunnel_identifier"]["ghostlabel"] = labels;
  WriteToConfig(config, root);
  config->Initialize();
  EXPECT_TRUE(config->FilterActive(&(config->allow_)));
  ghost::SfcFilter sfc_filter;
  ghost::GhostFilter* filter = sfc_filter.add_filter_layers()->mutable_ghost_filter();
  ghost::GhostTunnelIdentifier* tunnel_id = filter->mutable_tunnel_id();
  ghost::GhostLabel* terminal_label = tunnel_id->mutable_terminal_label();
  terminal_label->set_value(100);
  ghost::GhostLabel* service_label = tunnel_id->mutable_service_label();
  service_label->set_value(1);
  EXPECT_TRUE(config->FilterMatch(&(config->allow_), &sfc_filter));
  terminal_label->set_value(999);
  EXPECT_FALSE(config->FilterMatch(&(config->allow_), &sfc_filter));
  service_label->set_value(2);
  EXPECT_TRUE(config->FilterMatch(&(config->allow_), &sfc_filter));
  delete config;
}
// Tests if config can match two route filters.
TEST(ConfigTest, CanMatchFilterRoute) {
  usps_api_server::Config *config = CreateConfig();
  config->Initialize();
  EXPECT_FALSE(config->FilterActive(&(config->allow_)));
  Json::Value root;
  Json::Value labels;
  Json::Value label1;
  Json::Value label2;
  label1["value"] = 100;
  label1["prefix_len"] = 1;
  labels.append(label1);
  label2["value"] = 999;
  label2["prefix_len"] = 2;
  labels.append(label2);
  root["sfcfilter"]["allow"]["ghost_routing_identifier"]["destination_label_prefix"] = labels;
  WriteToConfig(config, root);
  config->Initialize();
  EXPECT_TRUE(config->FilterActive(&(config->allow_)));
  ghost::SfcFilter sfc_filter;
  ghost::GhostFilter* filter = sfc_filter.add_filter_layers()->mutable_ghost_filter();
  ghost::GhostRoutingIdentifier* routing_id = filter->mutable_routing_id();
  ghost::GhostLabelPrefix* dest_label = routing_id->mutable_destination_label_prefix();
  dest_label->set_value(100);
  dest_label->set_prefix_len(1);
  EXPECT_TRUE(config->FilterMatch(&(config->allow_), &sfc_filter));
  dest_label->set_value(999);
  EXPECT_FALSE(config->FilterMatch(&(config->allow_), &sfc_filter));
  dest_label->set_prefix_len(2);
  EXPECT_TRUE(config->FilterMatch(&(config->allow_), &sfc_filter));
  delete config;
}
