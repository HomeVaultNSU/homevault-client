#include "hv/cli/CLISetup.hpp"

#include <core/Homevault.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace CLISetup
{
void PrintList(hv::Homevault& hvClient, int depth, std::string listPath)
{
    auto result = hvClient.listRemoteFiles(listPath, depth);
    if (result.status() == hv::Status::eSuccess)
    {
        std::cout << result.value().toTreeString(depth) << "\n";
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

void Download(const std::vector<std::string>& remoteFilePaths,
              hv::Homevault& hvClient)
{
    for (const auto& remotePath : remoteFilePaths)
    {
        auto result = hvClient.download(".", remotePath);
        if (result.status() != hv::Status::eSuccess)
        {
            std::cerr << "Failed to download " << remotePath << ": "
                      << result.message() << "\n";
        }
        else
        {
            std::cout << "Successfully downloaded " << remotePath << "\n";
        }
    }
}

int Register(hv::Homevault& hvClient)
{
    std::string username;
    std::string password;
    std::string passwordConfirmation;

    std::cout << "Username: ";
    std::cin >> username;
    std::cout << "Password: ";
    std::cin >> password;
    std::cout << "Password confirmation: ";
    std::cin >> passwordConfirmation;

    if (password != passwordConfirmation)
    {
        std::cout << "Passwords do not match\n";
        return EXIT_FAILURE;
    }

    hv::Result result = hvClient.registerUser(username, password);
    if (result.status() != hv::Status::eSuccess)
    {
        std::cerr << result.message();
        return EXIT_FAILURE;
    }

    std::cout << "User created successfully\n";
    return EXIT_SUCCESS;
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
    download->add_option("files", downloadStorage.files, "Files to download")
        ->required()
        ->expected(-1);  // Allow multiple files
    download->callback([&]() { Download(downloadStorage.files, hvClient); });
}

void SetupRegisterSubcommand(CLI::App& app, hv::Homevault& hvClient)
{
    auto registerCmd = app.add_subcommand("register", "Add user to server");
    registerCmd->callback([&]() { Register(hvClient); });
}
}  // namespace CLISetup
