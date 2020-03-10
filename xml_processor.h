
#ifndef ZIX_XML_PROCESSOR_H
#define ZIX_XML_PROCESSOR_H

#include "xml_request.h"
#include "interface_connection.h"


#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <istream>
#include <queue>
#include <functional>
#include <list>
#include <memory>

class InterfaceConnection;
class XmlRequest;

/**
 * \brief Central Element which priorises and processes XmlRequests
 *
 * Call XmlProcessor::parse_stream when a Handler has an Object
 */
class XmlProcessor : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<XmlProcessor>;
    using ReqPtr = Glib::RefPtr<XmlRequest>;

    XmlProcessor();

    static RefPtr create();

    void parse_stream(std::istream& is, int tid, int prio, std::weak_ptr<InterfaceConnection> ic,
                      const Glib::ustring& interface = "", bool restart_proc = false);

    void init();

protected:
    void execute_top_request();
    void execute_next_high_prio_request();
    void execute_next_gui_request();
    void request_finished();
    void high_prio_request_finished();
    void gui_request_finished();

private:
    std::priority_queue<ReqPtr,
                        std::vector<ReqPtr>,
                        std::function<bool(const ReqPtr&, const ReqPtr&)> >
    _prio_queue, _high_prio_queue, _gui_queue;

    Glib::RefPtr<XmlRequest> _current_request;
    Glib::RefPtr<XmlRequest> _current_high_prio_request;
    Glib::RefPtr<XmlRequest> _current_gui_request;
    std::shared_ptr<InterfaceConnection> _procedure_connection;
};

#endif
