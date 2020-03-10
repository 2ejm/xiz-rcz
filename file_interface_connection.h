#ifndef _FILE_INTERFACE_CONNECTION_H_
#define _FILE_INTERFACE_CONNECTION_H_

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include <string>
#include <memory>

#include "interface_connection.h"

class FileInterfaceConnection : public InterfaceConnection
{
public:
    FileInterfaceConnection(const std::string& in_path,
                            const std::string& content,
                            const std::string& out_path = "");

    static inline std::shared_ptr<InterfaceConnection> create_for_content(
        const std::string& in_path, const std::string& content, const std::string& out_path = "")
    {
        return std::shared_ptr<InterfaceConnection>(new FileInterfaceConnection(in_path, content, out_path));
    }

    static inline std::shared_ptr<InterfaceConnection> create_for_path(
        const std::string& in_path, const std::string& out_path = "")
    {
        return std::shared_ptr<InterfaceConnection>(new FileInterfaceConnection(in_path, "", out_path));
    }

    static inline std::shared_ptr<InterfaceConnection> create_reply_connection (
        const std::string& out_path)
    {
        return std::shared_ptr<InterfaceConnection>(new FileInterfaceConnection("", "", out_path));
    }

    void emit_result(const Glib::ustring& result, int tid) override;
    void cancel() override;

    void read_file();
    void stream_complete();

private:
    std::string _in_path;
    std::string _content;
    std::string _out_path;
};

#endif /* _FILE_INTERFACE_CONNECTION_H_ */
