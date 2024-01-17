#include <iostream>
#include <string>
#include <list>
#include <array>
#include <memory>
#include <sstream>

std::string exec(const char* cmd);
std::string menu(std::string launcher, std::string theme, std::string prompt, std::list<std::string> options);
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

const char* help = "acá iría la ayuda si existiera.";

int main(int argc, char *argv[])
{
	std::string feedback;
	std::string selected;
	std::string command;
	std::string device;
	std::string resolution;
	size_t begPos, endPos;

	std::string theme, theme_no_entry, launcher;

	for(int i = 1; i < argc; i++)
	{
		if(std::string(argv[i]) == "-theme")
		{
			theme = std::string(argv[i+1]);
			i++;
		}
		else if(std::string(argv[i]) == "-launcher")
		{
			launcher = argv[i+1];
			i++;
		}
		else
		{
			command = "notify-send 'dmenu-mons: Unknown argument: "+std::string(argv[i])+".\n"+std::string(help)+"' -u critical";
			exec(command.c_str());
			return EXIT_FAILURE;
		}
	}

	if(launcher.empty())
	{
		command = "notify-send 'dmenu-mons: Please specify a launcher with -launcher. Use rofi or dmenu' -u critical";
		exec(command.c_str());
		return EXIT_FAILURE;
	}

	if(launcher == "rofi") launcher = "rofi -dmenu";
	else if(launcher != "dmenu")
	{
		command = "notify-send 'dmenu-mons: Unknown launcher: "+launcher+". Use rofi or dmenu' -u critical";
		exec(command.c_str());
		return EXIT_FAILURE;
	}

	//const std::string theme = "-theme $HOME/.config/rofi/launchers/type-1/style-3.rasi -l 5";
	theme += " -l 5";
	theme_no_entry = theme;
	if(launcher == "rofi") theme_no_entry += " -theme-str \"entry {enabled: false;}\"";
	//const std::string theme = "-theme $HOME/.config/rofi/applets/type-1/style-3.rasi";
	//const std::string theme = "";
	//const std::string launcher = "rofi -dmenu";
	//const std::string launcher = "dmenu";

	std::list<std::string> options;

	options.push_back("↔ extend");
	options.push_back("= duplicate");
	options.push_back("‖ mirror");
	options.push_back("↺ auto");
	options.push_back(" mode");

	getScreens();
	for (std::list<Screen>::iterator it = connectedScreens.begin(); it != connectedScreens.end(); it++)
	{
		std::cout << "connected device: " << it->name << " - " << it->width << "x" << it->height << std::endl;
	}

	selected = menu(launcher, theme_no_entry, "mons", options);
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
		
		selected = menu(launcher, theme_no_entry, "extend screen", options);
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
		selected = menu(launcher, theme_no_entry, "select device", options);
		if(selected.empty()) return EXIT_FAILURE;

		device = selected;
		
		command = launcher+" -p \"⤢ resolution\" "+theme+" < /dev/null";
		selected = exec(command.c_str());

		if(selected.empty()) return EXIT_FAILURE;

		selected.pop_back(); // borro el salto de línea

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

	/*
	if()
	command = "echo \"1920x1080\n1366x768\n1024x768\n800x600\n640x480\" | dmenu -p \"resolution\"";
	selected = exec(command.c_str());
	if(selected.empty())
		return EXIT_FAILURE;
	selected.pop_back(); // borro el salto de línea
	begPos = selected.find_first_of('x');
	command = "cvt ";
	*/

	feedback = exec(command.c_str());

	std::cout << feedback << std::endl;

	// reimensionar el fondo para que ocupe toda la nueva pantalla
	// debería hacerlo sólo si salió bien el comando anterior (chequear feedback)
	command = "feh --bg-scale '/home/gabriel/Imágenes/Wallpapers/gris.jpg'";
	feedback = exec(command.c_str());

	std::cout << feedback << std::endl;

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
	command.pop_back(); //botto el último \n
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