#ifndef _COPY_QUEUE_ENTRY_H_
#define _COPY_QUEUE_ENTRY_H_

#include <string>

class CopyQueueEntry
{
public:
    CopyQueueEntry()
    {}

    CopyQueueEntry(const std::string& s1, const std::string& s2) :
        src{s1}, dest{s2}
    {}

    std::string src;
    std::string dest;
};


#endif /* _COPY_QUEUE_ENTRY_H_ */
