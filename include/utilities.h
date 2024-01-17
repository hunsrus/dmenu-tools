#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <memory>
#include <sstream>
#include <list>
#include <array>

std::string exec(const char* cmd);
std::string menu(std::string launcher, std::string theme, std::string prompt, std::list<std::string> options);

#endif // UTILITIES_H