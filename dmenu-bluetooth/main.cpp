// agregar antes de cada break una salida por notificación
// posiblemente usando dunst (o dunstify no se como es)
#include <iostream>
#include <chrono>       //para dunstifyScan()
#include <thread>
#include "../include/utilities.h"

struct Device
{
    std::string name;
    std::string addr;
};

float mapear(float val, float valMin, float valMax, float outMin, float outMax);
void notifyProgress(void);

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

    std::list<std::string> options;
	std::string THEME, theme_no_entry, LAUNCHER;

	if(parseArguments(argc, argv, THEME, LAUNCHER) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

    THEME += " -l 3";
	theme_no_entry = THEME;
	if(LAUNCHER == "rofi -dmenu") theme_no_entry += " -theme-str \"entry {enabled: false;}\"";

    while(true)
    {
        //command = preProcess+"echo \"devices\nscan\npower on/off\" | dmenu -p \"";
        //preProcess.clear();

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
        options.clear();
        options.push_back("devices");
        options.push_back("scan");
        options.push_back("on/off");
        
        selected = menu(LAUNCHER, theme_no_entry, status, options);
        if(selected.empty()) return EXIT_FAILURE;
        //command.append(status+"\"");

        //selected = exec(command.c_str());

        // si antes de elegir la opción se corrió el pre-proceso de scan, entonces ignoro
        // el feedback de éste y busco el feedback que corresponde a la selección de dmenu
        if(selected.find("Discovery") != std::string::npos)
        {
            endPos = selected.find_last_of('\n')-1;
            begPos = selected.find_last_of('\n',endPos)+1;
            selected = selected.substr(begPos,endPos);
        }

        if(selected == "devices")
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
            
            options.clear();
            for (std::list<Device>::iterator it = devices.begin(); it != devices.end(); it++)
                options.push_back(it->name);
            // borro el último salto de línea para que no genere un elemento vacío

            selected = menu(LAUNCHER, theme_no_entry, "devices", options);
            if(selected.empty()) return EXIT_FAILURE;

            command.clear();
            
            for (std::list<Device>::iterator it = devices.begin(); it != devices.end(); it++)
            {
                if(selected == it->name)
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
        }else if(selected == "scan")
        {
            // en lugar de correr el proceso directamente y esperar el timeout sin posibilidad
            // de seguir usando el programa, lo paso como un proceso que se va a ejecutar en conjunto
            // con la proxima instancia de dmenu
            //preProcess = "dunstify \"scanning...\" && ";
            std::thread t1(notifyProgress);
            //std::thread t1(threadTest, "AEEA");
            //preProcess.append("bluetoothctl --timeout 5 scan on &&");
            exec("bluetoothctl --timeout 5 scan on");
            t1.join();
            //preProcess.append("dunstify \"scan complete\" & ");
        }else if(selected == "on/off")
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

void notifyProgress(void)
{
    std::string feedback;
    std::string command;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(), now;
    int64_t elapsed = 0;
    int percent = 0;

    while(percent <= 100)
    {
        command = "notify-send \"bluetootctl\" \"scanning...\" -h int:value:"+std::to_string(percent)+" -h string:x-canonical-private-synchronous:scan";
        exec(command.c_str());
        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now-begin).count();
        percent = mapear((int)elapsed,0,5000,0,100);
    }
    command = "notify-send \"bluetootctl\" \"scan complete\" -h string:x-canonical-private-synchronous:scan -t 2000";
    exec(command.c_str());
}

float mapear(float val, float valMin, float valMax, float outMin, float outMax)
{
    return (val - valMin)*(outMax-outMin)/(valMax-valMin) + outMin;
}