#include <filesystem>
#include <stdexcept>
#include <webdav/WebDAVResource.hpp>

namespace hv
{

WebDAVResource::WebDAVResource()
    : m_type(ResourceType::UNKNOWN), m_contentLength(0)
{
}

WebDAVResource::WebDAVResource(const std::string& href, ResourceType type)
    : m_href(href), m_type(type), m_contentLength(0)
{
}

const std::string& WebDAVResource::getHref() const
{
    return m_href;
}

WebDAVResource::ResourceType WebDAVResource::getResourceType() const
{
    return m_type;
}

const std::string& WebDAVResource::getDisplayName() const
{
    return m_displayName;
}

const std::string& WebDAVResource::getContentType() const
{
    return m_contentType;
}

size_t WebDAVResource::getContentLength() const
{
    return m_contentLength;
}

const std::string& WebDAVResource::getLastModified() const
{
    return m_lastModified;
}

const std::string& WebDAVResource::getETag() const
{
    return m_etag;
}

void WebDAVResource::setDisplayName(const std::string& displayName)
{
    m_displayName = displayName;
}

void WebDAVResource::setContentType(const std::string& contentType)
{
    m_contentType = contentType;
}

void WebDAVResource::setContentLength(size_t contentLength)
{
    m_contentLength = contentLength;
}

void WebDAVResource::setLastModified(const std::string& lastModified)
{
    m_lastModified = lastModified;
}

void WebDAVResource::setETag(const std::string& etag)
{
    m_etag = etag;
}

bool WebDAVResource::isDirectory() const
{
    return m_type == ResourceType::DIRECTORY;
}

bool WebDAVResource::isFile() const
{
    return m_type == ResourceType::FILE;
}

FileInfo WebDAVResource::toFileInfo()
{
    std::filesystem::path path = getHref();

    switch (m_type)
    {
        case ResourceType::FILE:
            return FileInfo::RegularFile(path.filename(), getLastModified());
            break;

        case ResourceType::DIRECTORY:
            return FileInfo::Directory(
                path.parent_path().filename().string() + "/",
                getLastModified());
            break;

        default:
            throw std::runtime_error("Unknown WebDAV Resource Type");
    }
}

}  // namespace hv
