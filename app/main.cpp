#include <iostream>

#include <render/render.h>
#include <cxxopts.hpp>

int main(int argc, char** argv)
{
    cxxopts::Options options( "NeVK [MODEL PATH] [MTL PATH]", "commands");

    options.add_options()
        ("m, mesh", "mesh path", cxxopts::value<std::string>()->default_value("misc/Cube/Cube.gltf"))
        ("t, texture", "texture path", cxxopts::value<std::string>()->default_value("misc/"))
        ("h, help", "Print usage")
        ;

    options.parse_positional({"m", "t"});
    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    Render r(result["m"].as<std::string>(), result["t"].as<std::string>());
    r.run();

    return 0;
}
