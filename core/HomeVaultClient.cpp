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
    m_webdavClient =
        std::make_unique<WebDAVClient>(hostname, username, password);
}

HomeVaultClient::~HomeVaultClient() = default;


ResultValue<FileInfo> HomeVaultClient::listRemoteFiles(const std::string& path)
{
    WebDAVResponse response = m_webdavClient->propfind(path, "1");

    if (!response.isSuccess())
    {
        std::cerr << "WebDAV request failed with status: "
                  << response.getStatusCode() << std::endl;
        std::cerr << "Response content: " << response.getBody() << std::endl;
        return ResultValue<FileInfo>(Status::eUnknownError);
    }

    if (!response.isXml())
    {
        std::cerr << "Response is not XML: " << response.getBody() << std::endl;
        return ResultValue<FileInfo>(Status::eUnknownError);
    }

    FileInfo rootFileInfo = FileInfo::Directory(path);

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
            std::string dirPath = resource.getHref();
            // Skip the current directory to avoid infinite recursion
            if (dirPath != path)
            {
                // Recursively get contents of subdirectory
                auto subDirResult = listRemoteFiles(dirPath);
                if (subDirResult.status() == Status::eSuccess)
                {
                    rootFileInfo.addChild(subDirResult.value());
                }
                else
                {
                    // If we can't access subdirectory, add it as empty
                    rootFileInfo.addChild(FileInfo::Directory(
                        dirPath, resource.getLastModified()));
                }
            }
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
