#include "hv/cli/CLISetup.hpp"

#include <core/Homevault.hpp>
#include <iostream>
#include <vector>

namespace CLISetup
{
void PrintList(hv::Homevault& hvClient, int depth, std::string listPath)
{
    auto result = hvClient.listRemoteFiles(listPath, depth);
    if (result.status() == hv::Status::eSuccess)
    {
        std::cout << result.value().toTreeString() << "\n";
    }
    else
    {
        std::cerr << "Error:\n\t" << result.message() << "\n";
    }
}

void Upload(const std::vector<std::string>& files, hv::Homevault& hvClient)
{
    for (const auto& file : files)
    {
        auto result = hvClient.upload(file, "/");
        if (result.status() != hv::Status::eSuccess)
        {
            std::cerr << "Failed to upload " << file << ": " << result.message()
                      << "\n";
        }
        else
        {
            std::cout << "Successfully uploaded " << file << "\n";
        }
    }
}

void Download(const std::vector<std::string>& files, hv::Homevault& hvClient)
{
    for (const auto& file : files)
    {
        auto result = hvClient.download(file, ".");
        if (result.status() != hv::Status::eSuccess)
        {
            std::cerr << "Failed to download " << file << ": "
                      << result.message() << "\n";
        }
        else
        {
            std::cout << "Successfully downloaded " << file << "\n";
        }
    }
}

void SetupListSubcommand(CLI::App& app, hv::Homevault& hvClient,
                         CLISetup::ListStorage& listStorage)
{
    const auto list =
        app.add_subcommand("list", "List all available files on server");
    list->add_option("-d,--depth", listStorage.depth,
                     "Depth of file tree to print (1 by default)")
        ->default_val(1);
    list->add_option("path", listStorage.listPath, "List path")
        ->default_str("/")
        ->required(false);
    list->callback(
        [&]()
        { PrintList(hvClient, listStorage.depth, listStorage.listPath); });
}

void SetupUploadSubcommand(CLI::App& app, hv::Homevault& hvClient,
                           CLISetup::UploadStorage& uploadStorage)
{
    auto upload = app.add_subcommand("upload", "Upload file to server");
    upload->add_option("files", uploadStorage.files, "Files to upload")
        ->required()
        ->expected(-1);  // Allow multiple files
    upload->callback([&]() { Upload(uploadStorage.files, hvClient); });
}

void SetupDownloadSubcommand(CLI::App& app, hv::Homevault& hvClient,
                             CLISetup::DownloadStorage& downloadStorage)
{
    auto download = app.add_subcommand("download", "Download file from server");
    download->callback([&]() { Download(downloadStorage.files, hvClient); });
}
}  // namespace CLISetup
