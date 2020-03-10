#include <libxml++/nodes/element.h>
#include <libxml++/nodes/contentnode.h>
#include <libxml++/nodes/node.h>
#include <libxml++/libxml++.h>
#include <libxml/tree.h>

#include "query_client.h"
#include "xml_query_get_parameter.h"
#include "utils.h"

#include "restriction_check_request.h"

// Note: This contains only the true values, everything else is considered false
const RestrictionCheckRequest::RestCheckMap RestrictionCheckRequest::rest_check_map =
{
    { STR_ZIXINF_LANWEBSERVICE, {
            { "update", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplate", true },
            { "delFile", true }
        },
    },
    { STR_ZIXINF_LANWEBSERVER, {
            { "update", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplate", true },
            { "delFile", true }
        },
    },
    { STR_ZIXINF_LANSHAREDFOLDER, {
            { "update", true },
            { "procedure", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplate", true },
            { "delFile", true }
        },
    },
    { STR_ZIXINF_LANSOCKET, {
            { "update", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplate", true },
            { "delFile", true }
        },
    },
    { STR_ZIXINF_USB, {
            { "update", true },
            { "procedure", true },
            { "setCalibration", true },
            { "setDefaults", true },
            { "setTemplatesList", true },
            { "setTemplate", true },
            { "setFile", true },
            { "delTemplate", true },
            { "delFile", true }
        },
    },
    { STR_ZIXINF_COM1, {
            { "update", true },
       },
    },
    { STR_ZIXINF_COM2, {
            { "update", true },
       },
    },
};

void RestrictionCheckRequest::start_query_dc()
{
    auto dc = QueryClient::get_instance();
    auto xq = XmlQueryGetParameter::create(_restrictions);
    _write_query = dc->create_query(xq);
    _write_query->finished.connect(
        sigc::mem_fun(*this, &RestrictionCheckRequest::on_write_query_finish));
    dc->execute(_write_query);
}

bool RestrictionCheckRequest::check_needed(const ZixInterface& channel) const
{
    auto it1 = rest_check_map.find(channel.to_string());
    if (it1 == rest_check_map.end()) {
        PRINT_DEBUG("Unknown Interface used: " << channel.to_string());
        return false;
    }

    auto it2 = it1->second.find(_fid);
    if (it2 == it1->second.end())
        return false;

    return it2->second;
}

void RestrictionCheckRequest::start_check(const ZixInterface& channel)
{
    if (!check_needed(channel)) {
        finished.emit(RestrictionCheckResult::create(true, ""));
        return;
    }

    if (!_restrictions.size()) {
        finished.emit(
            RestrictionCheckResult::create(
                false, Glib::ustring::compose(
                    "The function '%1' should be protected by restrictions, but none given!",
                    _fid)));
        return;
    }

    start_query_dc();
}

bool RestrictionCheckRequest::evaluate() const
{
    bool result = false;

    for (auto&& res : _restrictions) {
        bool par_result = true;
        for (auto&& par : res->params())
            par_result = par_result && par->evaluate();
        result = result || par_result;
    }

    return result;
}

void RestrictionCheckRequest::enrich_values(const Glib::ustring& xml)
{
    xmlpp::DomParser parser;

    parser.parse_memory(xml);
    auto *root = parser.get_document()->get_root_node();
    if (!root)
        return;

    for (auto&& c : root->get_children()) {
        xmlpp::Element *en = dynamic_cast<xmlpp::Element *>(c);
        if (!en)
            continue;
        if (en->get_name() != "parameter")
            continue;
        auto *id_attr  = en->get_attribute("id");
        auto *val_attr = en->get_attribute("value");

        if (!id_attr || !val_attr)
            continue;

        const auto& id  = id_attr->get_value();
        const auto& val = val_attr->get_value();

        for (auto&& res : _restrictions)
            for (auto&& par : res->params())
                if (par->id() == id)
                    par->value() = val;
    }
}

void RestrictionCheckRequest::on_write_query_finish(
    const Glib::RefPtr<XmlResult>& result)
{
    std::string error_msg;
    bool ret;

    try {
        if (result->get_status() != 200)
            EXCEPTION("Couldn't get parameters from DC, reply status is "
                      << result->get_status());

        // save values
        enrich_values(result->to_xml());

        // evaluate
        ret = evaluate();
    } catch (const std::exception& ex) {
        error_msg = ex.what();
        ret = false;
    }

    // build result
    if (!ret)
        error_msg = "Restrictions aren't met";
    auto res = RestrictionCheckResult::create(ret, error_msg);
    finished.emit(res);
}
