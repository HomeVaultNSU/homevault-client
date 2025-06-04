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
    ApiClient(const std::string& baseUrl, const std::string& username,
              const std::string& password);
    ~ApiClient() = default;

    // file server endpoints
    DirectoryListing getDirectoryListing(const std::string& path = "/",
                                         int depth = 1);
    UploadResponse uploadFile(const std::string& filePath,
                              const std::string& targetDir = "/");
    std::vector<uint8_t> downloadFile(const std::string& path);

    // Authentication endpoints
    void registerUser(const std::string& username, const std::string& password);
    TokenResponse login(const std::string& username,
                        const std::string& password);
    DecodedTokenResponse decodeToken(const std::string& token);

private:
    std::string baseUrl;
    std::string username;
    std::string password;
    std::optional<std::string> authToken;

    std::string getAuthorizationHeader();

    // HTTP methods
    nlohmann::json get(const std::string& endpoint,
                       const std::map<std::string, std::string>& params = {},
                       const std::list<std::string>& headers = {});
    std::vector<uint8_t> getBinary(
        const std::string& endpoint,
        const std::map<std::string, std::string>& params = {},
        const std::list<std::string>& headers = {});
    nlohmann::json post(const std::string& endpoint,
                        const std::map<std::string, std::string>& params = {},
                        const std::list<std::string>& headers = {});
    nlohmann::json postJson(const std::string& endpoint,
                            const nlohmann::json& jsonData,
                            const std::list<std::string>& headers = {});
    nlohmann::json putJson(const std::string& endpoint,
                           const nlohmann::json& jsonData,
                           const std::list<std::string>& headers = {});
    nlohmann::json postMultipart(
        const std::string& endpoint, const std::string& filePath,
        const std::map<std::string, std::string>& params = {},
        const std::list<std::string>& headers = {});

    // Utility methods
    std::string buildUrl(const std::string& endpoint,
                         const std::map<std::string, std::string>& params = {});
    void handleErrorResponse(const std::string& response);
};

}  // namespace hv

#endif  // !APICLIENT_HPP
