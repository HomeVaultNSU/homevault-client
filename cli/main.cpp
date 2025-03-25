#include <cstdlib>

#include <CLI/CLI.hpp>


void do_list()
{

}

void setup_list_subcommand(CLI::App& app)
{
    CLI::App *list = app.add_subcommand("list", "List all available files on server");
    list->callback(do_list);
}

void do_upload()
{

}

void setup_upload_subcommand(CLI::App& app)
{
    CLI::App *upload = app.add_subcommand("upload", "Upload file to server");
    upload->callback(do_upload);
}

void do_download()
{

}

void setup_download_subcommand(CLI::App& app)
{
    CLI::App *upload = app.add_subcommand("upload", "Upload file to server");
    upload->callback(do_upload);
}

int main(int argc, char *argv[])
{
    CLI::App app{"homevault-cli"};
    app.require_subcommand(1);

    setup_list_subcommand(app);
    setup_upload_subcommand(app);
    setup_download_subcommand(app);

    CLI11_PARSE(app, argc, argv);

    return EXIT_SUCCESS;
}
