
#include "simple_crypt.h"
#include "file_handler.h"

std::string
simple_crypt (const std::string & data, const std::string & key)
{
    std::string ret;

    if (key.empty ()) {
	/* empty key, dont crypt, not even base64
	 */
	return data;
    }

    auto key_iter = key.begin ();

    for (auto c : data) {
	ret.push_back (c ^ (*key_iter));

	key_iter++;

	/* when key_iter at end, restsrt
	 */
	if (key_iter == key.end ())
	    key_iter = key.begin ();
    }

    return FileHandler::base64_encode (ret);
}

std::string
simple_decrypt (const std::string & data, const std::string & key)
{
    if (key.empty ()) {
	/* empty key, dont crypt, not even base64
	 */
	return data;
    }

    std::string unbased = FileHandler::base64_decode (data);
    std::string ret;

    auto key_iter = key.begin ();

    for (auto c : unbased) {
	ret.push_back (c ^ (*key_iter));

	key_iter++;

	/* when key_iter at end, restsrt
	 */
	if (key_iter == key.end ())
	    key_iter = key.begin ();
    }

    return ret;
}
