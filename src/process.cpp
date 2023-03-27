#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "processor.h"

using std::string;
using std::to_string;
using std::vector;

// Return this process's ID

Process::Process(int pid) : pid(pid) {}

int Process::Pid() { return pid; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  return float(LinuxParser::ActiveJiffies(pid)) / LinuxParser::Jiffies();
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid); }

// Overloading the "less than" comparison operator for Process objects

bool Process::operator<(Process const& a [[maybe_unused]]) const {
  return this->CpuUtilization() < a.CpuUtilization();
}

void Process::Pid(int pid) { this->pid = pid; }