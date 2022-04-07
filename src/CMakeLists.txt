cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE off)
PROJECT(sipd)

add_executable(sipd main.cpp siprouter.cpp sipmessage.cpp)

install(TARGETS sipd
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
