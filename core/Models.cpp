#include "core/Models.hpp"
#include <iostream>

namespace hv {

// FileItem implementation
nlohmann::json FileItem::toJson() const {
    nlohmann::json json;
    json["name"] = name;
    json["type"] = static_cast<char>(type);
    if (size) {
        json["size"] = *size;
    } else {
        json["size"] = nullptr;
    }
    json["modified_at"] = modifiedAt;
    return json;
}

FileItem FileItem::fromJson(const nlohmann::json& json) {
    std::cerr << json.dump();

    FileItem item;
    item.name = json["name"].get<std::string>();
    item.type = json["type"].get<std::string>() == "F" ? FileType::FILE : FileType::DIRECTORY;
    
    if (!json["size"].is_null()) {
        item.size = json["size"].get<size_t>();
    }
    
    item.modifiedAt = json["lastModifiedAt"].get<std::string>();
    return item;
}

// DirectoryListing implementation
nlohmann::json DirectoryListing::toJson() const {
    nlohmann::json json;
    json["path"] = path;
    
    nlohmann::json items_array = nlohmann::json::array();
    for (const auto& item : items) {
        items_array.push_back(item.toJson());
    }
    json["items"] = items_array;
    
    nlohmann::json subdirs_array = nlohmann::json::array();
    for (const auto& subdir : subdirectories) {
        subdirs_array.push_back(subdir.toJson());
    }
    json["subdirectories"] = subdirs_array;
    
    return json;
}

DirectoryListing DirectoryListing::fromJson(const nlohmann::json& json) {
    DirectoryListing listing;
    listing.path = json["path"].get<std::string>();
    
    for (const auto& item_json : json["items"]) {
        listing.items.push_back(FileItem::fromJson(item_json));
    }
    
    if (json.contains("subdirectories")) {
        for (const auto& subdir_json : json["subdirectories"]) {
            listing.subdirectories.push_back(DirectoryListing::fromJson(subdir_json));
        }
    }
    
    return listing;
}

std::string DirectoryListing::toTreeString() {
    std::stringstream result;
    // Helper function to recursively build the tree string with appropriate indentation
    std::function<void(const DirectoryListing&, std::stringstream&, const std::string&, bool)> 
    buildTreeString = [&](const DirectoryListing& dir, std::stringstream& ss, const std::string& indent, bool isLast) {
        // Extract directory name from path
        std::string dirName = dir.path;
        size_t lastSlash = dirName.find_last_of('/');
        if (lastSlash != std::string::npos) {
            dirName = dirName.substr(lastSlash + 1);
        }
        if (dirName.empty()) {
            dirName = "/"; // Root directory
        }
        
        // Print the directory name
        ss << indent;
        if (!indent.empty()) {
            ss << (isLast ? "└── " : "├── ");
        }
        ss << dirName << "/" << std::endl;
        
        // Prepare the indentation for children
        std::string childIndent = indent;
        if (!indent.empty()) {
            childIndent += isLast ? "    " : "│   ";
        }
        
        // Print files first
        size_t totalChildren = dir.items.size() + dir.subdirectories.size();
        size_t count = 0;
        
        // Format file size to human-readable format
        auto formatFileSize = [](size_t size) -> std::string {
            const char* units[] = {"B", "KB", "MB", "GB", "TB"};
            int unitIndex = 0;
            double fileSize = static_cast<double>(size);

            while (fileSize >= 1024.0 && unitIndex < 4) {
                fileSize /= 1024.0;
                unitIndex++;
            }

            std::stringstream ss;
            ss.precision(2);
            ss << std::fixed << fileSize << " " << units[unitIndex];
            return ss.str();
        };

        for (const auto& item : dir.items) {
            count++;
            bool itemIsLast = (count == totalChildren);
            ss << childIndent;
            ss << (itemIsLast ? "└── " : "├── ");
            ss << item.name;
            
            // Add file size if available
            if (item.size.has_value()) {
                ss << " (" << formatFileSize(item.size.value()) << ")";
            }
            
            ss << std::endl;
        }
        
        // Print subdirectories
        for (size_t i = 0; i < dir.subdirectories.size(); ++i) {
            count++;
            bool subDirIsLast = (count == totalChildren);
            buildTreeString(dir.subdirectories[i], ss, childIndent, subDirIsLast);
        }
    };
    
    // Start building the tree from the root
    buildTreeString(*this, result, "", true);
    return result.str();
}
// UploadResponse implementation
nlohmann::json UploadResponse::toJson() const {
    nlohmann::json json;
    json["path"] = path;
    return json;
}

UploadResponse UploadResponse::fromJson(const nlohmann::json& json) {
    UploadResponse response;
    response.path = json["path"].get<std::string>();
    return response;
}

// ErrorResponse implementation
nlohmann::json ErrorResponse::toJson() const {
    nlohmann::json json;
    json["error"] = error;
    json["timestamp"] = timestamp;
    return json;
}

ErrorResponse ErrorResponse::fromJson(const nlohmann::json& json) {
    ErrorResponse response;
    response.error = json["error"].get<std::string>();
    response.timestamp = json["timestamp"].get<std::string>();
    return response;
}

void ErrorResponse::throwIfError(const nlohmann::json& json) {
    if (json.contains("error")) {
        ErrorResponse error = ErrorResponse::fromJson(json);
        throw HomevaultApiException(error.error);
    }
}

} // namespace hv
