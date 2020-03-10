#include <glibmm/init.h>
#include <giomm/init.h>

#include <iostream>
#include <cstdlib>

#include "id_mapper.h"
#include "utils.h"

void test_mapper()
{
    auto id_mapper = IdMapper::get_instance();

    // add 3 mappings
    id_mapper->add_mapping({ "id1", "00001", "2016-07-13", "14:47:02", "UTC+01:00" });
    id_mapper->add_mapping({ "id2", "00002", "2016-07-13", "14:47:02", "UTC+01:00" });
    id_mapper->add_mapping({ "GBh 93 asdf", "00003", "2016-07-13", "14:47:02", "UTC+01:00" });

    // update second
    id_mapper->add_mapping({ "Gbh asd", "00002", "2016-07-13", "14:47:02", "UTC+01:00" });

    // get it
    auto iid = id_mapper->get_iid("Gbh asd");
    if (iid != "00002")
        PRINT_ERROR("IID Check failed");

    // remove it
    id_mapper->remove_mapping("00002");

    // try lookup
    bool thrown = false;
    try {
        iid = id_mapper->get_iid("Gbh asd");
    } catch (const std::exception& ex) {
        thrown = true;
    }
    if (!thrown)
        PRINT_ERROR("Lookup should fail, as mapping has been removed");
}

int main(void)
{
    Glib::init();
    Gio::init();

    test_mapper();

    return EXIT_SUCCESS;
}
