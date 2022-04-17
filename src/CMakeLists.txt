cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE off)
PROJECT(sipd)

add_executable(sipd main.cpp siprouter.cpp sipmessage.cpp sslserver.cpp wsserver.cpp
		sipendpoint.cpp ubushandle.cpp configure.cpp callid.cpp)

TARGET_LINK_LIBRARIES(sipd crypto)
TARGET_LINK_LIBRARIES(sipd ssl service curl ev)

install(TARGETS sipd
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
