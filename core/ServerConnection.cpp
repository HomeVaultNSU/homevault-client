#include "core/ServerConnection.hpp"

#include "core/Result.hpp"

namespace hv
{

// ServerConnection class private data (Pimpl idiom)
struct ServerConnection::impl
{
    std::string ip;
};

ResultValue<std::vector<FileInfo>> ServerConnection::listRemoteFiles()
{
    std::vector<FileInfo> fileInfos;

    return fileInfos;
}

Result ServerConnection::upload(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

Result ServerConnection::download(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

};  // namespace hv
