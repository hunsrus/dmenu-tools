// agregar antes de cada break una salida por notificación
// posiblemente usando dunst (o dunstify no se como es)

#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <array>        //para funcion exec (array)
#include <memory>       //para funcion exec (unique_ptr)

struct Device
{
    std::string name;
    std::string addr;
};

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
    std::list<Device> devices;
    Device auxDevice;
    Device connectedDevice;
    size_t begPos, endPos;

    while(true)
    {
        command = preProcess+"echo \"devices\nscan\npower on/off\" | dmenu -p \"";
        preProcess.clear();

        // primero me fijo si el controlador está encendido
        feedback = exec("bluetoothctl show");
        if(feedback.find("Powered: no") != std::string::npos)
        {
            connectedDevice.addr.clear();
            connectedDevice.name.clear();
            status = "bluetooth off";
        }else
        {
            // si está encendido, me fijo si está conectado
            feedback = exec("bluetoothctl info");
        
            if(feedback.find("Missing device") != std::string::npos)
            {
                connectedDevice.addr.clear();
                connectedDevice.name.clear();
                status = "disconnected";
            }else{
                // si está conectado, me fijo a qué
                begPos = feedback.find("Name")+6;
                endPos = feedback.find_first_of('\n',begPos);
                connectedDevice.name = feedback.substr(begPos, endPos-begPos);
                begPos = feedback.find_first_of(' ')+1;
                endPos = feedback.find_first_of(' ',begPos);
                connectedDevice.addr = feedback.substr(begPos,endPos-begPos);
                status = connectedDevice.name;
            }
        }
        // escribo el estado actual como prompt de dmenu (después de -p)
        command.append(status+"\"");

        selected = exec(command.c_str());

        // si antes de elegir la opción se corrió el pre-proceso de scan, entonces ignoro
        // el feedback de éste y busco el feedback que corresponde a la selección de dmenu
        if(selected.find("Discovery") != std::string::npos)
        {
            endPos = selected.find_last_of('\n')-1;
            begPos = selected.find_last_of('\n',endPos)+1;
            selected = selected.substr(begPos,endPos);
        }

        if(selected == "devices\n")
        {
            devices.clear();
            fullStream.str(exec("bluetoothctl devices"));

            command = "echo \"";

            while (std::getline(fullStream, aux, '\n'))
            {
                size_t separator1Pos = aux.find_first_of(' ')+1;
                size_t separator2Pos = aux.find_first_of(' ',separator1Pos)+1;
                auxDevice.name = aux.substr(separator2Pos);
                auxDevice.addr = aux.substr(separator1Pos,aux.size()-separator1Pos-1-auxDevice.name.size());
                // si quiero que los dispositivos conocidos aparezcan primero, tengo que meterlos de esta manera
                // pero después tengo que recorrer la lista una vez mas (despues de este bucle) para sacarlos ordenados
                devices.push_front(auxDevice);
            }
            for (std::list<Device>::iterator it = devices.begin(); it != devices.end(); it++)
                command.append(it->name+"\n");
            // borro el último salto de línea para que no genere un elemento vacío
            command.pop_back();

            command.append("\" | dmenu -p devices");
            selected = exec(command.c_str());

            command.clear();
            
            for (std::list<Device>::iterator it = devices.begin(); it != devices.end(); it++)
            {
                if(selected == it->name+"\n")
                {
                    if(connectedDevice.addr == it->addr)
                        command = "bluetoothctl disconnect "+it->addr;
                    else
                        command = "bluetoothctl connect "+it->addr;
                }
            }

            
            
            // si no se elije ningun dispositivo, no se ejecuta el comando
            if(!command.empty())
            {
                selected.pop_back(); // le borro el último salto de línea
                if(command.find("disconnect") != std::string::npos)
                    feedback = exec(std::string("dunstify \"disconnecting from "+selected+"...\"").c_str());
                else
                    feedback = exec(std::string("dunstify \"connecting to "+selected+"...\"").c_str());

                feedback = exec(command.c_str());

                if(feedback.find("Connection successful") != std::string::npos)
                    feedback = exec(std::string("dunstify \""+selected+" connected\"").c_str());
                else if(feedback.find("Successful disconnected") != std::string::npos)
                    feedback = exec(std::string("dunstify \""+selected+" disconnected\"").c_str());
                else
                    feedback = exec(std::string("dunstify \"error connecting to "+selected+"\"").c_str());
            }
            
            // borro el stream para volver a usarlo en el proximo ciclo
            // UPDATE al final voy a hacer que esta acción muestre una notificación y termine el programa
            //fullStream.clear();
            break;
        }else if(selected == "scan\n")
        {
            // en lugar de correr el proceso directamente y esperar el timeout sin posibilidad
            // de seguir usando el programa, lo paso como un proceso que se va a ejecutar en conjunto
            // con la proxima instancia de dmenu
            preProcess = "dunstify \"scanning...\" && ";
            preProcess.append("bluetoothctl --timeout 5 scan on &&");
            preProcess.append("dunstify \"scan complete\" & ");
        }else if(selected == "power on/off\n")
        {
            if(status == "bluetooth off")
            {
                feedback = exec("bluetoothctl power on");
            }else
            {
                feedback = exec("bluetoothctl power off");
            }
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