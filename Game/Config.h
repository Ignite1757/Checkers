#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config // processing game settings
{
  public:
    Config() // creating settings
    {
        reload();
    }

    void reload() // loading settings from file
    {
        std::ifstream fin(project_path + "settings.json"); // path to the .json game settings file
        fin >> config;
        fin.close();
    }

    auto operator()(const string &setting_dir, const string &setting_name) const // getting setting from setting_dir and setting_name
    {
        return config[setting_dir][setting_name];
    }

  private: // save
    json config;
};
