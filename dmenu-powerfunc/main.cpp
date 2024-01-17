#include <iostream>
#include "../include/utilities.h"

int main(int argc, char *argv[])
{
	std::string feedback, selected;
	std::string command = "echo -e \"";
	std::list<std::string> options;

	std::string THEME, theme_no_entry, LAUNCHER;

	if(parseArguments(argc, argv, THEME, LAUNCHER) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	THEME += " -l 4";
	theme_no_entry = THEME;
	if(LAUNCHER == "rofi -dmenu") theme_no_entry += " -theme-str \"entry {enabled: false;}\"";


	options.push_back("shutdown");
	options.push_back("reboot");
	options.push_back("suspend");
	options.push_back("hibernate");
	
	selected = menu(LAUNCHER, theme_no_entry, "power", options);
	if(selected.empty()) return EXIT_FAILURE;

	if(selected == "shutdown") feedback = exec("sudo systemctl poweroff");
	else if(selected == "reboot") feedback = exec("sudo systemctl reboot");
	else if(selected == "suspend") feedback = exec("echo \"no se como suspender todavía\"");
	else if(selected == "hibernate") feedback = exec("echo \"no se como hibernar todavía\"");

	std::cout << feedback << std::endl;

	return 0;
}
