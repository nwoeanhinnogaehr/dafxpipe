#include "JackClient.h"
#include "OscHandler.h"
#include "PythonProcessor.h"
#include <chrono>
#include <iostream>
#include <thread>

int
main(int argc, char** argv)
{
    if (argc != 4) {
        std::cout << "Usage: \n" << argv[0] << " ServerIP ServerPort ClientID\n";
        return 0;
    }
    std::string serverIP(argv[1]);
    int serverPort = std::stoi(argv[2]);
    int clientID = std::stoi(argv[3]);

    PythonProcessor proc;
    JackClient client(&proc, clientID);
    OscHandler osc(serverIP, serverPort, serverPort + clientID + 1, &proc);

    client.start();
    osc.start();

    // sleep forever
    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());
}
