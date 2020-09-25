#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <unordered_set>
#include <vector>

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
// Note: from lecture video "String Parsing", revised this function
// to account for version in string stream
string LinuxParser::Kernel() {
  string os, version, kernel;
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
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  string line;
  string key, value;
  string MemTotalKey = "MemTotal";
  string MemFreeKey = "MemFree";

  float MemFree, MemTotal;

  while (stream.is_open()) {
    std::getline(stream, line);
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == MemTotalKey) {
      MemTotal = stof(value);
    } else if (key == MemFreeKey) {
      MemFree = stof(value);
      break;
    }
  }

  return (MemTotal - MemFree) / MemTotal;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  string line;
  long uptime, idletime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }
  return uptime;
}

// ADDED: Return vector of system jiffies
vector<long> LinuxParser::JiffiesVect() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  string key;
  long value;
  vector<long> Jiffies;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> key;
    while (linestream >> value) {
      Jiffies.emplace_back(value);
    }
  }
  return Jiffies;
}

// ADDED: Return vector of PID jiffies
vector<long> LinuxParser::JiffiesPidVect(int pid) {
  const string pidStr = to_string(pid);
  std::ostringstream path;
  path << kProcDirectory << pidStr << kStatFilename;
  std::ifstream stream(path.str());
  string line;
  string key;
  string value;
  vector<long> Jiffies;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    std::unordered_set<int> s{13, 14, 15, 16, 21};
    for (int i = 0; i < 22; i++) {
      linestream >> value;
      if (s.find(i) != s.end()) {
        Jiffies.emplace_back(std::stol(value));
      }
    }
  }

  return Jiffies;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  vector<long> v = LinuxParser::JiffiesVect();
  long jiffies{0};
  for (int i = 0; i < 8; i++) {
    jiffies += v[i];
  }
  return jiffies;
}

// DONE: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  vector<long> v = LinuxParser::JiffiesPidVect(pid);
  long activeJiffies{0};
  for (const int& i : v) {
    activeJiffies += i;
  }
  activeJiffies -= v.back();

  return activeJiffies;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  return LinuxParser::Jiffies() - LinuxParser::IdleJiffies();
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<long> v = LinuxParser::JiffiesVect();
  long idle{0};
  for (int i = 3; i < 5; i++) {
    idle += v[i];
  }
  return idle;
}

// NOT USED: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { return {}; }

// ADDED: Process CPU Utilization
float LinuxParser::CpuUtilization(int pid) {
  long activeJiffies = LinuxParser::ActiveJiffies(pid);
  long uptime = LinuxParser::UpTime(pid);
  return (float)(activeJiffies / sysconf(_SC_CLK_TCK)) / (float)uptime;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string key = "processes";
  return LinuxParser::Parser<string, int>(key, stream);
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string key = "procs_running";
  return LinuxParser::Parser<string, int>(key, stream);
}

// DONE: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  const string pidStr = to_string(pid);
  std::ostringstream path;
  path << kProcDirectory << pidStr << kCmdlineFilename;
  std::ifstream stream(path.str());
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    return line;
  }

  return string();
}

// DONE: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
// string LinuxParser::Ram(int pid[[maybe_unused]]) { return string(); }
string LinuxParser::Ram(int pid) {
  const string pidStr = to_string(pid);
  std::ostringstream path;
  path << kProcDirectory << pidStr << kStatusFilename;
  std::ifstream stream(path.str());
  string key = "VmSize:";
  long ramKb = LinuxParser::Parser<string, long>(key, stream);
  return to_string(ramKb / 1000);
}

// DONE: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  const string pidStr = to_string(pid);
  std::ostringstream path;
  path << kProcDirectory << pidStr << kStatusFilename;
  std::ifstream stream(path.str());
  string key = "Uid:";
  return LinuxParser::Parser<string, string>(key, stream);
}

// DONE: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  std::ifstream stream(kPasswordPath);
  string line;
  const string uid = LinuxParser::Uid(pid);
  string user, passwd, uidCurr;

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> user >> passwd >> uidCurr;
      if (uidCurr == uid) {
        return user;
      }
    }
  }
  return user;
}

// DONE: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  long uptime = LinuxParser::UpTime();
  long starttime = LinuxParser::JiffiesPidVect(pid).back();
  return uptime - (starttime / sysconf(_SC_CLK_TCK));
}