#ifndef WEBDAV_PARSER_H
#define WEBDAV_PARSER_H

#include <map>
#include <pugixml.hpp>
#include <string>
#include <vector>

#include "WebDAVResource.hpp"

namespace hv
{

class WebDAVParser
{
public:
    static std::vector<WebDAVResource> ParsePropfindResponse(
        const pugi::xml_document& doc);
    static std::map<std::string, std::string> ParsePropertyValues(
        const pugi::xml_document& doc);

private:
    static WebDAVResource ParseResourceElement(const pugi::xml_node& response);
    static std::string GetNodeText(const pugi::xml_node& node);
    static std::string ExtractHref(const pugi::xml_node& response);
    static std::string ExtractPropertyValue(const pugi::xml_node& propstat,
                                            const std::string& propName);
};

}  // namespace hv

#endif  // WEBDAV_PARSER_H
