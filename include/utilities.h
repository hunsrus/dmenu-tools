#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <memory>
#include <sstream>
#include <list>
#include <array>

#define HELP "acá iría la ayuda si existiera."

std::string exec(const char* cmd);
std::string menu(std::string launcher, std::string theme, std::string prompt, std::list<std::string> options);
int parseArguments(int argc, char *argv[], std::string &THEME, std::string &LAUNCHER);

#endif // UTILITIES_H