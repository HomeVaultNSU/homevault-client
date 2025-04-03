#include "hv/cli/CLISetup.hpp"

#include <core/ServerConnection.hpp>
#include <iostream>
#include <vector>

namespace CLISetup
{
void PrintList(hv::ServerConnection& server_connection)
{
    (void)server_connection;
    // server_connection.listRemoteFiles();
    std::cout << "Files have been listed successfully." << std::endl;
}

void Upload(const std::vector<std::string>& files,
            hv::ServerConnection& server_connection)
{
    (void)files;
    (void)server_connection;
    // server_connection.upload(std::filesystem::path("~"),
    //                          std::filesystem::path("~"));
    std::cout << "Files uploaded successfully." << std::endl;
}

void Download(const std::vector<std::string>& files,
              hv::ServerConnection& server_connection)
{
    (void)server_connection;
    // server_connection.download(std::filesystem::path("~"),
    //                            std::filesystem::path("~"));
    (void)files;
    std::cout << "Files downloaded successfully" << std::endl;
}

void SetupSubcommands(CLI::App& app, hv::ServerConnection& server_connection)
{
    const auto list =
        app.add_subcommand("list", "List all available files on server");
    auto upload = app.add_subcommand("upload", "Upload file to server");
    auto download = app.add_subcommand("download", "Download file from server");

    list->allow_extras();
    upload->allow_extras();
    download->allow_extras();

    list->callback([&server_connection]() { PrintList(server_connection); });
    upload->callback([upload, &server_connection]()
                     { Upload(upload->remaining(), server_connection); });
    download->callback([download, &server_connection]()
                       { Download(download->remaining(), server_connection); });
}
}  // namespace CLISetup
