#include "hv/cli/CLISetup.hpp"

#include <core/HomeVaultClient.hpp>
#include <iostream>
#include <vector>

namespace CLISetup
{
void PrintList(hv::HomeVaultClient& hvClient)
{
    hv::ResultValue<hv::FileInfo> result = hvClient.listRemoteFiles("/");
    if (result.status() == hv::Status::eSuccess)
    {
        std::cout << result.value().toTreeString() << "\n";
    }
    else
    {
        std::cout << result.message() << "\n";
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

void SetupSubcommands(CLI::App& app, hv::HomeVaultClient& hvClient)
{
    const auto list =
        app.add_subcommand("list", "List all available files on server");
    auto upload = app.add_subcommand("upload", "Upload file to server");
    auto download = app.add_subcommand("download", "Download file from server");

    list->allow_extras();
    upload->allow_extras();
    download->allow_extras();

    list->callback([&hvClient]() { PrintList(hvClient); });
    upload->callback([upload, &hvClient]()
                     { Upload(upload->remaining(), hvClient); });
    download->callback([download, &hvClient]()
                       { Download(download->remaining(), hvClient); });
}
}  // namespace CLISetup
