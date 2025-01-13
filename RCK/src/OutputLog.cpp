#include "OutputLog.h"
#include <iomanip>

OutputLog* gLog;

OutputLog::OutputLog() : OutputLog("game.txt")
{
	
}

OutputLog::OutputLog(std::string filename)
{
	outputFile.open(filename, std::ios::out | std::ios::app);
	previousTime = 0;
}

OutputLog::~OutputLog()
{
	outputFile.close();
}

void OutputLog::Log(std::string source, std::string message)
{
	std::time_t t = std::time(nullptr);
	if (t > previousTime)
	{
		outputFile << std::put_time(std::gmtime(&t), "%c %Z") << std::endl;
		previousTime = t;
	}
	
	std::string logLine = source + ":" + message;
	outputFile << logLine << std::endl;
}