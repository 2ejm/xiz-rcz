#ifndef _UTILS_H_
#define _UTILS_H_

#include <iostream>
#include <stdexcept>
#include <sstream>

#include "xml_result_ok.h"
#include "xml_result_bad_request.h"
#include "xml_result_forbidden.h"
#include "xml_result_not_found.h"
#include "xml_result_too_many_requests.h"
#include "xml_result_internal_device_error.h"

#include "log.h"

#define UNUSED(x)                               \
    do {                                        \
        (void)(x);                              \
    } while (0)

#ifdef NO_LIBZIX_LOG
#define PRINT_ERROR(msg)                                        \
    do {                                                        \
        std::cerr << "[ERROR " << __FILE__ << ":" << __LINE__   \
                  << "]: " << msg << std::endl;                 \
    } while (0)

#define PRINT_WARNING(msg)                                      \
    do {                                                        \
        std::cerr << "[WARNING " << __FILE__ << ":" << __LINE__ \
                  << "]: " << msg << std::endl;                 \
    } while (0)

#define PRINT_INFO(msg)                                          \
    do {                                                         \
        std::cout << "[INFO " << __FILE__ << ":" << __LINE__     \
                  << "]: " << msg << std::endl;                  \
    } while (0)

#ifndef NDEBUG
#define PRINT_DEBUG(msg)                                        \
    do {                                                        \
        std::cout << "[DEBUG " << __FILE__ << ":" << __LINE__   \
                  << "]: " << msg << std::endl;                 \
    } while (0)
#else
#define PRINT_DEBUG(msg)
#endif
#else
#define PRINT_ERROR(msg) \
   do { \
   std::stringstream strstr; \
   strstr << msg; \
   logger("Error", strstr.str()); \
   } while (0)
#define PRINT_DEBUG(msg) \
   do { \
   std::stringstream strstr; \
   strstr << msg; \
   logger("Debug", strstr.str()); \
   } while (0)
#define PRINT_INFO(msg) \
   do { \
   std::stringstream strstr; \
   strstr << msg; \
   logger("Info", strstr.str()); \
   } while (0)
#define PRINT_WARNING(msg) \
   do { \
   std::stringstream strstr; \
   strstr << msg; \
   logger("Warning", strstr.str()); \
   } while (0)

#endif

#define EXCEPTION_TYPE(type, msg)                       \
    do {                                                \
        std::stringstream ss;                           \
        ss << msg;                                      \
        PRINT_ERROR(ss.str());                          \
        throw std::type(ss.str());                      \
    } while (0)

#define EXCEPTION(msg)                          \
    EXCEPTION_TYPE(logic_error, msg)

#define SEGFAULT {int (*functionPtr)();functionPtr=NULL;functionPtr();}


#define XML_RESULT(type, msg)                                       \
    do {                                                            \
        std::stringstream ss;                                       \
        ss << msg;                                                  \
        auto xml_result = type::create(ss.str());                   \
        call_finished(xml_result);                                  \
    } while (0)

#define XML_RESULT_OK(msg)                      \
    XML_RESULT(XmlResultOk, msg)

#define XML_RESULT_OK_RET(msg, ret)                                 \
    do {                                                            \
        std::stringstream ss;                                       \
        ss << msg;                                                  \
        auto xml_result = XmlResultOk::create(ss.str(), ret);       \
        call_finished(xml_result);                                  \
    } while (0)

#define XML_RESULT_BAD_REQUEST(msg)             \
    XML_RESULT(XmlResultBadRequest, msg)

#define XML_RESULT_NOT_FOUND(msg)               \
    XML_RESULT(XmlResultNotFound, msg)

#define XML_RESULT_FORBIDDEN(msg)               \
    XML_RESULT(XmlResultForbidden, msg)

#define XML_RESULT_INTERNAL_DEVICE_ERROR(msg)   \
    XML_RESULT(XmlResultInternalDeviceError, msg)

#define XML_RESULT_TOO_MANY_REQUEST(msg)        \
    XML_RESULT(XmlResultTooManyRequests, msg)


#endif /* _UTILS_H_ */
