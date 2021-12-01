#pragma once

#include <iostream>
#include <regex>
#include <optional>

#include <nlohmann/json.hpp>

#include "utils.h"

/**
 * @namespace SpiceQL types
 *
 */
namespace SpiceQL {

  /**
   * @brief Object for interacting with SpiceQL target configs
   * 
   * The config class can wrap multiple config files and give an interface for interacting 
   * with the configs and obtaining kernel lists. 
   * 
   */
  class Config {
    public:

      /**
       * @brief Construct a new Config object
       * 
       * Loads all config files into a config object. 
       */
      Config();


      /**
       * @brief Construct a new Config object from json file
       * 
       * @param json path to json config file
       */
      Config(std::string json);


      /**
       * @brief get a value from the config object
       * 
       * @param pointer json pointer to a key inside the config file
       * @return Config Returns a new config instance representing the sub object
       */
      Config operator[](std::string pointer);


      /**
       * @brief 
       * 
       * @return unsigned int 
       */
      unsigned int size();


      /**
       * @brief Given a conf json object, expands the regeexes to paths 
       * 
       * @param pointer Json key or pointer to expand
       * @param merge if true, merges with the internal 
       * @return nlohmann::json 
       */
      nlohmann::json getJson(std::string pointer, bool merge=false);


      /**
       * @brief returns the kernels 
       * 
       * @return nlohmann::json 
       */
      nlohmann::json getJson();


      /**
       * @brief Get the Global Json object
       * 
       * @return nlohmann::json 
       */
      nlohmann::json getRawConfig();


      /**
       * @brief Search for a key inside the config
       * 
       * @param key a key or json style pointer 
       * @param recursive if true, searches recursively for all matches
       * @return std::vector<nlohmann::json::json_pointer> vector of pointers to where the key was found
       */
      std::vector<std::string> findKey(std::string key, bool recursive);


    private:
      /**
       * @brief Construct a new Config object from a json object
       * 
       * @param json json object representing a target config
       */
      Config(nlohmann::json json);


      /**
       * @brief Given a conf json object, expands the regexes to paths 
       * 
       * @param eval_json Json to expand
       * @param merge if true, merges with the internal 
       * @return nlohmann::json 
       */
      nlohmann::json evaluateJson(nlohmann::json eval_json, bool merge=false);


      //! internal json config
      nlohmann::json config;
  };

}
