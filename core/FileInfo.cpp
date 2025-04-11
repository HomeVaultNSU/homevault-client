#include "core/FileInfo.hpp"

#include <sstream>

namespace hv
{

FileInfo FileInfo::RegularFile(const std::string& name,
                               const std::string& lastModified)
{
    return FileInfo(name, false, lastModified);
}

FileInfo FileInfo::Directory(const std::string& name,
                             const std::string& lastModified)
{
    return FileInfo(name, true, lastModified);
}

FileInfo::FileInfo(const std::string& name, bool isDirectory,
                   const std::string& lastModified)
    : m_name(name), m_lastModified(lastModified), m_isDirectory(isDirectory)
{
}

const std::string& FileInfo::getName() const
{
    return m_name;
}

const std::string& FileInfo::getLastModified() const
{
    return m_lastModified;
}

bool FileInfo::isDirectory() const
{
    return m_isDirectory;
}

bool FileInfo::isRegularFile() const
{
    return !m_isDirectory;
}

const std::vector<FileInfo> FileInfo::getChildren() const
{
    return m_children;
}

void FileInfo::addChild(const FileInfo& fileInfo)
{
    m_children.push_back(fileInfo);
}

std::string FileInfo::toTreeString() const
{
    std::stringstream ss;

    toTreeStringImpl(ss, "", false);

    return ss.str();
}

void FileInfo::toTreeStringImpl(std::stringstream& ss, const std::string& prefix,
                            bool isLast) const
{
    ss << prefix;
    ss << (isLast ? "└── " : "├── ");

    ss << (isDirectory() ? "[D] " : "[F] ") << m_name;

    // disable for now cause it looks ugly
    // if (!m_lastModified.empty())
    // {
    //     ss << " (" << m_lastModified << ")";
    // }

    ss << "\n";

    for (size_t i = 0; i < m_children.size(); ++i)
    {
        const auto& child = m_children[i];
        bool last = (i == m_children.size() - 1);
        child.toTreeStringImpl(ss, prefix + (isLast ? "    " : "│   "), last);
    }
}

}  // namespace hv
