#ifndef CLISETUP_HPP
#define CLISETUP_HPP

#include <CLI/App.hpp>
#include <core/Homevault.hpp>
#include <string>
#include <vector>

namespace CLISetup
{
struct ListStorage
{
    int depth;
    std::string listPath;

    ListStorage() : listPath("") {}
};

struct UploadStorage
{
    std::vector<std::string> files;
};

struct DownloadStorage
{
    std::vector<std::string> files;
};

void SetupListSubcommand(CLI::App& app, hv::Homevault& hvClient,
                         CLISetup::ListStorage& listStorage);

void SetupUploadSubcommand(CLI::App& app, hv::Homevault& hvClient,
                           CLISetup::UploadStorage& uploadStorage);

void SetupDownloadSubcommand(CLI::App& app, hv::Homevault& hvClient,
                             CLISetup::DownloadStorage& downloadStorage);
}  // namespace CLISetup

#endif  // !CLISETUP_HPP
