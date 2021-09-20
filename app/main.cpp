#include <render/render.h>

#include <cxxopts.hpp>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    // config. options
    cxxopts::Options options("nevk [MODEL PATH]", "commands");

    options.add_options()
        ("m, mesh", "mesh path", cxxopts::value<std::string>()->default_value("")) //misc/Cube/Cube.gltf
                ("width", "window width", cxxopts::value<uint32_t>()->default_value("800"))
                ("height", "window height", cxxopts::value<uint32_t>()->default_value("600"))
                    ("h, help", "Print usage");

    options.parse_positional({"m"});
    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    // check params
    std::string mesh(result["m"].as<std::string>());
    if (!fs::exists(mesh) && !mesh.empty())
    {
        throw std::runtime_error("mesh file doesn't exist");
    }

    // initialise & run render
    nevk::Render r;

    r.MODEL_PATH = mesh;
    r.WIDTH = result["width"].as<uint32_t>();
    r.HEIGHT = result["height"].as<uint32_t>();

    r.run();

    return 0;
}
