#include "core/FilesystemCrawler.hpp"

namespace hv {

FileSystemCrawler::FileSystemCrawler(ApiClient& apiClient) : apiClient(apiClient) {}

DirectoryListing FileSystemCrawler::getDirectoryTreeWithDepth(const std::string& path, int maxDepth) {
    // Validate maximum depth
    if (maxDepth < 0) {
        throw std::invalid_argument("Maximum depth cannot be negative");
    }
    
    // Start the recursive crawling from the specified path
    return buildDirectoryTree(path, 0, maxDepth);
}

DirectoryListing FileSystemCrawler::buildDirectoryTree(const std::string& currentPath, 
                                                       int currentDepth, 
                                                       int maxDepth) {
    // Get basic directory listing (with depth=1 to include immediate subdirectories)
    DirectoryListing listing = apiClient.getDirectoryListing(currentPath, 1);
    
    // If we've reached the maximum depth or there are no subdirectories, return the current listing
    if (currentDepth >= maxDepth || listing.subdirectories.empty()) {
        return listing;
    }
    
    // Store the current subdirectories for iteration
    std::vector<DirectoryListing> currentSubdirs = listing.subdirectories;
    
    // Clear the subdirectories to rebuild them with deeper listings
    listing.subdirectories.clear();
    
    // For each subdirectory, recursively build its tree
    for (const auto& subdir : currentSubdirs) {
        std::string subdirPath = subdir.path;
        
        // Recursively get the full directory tree for this subdirectory
        DirectoryListing subdirTree = buildDirectoryTree(subdirPath, currentDepth + 1, maxDepth);
        
        // Add the complete subdirectory tree to our listing
        listing.subdirectories.push_back(subdirTree);
    }
    
    return listing;
}

} // namespace hv
