#ifndef _ID_MAPPER_H_
#define _ID_MAPPER_H_

#include <libxml++/document.h>
#include <libxml++/parsers/domparser.h>

#include <glibmm/ustring.h>
#include <glibmm/refptr.h>
#include <glibmm/object.h>

#include <string>

#include "mapping_entry.h"

/**
 * \brief This class manages the mappings between measurement internal and
 *        external ids.
 */
class IdMapper : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<IdMapper>;

    static inline RefPtr get_instance()
    {
        if (!instance)
            return create();
        return instance;
    }

    void add_mapping(const MappingEntry& entry);
    void remove_mapping(const Glib::ustring& iid);
    Glib::ustring get_iid(const Glib::ustring& id);

    Glib::ustring get_all_mappings() const;
    Glib::ustring get_all_allocated_mappings() const;

private:
    static RefPtr instance;

    xmlpp::DomParser _parser;
    xmlpp::Node *_root;
    std::string _file_name;
    std::string _file_name_tmp;

    inline IdMapper() :
        _root{nullptr}
    {
        setup();
    }

    static inline RefPtr create()
    {
        return RefPtr(new IdMapper());
    }

    void setup();
    void save();

    xmlpp::Element *find_mapping_by_iid(const Glib::ustring& iid);
    xmlpp::Element *find_mapping_by_id(const Glib::ustring& id);

    Glib::ustring map_elem_to_str(const xmlpp::Element *elem) const;
};

#endif /* _ID_MAPPER_H_ */
