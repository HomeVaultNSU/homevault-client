#include "core/FilesystemCrawler.hpp"

#include <vector>

namespace hv
{

FileSystemCrawler::FileSystemCrawler(ApiClient& client) : apiClient(client)
{
}  //

DirectoryListing FileSystemCrawler::getDirectoryTreeWithDepth(
    const std::string& path, int maxDepth)  //
{
    // Validate depth (as per OpenAPI spec 0-5, but crawler can handle >= 0)
    if (maxDepth < 0)  //
    {
        throw std::invalid_argument("Maximum depth cannot be negative");  //
    }
    // The Homevault::listRemoteFiles method should enforce the max=5
    // constraint.

    // Start recursive build
    return buildDirectoryTree(path, 0, maxDepth);  //
}

DirectoryListing FileSystemCrawler::buildDirectoryTree(
    const std::string& currentPath, int currentDepth, int maxDepth)  //
{
    // Optimization: Request depth 1 initially to get immediate children AND
    // know about subdirs. The API's 'depth' parameter controls how deep the
    // *server* looks. We request depth=1 from the server initially. Our
    // client-side recursion handles client-side depth. If the API *could*
    // return the whole structure based on depth > 1, this client-side recursion
    // might be simpler, just calling getDirectoryListing once with the
    // requested depth. Assuming the API only returns nested structure if *it*
    // is asked with depth > 0, and our client needs to assemble deeper
    // structures. Let's stick to the provided logic:
    DirectoryListing listing =
        apiClient.getDirectoryListing(currentPath, 1);  // Fetch level 1 data

    // Base case: If max depth reached OR the server returned no further
    // subdirectories in its depth=1 response Note: The check
    // `listing.subdirectories.empty()` relies on the server *populating* the
    // subdirectories array even when called with depth=1, perhaps just with
    // paths/names but empty 'items'. If the server *only* returns subdirs when
    // depth>1 is *requested*, this logic needs adjustment. Assuming the
    // provided code's logic matches the server behavior:
    if (currentDepth >= maxDepth || listing.subdirectories.empty())  //
    {
        // We might want to clear subdirectories if maxDepth is reached but
        // server sent some?
        if (currentDepth >= maxDepth)
        {
            listing.subdirectories
                .clear();  // Don't show further levels if maxDepth reached
        }
        return listing;  //
    }

    // Recursive step: Fetch deeper info for subdirectories
    std::vector<DirectoryListing> populatedSubdirs;  // Build the new list here
    for (const auto& shallowSubdir :
         listing
             .subdirectories)  // Iterate over subdirs reported by initial call
    {
        // Recursively call to get the deeper tree for this subdirectory
        DirectoryListing deepSubdirTree = buildDirectoryTree(
            shallowSubdir.path, currentDepth + 1, maxDepth);  //
        populatedSubdirs.push_back(deepSubdirTree);           //
    }

    // Replace the shallow subdirectories with the recursively populated ones
    listing.subdirectories = std::move(populatedSubdirs);  // Update the listing

    return listing;  //
}

}  // namespace hv
