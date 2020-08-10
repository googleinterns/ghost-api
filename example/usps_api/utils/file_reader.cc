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
#include "file_reader.h"
#include <string>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

// Reads a file as a string.
std::string FileReader::ReadString(std::string filename) {
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
// Parses a csv file for identifiers and returns them in a list.
std::list<std::vector<std::string>> FileReader::ParseCSV(std::string filename) {
  std::list<std::vector<std::string>> parsed_entries;
  std::ifstream file(filename);
  if (!file.good()) {
    return parsed_entries;
  }
  std::string tmp;
  char delim = ',';
  std::stringstream buffer;
  buffer << file.rdbuf();
  while (std::getline(buffer, tmp, '\n')) {
    std::stringstream line(tmp);
    std::string entry;
    std::vector<std::string> entries;
    while (std::getline(line, entry, delim)) {
      entries.push_back(entry);
    }
    parsed_entries.push_back(entries);
  }
  return parsed_entries;
}


