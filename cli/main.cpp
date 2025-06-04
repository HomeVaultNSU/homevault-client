#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Error.hpp>
#include <core/Homevault.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>

#include "hv/cli/CLISetup.hpp"

std::string RequireEnvVar(std::string const& key)
{
    char const* val = std::getenv(key.c_str());
    if (!val)
    {
        std::cerr << "Error: " << key << " environment variable is not set"
                  << std::endl;
        return "";
    }

    return val;
}

int main(int argc, char* argv[])
{
    CLI::App app{"homevault-cli"};
    app.require_subcommand(1);

    std::string fileServerHostname = RequireEnvVar("HV_FILE_HOSTNAME");
    std::string authServerHostname = RequireEnvVar("HV_AUTH_HOSTNAME");
    std::string username = RequireEnvVar("HV_USERNAME");
    std::string password = RequireEnvVar("HV_PASSWORD");

    if (fileServerHostname.empty() || authServerHostname.empty() ||
        username.empty() || password.empty())
    {
        return EXIT_FAILURE;
    }

    std::unique_ptr<hv::Homevault> hvClient = std::make_unique<hv::Homevault>(
        fileServerHostname, authServerHostname, username, password);

    static CLISetup::ListStorage listStorage;
    static CLISetup::UploadStorage uploadStorage;
    static CLISetup::DownloadStorage downloadStorage;

    CLISetup::SetupListSubcommand(app, *hvClient, listStorage);
    CLISetup::SetupUploadSubcommand(app, *hvClient, uploadStorage);
    CLISetup::SetupDownloadSubcommand(app, *hvClient, downloadStorage);
    CLISetup::SetupRegisterSubcommand(app, *hvClient);

    CLI11_PARSE(app, argc, argv);

    return EXIT_SUCCESS;
}
