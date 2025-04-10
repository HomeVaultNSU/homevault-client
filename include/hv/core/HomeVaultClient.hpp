#ifndef HV_CLIENT_H
#define HV_CLIENT_H

#include <filesystem>
#include <string>

#include "FileInfo.hpp"
#include "Result.hpp"

namespace hv
{

class WebDAVClient;

class HomeVaultClient
{
public:
    HomeVaultClient(const std::string& hostname,
                    const std::string& username = "",
                    const std::string& password = "");

    ~HomeVaultClient();

    ResultValue<FileInfo> listRemoteFiles(const std::string& path);

    Result upload(const std::filesystem::path& local_path,
                  const std::filesystem::path& remote_path);

    Result download(const std::filesystem::path& local_path,
                    const std::filesystem::path& remote_path);

private:
    std::unique_ptr<WebDAVClient> m_webdavClient;
};

}  // namespace hv

#endif  // !HV_CLIENT_H
