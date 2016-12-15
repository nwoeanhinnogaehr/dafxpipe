#include "OscHandler.h"
#include "debug.h"
#include <iostream>

void
error_handler(int num, const char* m, const char* path)
{
    std::cout << "Error " << num << " in " << path << ":\n" << m << std::endl;
    exit(1);
}

OscHandler::OscHandler(std::string serverIP, int serverPort, int clientPort, PythonProcessor* proc)
  : proc(proc)
  , server(clientPort, error_handler)
  , client(serverIP, std::to_string(serverPort))
{
}

void
OscHandler::registerHandlers()
{
    server.add_method("exec", "s", [this](lo_arg** argv, int argc) {
        if (argc != 1)
            return 1;
        std::string code(&argv[0]->s);
        proc->exec(code);
        return 0;
    });
}

void
OscHandler::start()
{
    registerHandlers();

    server.start();
    DBG_PRINT("Listening on " << server.url());

    client.send("exec", "s", "import numpy as np\n"
                             "def process(input, output):\n"
                             "    output[:] = np.abs(input)\n");
}
