#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <array>        //para funcion exec (array)
#include <memory>       //para funcion exec (unique_ptr)

std::string exec(const char* cmd);

int main(int argc, char *argv[])
{
    std::string command;
    std::string preProcess;
    std::string selected;
    std::string feedback;
    std::string aux;
    std::istringstream fullStream;
    std::string status;
    size_t begPos, endPos;

    while(true)
    {
        command = "echo \"history\ntoggle\nclear\" | dmenu -p \"";

        // primero me fijo si las notificaciones están activadas
        feedback = exec("dunstctl is-paused");
        if(feedback == "false\n")
        {
            command.append("running");
            feedback = exec("dunstctl count history");
        }else
        {
            command.append("paused");
            feedback = exec("dunstctl count waiting");
        }
        feedback.pop_back();
        command.append(" ("+feedback+")\"");
        
        selected = exec(command.c_str());

        if(selected == "toggle\n")
        {
            feedback = exec("dunstctl set-paused toggle");
        }else if(selected == "history\n")
        {
            feedback = exec("dunstctl history-pop");
        }else if(selected == "clear\n")
        {
            feedback = exec("dunstctl close-all");
        }else break; // si no es ninguna de las opciones posibles, salir

        std::cout << feedback << std::endl;
    }

    return EXIT_SUCCESS;
}

std::string exec(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}