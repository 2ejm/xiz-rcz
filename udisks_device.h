#ifndef _UDISKS_DEVICE_H_
#define _UDISKS_DEVICE_H_

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <giomm/dbusconnection.h>
#include <giomm/dbusproxy.h>

#include <vector>
#include <map>
#include <sstream>

#include "utils.h"

/**
 * \brief This class represents one Dbus Udisks Device.
 */
class UdisksDevice : public Glib::Object
{
public:
    using RefPtr = Glib::RefPtr<UdisksDevice>;
    using String = Glib::ustring;

    inline UdisksDevice(const Glib::RefPtr<Gio::DBus::Connection>& bus, const String& device) :
        _bus{bus}, _device{device}
    {
        _dev_proxy = Gio::DBus::Proxy::create_sync(
            _bus, "org.freedesktop.UDisks", _device, "org.freedesktop.UDisks.Device");
        _prop_proxy = Gio::DBus::Proxy::create_sync(
            _bus, "org.freedesktop.UDisks", _device, "org.freedesktop.DBus.Properties");

        update_properties();
    }

    static inline RefPtr create(const Glib::RefPtr<Gio::DBus::Connection>& bus,
                                const String& device)
    {
        return RefPtr(new UdisksDevice(bus, device));
    }

    void dump() const;
    String mount();
    void unmount();

    inline const String& operator[] (const String& key) const
    {
        auto it = _props.find(key);
        if (it == _props.end())
            EXCEPTION("Unknown property request: " << key);
        return it->second;
    }

    inline String& operator[] (const String& key)
    {
        auto it = _props.find(key);
        if (it == _props.end())
            EXCEPTION("Unknown property request: " << key);
        return it->second;
    }

    inline const String& get(const String& key) const
    {
        auto it = _props.find(key);
        if (it == _props.end())
            EXCEPTION("Unknown property request: " << key);
        return it->second;
    }

    inline String& get(const String& key)
    {
        auto it = _props.find(key);
        if (it == _props.end())
            EXCEPTION("Unknown property request: " << key);
        return it->second;
    }

    void update_properties();

private:
    Glib::RefPtr<Gio::DBus::Connection> _bus;
    Glib::RefPtr<Gio::DBus::Proxy> _dev_proxy;
    Glib::RefPtr<Gio::DBus::Proxy> _prop_proxy;
    std::map<String, String> _props;

    String _device;

    template<typename T>
    void get_prop(const String& prop_name)
    {
        std::vector<Glib::VariantBase> args;
        args.emplace_back(Glib::Variant<String>::create("org.freedesktop.UDisks.Device"));
        args.emplace_back(Glib::Variant<String>::create(prop_name));
        auto base = _prop_proxy->call_sync(
            "Get", Glib::VariantContainerBase::create_tuple(args));

        Glib::Variant<T> item;
        Glib::VariantIter it(base.get_child(0));
        if (it.next_value(item)) {
            std::stringstream ss;
            ss << item.get();
            _props[prop_name] = ss.str();
        }
    }

    template<typename T>
    void get_prop_arr(const String& prop_name)
    {
        std::vector<Glib::VariantBase> args;
        args.emplace_back(Glib::Variant<String>::create("org.freedesktop.UDisks.Device"));
        args.emplace_back(Glib::Variant<String>::create(prop_name));
        auto base = _prop_proxy->call_sync(
            "Get", Glib::VariantContainerBase::create_tuple(args));

        Glib::Variant<std::vector<T> > item;
        Glib::VariantIter it(base.get_child(0));
        if (it.next_value(item)) {
            if (item.get().empty()) {
                _props[prop_name] = "";
                return;
            }

            std::stringstream ss;
            ss << item.get().at(0);
            _props[prop_name] = ss.str();
        }
    }
};

#endif /* _UDISKS_DEVICE_H_ */
