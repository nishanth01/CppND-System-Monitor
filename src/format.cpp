#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    long minutes = seconds / 60;
    long hours = minutes / 60;
    long lseconds = int(seconds%60);
    minutes = int(minutes%60);
    string result = std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(lseconds);
    return result;
}