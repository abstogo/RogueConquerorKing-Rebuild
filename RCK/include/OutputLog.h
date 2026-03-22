#pragma once
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

class OutputLog {
  std::ofstream outputFile;

  std::time_t previousTime;

 public:
  OutputLog();

  OutputLog(std::string filename);

  ~OutputLog();

  void Log(std::string source, std::string message);
};

extern OutputLog* gLog;
