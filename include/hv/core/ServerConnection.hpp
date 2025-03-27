#ifndef HV_CLIENT_H
#define HV_CLIENT_H

#include <string>
#include <vector>

#include "FileInfo.hpp"
#include "Status.hpp"

namespace hv {

class ServerConnection {

public:
  ServerConnection(const std::string &ip);

  std::vector<FileInfo> listRemoteFiles();
  Status upload(const std::filesystem::path &local_path,
                const std::filesystem::path &remote_path);

  Status download(const std::filesystem::path &local_path,
                  const std::filesystem::path &remote_path);

private:
  struct impl;
  std::unique_ptr<impl> pImpl;
};

} // namespace hv

#endif // !HV_CLIENT_H
