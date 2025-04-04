#pragma once
#include <CLI/App.hpp>
#include <core/HomeVaultClient.hpp>

namespace CLISetup
{
void SetupSubcommands(CLI::App& app, hv::HomeVaultClient& server_connection);
}
