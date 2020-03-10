#include "file_interface_handler.h"

#include "file_interface_connection.h"
#include "utils.h"

FileInterfaceHandler::FileInterfaceHandler(
    ZixInterface inf,
    Glib::RefPtr<XmlProcessor> xml_processor) :
    Glib::ObjectBase(typeid (FileInterfaceHandler)),
    InterfaceHandler(inf, inf.to_string(), xml_processor)
{
    _file_service = FileService::get_instance();
    _file_service->new_file.connect(
        sigc::mem_fun(*this, &FileInterfaceHandler::incoming_file));
    _file_service->new_file_content.connect(
        sigc::mem_fun(*this, &FileInterfaceHandler::incoming_file_content));
}

Glib::RefPtr<FileInterfaceHandler> FileInterfaceHandler::create(
    ZixInterface inf,
    Glib::RefPtr<XmlProcessor> xml_processor)
{
    return Glib::RefPtr<FileInterfaceHandler>(new FileInterfaceHandler(inf, xml_processor));
}

void FileInterfaceHandler::incoming_file(
    ZixInterface inf, const std::string& in_path, const std::string& out_path)
{
    if (inf != _inf)
        return;

    auto iface_connection = FileInterfaceConnection::create_for_path(in_path, out_path);
    auto file_connection  = std::dynamic_pointer_cast<FileInterfaceConnection>(iface_connection);

    if (!file_connection) {
	PRINT_ERROR("Wrong connection type in file interface handler");
	return;
    }

    register_connection(iface_connection);

    iface_connection->connection_closed.connect(
        sigc::bind<std::weak_ptr<InterfaceConnection> >(
            sigc::mem_fun(*this, &FileInterfaceHandler::closed_connection),
            std::weak_ptr<InterfaceConnection>(iface_connection)));

    file_connection->read_file();
}

void FileInterfaceHandler::incoming_file_content(
    ZixInterface inf, const std::string& in_path, const std::string& content,
    const std::string& out_path)
{
    PRINT_DEBUG ("FileInterfaceHandler::incoming_file_content (out_path =" << out_path << ")");
    if (inf != _inf) {
	PRINT_DEBUG ("exiting because inf != _inf ( " << inf << " " << _inf << " )");
        return;
    }

    auto iface_connection = FileInterfaceConnection::create_for_content(in_path, content, out_path);
    auto file_connection  = std::dynamic_pointer_cast<FileInterfaceConnection>(iface_connection);

    if (!file_connection) {
	PRINT_ERROR("Wrong connection type in file interface handler");
	return;
    }

    register_connection(iface_connection);

    iface_connection->connection_closed.connect(
        sigc::bind<std::weak_ptr<InterfaceConnection> >(
            sigc::mem_fun(*this, &FileInterfaceHandler::closed_connection),
            std::weak_ptr<InterfaceConnection>(iface_connection)));

    file_connection->stream_complete();
}

void FileInterfaceHandler::closed_connection(std::weak_ptr<InterfaceConnection> conn)
{
    deregister_connection(conn);
}
