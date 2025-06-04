#ifndef HOMEVAULT_HPP
#define HOMEVAULT_HPP

#include <filesystem>
#include <string>

#include "core/Models.hpp"
#include "core/Result.hpp"

namespace hv
{

class ApiClient;

class Homevault
{
public:
    explicit Homevault(const std::string& fileServerHostname,
                       const std::string& authServerHostname,
                       const std::string& username,
                       const std::string& password);
    ~Homevault();

    /**
     * @brief List contents of remote directory
     * @param path remote path to list
     * @param depth list depth (depth = -1 - infinite)
     */
    ResultValue<DirectoryListing> listRemoteFiles(const std::string& path = "/",
                                                  int depth = -1);

    Result upload(const std::filesystem::path& local_path,
                  const std::filesystem::path& remote_dir_path = "/");

    Result download(const std::filesystem::path& local_path,
                    const std::filesystem::path& remote_path);

    Result registerUser(const std::string& username,
                        const std::string& password);

private:
    std::string normalizePath(const std::string& path);

    std::unique_ptr<ApiClient> m_fileServerClient;
    std::unique_ptr<ApiClient> m_authServerClient;
};

}  // namespace hv

#endif  // !HOMEVAULT_HPP
