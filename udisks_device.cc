#include <vector>

#include "udisks_device.h"

#include "utils.h"

void UdisksDevice::update_properties()
{
    get_prop<String>("DeviceFile");
    get_prop<String>("DriveConnectionInterface");
    get_prop_arr<String>("DeviceMountPaths");
    get_prop<bool>("DeviceIsDrive");
    get_prop<bool>("DeviceIsSystemInternal");
    get_prop<bool>("DeviceIsRemovable");
    get_prop<bool>("DeviceIsPartitionTable");
    get_prop<bool>("DeviceIsMounted");
    get_prop<bool>("DeviceIsPartition");
    get_prop<bool>("DeviceIsReadOnly");
}

void UdisksDevice::dump() const
{
    PRINT_INFO("Dump of " << _device);
    for (auto&& e : _props)
        std::cout << e.first << " : " << e.second << std::endl;
}

UdisksDevice::String UdisksDevice::mount()
{
    std::vector<Glib::VariantBase> args;
    args.emplace_back(Glib::Variant<String>::create(""));
    args.emplace_back(Glib::Variant<std::vector<String> >::create({ "sync", "rw" }));
    auto base = _dev_proxy->call_sync(
        "FilesystemMount", Glib::VariantContainerBase::create_tuple(args));

    Glib::Variant<String> item;
    base.get_child(item, 0);
    return item.get();
}

void UdisksDevice::unmount()
{
    std::vector<Glib::VariantBase> args;
    args.emplace_back(Glib::Variant<std::vector<String> >::create({ }));
    auto base = _dev_proxy->call_sync(
        "FilesystemUnmount", Glib::VariantContainerBase::create_tuple(args));
}
