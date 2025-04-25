#include <core/ApiClient.hpp>
#include <core/FilesystemCrawler.hpp>
#include <core/Homevault.hpp>
#include <core/Models.hpp>
#include <core/Result.hpp>
#include <memory>

namespace hv
{

Homevault::Homevault(const std::string& hostname)
{
    m_apiClient = std::make_unique<ApiClient>(hostname);
}

Homevault::~Homevault() = default;

ResultValue<DirectoryListing> Homevault::listRemoteFiles(
    const std::string& path, int depth)
{
    try
    {
        if (depth < 0 || depth > 5)
        {
            return ResultValue<DirectoryListing>(
                Status::eInvalidArgument, "Depth must be between 0 and 5");
        }

        // Normalize path (ensure it starts with /, remove trailing / unless
        // it's just "/")
        std::string normalizedPath = path.empty() ? "/" : path;
        if (!(normalizedPath[0] == '/'))
        {  // Ensure leading slash
            normalizedPath = "/" + normalizedPath;
        }
        if (normalizedPath.length() > 1 && normalizedPath.back() == '/')
        {
            normalizedPath.pop_back();
        }

        // Use the FilesystemCrawler to handle potential recursion
        FileSystemCrawler filesystemCrawler(*m_apiClient);  //
        DirectoryListing listing = filesystemCrawler.getDirectoryTreeWithDepth(
            normalizedPath, depth);  //

        // Success
        return ResultValue<DirectoryListing>(listing);  //
    }
    // Catch specific API exceptions from ApiClient/Crawler
    catch (const HomevaultNotFoundException& e)  // Specific catch
    {
        return ResultValue<DirectoryListing>(
            Status::eNotFound, e.what());  // Use exception message
    }
    catch (const HomevaultBadRequestException& e)  // Specific catch
    {
        return ResultValue<DirectoryListing>(
            Status::eInvalidArgument, e.what());  // Use exception message
    }
    catch (const HomevaultApiException& e)  // Catch other API errors
    {
        // Use status code from exception if available, otherwise default to
        // UnknownError
        Status status = Status::eUnknownError;
        if (e.getStatusCode() == 404)
            status = Status::eNotFound;  //
        else if (e.getStatusCode() == 400)
            status = Status::eInvalidArgument;  //
        // Add more specific mappings if needed

        return ResultValue<DirectoryListing>(
            status, "Server error: " + std::string(e.what()));  //
    }
    // Catch standard exceptions (like invalid_argument from crawler depth
    // check)
    catch (const std::invalid_argument& e)
    {
        return ResultValue<DirectoryListing>(
            Status::eInvalidArgument,
            "Invalid argument: " + std::string(e.what()));
    }
    // Catch any other unexpected exceptions
    catch (const std::exception& e)  //
    {
        return ResultValue<DirectoryListing>(
            Status::eUnknownError,
            "Unexpected error: " + std::string(e.what()));  //
    }
}

Result Homevault::upload(const std::filesystem::path& local_path,
                         const std::filesystem::path& remote_dir_path)
{
    try
    {
        // Basic validation - check if path exists
        if (!std::filesystem::exists(local_path))
        {
            return Result(Status::eInvalidArgument,
                          "Path not found: " + local_path.string());
        }

        // Normalize remote directory path
        std::string remoteDirStr = remote_dir_path.string();
        if (remoteDirStr.empty())
        {
            remoteDirStr = "/";
        }
        if (remoteDirStr[0] != '/')
        {
            remoteDirStr = "/" + remoteDirStr;
        }

        // Handle file or directory
        if (std::filesystem::is_regular_file(local_path))
        {
            // Single file upload
            UploadResponse response =
                m_apiClient->uploadFile(local_path.string(), remoteDirStr);
            return Result(Status::eSuccess,
                          "File uploaded successfully to: " + response.path);
        }
        else if (std::filesystem::is_directory(local_path))
        {
            // Directory upload - process recursively
            std::error_code ec;
            for (const auto& entry :
                 std::filesystem::recursive_directory_iterator(local_path, ec))
            {
                if (ec)
                {
                    return Result(Status::eFileError,
                                  "Error reading directory: " + ec.message());
                }

                if (entry.is_regular_file())
                {
                    // Calculate relative path to maintain directory structure
                    auto relPath =
                        std::filesystem::relative(entry.path(), local_path);
                    auto targetDir =
                        remoteDirStr + "/" + relPath.parent_path().string();

                    // Upload file to corresponding remote directory
                    try
                    {
                        UploadResponse response = m_apiClient->uploadFile(
                            entry.path().string(), targetDir);
                    }
                    catch (const HomevaultApiException& e)
                    {
                        return Result(Status::eUnknownError,
                                      "Failed to upload " +
                                          entry.path().string() + ": " +
                                          e.what());
                    }
                }
            }
            return Result(
                Status::eSuccess,
                "Directory uploaded successfully to: " + remoteDirStr);
        }
        else
        {
            return Result(
                Status::eInvalidArgument,
                "Path is neither a file nor directory: " + local_path.string());
        }
    }
    catch (const HomevaultBadRequestException& e)
    {
        return Result(Status::eInvalidArgument,
                      "Upload failed (Bad Request): " + std::string(e.what()));
    }
    catch (const HomevaultNotFoundException& e)
    {
        return Result(Status::eNotFound,
                      "Upload failed (Not Found): " + std::string(e.what()));
    }
    catch (const HomevaultServerException& e)
    {
        return Result(Status::eUnknownError,
                      "Upload failed (Server Error): " + std::string(e.what()));
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        return Result(Status::eFileError,
                      "Filesystem error: " + std::string(e.what()));
    }
    catch (const std::exception& e)
    {
        return Result(
            Status::eUnknownError,
            "Unexpected error during upload: " + std::string(e.what()));
    }
}

Result Homevault::download(
    [[maybe_unused]] const std::filesystem::path& local_path,
    [[maybe_unused]] const std::filesystem::path& remote_path)
{
    return Status::eSuccess;
}

};  // namespace hv
