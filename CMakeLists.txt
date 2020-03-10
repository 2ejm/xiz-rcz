#-----------------------------------------------------------------------------
#
# \brief	CMake file for libzix
#
# \author	Maximilian Seesslen <maximilian.seesslen@linutronix.de>
#
#-----------------------------------------------------------------------------

project(zix CXX ASM C)

cmake_minimum_required(VERSION 2.8)


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


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
   add_custom_target(doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
   )
endif(DOXYGEN_FOUND)


if( DEBUG_CLASSES )
   add_definitions( "-DDEBUG_CLASSES" )
endif( DEBUG_CLASSES )

if( SOCKET_OPEN )
   add_definitions( "-DSOCKET_OPEN" )
endif( SOCKET_OPEN )


if( USE_DEVICECONTROLLER )
   add_definitions( "-DUSE_DEVICECONTROLLER" )
endif( USE_DEVICECONTROLLER )

if( GLOBAL_INSTALLATION )
   add_definitions( "-DGLOBAL_INSTALLATION" )
endif( GLOBAL_INSTALLATION )

set(CMAKE_C_FLAGS "-Werror -Wall -g -Wsign-compare -Wdeclaration-after-statement -W --std=c99 -Wno-format-truncation -Wno-int-in-bool-context ")
set(CMAKE_CXX_FLAGS "-Werror -Wall -g -Wsign-compare -W -Wno-deprecated-declarations --std=c++11 -Wno-format-truncation -Wno-int-in-bool-context")


set(C_LIBRARIES
         ${PROJECT_NAME}
         ${C_GLIBMM_LIBRARIES}
         ${C_GIOMM_LIBRARIES}
         ${C_XML_LIBRARIES}
)


add_library(${PROJECT_NAME} SHARED
	byteslist_istream.cc
	core_function_call.cc
	core_function_log.cc
	core_function_get_files_list.cc
	core_function_del_file.cc
	core_function_get_file.cc
	core_function_set_file.cc
	core_function_set_conf.cc
	core_function_get_conf.cc
	core_function_data_sync.cc
	core_function_data_out.cc
	core_function_data_free.cc
	core_function_wrap_dc.cc
	core_function_get_measurement.cc
        core_function_get_measurements_list.cc
	core_function_procedure.cc
        core_function_data_in.cc
        core_function_exit.cc
        core_function_update.cc
	crc32.cc
	gio_istream_adapter.cc
	hexio.cc
	interface_connection.cc
	interface_handler.cc
	serial_interface_handler.cc
	usbserial_interface_handler.cc
	serial_message.cc
	ustring_istream.cc
	socket_interface_handler.cc
        unix_socket_interface_handler.cc
	socket_interface_connection.cc
        file_interface_connection.cc
	xml_description.cc
	xml_exception.cc
	xml_function.cc
	xml_helpers.cc
	xml_parameter.cc
	xml_parameter_list.cc
	xml_processor.cc
	xml_request.cc
	xml_resource.cc
	xml_resource_parameter.cc
	xml_conf_parameter.cc
	xml_result.cc
	xml_result_bad_request.cc
	xml_result_ok.cc
	xml_result_not_found.cc
	xml_result_forbidden.cc
	xml_result_internal_device_error.cc
	xml_result_too_many_requests.cc
	xml_result_parsed.cc
	xml_result_timeout.cc
	xml_string_parameter.cc
	xml_node_parameter.cc
	xml_file.cc
	xml_measurement.cc
	xml_image.cc
	xml_query.cc
	xml_query_generic.cc
	xml_query_pass.cc
	xml_query_set_file.cc
	xml_query_get_file.cc
	xml_query_del_file.cc
	xml_query_get_files_list.cc
	xml_query_get_parameter.cc
	xml_query_replay_status.cc
	xml_query_set_parameter.cc
	xml_query_set_datetime.cc
	xml_query_gui_io.cc
        xml_query_get_measurements_list.cc
	xml_restriction.cc
	xml_restriction_parameter.cc
	xml_signature.cc
	log_handler.cc
	log_file_entry.cc
	conf_handler.cc
	file_handler.cc
	process_request.cc
	time_utilities.cc
	read_file_request.cc
	write_file_request.cc
	copy_file_request.cc
	monitor.cc
	monitor_cpu.cc
	monitor_memory.cc
	monitor_network.cc
	monitor_directory.cc
	monitor_uptime.cc
	monitor_disk_usage.cc
	monitor_manager.cc
	disk_usage_manager.cc
	signature_check_request.cc
        signature_creation_request.cc
	ustring_utils.cc
	query_client.cc
	query_client_dummy.cc
	restriction_check_request.cc
	query_client_serial.cc
	log.cc
	libzix.cc
        file_destination.cc
        post_processing_builder.cc
        watchdog.cc
        lan_watchdog.cc
        usb_watchdog.cc
        watchdog_manager.cc
        zix_interface.cc
        file_service.cc
        file_interface_handler.cc
        timer.cc
        udisks_device.cc
        network_config.cc
        network_config_helpers.cc
        network_helpers.cc
        udp_channel.cc
        start_stoppable.cc
        accessibility_checker.cc
	procedure_step_handler.cc
        id_mapper.cc
	function_call.cc
	function_call_restricted.cc
	function_call_sigcheck.cc
        config_serial_interface_handler.cc
        samba_mounter.cc
        printer_monitor.cc
        webservice.cc
	state_variable.cc
	lan_socket_monitor.cc
	update_signature_check_request.cc
	ntp_monitor.cc
	simple_crypt.cc
	serial_helper.cc
)


add_executable( libzix_daemon
			libzix_daemon.cc
)


target_link_libraries ( libzix_daemon
			${C_LIBRARIES}
)


add_executable(test_socket_interface
         test_socket_interface.cc
)


target_link_libraries ( test_socket_interface
         ${C_LIBRARIES}
)


add_executable(test_serial_interface
         test_serial_interface.cc
)


target_link_libraries ( test_serial_interface
         ${C_LIBRARIES}
)


add_executable(test_monitor
         test_monitor.cc
)

target_link_libraries ( test_monitor
         ${C_LIBRARIES}
)

add_executable(test_conf test_conf.cc)
target_link_libraries ( test_conf ${C_LIBRARIES} )


add_executable( test_client
         test_client.cc
         socket_client.cc
)
target_link_libraries ( test_client
         ${C_LIBRARIES}
)


add_executable(test_mapping test_mapping.cc)
target_link_libraries ( test_mapping ${C_LIBRARIES} )

add_executable(test_serial_utf8 test_serial_utf8.cc)
target_link_libraries ( test_serial_utf8 ${C_LIBRARIES} )

# add_subdirectory( visux_daemon )

install(
         TARGETS libzix_daemon
         RUNTIME DESTINATION bin
)


install(
         TARGETS ${PROJECT_NAME}
         LIBRARY DESTINATION lib
)


install(
         FILES defaults.xml DESTINATION /usr/share/zix
)


#---fin.----------------------------------------------------------------------
