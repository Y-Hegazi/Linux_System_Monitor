#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;
using std::to_string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS

string Format::ElapsedTime(long seconds) {
  int min;
  int hour;
  int sec;
  sec = int(seconds % 60);
  min = int((seconds / 60) % 60);
  hour = int(seconds / 3600);

  std::ostringstream stream;
  stream << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2)
         << min << ":" << std::setw(2) << sec;
  return stream.str();
}