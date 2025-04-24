#include <algorithm>
#include <core/ApiClient.hpp>
#include <core/FilesystemCrawler.hpp>
#include <core/Homevault.hpp>
#include <core/Models.hpp>
#include <core/Result.hpp>
#include <iostream>
#include <memory>

namespace hv
{

Homevault::Homevault(const std::string& hostname)
{
    m_apiClient = std::make_unique<ApiClient>(hostname);
}

Homevault::~Homevault() = default;

ResultValue<DirectoryListing> Homevault::listRemoteFiles(
    const std::string& path, int depth)
{
    try
    {
        FileSystemCrawler filesystemCrawler(*m_apiClient);
        DirectoryListing listing = filesystemCrawler.getDirectoryTreeWithDepth(path, depth);
        return ResultValue(listing);
    }
    catch (HomevaultApiException& e)
    {
        return ResultValue<DirectoryListing>(Status::eUnknownError, e.what());
    }
}

Result Homevault::upload(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

Result Homevault::download(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

};  // namespace hv
