#include <iostream>
#include <string>
#include <array>
#include <memory>

std::string exec(const char* cmd);

int main(int argc, char *argv[])
{
	std::string feedback;
	std::string command = "echo -e \"";
    
	command.append("shutdown");
    command.append("\n");
	command.append("reboot");
    command.append("\n");
	command.append("suspend");
    command.append("\n");
	command.append("hibernate");
    command.append("\n");
    
	command.append("\" | dmenu");
	
	std::string selected = exec(command.c_str());

	if(selected == "shutdown\n") feedback = exec("sudo systemctl poweroff");
	else if(selected == "reboot\n") feedback = exec("sudo systemctl reboot");
	else if(selected == "suspend\n") feedback = exec("echo \"no se como suspender todavía\"");
	else if(selected == "hibernate\n") feedback = exec("echo \"no se como hibernar todavía\"");

	std::cout << feedback << std::endl;

	return 0;
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
