#pragma once

#include <unordered_map>

namespace plum
{
    class Config
    {
        public:
            std::unordered_map<std::string, std::string> data;

            Config(const std::string& name);
            template<typename T> T get(const std::string& key, T fallback);
    };
}

