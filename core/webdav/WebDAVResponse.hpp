#ifndef WEBDAV_RESPONSE_H
#define WEBDAV_RESPONSE_H

#include <map>
#include <pugixml.hpp>
#include <string>

namespace hv
{

class WebDAVResponse
{
public:
    WebDAVResponse();
    WebDAVResponse(int statusCode, const std::string& body,
                   const std::map<std::string, std::string>& headers);
    WebDAVResponse(int statusCode, const std::string& body);

    int getStatusCode() const;
    const std::string& getBody() const;
    const std::map<std::string, std::string>& getHeaders() const;

    pugi::xml_document getXmlDocument() const;
    bool isSuccess() const;
    bool isXml() const;

private:
    int m_statusCode;
    std::string m_body;
    std::map<std::string, std::string> m_headers;
};

}  // namespace hv

#endif  // WEBDAV_RESPONSE_H
