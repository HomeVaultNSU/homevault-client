#pragma once
#include <CLI/App.hpp>
#include <core/ServerConnection.hpp>

namespace CLISetup
{
void SetupSubcommands(CLI::App& app, hv::ServerConnection& server_connection);
}