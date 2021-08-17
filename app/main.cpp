#include <render/render.h>

#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    // config. options
    cxxopts::Options options("nevk [MODEL PATH] [MTL PATH]", "commands");

    options.add_options()("m, mesh", "mesh path", cxxopts::value<std::string>()->default_value("")) //misc/Cube/Cube.gltf
        ("t, texture", "texture path", cxxopts::value<std::string>()->default_value("misc/"))
            ("width", "window width", cxxopts::value<uint32_t>()->default_value("800"))
                ("height", "window height", cxxopts::value<uint32_t>()->default_value("600"))
                    ("perfTest", "perf test mode", cxxopts::value<bool>()->default_value("false"))
                        ("framesDelay", "frames delay", cxxopts::value<uint32_t>()->default_value("0"))
                            ("framesReport", "frames report", cxxopts::value<uint32_t>()->default_value("0"))
                                ("h, help", "Print usage");

    options.parse_positional({ "m", "t" });
    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    // check params
    std::string mesh(result["m"].as<std::string>());
    std::string texture(result["t"].as<std::string>());
    if (!fs::exists(mesh) && !mesh.empty())
    {
        std::cerr << "mesh file doesn't exist";
        exit(0);
    }
    if (!fs::exists(texture))
    {
        std::cerr << "texture file doesn't exist";
        exit(0);
    }

    // initialise & run render
    Render r;

    r.MODEL_PATH = mesh;
    r.MTL_PATH = texture;
    r.WIDTH = result["width"].as<uint32_t>();
    r.HEIGHT = result["height"].as<uint32_t>();
    r.perfTestMode = result["perfTest"].as<bool>();
    r.framesDelay = result["framesDelay"].as<uint32_t>();
    r.framesReport = result["framesReport"].as<uint32_t>();

    r.run();

    return 0;
}
