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
#include "sqlite3.h"


void run(const char *pathname, unsigned short port)
{
	boost::asio::io_context io_context;
	printf("to init router\r\n");
	SIPRouter router(io_context, "www.ipv4.mtlink.cn");
	sqlite3DB::getInstance().addUser("1001", "1234");
	sqlite3DB::getInstance().addUser("1002", "1234");

	printf("to ws server\r\n");
	wsServer ws(&router, io_context, pathname, port);

	printf("to ssl server\r\n");
	sslServer ssl(&router, io_context, 5080);

	printf("to int ubus handle\r\n");
	ubusASIOContextHandler g_handler(io_context);
	g_handler.waitUBUSHandleEventAsync();

	printf("to add configure service\r\n");
	addConfigureService();

	printf("to run loop\r\n");
	io_context.run();
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    try
    {
		printf("in main to run\r\n");
		run(argv[1], (unsigned short)atoi(argv[2]));
    }
    catch (const std::exception & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}