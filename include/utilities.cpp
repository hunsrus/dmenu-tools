#include "utilities.h"

std::string exec(const char* cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if(!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}
	return result;
}

std::string menu(std::string launcher, std::string theme, std::string prompt, std::list<std::string> options)
{
	std::string command = "echo \"";
	while(!options.empty())
	{
		command += options.front()+"\n";
		options.pop_front();
	}
	command.pop_back(); // borro el último salto de línea
	command += "\" | "+launcher+" -p \""+prompt+"\" -theme "+theme;

	std::string selected = exec(command.c_str());
	if(!selected.empty())
	{
		selected.pop_back(); // borro el salto de línea
	}
	
	return selected;
}