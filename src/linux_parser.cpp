#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

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

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

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
// Read and returns the system memory utilization

float LinuxParser::MemoryUtilization() {
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  string line;
  string key;
  float value;
  float MemTot;
  float MemAv;

  while (getline(filestream, line)) {
    std::istringstream stringstream(line);
    stringstream >> key >> value;
    if (key == "MemTotal:") MemTot = value;
    if (key == "MemAvailable:") {
      MemAv = value;
      break;
    }
  }

  return (MemTot - MemAv) / MemTot;
}
// Reads and returns the system uptime
long LinuxParser::UpTime() {
  long UpTime;
  std::string line;

  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linstream(line);
    linstream >> UpTime;
  }

  return UpTime;
}
// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  std::string line;
  long jeffies;
  std::string cpu;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
  }
  jeffies = user + nice + system + idle + iowait + irq + softirq + steal +
            guest + guest_nice;

  return jeffies;
}

// The ActiveJiffies function returns the total amount of time that the process
// has been using the CPU.
long LinuxParser::ActiveJiffies(int pid) {
  std::string line, value;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    std::vector<std::string> values;

    while (linestream >> value) {
      values.push_back(value);
    }
    long utime = std::stol(values[13]);
    long stime = std::stol(values[14]);
    long cutime = std::stol(values[15]);
    long cstime = std::stol(values[16]);

    return utime + stime + cutime + cstime;
  }
  return 0;
}

// To calculate the active jiffies, we will need to subtract the idle jiffies
// from the total jiffies

long LinuxParser::ActiveJiffies() {
  return LinuxParser::Jiffies() - LinuxParser::IdleJiffies();
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
  std::string line;

  std::string cpu;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal >> guest >> guest_nice;
  }

  return idle + iowait;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::string line, cpu, value;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    vector<string> values;
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> value) {
      values.push_back(value);
    }

    return values;
  }

  return {};
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::string line, trash;
  int totalProcesses;
  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> trash;

      if (trash == "processes") linestream >> totalProcesses;
    }
    return totalProcesses;
  }
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::string line, trash;
  int running;
  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> trash;

      if (trash == "procs_running") linestream >> running;
    }
    return running;
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmd, line;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cmd;
  }
  filestream.close();
  return cmd;
}

// Read and return the memory used by a process

string LinuxParser::Ram(int pid) {
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  string line, key;
  int memory;

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> memory;
        break;
      }
    }

    return to_string(memory / 1024);
  }
  return "";
}

// Read and return the user ID associated with a process

string LinuxParser::Uid(int pid) {
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  string line, key;
  int uid;

  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> uid;
        break;
      }
    }

    return to_string(uid);
  }
  return "";
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string user, line, value;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      vector<string> target;
      std::istringstream linestream(line);
      while (getline(linestream, value, ':')) {
        target.push_back(value);
      }
      if (uid == target[2]) {
        user = target[0];
        break;
      }
    }
    return user;
  }

  return string();
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, strtime;
  int i;
  int uptime;

  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    for (i = 0; i < 22; i++) linestream >> strtime;
    try {
      uptime = std::stol(strtime) / sysconf(_SC_CLK_TCK);
    } catch (const std::invalid_argument& e) {
      return 0;
    } catch (...) {
      return 0;
    }
    return uptime;
  }
  return 0;
}