#ifndef HV_FILEINFO_H
#define HV_FILEINFO_H

#include <filesystem>

namespace hv
{

struct FileInfo
{
    std::filesystem::path path;
    // other file properties will be placed here
};


}

#endif // !HV_FILEINFO_H
