#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Error.hpp>
#include <cstdlib>
#include <iostream>

void print_list() { std::cout << "listuploaded\n"; }

void do_upload() { std::cout << "upload done\n"; }

void do_download() {}

void SetupSubcommands(CLI::App &app) {
  app.add_subcommand("list", "List all available files on server")
      ->callback(print_list);
  app.add_subcommand("upload", "Upload file to server")->callback(do_upload);
  app.add_subcommand("download", "Download file from server")
      ->callback(do_download);
}

/*
 *TODO:
 - how to structure the code files (will main.cpp be outside the cli, core etc.
 directories?)
 - will main.cpp be the entry point?
 - what will be the api between microservices? (will there be an object core
 that will have method upload or smth? how will it all look like? what's the
 structure?)
 * */

int main(int argc, char *argv[]) {
  CLI::App app{"homevault-cli"};
  app.require_subcommand(1);

  SetupSubcommands(app);

  CLI11_PARSE(app, argc, argv);

  return EXIT_SUCCESS;
}
