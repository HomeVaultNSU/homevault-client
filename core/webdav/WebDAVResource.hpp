#ifndef WEBDAV_RESOURCE_H
#define WEBDAV_RESOURCE_H

#include <string>

#include <core/FileInfo.hpp>

namespace hv
{

class WebDAVResource
{
public:
    enum class ResourceType
    {
        FILE,
        DIRECTORY,
        UNKNOWN
    };

    WebDAVResource();
    WebDAVResource(const std::string& href, ResourceType type);

    const std::string& getHref() const;
    ResourceType getResourceType() const;
    const std::string& getDisplayName() const;
    const std::string& getContentType() const;
    size_t getContentLength() const;
    const std::string& getLastModified() const;
    const std::string& getETag() const;

    void setDisplayName(const std::string& displayName);
    void setContentType(const std::string& contentType);
    void setContentLength(size_t contentLength);
    void setLastModified(const std::string& lastModified);
    void setETag(const std::string& etag);

    bool isDirectory() const;
    bool isFile() const;

    FileInfo toFileInfo();

private:
    std::string m_href;
    ResourceType m_type;
    std::string m_displayName;
    std::string m_contentType;
    size_t m_contentLength;
    std::string m_lastModified;
    std::string m_etag;
};

}  // namespace hv

#endif  // WEBDAV_RESOURCE_H
