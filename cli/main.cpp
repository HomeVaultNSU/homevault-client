#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Error.hpp>
#include <core/HomeVaultClient.hpp>
#include <cstdlib>
#include <string>

#include "hv/cli/CLISetup.hpp"

std::string get_env_var(std::string const& key)
{
    char const* val = std::getenv(key.c_str()); 
    return val == NULL ? std::string() : std::string(val);
}

int main(int argc, char *argv[])
{
    CLI::App app{"homevault-cli"};
    app.require_subcommand(1);

    std::string hostname = get_env_var("HV_HOSTNAME");
    std::string username = get_env_var("HV_USERNAME");
    std::string password = get_env_var("HV_PASSWORD");

    hv::HomeVaultClient hv_client(hostname, username, password);

    CLISetup::SetupSubcommands(app, hv_client);

    CLI11_PARSE(app, argc, argv);

    return EXIT_SUCCESS;
}
