#ifndef CLISETUP_HPP
#define CLISETUP_HPP

#include <CLI/App.hpp>
#include <core/Homevault.hpp>
#include <string>

namespace CLISetup
{

struct CLIStorage
{
    std::string listPath;
    int depth;

    CLIStorage() : listPath("") {}
};

void SetupSubcommands(CLI::App& app, hv::Homevault& server_connection,
                      CLISetup::CLIStorage& cliStorage);
}  // namespace CLISetup

#endif // !CLISETUP_HPP
