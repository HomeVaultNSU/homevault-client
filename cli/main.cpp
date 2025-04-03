#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Error.hpp>
#include <core/ServerConnection.hpp>
#include <cstdlib>
#include <string>

#include "hv/cli/CLISetup.hpp"

int main(int argc, char *argv[])
{
    CLI::App app{"homevault-cli"};
    app.require_subcommand(1);

    std::string ip("127.0.0.1");

    hv::ServerConnection server_connection(ip);

    CLISetup::SetupSubcommands(app, server_connection);

    CLI11_PARSE(app, argc, argv);

    return EXIT_SUCCESS;
}
