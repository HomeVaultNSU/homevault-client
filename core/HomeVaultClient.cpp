#include "core/HomeVaultClient.hpp"

#include <algorithm>
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

ResultValue<FileInfo> HomeVaultClient::listRemoteFiles(const std::string& path,
                                                       int depth)
{
    WebDAVResponse response =
        m_webdavClient->propfind(path, depth == 0 ? "0" : "1");


    if (!response.isSuccess())
    {
        std::stringstream msg;
        msg << "Status " << response.getStatusCode() << ", "
            << response.getBody();
        return ResultValue<FileInfo>(Status::eUnknownError, msg.str());
    }

    if (!response.isXml())
    {
        std::stringstream msg;
        msg << "Response is not XML";
        return ResultValue<FileInfo>(Status::eUnknownError, msg.str());
    }

    pugi::xml_document doc = response.getXmlDocument();
    auto resources = WebDAVParser::ParsePropfindResponse(doc);

    WebDAVResource& rootResource =
        *std::find_if(resources.begin(), resources.end(),
                      [&](WebDAVResource& r) { return r.getHref() == path; });

    FileInfo rootFileInfo = rootResource.toFileInfo();

    if (depth == 0)
    {
        // return only the root file on depth = 0
        return ResultValue(rootFileInfo);
    }

    for (WebDAVResource& resource : WebDAVParser::ParsePropfindResponse(doc))
    {
        if (resource.getHref() == path)
        {
            continue;
        }

        if (resource.isFile())
        {
            rootFileInfo.addChild(resource.toFileInfo());
        }

        if (resource.isDirectory())
        {
            std::string dirPath = resource.getHref();

            // Recursively get contents of subdirectory
            ResultValue<FileInfo> subDirResult =
                listRemoteFiles(dirPath, depth == -1 ? -1 : depth - 1);

            if (subDirResult.status() == Status::eSuccess)
            {
                rootFileInfo.addChild(subDirResult.value());
            }
            else
            {
                // If we can't access subdirectory, propagate the error
                return subDirResult;
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
