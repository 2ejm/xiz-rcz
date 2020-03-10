#-----------------------------------------------------------------------------
#
# \brief    CMake file for visux daemon
#
# \author   Maximilian Seesslen <maximilian.seesslen@linutronix.de>
#
#-----------------------------------------------------------------------------

project( visux CXX ASM C )

cmake_minimum_required( VERSION 2.8 )


# aux_source_directory(. SRC_LIST)


find_package (PkgConfig REQUIRED)
pkg_check_modules (C_GLIB REQUIRED glib-2.0)
pkg_check_modules (C_GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules (C_GIOMM REQUIRED giomm-2.4)
pkg_check_modules (C_XML REQUIRED libxml++-2.6)

include_directories (
         ${GLIB_INCLUDE_DIRS}
         ${GLIBMM_INCLUDE_DIRS}
         ${C_GIOMM_INCLUDE_DIRS}
	 ${C_XML_INCLUDE_DIRS}
)

# message ( "GIOMM_INCLUDE_DIRS: ${_GIOMM_INCLUDE_DIRS}" )

link_directories (
         ${GLIBMM_LIBRARY_DIRS}
         ${GIOMM_LIBRARY_DIRS}
	 ${C_XML_LIBRARY_DIRS}
)


if( DEBUG_CLASSES )
   add_definitions( "-DDEBUG_CLASSES" )
endif( DEBUG_CLASSES )


set(CMAKE_C_FLAGS "-Werror -Wall -g -Wsign-compare -Wdeclaration-after-statement -W --std=c99")
set(CMAKE_CXX_FLAGS "-Werror -Wall -g -Wsign-compare  -W --std=c++11")

set(C_LIBRARIES
         ${C_GLIBMM_LIBRARIES}
         ${C_GIOMM_LIBRARIES}
         ${C_XML_LIBRARIES}
         pthread
)


add_executable( controller
         udp_channel.cc
         visux.cc
         network_helpers.cc
         helpers.cc
         controller.cc
)


target_link_libraries ( controller
         ${C_LIBRARIES}

)


add_executable( visux_daemon
         udp_channel.cc
         visux.cc
         network_helpers.cc
         helpers.cc
         daemon.cc
)

target_link_libraries ( visux_daemon
         ${C_LIBRARIES}
)


install(
         TARGETS visux_daemon
         RUNTIME DESTINATION bin
)


include_directories("${zix_SOURCE_DIR}")

#add_executable( test
#         test.cc
#)

#target_link_libraries ( test
#         ${C_LIBRARIES}
#)


#---fin.----------------------------------------------------------------------
