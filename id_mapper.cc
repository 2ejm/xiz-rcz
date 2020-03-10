#include <libxml++/nodes/element.h>
#include <libxml++/nodes/node.h>
#include <libxml++/libxml++.h>
#include <libxml/tree.h>

#include "file_handler.h"
#include "conf_handler.h"
#include "utils.h"

#include "id_mapper.h"

IdMapper::RefPtr IdMapper::instance(nullptr);

void IdMapper::setup()
{
    try {
        _file_name = ConfHandler::get_instance()->getDirectory("measurements");
        if (_file_name.empty())
            EXCEPTION("Failed to find measurements directory");
        _file_name += "/ML.xml";
        _file_name_tmp = _file_name + ".tmp";

        if (FileHandler::file_exists(_file_name)) {
            _parser.parse_file(_file_name);
            _root = _parser.get_document()->get_root_node();
        } else {
            _parser.parse_memory("<?xml version=\"1.0\" encoding=\"UTF-8\"?><map></map>");
            _root = _parser.get_document()->get_root_node();
        }
    } catch (...) {
        EXCEPTION("Failed to setup Id Mapping XML");
    }
}

void IdMapper::save()
{
    _parser.get_document()->write_to_file_formatted(_file_name_tmp);
    FileHandler::move_file(_file_name_tmp, _file_name);
}

xmlpp::Element *IdMapper::find_mapping_by_iid(const Glib::ustring& iid)
{
    auto nodeSet = _root->find(
        Glib::ustring::compose("//mapping[@iid='%1']", iid));

    if (nodeSet.size() == 0)
        return nullptr;

    if (nodeSet.size() > 1)
        PRINT_ERROR("More than id mapping found for iid. Using the first one");

    return dynamic_cast<xmlpp::Element *>(nodeSet.front());
}

xmlpp::Element *IdMapper::find_mapping_by_id(const Glib::ustring& id)
{
    auto nodeSet = _root->find(
        Glib::ustring::compose("//mapping[@id='%1']", id));

    if (nodeSet.size() == 0)
        return nullptr;

    if (nodeSet.size() > 1)
        PRINT_ERROR("More than id mapping found for id. Using the first one");

    return dynamic_cast<xmlpp::Element *>(nodeSet.front());
}

void IdMapper::add_mapping(const MappingEntry& entry)
{
    auto *child = find_mapping_by_iid(entry.iid());

    // two cases: update or create
    if (child) {
        if (!entry.id().empty())
            child->set_attribute("id", entry.id());
        if (!entry.date().empty())
            child->set_attribute("stamp_date", entry.date());
        if (!entry.time().empty())
            child->set_attribute("stamp_time", entry.time());
        if (!entry.tz().empty())
            child->set_attribute("stamp_timeZone", entry.tz());
    } else {
        auto *node = _root->add_child("mapping");
        if (!node)
            EXCEPTION("Failed to add mapping");
        auto *elem = dynamic_cast<xmlpp::Element *>(node);
        if (!elem)
            EXCEPTION("Failed to add mapping");

        elem->set_attribute("id", entry.id());
        elem->set_attribute("iid", entry.iid());
        elem->set_attribute("stamp_date", entry.date());
        elem->set_attribute("stamp_time", entry.time());
        elem->set_attribute("stamp_timeZone", entry.tz());
    }

    save();
}

Glib::ustring IdMapper::get_iid(const Glib::ustring& id)
{
    auto *child = find_mapping_by_id(id);
    if (!child)
        EXCEPTION("Failed to get iid for id " << id);

    auto iid = child->get_attribute_value("iid");
    if (iid.empty())
        EXCEPTION("Failed to get iid for id " << id);

    return iid;
}

void IdMapper::remove_mapping(const Glib::ustring& iid)
{
    auto *child = find_mapping_by_iid(iid);
    if (!child)
        return;

    _root->remove_child(child);
    save();
}

Glib::ustring IdMapper::map_elem_to_str(const xmlpp::Element *elem) const
{
    Glib::ustring ret;

    if (!elem)
        return "";

    auto id   = elem->get_attribute_value("id");
    auto iid  = elem->get_attribute_value("iid");
    auto date = elem->get_attribute_value("stamp_date");
    auto time = elem->get_attribute_value("stamp_time");
    auto tz   = elem->get_attribute_value("stamp_timeZone");

    ret = "<measurement ";
    if (!id.empty())
        ret += Glib::ustring::compose("id=\"%1\" ", id);
    if (!iid.empty())
        ret += Glib::ustring::compose("iid=\"%1\" ", iid);
    if (!date.empty())
        ret += Glib::ustring::compose("stamp_date=\"%1\" ", date);
    if (!time.empty())
        ret += Glib::ustring::compose("stamp_time=\"%1\" ", time);
    if (!tz.empty())
        ret += Glib::ustring::compose("stamp_timeZone=\"%1\" ", tz);
    ret += "/>";

    return ret;
}

Glib::ustring IdMapper::get_all_mappings() const
{
    Glib::ustring ret;

    for (auto&& node : _root->get_children()) {
        if (node->get_name() != "mapping")
            continue;

        auto *elem = dynamic_cast<xmlpp::Element *>(node);
        if (!elem)
            continue;

        ret += map_elem_to_str(elem) + "\n";
    }

    return ret;
}

Glib::ustring IdMapper::get_all_allocated_mappings() const
{
    Glib::ustring ret;

    for (auto&& node : _root->get_children()) {
        if (node->get_name() != "mapping")
            continue;

        auto *elem = dynamic_cast<xmlpp::Element *>(node);
        if (!elem)
            continue;

        auto id  = elem->get_attribute_value("id");
        if (!id.empty())
            ret += map_elem_to_str(elem) + "\n";
    }

    return ret;
}
