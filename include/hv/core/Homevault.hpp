#ifndef HOMEVAULT_HPP
#define HOMEVAULT_HPP

#include <filesystem>
#include <string>

#include "core/Result.hpp"
#include "core/Models.hpp"

namespace hv
{

class ApiClient;

class Homevault
{
public:
    Homevault(const std::string& hostname);
    ~Homevault();
    
    /**
    * @brief List contents of remote directory
    * @param path remote path to list
    * @param depth list depth (depth = -1 - infinite) 
    */
    ResultValue<DirectoryListing> listRemoteFiles(const std::string& path, int depth = -1);

    Result upload(const std::filesystem::path& local_path,
                  const std::filesystem::path& remote_path);

    Result download(const std::filesystem::path& local_path,
                    const std::filesystem::path& remote_path);

private:

    std::unique_ptr<ApiClient> m_apiClient;
};

}  // namespace hv

#endif  // !HOMEVAULT_HPP
