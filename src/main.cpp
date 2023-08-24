
#include "App.h"

int main()
{
    std::locale::global(std::locale(".utf8"));
    SetConsoleOutputCP(CP_UTF8);

    std::filesystem::current_path(CURRENT_WORKING_DIR);//setting path
    std::cout << "working path: " << std::filesystem::current_path() << std::endl;

    if(!std::filesystem::exists(".data/"))
    {
        std::filesystem::create_directories(".data");
    }

    try
    {
        App::get()->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Main Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
