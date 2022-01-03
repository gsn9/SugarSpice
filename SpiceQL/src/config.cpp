#include "config.h"
#include "query.h"

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

  
  Config::Config(json j, string pointer) {
    config = j;
    confPointer = pointer;
  }


  Config Config::operator[](string pointer) {
    if (pointer.at(0) != '/') {
      pointer = "/"+pointer; 
    }

    json::json_pointer p(pointer);
    json::json_pointer pbase(confPointer);
    pointer = (pbase / p).to_string(); 
    
    Config conf(config, pointer);
    return conf;
  }


  json Config::evaluateJson(json eval_json) {
    resolveConfigDependencies(eval_json, config);

    vector<json::json_pointer> json_to_eval = SpiceQL::findKeyInJson(eval_json, "kernels", true);

    for (auto pointer:json_to_eval) {
      vector<string> res = getPathsFromRegex(getDataDirectory(), eval_json[pointer]);
      eval_json[pointer] = res;
    }

    return eval_json;
  }


  unsigned int Config::size() {
    json::json_pointer cpointer(confPointer);
    return config[cpointer].size();
  }


  json Config::getRecursive(string key) {
    vector<string> pointers = findKey(key, true);
    json eval_json;
    
    for (auto &pointer : pointers) {
      json::json_pointer p(pointer);
      json::json_pointer cpointer(confPointer);
      eval_json[p] = config[cpointer / p];
    }

    return evaluateJson(eval_json);
  }


  json Config::getLatestRecursive(string key) {
    vector<string> pointers = findKey(key, true);
    json eval_json;
    
    for (auto &pointer : pointers) {
      json::json_pointer p(pointer);
      json::json_pointer cpointer(confPointer);
      eval_json[p] = config[cpointer / p];
    }

    json res = evaluateJson(eval_json);
    return getLatestKernels(res);
  }


  json Config::get(string pointer) {
    json::json_pointer cpointer(confPointer);

    if (pointer == "") {
      return evaluateJson(config[cpointer]);
    }

    if (pointer.at(0) != '/') {
      pointer = "/"+pointer; 
    }

    json eval_json;
    json::json_pointer p(pointer);

    eval_json[p] = config[p];

    return evaluateJson(eval_json);
  }


  json Config::getLatest(string pointer) {
    json::json_pointer cpointer(confPointer);

    if (pointer == "") {
      json res =  evaluateJson(config[cpointer]);
      return getLatestKernels(res);
    }

    if (pointer.at(0) != '/') {
      pointer = "/"+pointer; 
    }

    json eval_json;
    json::json_pointer p(pointer);

    eval_json[p] = config[cpointer / p];

    json res = evaluateJson(eval_json);
    return getLatestKernels(res);
  }


  json Config::get(vector<string> pointers) {
    json eval_json;
    json::json_pointer cpointer(confPointer);

    for (auto &pointer : pointers) {
      json j = get(cpointer); 
      eval_json.merge_patch(j);
    }
    return eval_json;
  }


  json Config::globalConf() {
    json::json_pointer cpointer(confPointer);
    return config[cpointer];
  }


  vector<string> Config::findKey(string key, bool recursive) {
    json::json_pointer cpointer(confPointer);

    vector<string> pointers;
    json subConf = config[cpointer];
    vector<json::json_pointer> ptrs = SpiceQL::findKeyInJson(subConf, key, recursive);
    for(auto &e : ptrs) {
      pointers.push_back(e.to_string());
    }

    return pointers; 
  }

}
