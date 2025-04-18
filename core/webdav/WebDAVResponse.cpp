#include "WebDAVResponse.hpp"

namespace hv
{

WebDAVResponse::WebDAVResponse() : m_statusCode(0)
{
}

WebDAVResponse::WebDAVResponse(
    int statusCode, const std::string& body,
    const std::map<std::string, std::string>& headers)
    : m_statusCode(statusCode), m_body(body), m_headers(headers)
{
}

WebDAVResponse::WebDAVResponse(int statusCode, const std::string& body)
    : m_statusCode(statusCode), m_body(body)
{
}

int WebDAVResponse::getStatusCode() const
{
    return m_statusCode;
}

const std::string& WebDAVResponse::getBody() const
{
    return m_body;
}

const std::map<std::string, std::string>& WebDAVResponse::getHeaders() const
{
    return m_headers;
}

pugi::xml_document WebDAVResponse::getXmlDocument() const
{
    pugi::xml_document doc;
    if (isXml())
    {
        doc.load_string(m_body.c_str());
    }
    return doc;
}

bool WebDAVResponse::isSuccess() const
{
    return m_statusCode >= 200 && m_statusCode < 300;
}

bool WebDAVResponse::isXml() const
{
    // Check for XML content by looking at content type or body content
    auto it = m_headers.find("Content-Type");
    if (it != m_headers.end() &&
        (it->second.find("application/xml") != std::string::npos ||
         it->second.find("text/xml") != std::string::npos))
    {
        return true;
    }

    // Simple check for XML structure in body
    return !m_body.empty() && m_body.find("<?xml") != std::string::npos;
}

}  // namespace hv
