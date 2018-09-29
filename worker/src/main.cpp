#include "JackClient.h"
#include "RpcServer.h"
#include "PythonProcessor.h"
#include <chrono>
#include <iostream>
#include <thread>

int
main(int argc, char** argv)
{
    if (argc != 4) {
        std::cout << "Usage: \n" << argv[0] << " BindAddress Port Language\n";
        return 0;
    }
    std::string bindAddress(argv[1]);
    int port = std::stoi(argv[2]);
    std::string language(argv[3]);

    Processor *proc;
    if (language == "python")
        proc = new PythonProcessor(port, argv[0]);
    else {
        std::cout << "Unsupported language. Currently supported: python, julia.\n";
        return 0;
    }
    JackClient jack(proc, port);
    jack.start();
    RpcServer rpc(bindAddress, port, proc, &jack);

    // sleep forever
    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());
}
