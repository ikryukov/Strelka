#include <render/render.h>

#include <cxxopts.hpp>
#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
    // config. options
    cxxopts::Options options("nevk [MODEL PATH] [MTL PATH]", "commands");

    options.add_options()
        ("m, mesh", "mesh path", cxxopts::value<std::string>()->default_value("misc/Cube/Cube.gltf"))
        ("t, texture", "texture path", cxxopts::value<std::string>()->default_value("misc/"))
                ("width", "window width", cxxopts::value<uint32_t>()->default_value("800"))
                ("height", "window height", cxxopts::value<uint32_t>()->default_value("600"))
                    ("h, help", "Print usage");

    options.parse_positional({ "m", "t" });
    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    // check params
    std::ifstream mesh(result["m"].as<std::string>());
    std::ifstream texture(result["t"].as<std::string>());
    if (mesh.fail())
    {
        std::cerr << "mesh file doesn't exist";
        exit(0);
    }
    if (texture.fail())
    {
        std::cerr << "texture file doesn't exist";
        exit(0);
    }

    // initialise & run render
    Render r;

    r.MODEL_PATH = result["m"].as<std::string>();
    r.MTL_PATH = result["t"].as<std::string>();
    r.WIDTH = result["width"].as<uint32_t>();
    r.HEIGHT = result["height"].as<uint32_t>();

    r.run();

    return 0;
}
