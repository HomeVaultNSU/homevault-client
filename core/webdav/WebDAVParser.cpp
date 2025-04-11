#include "WebDAVParser.hpp"

#include <map>
#include <sstream>

namespace hv
{

std::vector<WebDAVResource> WebDAVParser::ParsePropfindResponse(
    const pugi::xml_document& doc)
{
    std::vector<WebDAVResource> resources;

    // Get the DAV namespace declaration
    pugi::xml_node multistatus = doc.child("D:multistatus");
    if (!multistatus)
    {
        // Try without namespace prefix
        multistatus = doc.child("multistatus");
    }

    if (!multistatus)
    {
        return resources;
    }

    // Iterate through response elements
    for (pugi::xml_node response = multistatus.child("D:response"); response;
         response = response.next_sibling("D:response"))
    {
        WebDAVResource resource = ParseResourceElement(response);
        if (!resource.getHref().empty())
        {
            resources.push_back(resource);
        }
    }

    // If no responses with namespace were found, try without namespace
    if (resources.empty())
    {
        for (pugi::xml_node response = multistatus.child("response"); response;
             response = response.next_sibling("response"))
        {
            WebDAVResource resource = ParseResourceElement(response);
            if (!resource.getHref().empty())
            {
                resources.push_back(resource);
            }
        }
    }

    return resources;
}

std::map<std::string, std::string> WebDAVParser::ParsePropertyValues(
    const pugi::xml_document& doc)
{
    std::map<std::string, std::string> properties;

    // Try with and without namespace prefix
    pugi::xml_node propstat =
        doc.select_node("//D:propstat[D:status='HTTP/1.1 200 OK']/D:prop")
            .node();
    if (!propstat)
    {
        propstat =
            doc.select_node("//propstat[status='HTTP/1.1 200 OK']/prop").node();
    }

    if (propstat)
    {
        for (pugi::xml_node prop = propstat.first_child(); prop;
             prop = prop.next_sibling())
        {
            std::string name = prop.name();
            std::string value = GetNodeText(prop);

            // Remove namespace prefix if present
            size_t colonPos = name.find(':');
            if (colonPos != std::string::npos)
            {
                name = name.substr(colonPos + 1);
            }

            properties[name] = value;
        }
    }

    return properties;
}

WebDAVResource WebDAVParser::ParseResourceElement(
    const pugi::xml_node& response)
{
    // Extract href (resource path)
    std::string href = ExtractHref(response);
    if (href.empty())
    {
        return WebDAVResource();
    }

    // Find propstat with 200 OK status
    pugi::xml_node okPropstat;
    for (pugi::xml_node propstat = response.child("D:propstat");
         propstat && !okPropstat;
         propstat = propstat.next_sibling("D:propstat"))
    {
        pugi::xml_node status = propstat.child("D:status");
        if (status && std::string(status.text().as_string()).find("200 OK") !=
                          std::string::npos)
        {
            okPropstat = propstat;
        }
    }

    // If not found with namespace, try without
    if (!okPropstat)
    {
        for (pugi::xml_node propstat = response.child("propstat");
             propstat && !okPropstat;
             propstat = propstat.next_sibling("propstat"))
        {
            pugi::xml_node status = propstat.child("status");
            if (status &&
                std::string(status.text().as_string()).find("200 OK") !=
                    std::string::npos)
            {
                okPropstat = propstat;
            }
        }
    }

    if (!okPropstat)
    {
        return WebDAVResource(href, WebDAVResource::ResourceType::UNKNOWN);
    }

    // Determine resource type
    WebDAVResource::ResourceType resourceType =
        WebDAVResource::ResourceType::FILE;
    std::string resourceTypeVal =
        ExtractPropertyValue(okPropstat, "resourcetype");
    if (resourceTypeVal.find("<D:collection") != std::string::npos ||
        resourceTypeVal.find("<collection") != std::string::npos)
    {
        resourceType = WebDAVResource::ResourceType::DIRECTORY;
    }

    WebDAVResource resource(href, resourceType);

    // Extract other properties
    resource.setDisplayName(ExtractPropertyValue(okPropstat, "displayname"));
    resource.setContentType(ExtractPropertyValue(okPropstat, "getcontenttype"));

    // Parse content length
    std::string lengthStr =
        ExtractPropertyValue(okPropstat, "getcontentlength");
    if (!lengthStr.empty())
    {
        try
        {
            resource.setContentLength(std::stoull(lengthStr));
        }
        catch (...)
        {
            // Ignore conversion errors
        }
    }

    resource.setLastModified(
        ExtractPropertyValue(okPropstat, "getlastmodified"));
    resource.setETag(ExtractPropertyValue(okPropstat, "getetag"));

    return resource;
}

std::string WebDAVParser::GetNodeText(const pugi::xml_node& node)
{
    std::stringstream ss;
    for (pugi::xml_node child = node.first_child(); child;
         child = child.next_sibling())
    {
        if (child.type() == pugi::node_pcdata ||
            child.type() == pugi::node_cdata)
        {
            ss << child.value();
        }
        else
        {
            // For nested elements, include their XML representation
            ss << "<" << child.name();

            // Add attributes if any
            for (pugi::xml_attribute attr = child.first_attribute(); attr;
                 attr = attr.next_attribute())
            {
                ss << " " << attr.name() << "=\"" << attr.value() << "\"";
            }

            if (child.first_child())
            {
                ss << ">" << GetNodeText(child) << "</" << child.name() << ">";
            }
            else
            {
                ss << "/>";
            }
        }
    }
    return ss.str();
}

std::string WebDAVParser::ExtractHref(const pugi::xml_node& response)
{
    // Try with namespace first
    pugi::xml_node href = response.child("D:href");
    if (!href)
    {
        // Try without namespace
        href = response.child("href");
    }

    return href ? href.text().get() : "";
}

std::string WebDAVParser::ExtractPropertyValue(const pugi::xml_node& propstat,
                                               const std::string& propName)
{
    // Try with namespace
    pugi::xml_node prop = propstat.child("D:prop");
    if (!prop)
    {
        prop = propstat.child("prop");
    }

    if (!prop)
    {
        return "";
    }

    // Try with namespace
    pugi::xml_node node = prop.child(("D:" + propName).c_str());
    if (!node)
    {
        // apache2 webdav server uses lp1 namespace
        node = prop.child(("lp1:" + propName).c_str());
    }
    if (!node)
    {
        // Try without namespace
        node = prop.child(propName.c_str());
    }

    return node ? GetNodeText(node) : "";
}

}  // namespace hv
