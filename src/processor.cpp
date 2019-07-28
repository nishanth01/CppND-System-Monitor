#include <thread>
#include <chrono>
#include "processor.h"
#include "linux_parser.h"

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() { 

    std::vector<std::string> states1 = LinuxParser::CpuUtilization();
    float activeTime1 = LinuxParser::ActiveJiffies(states1);
    float idleTime1 = LinuxParser::IdleJiffies(states1);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::vector<std::string> states2 = LinuxParser::CpuUtilization();
    float activeTime2 = LinuxParser::ActiveJiffies(states2);
    float idleTime2 = LinuxParser::IdleJiffies(states2);

    float  activeTime  = activeTime2 - activeTime1;
    float  idleTime  = idleTime2 - idleTime1;
    float  totalTime = activeTime + idleTime;
    float  result = 100.0*(activeTime / totalTime);
    return result;
}