#ifndef FILESYSTEM_CRAWLER_HPP
#define FILESYSTEM_CRAWLER_HPP

#include <string>

#include "ApiClient.hpp"

namespace hv
{

class FileSystemCrawler
{
public:
    /**
     * @brief Construct a new File System Crawler object
     *
     * @param apiClient Reference to the API client
     */
    FileSystemCrawler(ApiClient& apiClient);

    /**
     * @brief Get directory listing with full tree structure up to specified
     * depth
     *
     * @param path Root path to start crawling from
     * @param maxDepth Maximum depth to crawl (0 means no recursion, just the
     * root)
     * @return DirectoryListing Full directory tree structure
     */
    DirectoryListing getDirectoryTreeWithDepth(const std::string& path = "/",
                                               int maxDepth = 3);

private:
    ApiClient& apiClient;

    /**
     * @brief Recursive helper to build the directory tree
     *
     * @param currentPath Current directory path
     * @param depth depth to explore
     * @return DirectoryListing Directory listing for the current path with
     * subdirectories
     */
    DirectoryListing buildDirectoryTree(const std::string& currentPath,
                                        int depth);
};

}  // namespace hv

#endif  // FILESYSTEM_CRAWLER_HPP
