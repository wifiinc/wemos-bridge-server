/**
 * @file main.cpp
 * @brief Main entrypoint for Wemos Bridge Server application.
 */

#include <unistd.h>

#include <atomic>
#include <csignal>
#include <iostream>

#include "wemosserver.h"

#define SERVER_PORT 5000
#define I2C_HUB_IP "192.168.226.245"
#define I2C_HUB_PORT 5000

std::atomic<bool> global_shutdown_flag(false);

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    if (signum == SIGINT || signum == SIGTERM) {
        global_shutdown_flag = true;
    }
}

int main() {
    setbuf(stdout, NULL);
    std::cout << "Starting Wemos Bridge on port " << SERVER_PORT << std::endl;

    // signal(SIGINT, signalHandler);
    // signal(SIGTERM, signalHandler);

    WemosServer server(SERVER_PORT, I2C_HUB_IP, I2C_HUB_PORT);

    sleep(1);

    server.start();

    return 0;
}
