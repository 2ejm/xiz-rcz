#ifndef _FILE_SERVICE_H_
#define _FILE_SERVICE_H_

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <sigc++/signal.h>

#include <string>

#include "zix_interface.h"

class FileService : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<FileService>;

    static inline RefPtr& get_instance()
    {
        if (!instance)
            instance = create();
        return instance;
    }

    sigc::signal<void, ZixInterface, const std::string&, const std::string&> new_file;
    sigc::signal<void, ZixInterface, const std::string&, const std::string&, const std::string&> new_file_content;

private:
    static RefPtr instance;

    FileService() :
        Glib::Object()
    {}

    static RefPtr create()
    {
        return RefPtr(new FileService());
    }
};

#endif /* _FILE_SERVICE_H_ */
