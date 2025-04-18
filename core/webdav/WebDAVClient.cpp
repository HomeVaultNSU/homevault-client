#include "WebDAVClient.hpp"

#include <curlpp/Infos.hpp>
#include <curlpp/cURLpp.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

#include "webdav/WebDAVResponse.hpp"

namespace hv
{

WebDAVResponse WebDAVClient::GetDirectoryListing(const std::string& path,
                                                 int depth)
{
    try
    {
        curlpp::Easy request;

        // Build query URL correctly without leading slash since buildUrl adds
        // it
        std::string encodedPath =
            (path == "/" || path.empty()) ? "%2F" : curlpp::escape(path);
        std::string queryUrl =
            "list?path=" + encodedPath + "&depth=" + std::to_string(depth);

        // Debug output
        std::cerr << "Query URL before buildUrl: " << queryUrl << std::endl;

        // Setup request
        setupCurlHandle(request, buildUrl(queryUrl));

        // Add required header for JSON response
        std::list<std::string> headers;
        headers.push_back("accept: application/json");
        request.setOpt(new curlpp::options::HttpHeader(headers));

        // Set HTTP method explicitly
        request.setOpt(new curlpp::options::CustomRequest("GET"));

        // Debug output for full URL
        std::cerr << "Making GET request to: " << buildUrl(queryUrl)
                  << std::endl;

        // Capture response
        std::stringstream responseStream;
        request.setOpt(new curlpp::options::WriteStream(&responseStream));

        request.perform();

        long statusCode = curlpp::infos::ResponseCode::get(request);
        std::string response = responseStream.str();

        return WebDAVResponse(statusCode, response);
    }
    catch (const std::exception& e)
    {
        return WebDAVResponse(500, std::string("Error: ") + e.what());
    }
}

WebDAVResponse WebDAVClient::UploadFile(const std::string& localPath,
                                        const std::string& remotePath)
{
    try
    {
        curlpp::Easy request;
        setupCurlHandle(request, buildUrl("/upload"));

        // Setup form data
        curlpp::Forms formParts;
        formParts.push_back(new curlpp::FormParts::File("file", localPath));
        formParts.push_back(new curlpp::FormParts::Content("path", remotePath));

        request.setOpt(new curlpp::options::HttpPost(formParts));

        // Capture response
        std::stringstream responseStream;
        request.setOpt(new curlpp::options::WriteStream(&responseStream));

        request.perform();

        long statusCode = curlpp::infos::ResponseCode::get(request);
        return WebDAVResponse(statusCode, responseStream.str());
    }
    catch (const std::exception& e)
    {
        return WebDAVResponse(500, std::string("Error: ") + e.what());
    }
}

WebDAVResponse WebDAVClient::DownloadFile(const std::string& remotePath,
                                          const std::string& localPath)
{
    try
    {
        curlpp::Easy request;
        setupCurlHandle(request, buildUrl("/download?path=" + remotePath));

        // Open file for writing
        std::ofstream ofs(localPath, std::ios::binary);
        if (!ofs)
        {
            return WebDAVResponse(500, "Cannot open local file for writing");
        }

        // Write response directly to file
        request.setOpt(new curlpp::options::WriteStream(&ofs));

        request.perform();

        long statusCode = curlpp::infos::ResponseCode::get(request);
        return WebDAVResponse(statusCode, "");
    }
    catch (const std::exception& e)
    {
        return WebDAVResponse(500, std::string("Error: ") + e.what());
    }
}

WebDAVClient::WebDAVClient(const std::string& baseUrl,
                           const std::string& username,
                           const std::string& password)
    : m_baseUrl(baseUrl), m_username(username), m_password(password)
{
    // Validate base URL
    if (m_baseUrl.empty())
    {
        throw std::runtime_error("Base URL cannot be empty");
    }

    // Debug the input URL
    std::cerr << "Input URL: " << m_baseUrl << std::endl;

    // Ensure base URL has protocol
    if (m_baseUrl.find("http://") != 0 && m_baseUrl.find("https://") != 0)
    {
        m_baseUrl = "http://" + m_baseUrl;
    }

    // Remove trailing slash from base URL
    if (!m_baseUrl.empty() && m_baseUrl.back() == '/')
    {
        m_baseUrl.pop_back();
    }

    // Debug the final URL
    std::cerr << "Final URL: " << m_baseUrl << std::endl;

    m_hasCredentials = !m_username.empty() && !m_password.empty();
    curlpp::initialize();
}

WebDAVClient::~WebDAVClient()
{
    curlpp::terminate();
}

WebDAVResponse WebDAVClient::propfind(const std::string& path)
{
    return propfind(path, "0");
}

WebDAVResponse WebDAVClient::propfind(const std::string& path,
                                      const std::string& depth)
{
    std::map<std::string, std::string> headers;
    headers["Depth"] = depth;

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

    // Don't add any slashes - the path should already be formatted correctly
    result += path;

    // Debug output
    std::cerr << "Built URL: " << result << std::endl;

    return result;
}

}  // namespace hv
