#include "core/HomeVaultClient.hpp"
#include <memory>

#include "core/Result.hpp"
#include "webdav/WebDAVClient.hpp"
#include "webdav/WebDAVParser.hpp"

namespace hv
{

HomeVaultClient::HomeVaultClient(const std::string& hostname,
                                 const std::string& username,
                                 const std::string& password)
{
    m_webdavClient = std::make_unique<WebDAVClient>(hostname, username, password);
}

HomeVaultClient::~HomeVaultClient() = default;

ResultValue<FileInfo> HomeVaultClient::listRemoteFiles(const std::string& path)
{
    WebDAVResponse response = m_webdavClient->propfind(path);

    if (!response.isSuccess() || !response.isXml())
    {
        return ResultValue<FileInfo>(Status::eUnknownError);
    }

    FileInfo rootFileInfo = FileInfo::Directory("/");

    pugi::xml_document doc = response.getXmlDocument();
    for (WebDAVResource& resource : WebDAVParser::ParsePropfindResponse(doc))
    {
        if (resource.isFile())
        {
            rootFileInfo.addChild(FileInfo::RegularFile(
                resource.getHref(), resource.getLastModified()));
        }
        if (resource.isDirectory())
        {
            // TODO: issue another request to fill directory
            rootFileInfo.addChild(FileInfo::Directory(
                resource.getHref(), resource.getLastModified()));
        }
    }

    return ResultValue(rootFileInfo);
}

Result HomeVaultClient::upload(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

Result HomeVaultClient::download(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

};  // namespace hv
