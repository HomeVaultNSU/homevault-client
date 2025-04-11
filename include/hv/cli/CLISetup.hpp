#pragma once
#include <CLI/App.hpp>
#include <core/HomeVaultClient.hpp>
#include <string>

namespace CLISetup
{

struct CLIStorage
{
    std::string listPath;

    CLIStorage() : listPath("") {}
};
void SetupSubcommands(CLI::App& app, hv::HomeVaultClient& server_connection,
                      CLISetup::CLIStorage& cliStorage);
}  // namespace CLISetup
