#ifndef _POST_PROCESSING_ENTRY_H_
#define _POST_PROCESSING_ENTRY_H_

#include <string>

class PostProcessingEntry
{
public:
    PostProcessingEntry()
    {}

    std::string generated_file; // complete path
    std::string post_processor;
    std::string format;
    std::string output_file;    // complete path
    std::string iid;
    std::string id;
};

#endif /* _POST_PROCESSING_ENTRY_H_ */
