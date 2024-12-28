#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config // обработка настроек игры
{
  public:
    Config() // создание настроек
    {
        reload();
    }

    void reload() // загрузка настроек из файла
    {
        std::ifstream fin(project_path + "settings.json"); // путь до .json файла настроек игры
        fin >> config;
        fin.close();
    }

    auto operator()(const string &setting_dir, const string &setting_name) const // получение настройки из setting_dir и setting_name
    {
        return config[setting_dir][setting_name];
    }

  private: // сохранение
    json config;
};
