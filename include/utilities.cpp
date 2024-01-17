#include "utilities.h"

int parseArguments(int argc, char *argv[], std::string &THEME, std::string &LAUNCHER)
{
	std::string command;

	for(int i = 1; i < argc; i++)
	{
		if(std::string(argv[i]) == "-theme")
		{
			THEME = std::string(argv[i+1]);
			i++;
		}
		else if(std::string(argv[i]) == "-launcher")
		{
			LAUNCHER = argv[i+1];
			i++;
		}
		else
		{
			command = "notify-send 'dmenu-mons: Unknown argument: "+std::string(argv[i])+".\n"+HELP+"' -u critical";
			exec(command.c_str());
			return EXIT_FAILURE;
		}
	}

	if(LAUNCHER.empty())
	{
		command = "notify-send 'dmenu-mons: Please specify a launcher with -launcher. Use rofi or dmenu' -u critical";
		exec(command.c_str());
		return EXIT_FAILURE;
	}

	if(LAUNCHER == "rofi") LAUNCHER = "rofi -dmenu";
	else if(LAUNCHER != "dmenu")
	{
		command = "notify-send 'dmenu-mons: Unknown launcher: "+LAUNCHER+". Use rofi or dmenu' -u critical";
		exec(command.c_str());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

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