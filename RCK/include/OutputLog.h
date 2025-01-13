#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

class OutputLog
{
	std::ofstream outputFile;

	std::time_t previousTime;
	
public:
	OutputLog();

	OutputLog(std::string filename);

	~OutputLog();
	
	void Log(std::string source, std::string message);

};

extern OutputLog* gLog;