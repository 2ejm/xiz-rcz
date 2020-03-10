#ifndef _LAN_FOLDER_ENTRY_H_
#define _LAN_FOLDER_ENTRY_H_

#include <string>

/**
 * \brief Represents one file entry in a LAN shared folder.
 */
class LanFolderEntry
{
public:
    LanFolderEntry()
    {}

    LanFolderEntry(const std::string& s1, const std::string& s2,
                   const std::string& s3, const std::string& s4,
                   const std::string& s5) :
        file_name{s1}, path{s2}, ext{s3}, pattern{s4}, id{s5}
    {}

    std::string file_name;
    std::string path;
    std::string ext;
    std::string pattern;
    std::string id;
};

#endif /* _LAN_FOLDER_ENTRY_H_ */
