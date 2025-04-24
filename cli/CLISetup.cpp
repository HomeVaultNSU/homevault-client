#include "hv/cli/CLISetup.hpp"

#include <core/Homevault.hpp>
#include <iostream>
#include <vector>

namespace CLISetup
{
void PrintList(hv::Homevault& hvClient, int depth,
               CLISetup::CLIStorage& cliStorage)
{
    auto result = hvClient.listRemoteFiles(cliStorage.listPath, depth);
    if (result.status() == hv::Status::eSuccess)
    {
        std::cout << result.value().toTreeString() << "\n";
    }
    else
    {
        std::cerr << "Error:\n\t" << result.message() << "\n";
    }
}

void Upload(const std::vector<std::string>& files,
            hv::Homevault& hvClient)
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

void Download(const std::vector<std::string>& files,
              hv::Homevault& hvClient)
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

void SetupSubcommands(CLI::App& app, hv::Homevault& hvClient,
                      CLISetup::CLIStorage& cliStorage)
{
    const auto list =
        app.add_subcommand("list", "List all available files on server");
    auto upload = app.add_subcommand("upload", "Upload file to server");
    auto download = app.add_subcommand("download", "Download file from server");

    list->add_option("-d,--depth", cliStorage.depth,
                     "Depth of file tree to print (infinite by default)")
        ->default_val(-1);
    list->add_option("path", cliStorage.listPath, "List path")
        ->default_str("/")
        ->required(false);

    list->callback([&]()
                   { PrintList(hvClient, cliStorage.depth, cliStorage); });
    upload->callback([upload, &hvClient]()
                     { Upload(upload->remaining(), hvClient); });
    download->callback([download, &hvClient]()
                       { Download(download->remaining(), hvClient); });
}

}  // namespace CLISetup
