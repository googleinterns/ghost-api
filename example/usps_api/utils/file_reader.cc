#include "file_reader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace FileReader {
  std::string ReadString(std::string filename) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }
}
