#include "config.h"

#include <fstream>

#include <ghc/fs_std.hpp>

using namespace std;
using json = nlohmann::json;

namespace SpiceQL {

  Config::Config() {
    string dbPath = getConfigDirectory(); 
    vector<string> json_paths = glob(dbPath, basic_regex("json"));

    for(const fs::path &p : json_paths) {
      ifstream i(p);
      json j;
      i >> j;
      for (auto it = j.begin(); it != j.end(); ++it) {
        config[it.key()] = it.value();
      }
    }
  }


  Config::Config(string j) {
    std::ifstream ifs(j);
    config = json::parse(ifs);
  }

  
  Config::Config(json j) {
    config = j;
  }


  Config Config::operator[](string pointer) {
    if (pointer.at(0) != '/') {
      pointer = "/"+pointer; 
    }

    json::json_pointer p(pointer);
    
    Config conf(config[p]);
    return conf;
  }


  json Config::evaluateJson(json eval_json, bool merge) {
    resolveConfigDependencies(eval_json, config);

    vector<json::json_pointer> json_to_eval = SpiceQL::findKeyInJson(eval_json, "kernels", true);

    for (auto pointer:json_to_eval) {
      vector<string> res = getPathsFromRegex(getDataDirectory(), eval_json[pointer]);
      eval_json[pointer] = res;
    }

    if (merge) {
      config.merge_patch(eval_json);
    }

    return eval_json;
  }


  unsigned int Config::size() {
    return config.size();
  }


  json Config::getJson(string pointer, bool merge) {
    json eval_json;
    json::json_pointer p(pointer);

    eval_json[p] = config[p];

    return evaluateJson(eval_json, merge);
  }


  json Config::getJson() {
    return evaluateJson(config, true);
  }


  json Config::getRawConfig() {
    return config;
  }


  vector<string> Config::findKey(string key, bool recursive) {
    vector<string> pointers;

    for(auto &e : SpiceQL::findKeyInJson(config, key, recursive)) {
      pointers.push_back(e.to_string());
    }

    return pointers; 
  }
}
