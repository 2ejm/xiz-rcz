#include "xml_query_replay_status.h"

Glib::ustring XmlQueryReplayStatus::to_xml() const
{
    Glib::ustring ret;

    ret += Glib::ustring::compose("<function fid=\"%1\">\n", _fid);

    auto id_iter = _ids.begin ();
    auto value_iter = _values.begin ();

    while (1) {
	ret += Glib::ustring::compose("<parameter id=\"%1\" value=\"%2\"/>\n", *id_iter, *value_iter);

	/* increment iterators
	 */
	id_iter ++;
	value_iter ++;

	/* now check, whether we are at some end
	 */
	if (id_iter == _ids.end ())
	    break;

	if (value_iter == _values.end ())
	    break;
    }

    ret += "</function>\n";

    return ret;
}
