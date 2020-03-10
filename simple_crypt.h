
#ifndef SIMPLE_CRYPT_H
#define SIMPLE_CRYPT_H

#include <string>

std::string
simple_crypt (const std::string & data, const std::string & key);

std::string
simple_decrypt (const std::string & data, const std::string & key);

#endif
