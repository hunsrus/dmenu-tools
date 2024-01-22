#include <iostream>
#include "../include/utilities.h"

void getScreens(void);
std::list<std::string> getDevices(void);
std::string selectDevice(void);

struct Screen
{
	std::string name;
	std::string type;
	int width;
	int height;
};

std::list<Screen> connectedScreens;

int main(int argc, char *argv[])
{
	std::string feedback;
	std::string selected;
	std::string command;
	std::string device;
	std::string resolution;
	size_t begPos, endPos;

	std::string THEME, theme_no_entry, LAUNCHER;

	if(parseArguments(argc, argv, THEME, LAUNCHER) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	//const std::string theme = "-theme $HOME/.config/rofi/launchers/type-1/style-3.rasi -l 5";
	THEME += " -l 5";
	theme_no_entry = THEME;
	if(LAUNCHER == "rofi -dmenu") theme_no_entry += " -theme-str \"entry {enabled: false;} configuration {show-icons: false;}\"";
	//const std::string theme = "-theme $HOME/.config/rofi/applets/type-1/style-3.rasi";
	//const std::string theme = "";
	//const std::string launcher = "rofi -dmenu";
	//const std::string launcher = "dmenu";

	std::list<std::string> options;

	/*
	options.push_back("↔ extend");
	options.push_back("= duplicate");
	options.push_back("‖ mirror");
	options.push_back("↺ auto");
	options.push_back(" mode");
	*/
	options.push_back(" extend");
	options.push_back("󰍺 duplicate");
	options.push_back(" mirror");
	options.push_back("󰁨 auto");
	options.push_back(" mode");

	getScreens();
	for (std::list<Screen>::iterator it = connectedScreens.begin(); it != connectedScreens.end(); it++)
	{
		std::cout << "connected device: " << it->name << " - " << it->width << "x" << it->height << std::endl;
	}

	selected = menu(LAUNCHER, theme_no_entry, "󰍹 mons", options);
	if(selected.empty()) return EXIT_FAILURE;
	
	// borro el icono inicial
	selected = selected.substr(selected.find_first_of(" ")+1);
	
	if(selected == "extend")
	{
		options.clear();
		options.push_back("left");
		options.push_back("right");
		options.push_back("top");
		options.push_back("bottom");
		
		selected = menu(LAUNCHER, theme_no_entry, " extend screen", options);
		if(selected.empty()) return EXIT_FAILURE;

		command = "mons -e "+selected;
	}else if(selected == "duplicate")
	{
		command = "mons -d";
	}else if(selected == "mirror")
	{
		command = "mons -m";
	}else if(selected == "auto")
	{
		command = "mons -a";
	}else if(selected == "mode")
	{	
		// selecciono el dispositivo cuyo modo quiero cambiar
		options = getDevices();
		selected = menu(LAUNCHER, theme_no_entry, "󰷜 select device", options);
		if(selected.empty()) return EXIT_FAILURE;

		device = selected;
		
		options.clear();
		options.push_back("1920 1080");
		options.push_back("1366 768");
		options.push_back("1360 768");
		options.push_back("1024 768");
		
		selected = menu(LAUNCHER, THEME, " resolution", options);
		if(selected.empty()) return EXIT_FAILURE;

		resolution = selected;

		command = "cvt "+resolution;
		feedback = exec(command.c_str());

		if(feedback.length() == 0)
		{
			command = "notify-send 'dmenu-mons: CVT error. Enter width and height separated with a single space' -u critical";
    		exec(command.c_str());
			return EXIT_FAILURE;
		}

		begPos = feedback.find("Modeline");
		endPos = begPos+std::string("Modeline ").length();
		
		std::string mode = feedback.substr(endPos);

		command = "xrandr --newmode "+mode;
		feedback = exec(command.c_str());

		endPos = mode.find_last_of("\"");
		mode = mode.substr(0, endPos+1);

		command = "xrandr --addmode "+device+" "+mode;
		feedback = exec(command.c_str());

		command = "xrandr --output "+device+" --mode "+mode;
		feedback = exec(command.c_str());
		
	}else EXIT_FAILURE;

	feedback = exec(command.c_str());

	std::cout << feedback << std::endl;

	// reimensionar el fondo para que ocupe toda la nueva pantalla
	// debería hacerlo sólo si salió bien el comando anterior (chequear feedback)
	command = "feh --bg-scale '/home/gabriel/Imágenes/Wallpapers/gris.jpg'";
	feedback = exec(command.c_str());

	std::cout << feedback << std::endl;

	return EXIT_SUCCESS;
}

std::list<std::string> getDevices(void)
{
	std::list<std::string> options;

	for (std::list<Screen>::iterator it = connectedScreens.begin(); it != connectedScreens.end(); it++)
	{
		options.push_back(it->name);
		std::cout << "connected device: " << it->name << " - " << it->width << "x" << it->height << std::endl;
	}

	return options;
}

std::string selectDevice(void)
{
	std::string command, selected;
	
	command = "echo \"";
	for (std::list<Screen>::iterator it = connectedScreens.begin(); it != connectedScreens.end(); it++)
	{
		command += it->name+"\n";
		std::cout << "connected device: " << it->name << " - " << it->width << "x" << it->height << std::endl;
	}
	command.pop_back(); //borro el último \n
	command += "\" | dmenu -p \"slect device\"";

	selected = exec(command.c_str());

	return selected;
}

void getScreens(void)
{
	std::istringstream fullStream;
	std::string aux;
	Screen auxScreen;
	size_t begPos, endPos;

	fullStream.str(exec("xrandr"));
    while (std::getline(fullStream, aux, '\n'))
    {
		begPos = aux.find_first_of(' ')+1;
		endPos = aux.find_first_of(' ',begPos);
		if(aux.substr(begPos,endPos-begPos) == "connected")
		{
			begPos = 0;
			endPos = aux.find_first_of(' ');
			auxScreen.name = aux.substr(begPos,endPos-begPos);
		}

		if(aux.find("*") != std::string::npos)
		{
			begPos = aux.find_first_not_of(' ');
			endPos = aux.find_first_of('x');
			std::stringstream(aux.substr(begPos,endPos-begPos)) >> auxScreen.width;
			begPos = endPos+1;
			endPos = aux.find_first_of(' ',begPos);
			std::stringstream(aux.substr(begPos,endPos-begPos)) >> auxScreen.height;
			connectedScreens.push_back(auxScreen);
		}

	}
}