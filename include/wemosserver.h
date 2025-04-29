/**
 * @file wemosserver.h
 * @brief Header file for wemosserver.cpp.
 * @details This file contains declarations for the classes and functions
 *          used in the Wemos server application.
 * @author Daan Breur
 */

#ifndef WEMOSSERVER_H
#define WEMOSSERVER_H

#include <string>

class WemosServer {
   private:
    int server_fd;

   public:
    WemosServer(int, const std::string &, int);
    ~WemosServer();

    WemosServer(const WemosServer &) = delete;
    WemosServer &operator=(const WemosServer &) = delete;
    WemosServer(WemosServer &&) = delete;
    WemosServer &operator=(WemosServer &&) = delete;

    void socket_setup();
    void loop();
    void tearDown();
};

#endif
