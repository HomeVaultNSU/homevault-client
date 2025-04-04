#include "WebDAVClient.hpp"

#include <algorithm>
#include <curlpp/Infos.hpp>
#include <curlpp/cURLpp.hpp>
#include <sstream>

#include "WebDAVParser.hpp"

namespace hv
{

WebDAVClient::WebDAVClient(const std::string& baseUrl,
                           const std::string& username,
                           const std::string& password)
    : m_baseUrl(baseUrl), m_username(username), m_password(password)
{
    // Remove trailing slash from base URL if present
    if (!m_baseUrl.empty() && m_baseUrl.back() == '/')
    {
        m_baseUrl.pop_back();
    }

    m_hasCredentials = !m_username.empty() && !m_password.empty();

    curlpp::initialize();
}

WebDAVClient::~WebDAVClient()
{
    curlpp::terminate();
}

WebDAVResponse WebDAVClient::propfind(const std::string& path, int depth)
{
    std::map<std::string, std::string> headers;

    headers["Depth"] = std::to_string(depth);

    std::string requestBody =
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
        "<D:propfind xmlns:D=\"DAV:\">"
        "  <D:allprop/>"
        "</D:propfind>";

    return sendRequest("PROPFIND", path, requestBody, headers);
}

WebDAVResponse WebDAVClient::proppatch(
    const std::string& path, const std::map<std::string, std::string>& props)
{
    std::stringstream requestBody;
    requestBody << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
                << "<D:propertyupdate xmlns:D=\"DAV:\">"
                << "  <D:set>";

    for (const auto& prop : props)
    {
        requestBody << "    <D:prop>"
                    << "      <D:" << prop.first << ">" << prop.second
                    << "</D:" << prop.first << ">"
                    << "    </D:prop>";
    }

    requestBody << "  </D:set>"
                << "</D:propertyupdate>";

    return sendRequest("PROPPATCH", path, requestBody.str());
}

WebDAVResponse WebDAVClient::mkcol(const std::string& path)
{
    return sendRequest("MKCOL", path);
}

WebDAVResponse WebDAVClient::get(const std::string& path)
{
    return sendRequest("GET", path);
}

WebDAVResponse WebDAVClient::put(const std::string& path,
                                 const std::string& content)
{
    return sendRequest("PUT", path, content);
}

WebDAVResponse WebDAVClient::copy(const std::string& source,
                                  const std::string& destination,
                                  bool overwrite)
{
    std::map<std::string, std::string> headers;
    headers["Destination"] = buildUrl(destination);
    headers["Overwrite"] = overwrite ? "T" : "F";

    return sendRequest("COPY", source, "", headers);
}

WebDAVResponse WebDAVClient::move(const std::string& source,
                                  const std::string& destination,
                                  bool overwrite)
{
    std::map<std::string, std::string> headers;
    headers["Destination"] = buildUrl(destination);
    headers["Overwrite"] = overwrite ? "T" : "F";

    return sendRequest("MOVE", source, "", headers);
}

WebDAVResponse WebDAVClient::remove(const std::string& path)
{
    return sendRequest("DELETE", path);
}

WebDAVResponse WebDAVClient::sendRequest(
    const std::string& method, const std::string& path,
    const std::string& requestBody,
    const std::map<std::string, std::string>& headers)
{
    try
    {
        std::string url = buildUrl(path);
        curlpp::Easy request;
        setupCurlHandle(request, url);

        request.setOpt(new curlpp::options::CustomRequest(method));

        std::list<std::string> headerList;
        for (const auto& header : headers)
        {
            headerList.push_back(header.first + ": " + header.second);
        }

        // Add content type for XML requests
        if (!requestBody.empty() &&
            requestBody.find("<?xml") != std::string::npos)
        {
            headerList.push_back(
                "Content-Type: application/xml; charset=utf-8");
        }

        if (!headerList.empty())
        {
            request.setOpt(new curlpp::options::HttpHeader(headerList));
        }

        if (!requestBody.empty())
        {
            request.setOpt(new curlpp::options::PostFields(requestBody));
            request.setOpt(
                new curlpp::options::PostFieldSize(requestBody.length()));
        }

        // Capture response body
        std::stringstream responseStream;
        request.setOpt(new curlpp::options::WriteStream(&responseStream));

        // Capture response headers
        std::map<std::string, std::string> responseHeaders;
        request.setOpt(new curlpp::options::HeaderFunction(
            [&responseHeaders](char* buffer, size_t size,
                               size_t nitems) -> size_t
            {
                std::string header(buffer, size * nitems);
                size_t pos = header.find(':');
                if (pos != std::string::npos)
                {
                    std::string key = header.substr(0, pos);
                    std::string value = header.substr(pos + 2);  // Skip ": "

                    // Remove trailing \r\n
                    if (!value.empty() && value.back() == '\n')
                    {
                        value.pop_back();
                    }
                    if (!value.empty() && value.back() == '\r')
                    {
                        value.pop_back();
                    }

                    responseHeaders[key] = value;
                }
                return size * nitems;
            }));

        request.perform();

        long statusCode = curlpp::infos::ResponseCode::get(request);
        std::string responseBody = responseStream.str();

        return WebDAVResponse(statusCode, responseBody, responseHeaders);
    }
    catch (curlpp::LogicError& e)
    {
        return WebDAVResponse(0, std::string("Logic error: ") + e.what(), {});
    }
    catch (curlpp::RuntimeError& e)
    {
        return WebDAVResponse(0, std::string("Runtime error: ") + e.what(), {});
    }
    catch (std::exception& e)
    {
        return WebDAVResponse(0, std::string("Error: ") + e.what(), {});
    }
}

void WebDAVClient::setupCurlHandle(curlpp::Easy& request,
                                   const std::string& url)
{
    request.setOpt(new curlpp::options::Url(url));
    request.setOpt(new curlpp::options::FollowLocation(true));
    request.setOpt(new curlpp::options::SslVerifyPeer(true));

    if (m_hasCredentials)
    {
        request.setOpt(
            new curlpp::options::UserPwd(m_username + ":" + m_password));
    }
}

std::string WebDAVClient::buildUrl(const std::string& path)
{
    std::string result = m_baseUrl;

    if (!path.empty())
    {
        if (path[0] != '/')
        {
            result += '/';
        }
        result += path;
    }

    return result;
}

}  // namespace hv
