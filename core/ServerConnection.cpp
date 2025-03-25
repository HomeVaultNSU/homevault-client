#include "core/ServerConnection.hpp"
#include "core/Status.hpp"

namespace hv {

// ServerConnection class private data (Pimpl idiom)
struct ServerConnection::impl
{
    std::string ip;
};

std::vector<FileInfo> ServerConnection::listRemoteFiles()
{
    std::vector<FileInfo> fileInfos;

    return fileInfos;
}

Status ServerConnection::upload(const std::filesystem::path& local_path,
                                const std::filesystem::path& remote_path)
{

    return Status::Success;
}

Status ServerConnection::download(const std::filesystem::path& local_path,
                                  const std::filesystem::path& remote_path)
{

    return Status::Success;
}

}; // namespace hv
