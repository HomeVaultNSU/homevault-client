#include "../include/hv/core/ServerConnection.hpp"
#include "../include/hv/core/Status.hpp"

namespace hv {

// ServerConnection class private data (Pimpl idiom)
struct ServerConnection::impl {
  std::string ip;
};

std::vector<FileInfo> ServerConnection::listRemoteFiles() {
  std::vector<FileInfo> fileInfos;

  return fileInfos;
}

Status ServerConnection::upload(const std::filesystem::path &local_path,
                                const std::filesystem::path &remote_path) {
  // placeholders to shut up strict compiler
  local_path.filename();
  remote_path.filename();

  return Status::Success;
}

Status ServerConnection::download(const std::filesystem::path &local_path,
                                  const std::filesystem::path &remote_path) {
  // placeholders to shut up strict compiler
  local_path.filename();
  remote_path.filename();

  return Status::Success;
}

}; // namespace hv
