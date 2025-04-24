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

std::string GetEnvVar(std::string const& key)
{
    char const* val = std::getenv(key.c_str());
    std::string result = val == NULL ? std::string() : std::string(val);

    // Add debug output
    std::cerr << "Environment variable " << key << ": " << result << std::endl;

    return result;
}

int main(int argc, char* argv[])
{
    CLI::App app{"homevault-cli"};
    app.require_subcommand(1);

    std::string hostname = GetEnvVar("HV_HOSTNAME");
    if (hostname.empty())
    {
        std::cerr << "Error: HV_HOSTNAME environment variable is not set"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::string username = GetEnvVar("HV_USERNAME");
    std::string password = GetEnvVar("HV_PASSWORD");

    // Create client with appropriate constructor
    std::unique_ptr<hv::Homevault> hvClient;

    if (!username.empty())
    {
        if (password.empty())
        {
            std::cerr << "Error: no password provided for user \"" << username
                      << "\" on hostname \"" << hostname << "\"." << std::endl;
            return EXIT_FAILURE;
        }
        hvClient =
            std::make_unique<hv::Homevault>(hostname);
    }
    else
    {
        hvClient = std::make_unique<hv::Homevault>(hostname);
    }

    static CLISetup::CLIStorage cliStorage;
    CLISetup::SetupSubcommands(app, *hvClient, cliStorage);

    CLI11_PARSE(app, argc, argv);

    return EXIT_SUCCESS;
}
