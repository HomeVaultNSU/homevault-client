#ifndef WEBDAV_CLIENT_H
#define WEBDAV_CLIENT_H

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <map>
#include <string>

#include "WebDAVResponse.hpp"

namespace hv
{

class WebDAVClient
{
public:
    WebDAVClient(const std::string& baseUrl, const std::string& username = "",
                 const std::string& password = "");
    ~WebDAVClient();

    // WebDAV operations
    WebDAVResponse propfind(const std::string& path);
    WebDAVResponse propfind(const std::string& path, const std::string& depth);

    WebDAVResponse proppatch(const std::string& path,
                             const std::map<std::string, std::string>& props);
    WebDAVResponse mkcol(const std::string& path);
    WebDAVResponse get(const std::string& path);
    WebDAVResponse put(const std::string& path, const std::string& content);
    WebDAVResponse copy(const std::string& source,
                        const std::string& destination, bool overwrite = false);
    WebDAVResponse move(const std::string& source,
                        const std::string& destination, bool overwrite = false);
    WebDAVResponse remove(const std::string& path);

private:
    WebDAVResponse sendRequest(
        const std::string& method, const std::string& path,
        const std::string& requestBody = "",
        const std::map<std::string, std::string>& headers = {});
    void setupCurlHandle(curlpp::Easy& request, const std::string& url);
    std::string buildUrl(const std::string& path);

    std::string m_baseUrl;
    std::string m_username;
    std::string m_password;
    bool m_hasCredentials;
};

}  // namespace hv

#endif  // WEBDAV_CLIENT_H
