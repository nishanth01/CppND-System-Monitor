#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version,kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line;
  string name1 = "MemTotal:";
  string name2 = "MemFree:";
  string name3 = "Buffers:";
  string name4 = "Cached:";
  string value;
  float result;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  float total_mem = 0;
  float free_mem = 0;
  float buffers = 0;   
  float cached  = 0; 
  bool done = false;
  int count = 0;
  if (stream.is_open()) {
    while(std::getline(stream, line)){
        if (done)
            break;
        if (line.compare(0, name1.size(), name1) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
            count +=1;
        }
        if (line.compare(0, name2.size(), name2) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
            count +=1;
        }
        if (line.compare(0, name3.size(), name3) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
            count +=1;
        }
        if (line.compare(0, name4.size(), name4) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            cached = stof(values[1]);
            count +=1;
        }
        if(count >=  4){
          done=true;
        }
    }
  }
  result  = float((total_mem-free_mem-buffers-cached)/total_mem);
  return result;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() { 
  string line,uptime_str;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime_str;
  }
  return stoi(uptime_str);
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  std::vector<std::string> values = LinuxParser::CpuUtilization();
  float activeTime = LinuxParser::ActiveJiffies(values);
  float idleTime = LinuxParser::IdleJiffies(values);  
  return activeTime+idleTime; 
}

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  string line,value;
  float result;
  std::ifstream stream(kProcDirectory + to_string(pid)  + "/"+ kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    string str = line;
    std::istringstream buf(str);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); 

    float utime = stof(values[13]);
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = LinuxParser::UpTime();
    float freq = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    result = ((total_time/freq)/seconds);
  }
  return result;  
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies(std::vector<std::string> values) { 
  return (stof(values[CPUStates::kUser_]) +
            stof(values[CPUStates::kNice_]) +
            stof(values[CPUStates::kSystem_]) +
            stof(values[CPUStates::kIRQ_]) +
            stof(values[CPUStates::kSoftIRQ_]) +
            stof(values[CPUStates::kSteal_]) +
            stof(values[CPUStates::kGuest_]) +
            stof(values[CPUStates::kGuestNice_]));
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies(std::vector<std::string> values) { 
  return (stof(values[CPUStates::kIdle_]) +
            stof(values[CPUStates::kIOwait_])); 
}

// DONE: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  string name = "cpu";  
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(),name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values;
        }
    }  
  }
  return {}; 
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 

  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  string name = "processes";  
  int result=0;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.compare(0, name.size(),name) == 0) {
        std::istringstream buf(line);
        std::istream_iterator<string> beg(buf), end;
        vector<string> values(beg, end);
        result += stoi(values[1]);
        break;
      }
    }
  }
  return result;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
 string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  string name = "procs_running";  
  int result=0;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.compare(0, name.size(),name) == 0) {
        std::istringstream buf(line);
        std::istream_iterator<string> beg(buf), end;
        vector<string> values(beg, end);
        result += stoi(values[1]);
        break;
      }
    }
  }
  return result; 
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  std::getline(stream, line);
  return line;
 }

//DONE:Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line;
  string name = "VmSize";
  string value;
  double result;
  std::ifstream stream(kProcDirectory + to_string(pid)  + kStatusFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line)){
        if (line.compare(0, name.size(),name) == 0) {
            std::istringstream buf(line);
            std::istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = (stof(values[1])/double(1024));
            break;
        }
    }
  }
  return to_string(result).substr(0,7);
}


// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 

  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  string name = "Uid:";
  string result ="";

  while (std::getline(stream, line)) {
    if (line.compare(0, name.size(),name) == 0) {
      std::istringstream buf(line);
      std::istream_iterator<string> beg(buf), end;
      vector<string> values(beg, end);
      result =  values[1];
      break;
    }
  }
  name = ("x:" + result);
  return name; 
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) { 

  string line;
  string result ="";
  string name =LinuxParser::Uid(pid);

  std::ifstream fstream(kPasswordPath);
  while (std::getline(fstream, line)) {
      if (line.find(name) != std::string::npos) {
          result = line.substr(0, line.find(":"));
          return result;
      }
  }
  return "";
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  string line,uptime_str;
  long result;
  std::ifstream stream(kProcDirectory + to_string(pid)  + "/" + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    string str = line;
    std::istringstream buf(str);
    std::istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end); 
    result =  long(stof(values[13])/sysconf(_SC_CLK_TCK));
  }
  return result;
}