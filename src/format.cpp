#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// DONE: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  long hours = (seconds / 60 / 60) % 24;
  long minutes = (seconds / 60) % 60;
  long secs = seconds % 60;
  std::ostringstream stream;
  stream << std::setw(2) << std::setfill('0') << std::to_string(hours) << ":"
         << std::setw(2) << std::setfill('0') << std::to_string(minutes) << ":"
         << std::setw(2) << std::setfill('0') << std::to_string(secs);

  return stream.str();
}