#ifndef HV_CLIENT_H
#define HV_CLIENT_H

#include <string>
#include <vector>

#include "FileInfo.hpp"
#include "Result.hpp"

namespace hv
{

class ServerConnection
{
public:
    ServerConnection(std::string& ip, uint16_t port = 80);
    ~ServerConnection();

    ResultValue<std::vector<FileInfo>> listRemoteFiles();

    Result upload(const std::filesystem::path& local_path,
                  const std::filesystem::path& remote_path);

    Result download(const std::filesystem::path& local_path,
                    const std::filesystem::path& remote_path);

private:
    struct impl;
    std::unique_ptr<impl> pImpl;
};

}  // namespace hv

#endif  // !HV_CLIENT_H
