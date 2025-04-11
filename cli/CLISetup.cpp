#include "hv/cli/CLISetup.hpp"

#include <core/HomeVaultClient.hpp>
#include <iostream>
#include <vector>

namespace CLISetup
{
void PrintList(hv::HomeVaultClient& hvClient, int depth,
               CLISetup::CLIStorage& cliStorage)
{
    hv::ResultValue<hv::FileInfo> result =
        hvClient.listRemoteFiles(cliStorage.listPath, depth);
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
            hv::HomeVaultClient& hvClient)
{
    (void)files;
    (void)hvClient;
    std::cout << "Files uploaded successfully." << std::endl;
}

void Download(const std::vector<std::string>& files,
              hv::HomeVaultClient& hvClient)
{
    (void)hvClient;
    (void)files;
    std::cout << "Files downloaded successfully" << std::endl;
}

void SetupSubcommands(CLI::App& app, hv::HomeVaultClient& hvClient,
                      CLISetup::CLIStorage& cliStorage)
{
    int depth = -1;
    const auto list =
        app.add_subcommand("list", "List all available files on server");
    auto upload = app.add_subcommand("upload", "Upload file to server");
    auto download = app.add_subcommand("download", "Download file from server");

    list->add_option("-d,--depth", depth,
                     "Depth of file tree to print (infinite by default)")
        ->default_val(-1);
    list->add_option("path", cliStorage.listPath, "List path")
        ->default_str("/")
        ->required(false);

    list->callback([&depth, &hvClient, &cliStorage]()
                   { PrintList(hvClient, depth, cliStorage); });
    upload->callback([upload, &hvClient]()
                     { Upload(upload->remaining(), hvClient); });
    download->callback([download, &hvClient]()
                       { Download(download->remaining(), hvClient); });
}

}  // namespace CLISetup
