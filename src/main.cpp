//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//
//------------------------------------------------------------------------------
//
// Example: WebSocket server, synchronous
//
//------------------------------------------------------------------------------
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include "wsserver.h"
#include "sslserver.h"
#include "ubushandle.h"
#include "configure.h"


void run(const char *pathname, unsigned short port)
{
	boost::asio::io_context io_context;
	SIPRouter router("main");

	wsServer ws(&router, io_context, pathname, port);
	sslServer ssl(&router, io_context, 5080);

	ubusASIOContextHandler g_handler(io_context);
	g_handler.waitUBUSHandleEventAsync();

	addConfigureService();

	io_context.run();
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    try
    {
		run(argv[1], (unsigned short)atoi(argv[2]));
    }
    catch (const std::exception & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}