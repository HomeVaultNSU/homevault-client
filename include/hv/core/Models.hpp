#ifndef MODELS_HPP
#define MODELS_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace hv
{

enum class FileType
{
    FILE = 'F',
    DIRECTORY = 'D'
};

struct FileItem
{
    std::string name;
    FileType type;
    std::optional<size_t> size;
    std::string modifiedAt;

    // Json serialization/deserialization
    nlohmann::json toJson() const;
    static FileItem fromJson(const nlohmann::json& json);
};

struct DirectoryListing
{
    std::string path;
    std::vector<FileItem> items;
    std::vector<DirectoryListing> subdirectories;

    // Json serialization/deserialization
    nlohmann::json toJson() const;
    static DirectoryListing fromJson(const nlohmann::json& json);

    std::string toTreeString();
};

struct UploadResponse
{
    std::string path;

    // Json serialization/deserialization
    nlohmann::json toJson() const;
    static UploadResponse fromJson(const nlohmann::json& json);
};

struct ErrorResponse
{
    std::string error;
    std::string timestamp;

    // Json serialization/deserialization
    nlohmann::json toJson() const;
    static ErrorResponse fromJson(const nlohmann::json& json);
    static void throwIfError(const nlohmann::json& json);
};

class HomevaultApiException : public std::runtime_error
{
public:
    HomevaultApiException(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class HomevaultNotFoundException : public HomevaultApiException
{
public:
    HomevaultNotFoundException(const std::string& message)
        : HomevaultApiException(message)
    {
    }
};

class HomevaultBadRequestException : public HomevaultApiException
{
public:
    HomevaultBadRequestException(const std::string& message)
        : HomevaultApiException(message)
    {
    }
};

class HomevaultServerException : public HomevaultApiException
{
public:
    HomevaultServerException(const std::string& message)
        : HomevaultApiException(message)
    {
    }
};

}  // namespace hv

#endif  // !MODELS_HPP
