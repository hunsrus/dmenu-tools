#include <iostream>
#include <string>
#include <array>        //para funcion exec (array)
#include <memory>       //para funcion exec (unique_ptr)
#include <chrono>

float mapear(float val, float valMin, float valMax, float outMin, float outMax);
std::string exec(const char* cmd);

int main(int argc, char* argv[])
{
    std::string feedback;
    std::string command;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now(), now;
    int64_t elapsed = 0;
    int percent = 0;

    while(percent <= 100)
    {
        exec(command.c_str());
        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now-begin).count();
        percent = mapear((int)elapsed,0,5000,0,100);
        command = "notify-send \"bluetootctl\" \"scanning...\" -h int:value:"+std::to_string(percent)+" -h string:x-canonical-private-synchronous:scan";
    }
    command = "notify-send \"bluetootctl\" \"scan complete\" -h string:x-canonical-private-synchronous:scan -t 2000";
    exec(command.c_str());

    return EXIT_SUCCESS;
}

float mapear(float val, float valMin, float valMax, float outMin, float outMax)
{
    return (val - valMin)*(outMax-outMin)/(valMax-valMin) + outMin;
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