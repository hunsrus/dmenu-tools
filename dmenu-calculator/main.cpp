#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <array>
#include <memory>

std::string DMENU_ARGS;

std::string exec(const char* cmd);
std::string calculate(std::string operation);
size_t countInStr(std::string string,const std::string str2count);

int main(int argc, char *argv[])
{
	// todos los argumentos se pasan directamente a dmenu
	// esto permite, por ejemplo, mostrar dmenu abajo con -b
	// o mostrar las opciones como una lista vertical con -l
	// o cualquier otra cosa
	if(argc > 1)
	{
		for(int i = 1; i < argc; i++)
		{
			DMENU_ARGS.append(" "+std::string(argv[i]));
		}
	}

	std::string feedback;
	std::string line;
	std::string operation;
	std::string command;
	std::string lastResult = "0";
	std::string filePath = std::string(getenv("HOME"))+"/.calc_history";
	std::list<std::string> historyContent;

	// crea el archivo si no existe (std::fstream::app evita que se reemplaze el contenido)
	std::fstream historyFile(filePath.c_str(), std::fstream::out | std::fstream::app);
	historyFile.close();
	// abre el archivo y lee el historial una única vez
	historyFile.open(filePath.c_str(), std::fstream::in);
	if(historyFile.is_open())
	{
		// al primer elemento de la lista no le pongo \n porque es el que va a quedar
		// al final del comando y generaría un elemento vacío en dmenu
		if(std::getline(historyFile,line)) historyContent.push_front(line);

		while(std::getline(historyFile,line))
		{
			historyContent.push_front(line+"\n");
		}
	}else
	{
		std::cout << "Error opening history file!" << std::endl;
		return EXIT_FAILURE;
	}
	historyFile.close();

	// reabre el archivo, esta vez para escribir el historial
	historyFile.open(filePath.c_str(), std::fstream::out | std::fstream::app);
	while(true)
	{
		command = "echo \""+lastResult+"\nver historial\nlimpiar historial\" | dmenu -p calcular"+DMENU_ARGS;
		operation = exec(command.c_str());

		if(operation == "ver historial\n")
		{
			if(historyContent.empty())
			{
				command = "echo \"historial vacío\" | dmenu"+DMENU_ARGS;
				exec(command.c_str());
			}
			else
			{
				command = "echo \"";

				for (std::list<std::string>::iterator it = historyContent.begin(); it != historyContent.end(); it++)
				{
					command.append(*it);
				}
				
				command.append("\" | dmenu"+DMENU_ARGS);

				operation = exec(command.c_str());
				// si no hay feedback (se presionó escape) volver al menu sin hacer nada
				if(!operation.empty())
				{
					// si el elemento seleccionado tiene un "=" tomo el resultado y lo paso a stdout
					if(operation.find("=") != std::string::npos)
					{
						// del string guardado en el historial, extraigo solo el resultado
						size_t pos = operation.find("=");
						operation = operation.substr(pos+1);
						// le saco el salto de linea final
						operation.pop_back();
						// este substring es el resultado de la operacion despues del "="
						// lo mando por stdout. externamente puede agregarse al clipboard
						std::cout << operation << std::endl;
						//tambien actualizo lastResult
						lastResult = operation;
						//break;
					}else
					{
						// si en cambio escribo una expresión nueva o presiono TAB para editar la
						// seleccionada, entonces recalculo normalmente
						if(operation.empty()) break;
						// si hay input, le saco el salto de linea que tiene al final por presionar enter
						operation.pop_back();

						feedback = calculate(operation);
						
						if(!feedback.empty())
						{
							// si el calculo no fue nulo, guardo el resultado en lastResult
							lastResult = feedback;
							// en el historial guardo la operacion y el resultado
							// para que sea mas claro y util leerlo
							feedback = operation+"="+lastResult+"\n";
							historyFile << feedback;
							historyContent.push_front(feedback);
							// en stdout solo escribo el resultado para que lo use otro script
							// (para mandarlo al clipboard o lo que fuera)
							std::cout << lastResult << std::endl;
						}
					}
				}
			}
		}else if(operation == "limpiar historial\n")
		{
			historyFile.close();
			// al reabrirse sin std::fstream::app borra el contenido
			historyFile.open(filePath.c_str(), std::fstream::out);
			historyContent.clear();
		}else if(operation == lastResult+"\n")
		{
			// si se elige el elemento del resultado, se pasa a stdout y se sigue normalmente
			// pero no se lo agrega al historial (porque ya está ahí)
			std::cout << lastResult << std::endl;
		}
		else
		{
			// si no se da ningun input salgo sin hacer nada
			if(operation.empty()) break;
			// si hay input, le saco el salto de linea que tiene al final por presionar enter
			operation.pop_back();

			feedback = calculate(operation);
			
			if(!feedback.empty())
			{
				// si el calculo no fue nulo, guardo el resultado en lastResult
				lastResult = feedback;
				// en el historial guardo la operacion y el resultado
				// para que sea mas claro y util leerlo
				feedback = operation+"="+lastResult+"\n";
				historyFile << feedback;
				historyContent.push_front(feedback);
				// en stdout solo escribo el resultado para que lo use otro script
				// (para mandarlo al clipboard o lo que fuera)
				std::cout << lastResult << std::endl;
			}
		}
	}

	historyFile.close();

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

std::string calculate(std::string operation)
{
	std::string result, command, ccalcArgs;

	if((operation.at(0) == '-') && (operation.at(1) != '-'))
	{
		ccalcArgs = operation.substr(2); // saco el argumento
		operation = operation.substr(2,operation.size()); // separo la operacion
	}

	// AGREGAR 2>&1 AL COMANDO CUANDO SE QUIERE OBTENER STDERR ADEMAS DE STDOUT (SE AGREGAN STDERR AL FINAL DEL STRING QUE RETORNA)
	command = "ccalc "+ccalcArgs+" \""+operation+"\" 2>&1";

	std::string feedback = exec(command.c_str());

	// si ccalc devolvió algún error, lo muestra en dmenu y sigue normalmente
	if(feedback.find("Error") != std::string::npos)
	{
		feedback = "echo \""+feedback+"\" | dmenu"+DMENU_ARGS;
		exec(feedback.c_str());
	}
	else if((operation == "--help") || (operation == "-?") || (operation == "--usage") || (operation == "--version"))
	{
		int linesAmount = countInStr(feedback,"\n");
		int divisor = linesAmount/16;
		if(divisor > 2) linesAmount /= divisor;
		feedback.pop_back();
		feedback = "echo \""+feedback+"\" | dmenu"+DMENU_ARGS+" -l "+std::to_string(linesAmount);
		exec(feedback.c_str());
	}
	else
	{
		// en result guardo el feedback sin el salto de linea final
		result = feedback;
		result.pop_back();
	}
	return result;
}

size_t countInStr(std::string string,const std::string str2count)
{
	size_t pos = 0, count = 0;

	while((pos = string.find_first_of(str2count, pos)) != std::string::npos)
	{
		count++;
		pos++;
	}

	return count;
}