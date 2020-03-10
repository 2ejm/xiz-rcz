#ifndef _CORE_FUNCTION_DATA_FREE_H_
#define _CORE_FUNCTION_DATA_FREE_H_

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>

#include "core_function_call.h"
#include "xml_parameter_list.h"

/**
 * \brief dataFree function
 *
 * This functions call does nothing except for one thing:
 * It flags a measurement directory for deletion by creating a hidden
 * file called '.deletion'.
 */
class CoreFunctionDataFree : public CoreFunctionCall
{
public:
    CoreFunctionDataFree(
        XmlParameterList parameters,
        Glib::RefPtr<XmlDescription> description,
	const Glib::ustring & text);

    ~CoreFunctionDataFree();

    void start_call();

    static Glib::RefPtr<CoreFunctionCall> factory (XmlParameterList parameters,
						   Glib::RefPtr <XmlDescription> description,
						   const Glib::ustring & text,
						   const xmlpp::Element * en);

private:
    std::string _measurement_dir;
    Glib::ustring _iid;

    void immediate_deletion() const;
    void set_deletion_flag() const;
    void free_iid_mapping(const std::string& iid) const;
};

#endif /* _CORE_FUNCTION_DATA_FREE_H_ */
