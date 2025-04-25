#include "core/Models.hpp"

#include <iostream>

namespace hv
{

// FileItem implementation
nlohmann::json FileItem::toJson() const
{
    nlohmann::json json;
    json["name"] = name;
    json["type"] = static_cast<char>(type);
    if (size)
    {
        json["size"] = *size;
    }
    else
    {
        json["size"] = nullptr;
    }
    json["modified_at"] = modifiedAt;
    return json;
}

FileItem FileItem::fromJson(const nlohmann::json& json)
{
    // std::cerr << json.dump(); // Debug output from original

    FileItem item;
    item.name = json.value("name", "");  // Use .value for safety
    std::string typeStr = json.value("type", "");
    item.type = (typeStr == "F") ? FileType::FILE : FileType::DIRECTORY;

    if (json.contains("size") &&
        !json["size"].is_null())  // Check existence and null
    {
        item.size = json["size"].get<size_t>();  //
    }

    // Original used "lastModifiedAt", OpenAPI uses "modified_at" - Assuming API
    // sends "modified_at"
    // Use "modified_at" as per OpenAPI spec
    item.modifiedAt = json.value("modified_at", "");
    // Original code used "lastModifiedAt": item.modifiedAt =
    // json["lastModifiedAt"].get<std::string>();
    return item;
}

// DirectoryListing implementation
nlohmann::json DirectoryListing::toJson() const
{
    nlohmann::json json;
    json["path"] = path;

    nlohmann::json items_array = nlohmann::json::array();
    for (const auto& item : items)
    {
        items_array.push_back(item.toJson());
    }
    json["items"] = items_array;

    nlohmann::json subdirs_array = nlohmann::json::array();
    for (const auto& subdir : subdirectories)
    {
        subdirs_array.push_back(subdir.toJson());
    }
    json["subdirectories"] = subdirs_array;

    return json;
}

DirectoryListing DirectoryListing::fromJson(const nlohmann::json& json)
{
    DirectoryListing listing;
    listing.path = json.value("path", "");

    if (json.contains("items") && json["items"].is_array())
    {
        for (const auto& item_json : json["items"])
        {
            listing.items.push_back(FileItem::fromJson(item_json));
        }
    }

    // Recursively parse subdirectories if present
    if (json.contains("subdirectories") && json["subdirectories"].is_array())
    {
        for (const auto& subdir_json : json["subdirectories"])
        {
            listing.subdirectories.push_back(
                DirectoryListing::fromJson(subdir_json));
        }
    }

    return listing;
}

std::string DirectoryListing::toTreeString(int depth)
{
    std::stringstream result;

    // Helper lambda function for recursive printing
    std::function<void(const DirectoryListing&, std::stringstream&,
                       const std::string&, bool, int)>
        buildTreeString = [&](const DirectoryListing& dir,
                              std::stringstream& ss, const std::string& indent,
                              bool isLast, int depth)
    {
        // Extract directory name logic (simplified)
        std::string dirName = dir.path;
        size_t lastSlash = dirName.find_last_of('/');
        if (lastSlash != std::string::npos && lastSlash + 1 < dirName.length())
        {
            dirName = dirName.substr(lastSlash + 1);
        }
        else if (dirName == "/")
        {
            dirName = "/";  // Handle root
        }
        else if (lastSlash == dirName.length() - 1 && dirName.length() > 1)
        {
            // Handle trailing slash if needed, though paths should be
            // normalized
            dirName = dirName.substr(0, dirName.length() - 1);
            lastSlash = dirName.find_last_of('/');
            if (lastSlash != std::string::npos &&
                lastSlash + 1 < dirName.length())
            {
                dirName = dirName.substr(lastSlash + 1);
            }
        }

        ss << indent;
        // Add prefix for non-root items or if indent exists
        if (!indent.empty() || dir.path != "/")
        {
            ss << (isLast ? "└── " : "├── ");
        }

        // Ensure trailing slash for dirs
        ss << (dirName.empty() ? "." : dirName) << (dirName == "/" ? "" : "/")
           << std::endl;

        if (depth <= 0)
        {
            return;
        }

        // Adjust child indent correctly based on whether parent had prefix
        std::string childIndent = indent;
        if (!indent.empty() || dir.path != "/")
        {
            childIndent += (isLast ? "    " : "│   ");
        }

        size_t totalChildren = dir.items.size() + dir.subdirectories.size();
        size_t count = 0;

        // Lambda to format file size
        auto formatFileSize = [](size_t size) -> std::string
        {
            const char* units[] = {"B", "KB", "MB", "GB", "TB"};
            int unitIndex = 0;
            double fileSize = static_cast<double>(size);

            while (fileSize >= 1024.0 && unitIndex < 4)
            {
                fileSize /= 1024.0;
                unitIndex++;
            }

            std::stringstream ss_size;
            // Use 1 decimal place
            ss_size << std::fixed << std::setprecision(1) << fileSize << " "
                    << units[unitIndex];
            return ss_size.str();
        };

        // Print subdirectories recursively
        for (size_t i = 0; i < dir.subdirectories.size(); ++i)
        {
            count++;
            bool subDirIsLast = (count == totalChildren);
            buildTreeString(dir.subdirectories[i], ss, childIndent,
                            subDirIsLast, depth - 1);
        }

        // Print files
        for (const auto& item : dir.items)
        {
            // dirs are printed separatly
            if (item.type == FileType::DIRECTORY)
            {
                continue;
            }

            count++;
            bool itemIsLast = (count == totalChildren);
            ss << childIndent;
            ss << (itemIsLast ? "└── " : "├── ");
            ss << item.name;
            if (item.size.has_value())
            {
                ss << " (" << formatFileSize(item.size.value()) << ")";
            }
            ss << std::endl;
        }
    };

    // Start the process from the current DirectoryListing object
    buildTreeString(*this, result, "", true, depth);
    return result.str();
}

// UploadResponse implementation
nlohmann::json UploadResponse::toJson() const
{
    nlohmann::json json;
    json["path"] = path;
    return json;
}

UploadResponse UploadResponse::fromJson(const nlohmann::json& json)  //
{
    UploadResponse response;
    // Assuming the API returns the full path in the 'path' field on success
    response.path = json.value("path", "");  // Use .value for safety
    return response;
}

// ErrorResponse implementation
nlohmann::json ErrorResponse::toJson() const
{
    nlohmann::json json;
    json["error"] = error;
    json["timestamp"] = timestamp;
    return json;
}

ErrorResponse ErrorResponse::fromJson(const nlohmann::json& json)
{
    ErrorResponse response;
    response.error = json.value("error", "Unknown error");
    response.timestamp = json.value("timestamp", "");
    return response;
}

void ErrorResponse::throwIfError(const nlohmann::json& json)
{
    // This logic is typically handled within the ApiClient::get method now
    // based on status codes, but could be used as a secondary check.
    if (json.contains("error"))  //
    {
        // Could throw a generic HomevaultApiException here if needed
        // ErrorResponse error = ErrorResponse::fromJson(json);
        // throw HomevaultApiException(error.error);
    }
}

}  // namespace hv
