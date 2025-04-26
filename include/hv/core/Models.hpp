#ifndef MODELS_HPP
#define MODELS_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>  // For exceptions
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

    std::string toTreeString(int depth);
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
private:
    int statusCode_ = 0;  // Store status code if available
public:
    explicit HomevaultApiException(const std::string& msg, int statusCode = 0)
        : std::runtime_error(msg), statusCode_(statusCode)
    {
    }
    int getStatusCode() const { return statusCode_; }
};

class HomevaultNotFoundException : public HomevaultApiException
{
public:
    explicit HomevaultNotFoundException(const std::string& msg)
        : HomevaultApiException(msg, 404)
    {
    }
};

class HomevaultBadRequestException : public HomevaultApiException
{
public:
    explicit HomevaultBadRequestException(const std::string& msg)
        : HomevaultApiException(msg, 400)
    {
    }
};

class HomevaultServerException : public HomevaultApiException
{
public:
    explicit HomevaultServerException(const std::string& msg)
        : HomevaultApiException(msg, 500)
    {
    }  // Default to 500 if not specified
};

}  // namespace hv

#endif  // !MODELS_HPP
