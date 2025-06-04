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

class TokenResponse
{
public:
    std::string token;

    // Constructor
    TokenResponse() = default;
    TokenResponse(const std::string& token) : token(token) {}

    // JSON serialization/deserialization
    static TokenResponse fromJson(const nlohmann::json& json)
    {
        TokenResponse response;
        if (json.contains("token") && json["token"].is_string())
        {
            response.token = json["token"].get<std::string>();
        }
        else
        {
            throw std::invalid_argument("Invalid TokenResponse JSON: missing or invalid 'token' field");
        }
        return response;
    }

    nlohmann::json toJson() const
    {
        nlohmann::json json;
        json["token"] = token;
        return json;
    }
};

class DecodedTokenResponse
{
public:
    int64_t userId;
    std::string role;

    // Constructor
    DecodedTokenResponse() = default;
    DecodedTokenResponse(int64_t userId, const std::string& role) 
        : userId(userId), role(role) {}

    // JSON serialization/deserialization
    static DecodedTokenResponse fromJson(const nlohmann::json& json)
    {
        DecodedTokenResponse response;
        
        if (json.contains("userId") && json["userId"].is_number_integer())
        {
            response.userId = json["userId"].get<int64_t>();
        }
        else
        {
            throw std::invalid_argument("Invalid DecodedTokenResponse JSON: missing or invalid 'userId' field");
        }

        if (json.contains("role") && json["role"].is_string())
        {
            response.role = json["role"].get<std::string>();
        }
        else
        {
            throw std::invalid_argument("Invalid DecodedTokenResponse JSON: missing or invalid 'role' field");
        }

        return response;
    }

    nlohmann::json toJson() const
    {
        nlohmann::json json;
        json["userId"] = userId;
        json["role"] = role;
        return json;
    }
};

// 401 Unauthorized - Authentication failed or token invalid
class HomevaultUnauthorizedException : public HomevaultApiException
{
public:
    explicit HomevaultUnauthorizedException(const std::string& message)
        : HomevaultApiException("Unauthorized: " + message) {}
};

// 409 Conflict - Resource conflict (e.g., username already exists)
class HomevaultConflictException : public HomevaultApiException
{
public:
    explicit HomevaultConflictException(const std::string& message)
        : HomevaultApiException("Conflict: " + message) {}
};
}  // namespace hv

#endif  // !MODELS_HPP
