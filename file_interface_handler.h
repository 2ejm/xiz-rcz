#ifndef _FILE_INTERFACE_HANDLER_H_
#define _FILE_INTERFACE_HANDLER_H_

#include <glibmm/refptr.h>

#include <string>
#include <istream>
#include <memory>

#include "interface_handler.h"
#include "file_service.h"
#include "xml_processor.h"
#include "zix_interface.h"

class FileInterfaceHandler : public InterfaceHandler
{
public:
    FileInterfaceHandler(ZixInterface inf, Glib::RefPtr<XmlProcessor> xml_processor);

    static Glib::RefPtr<FileInterfaceHandler>
    create(ZixInterface inf, Glib::RefPtr<XmlProcessor> xml_processor);

private:
    Glib::RefPtr<FileService> _file_service;

    void incoming_file(ZixInterface inf, const std::string& in_path, const std::string& out_path);
    void incoming_file_content(ZixInterface inf, const std::string& in_path,
                               const std::string& content, const std::string& out_path);
    void closed_connection(std::weak_ptr<InterfaceConnection> conn);
};

#endif /* _FILE_INTERFACE_HANDLER_H_ */
