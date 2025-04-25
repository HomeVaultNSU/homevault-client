#ifndef APICLIENT_HPP
#define APICLIENT_HPP

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Models.hpp"

namespace hv
{

class ApiClient
{
public:
    ApiClient(const std::string& baseUrl = "http://localhost:8080");
    ~ApiClient() = default;

    // API endpoints
    DirectoryListing getDirectoryListing(const std::string& path = "/",
                                         int depth = 1);
    UploadResponse uploadFile(const std::string& filePath,
                              const std::string& targetDir = "/");
    std::vector<uint8_t> downloadFile(const std::string& path);

private:
    std::string baseUrl;

    // HTTP methods
    nlohmann::json get(const std::string& endpoint,
                       const std::map<std::string, std::string>& params = {});
    std::vector<uint8_t> getBinary(
        const std::string& endpoint,
        const std::map<std::string, std::string>& params = {});
    nlohmann::json post(const std::string& endpoint,
                        const std::map<std::string, std::string>& params = {});
    nlohmann::json postMultipart(
        const std::string& endpoint, const std::string& filePath,
        const std::map<std::string, std::string>& params = {});

    // Utility methods
    std::string buildUrl(const std::string& endpoint,
                         const std::map<std::string, std::string>& params = {});
    void handleErrorResponse(const std::string& response);
};

}  // namespace hv

#endif  // !APICLIENT_HPP
