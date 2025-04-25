#include "core/FilesystemCrawler.hpp"

#include <cinttypes>
#include <vector>

namespace hv
{

FileSystemCrawler::FileSystemCrawler(ApiClient& client) : apiClient(client)
{
}

DirectoryListing FileSystemCrawler::getDirectoryTreeWithDepth(
    const std::string& path, int maxDepth)
{
    // Validate depth (as per OpenAPI spec 0-5, but crawler can handle >= 0)
    if (maxDepth < 0)
    {
        throw std::invalid_argument("Maximum depth cannot be negative");  //
    }

    return buildDirectoryTree(path, maxDepth);
}

DirectoryListing FileSystemCrawler::buildDirectoryTree(
    const std::string& currentPath, int depth)
{
    if (depth == 0) {
        return apiClient.getDirectoryListing(currentPath, 0);
    }

    if (depth == 1) {
        return apiClient.getDirectoryListing(currentPath, 1);
    }

    DirectoryListing listing = apiClient.getDirectoryListing(currentPath, 1);

    if (listing.subdirectories.empty()) {
        return listing;
    }

    std::vector<DirectoryListing> currentSubdirs = listing.subdirectories;
    listing.subdirectories.clear();

    for (const auto& subdir : currentSubdirs)
    {
        DirectoryListing subdirTree =
            buildDirectoryTree(subdir.path, depth - 1);

        listing.subdirectories.push_back(subdirTree);
    }

    return listing;
}

}  // namespace hv
