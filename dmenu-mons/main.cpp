#include <iostream>
#include <string>
#include <list>
#include <array>
#include <memory>
#include <sstream>

std::string exec(const char* cmd);
void getScreens(void);
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

	getScreens();
	for (std::list<Screen>::iterator it = connectedScreens.begin(); it != connectedScreens.end(); it++)
	{
		std::cout << "connected device: " << it->name << " - " << it->width << "x" << it->height << std::endl;
	}

	command = "echo \"extend\nduplicate\nmirror\nauto\nmode\" | dmenu -p \"mons\"";
	
	selected = exec(command.c_str());
	
	if(selected.empty())
		return EXIT_FAILURE;
	selected.pop_back(); // borro el salto de línea
	
	if(selected == "extend")
	{
		command = "echo \"left\nright\ntop\nbottom\" | dmenu -p \"extend screen\"";
		selected = exec(command.c_str());

		if(selected.empty())
		{
			return EXIT_FAILURE;
		}
		selected.pop_back(); // borro el salto de línea

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
		selected = selectDevice();

		if(selected.empty())
		{
			return EXIT_FAILURE;
		}
		selected.pop_back(); // borro el salto de línea

		device = selected;

		command = "dmenu -p \"resolution\" < /dev/null";
		selected = exec(command.c_str());

		if(selected.empty())
		{
			return EXIT_FAILURE;
		}
		selected.pop_back(); // borro el salto de línea

		resolution = selected;

		command = "cvt "+resolution;
		feedback = exec(command.c_str());

		std::cout << feedback << std::endl;

		if(feedback.length() == 0)
		{
			command = "notify-send 'CVT error. Enter width and height separated with a single space' -u critical";
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