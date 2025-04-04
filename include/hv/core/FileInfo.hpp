#ifndef HV_FILEINFO_H
#define HV_FILEINFO_H

#include <string>
#include <vector>

namespace hv
{

class FileInfo
{
public:
    static FileInfo RegularFile(const std::string& name,
                                const std::string& lastModified = "");
    static FileInfo Directory(const std::string& name,
                              const std::string& lastModified = "");

    FileInfo() = default;
    FileInfo(const std::string& name, bool isDirectory = false,
             const std::string& lastModified = "");

    const std::string& getName() const;
    const std::string& getLastModified() const;

    bool isDirectory() const;
    bool isRegularFile() const;

    const std::vector<FileInfo> getChildren() const;

    void addChild(const FileInfo& fileInfo);

    std::string toTreeString() const;

private:
    void toTreeStringImpl(std::stringstream& ss, const std::string& prefix,
                      bool isLast) const;

    std::string m_name;
    std::string m_lastModified;
    bool m_isDirectory;
    std::vector<FileInfo> m_children;
};

}  // namespace hv

#endif  // !HV_FILEINFO_H
