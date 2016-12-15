#include "jackclient.h"
#include "pythonprocessor.h"
#include <chrono>
#include <thread>

int
main()
{
    // initial ports from constructor created here.
    PythonProcessor proc;
    JackClient client(&proc);
    client.start(); // activate the client

    // connect our ports to physical ports
    client.connectToPhysical(0, 0);
    client.connectToPhysical(1, 1);
    client.connectFromPhysical(0, 0);
    client.connectFromPhysical(1, 1);

    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());
}
