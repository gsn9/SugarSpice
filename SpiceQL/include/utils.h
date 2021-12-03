/**
 * @file
 *
 *
 **/
#pragma once

#include <iostream>
#include <regex>
#include <optional>

#include <nlohmann/json.hpp>

#include "spice_types.h"

/**
 * @namespace SpiceQL
 *
 */
namespace SpiceQL {

  /**
   * @brief force a string to upper case
   *
   * @param s input string
   * @return copy of input string, in upper case
   */
  std::string toUpper(std::string s);


  /**
   * @brief force a string to lower case
   *
   * @param s input string
   * @return copy of input string, in lower case
   */
  std::string toLower(std::string s);


  /**
   * @brief find and replace one substring with another
   *
   * @param str input string to search
   * @param from substring to find
   * @param to substring to replace "from" instances to
   * @return std::string
   */
  std::string replaceAll(std::string str, const std::string &from, const std::string &to);


  /**
    * @brief glob, but with json
    *
    * Lambda for globbing files from a regular expression stored
    * in json. As they can be a single expression or a
    * list, we need to massage the json a little.
    *
    * @param root root path to search
    * @param r json list of regexes
    * @returns vector of paths
   **/
  std::vector<std::string> getPathsFromRegex (std::string root, nlohmann::json r);


  /**
   * @brief Merge two json configs
   *
   * When arrays are merged, the values from the base config will appear
   * first in the merged config.
   *
   * @param baseConfig The config to merge into
   * @param mergingConfig The config to merge into the base config
   */
  void mergeConfigs(nlohmann::json &baseConfig, const nlohmann::json &mergingConfig);


  /**
    * @brief ls, like in unix, kinda. Also it's a function.
    *
    * Iterates the input path and returning a list of files. Optionally, recursively.
    *
    * @param root The root directory to search
    * @param recursive recursively iterates through directories if true
    *
    * @returns list of paths
   **/
  std::vector<std::string> ls(std::string const & root, bool recursive);


  /**
    * @brief glob, like python's glob.glob, except C++
    *
    * Given a root and a regular expression, give all the files that match.
    *
    * @param root The root directory to search
    * @param reg std::regex object to pattern to search, defaults to ".*", or match averything.
    * @param recursive recursively iterates through directories if true
    *
    * @returns list of paths matching regex
   **/
  std::vector<std::string> glob(std::string const & root,
                             std::regex const & reg = std::regex(".*"),
                             bool recursive=false);


  /**
    * @brief Get start and stop times a kernel.
    *
    * For each segment in the kernel, get all start and stop times as a vector of double pairs.
    * This gets all start and stop times regardless of the frame associated with it.
    *
    * Input kernel is assumed to be a binary kernel with time dependant external orientation data.
    *
    * @param kpath Path to the kernel
    * @returns std::vector of start and stop times
   **/
  std::vector<std::pair<double, double>> getTimeIntervals(std::string kpath);


  /**
   * @brief Simple struct for holding target states
   */
  //! @cond Doxygen_Suppress
  struct targetState {double lt; std::array<double,6> starg;};
  //! @endcond

  /**
   * @brief Gives the position and velocity for a given frame at some ephemeris time
   *
   * Mostly a C++ wrap for NAIF's spkezr_c
   *
   * @param et ephemeris time at which you want to optain the target state
   * @param target NAIF ID for the target frame
   * @param observer NAIF ID for the observing frame
   * @param frame The reference frame in which to get the positions in
   * @param abcorr aborration correction flag, default it NONE.
   *        This can set to:
   *           "NONE" - No correction
   *        For the "reception" case, i.e. photons from the target being recieved by the observer at the given time.
   *           "LT"   - One way light time correction
   *           "LT+S" - Correct for one-way light time and stellar aberration correction
   *           "CN"   - Converging Newtonian light time correction
   *           "CN+S" - Converged Newtonian light time correction and stellar aberration correction
   *        For the "transmission" case, i.e. photons emitted from the oberver hitting at target at the given time
   *           "XLT"   - One-way light time correction using a newtonian formulation
   *           "XLT+S" - One-way light time and stellar aberration correction using a newtonian formulation
   *           "XCN"   - converged Newtonian light time correction
   *           "XCN+S" - converged Newtonian light time correction and stellar aberration correction.
   *  @return A TargetState struct with the light time adjustment and a Nx6 state vector of positions and velocities in x,y,z,vx,vy,vz format.
  **/
  targetState getTargetState(double et, std::string target, std::string observer, std::string frame="J2000", std::string abcorr="NONE");


  /**
   * @brief simple struct for holding target orientations
   */
  //! @cond Doxygen_Suppress
  struct targetOrientation {std::array<double,4> quat; std::optional<std::array<double,3>> av;};
  //! @endcond

  /**
   * @brief Gives quaternion and angular velocity for a given frame at a given ephemeris time
   *
   * Orientations for an input frame in some reference frame.
   * The orientations returned from this function can be used to transform a position
   * in the source frame to the ref frame.
   *
   * @param et ephemeris time at which you want to optain the target pointing
   * @param toframe the source frame's NAIF code.
   * @param refframe the reference frame's NAIF code, orientations are relative to this reference frame
   * @returns SPICE-style quaternions (w,x,y,z) and optional angular velocity
  **/
  targetOrientation getTargetOrientation(double et, int toframe, int refframe=1); // use j2000 for default reference frame


  /**
    * @brief finds key:values in kernel pool
    *
    * Given a key template, returns matching key:values from the kernel pool
    *   by using gnpool, gcpool, gdpool, and gipool
    *
    * @param keytpl input key template to search for
    *
    * @returns json list of found key:values
   **/
  nlohmann::json findKeywords(std::string keytpl);


  /**
    * @brief recursively search keys in json.
    *
    * Given a root and a regular expression, give all the files that match.
    *
    * @param in input json to search
    * @param key key to search for
    * @param recursive recursively iterates through objects if true
    *
    * @returns vector of refernces to matching json objects
   **/
  std::vector<nlohmann::json::json_pointer> findKeyInJson(nlohmann::json in, std::string key, bool recursive=true);


  /**
    * @brief get the Kernel type (CK, SPK, etc.)
    *
    *
    * @param kernelPath path to kernel
    * @returns Kernel type as a string
   **/
   std::string getKernelType(std::string kernelPath);


  /**
   * @brief Get the directory pointing to the db files
   * 
   * Default behavior returns the installed DB files in $CONDA_PREFIX/etc/SpiceQL/db.
   *
   * If the env var $SSPICE_DEBUG is set, this returns the local source path of 
   * _SOURCE_PREFIX/SpiceQL/db/ 
   * 
   * @return std::string directory containing db files
   */
  std::string getConfigDirectory();
  

  /**
   * @brief Returns a vector of all the available configs
   * 
   * Returns the db files in either the installed or debug directory depending 
   * on whether or not SSPICE_DEBUG is set. 
   *
   * @see getConfigDirectory
   *
   * @return std::vector<std::string> 
   */
  std::vector<std::string> getAvailableConfigFiles();


  /**
   * @brief Get names of available config files as a json vector
   *
   * This iterates through all the configs in the db folder either installed 
   * or in the debug directory depending on whether or not SSPICE_DEBUG is set. Loads them 
   * as vector of json obects and returns the vector. 
   *
   * @return std::vector<nlohmann::json> 
   */
  std::vector<nlohmann::json> getAvailableConfigs();


  /**
    * @brief Returns the path to the Mission specific Spice config file.
    *
    * Given a mission, search a prioritized list of directories for
    * the json config file. This function checks in the order:
    *
    *   1. The local build dir, i.e. $CMAKE_SOURCE_DIR
    *   2. The install dir, i.e. $CMAKE_PREFIX
    *
    * @param mission mission name of the config file
    *
    * @returns path object of the condig file
   **/
   std::string getMissionConfigFile(std::string mission);


  /**
    * @brief Returns the path to the Mission specific Spice config file.
    *
    * Given a mission, search a prioritized list of directories for
    * the json config file. This function checks in the order:
    *
    *   1. The local build dir, i.e. $CMAKE_SOURCE_DIR
    *   2. The install dir, i.e. $CMAKE_PREFIX
    *
    * @param mission mission name of the config file
    *
    * @returns path object of the config file
   **/
   nlohmann::json getMissionConfig(std::string mission);


   /**
    * @brief resolve the dependencies in a config in place
    *
    * Given a config with "deps" keys in it and a second config to extract
    * dependencies from, recursively resolve all of the deps into their actual
    * values. Only allows up to 10 recurssions.
    *
    * @param config The config to populate
    * @param dependencies The config to pull dependencies from
    *
    * @returns The full instrument config
    */
   void resolveConfigDependencies(nlohmann::json &config, const nlohmann::json &dependencies);


   /**
    * @brief erase a part of a json object based on a json pointer
    *
    * @param j The json object ot erase part of. Modified in place
    * @param ptr The object to erase
    *
    * @returns The number of objects removed
    */
   size_t eraseAtPointer(nlohmann::json &j, nlohmann::json::json_pointer ptr);


  /**
    * @brief Returns std::vector<string> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns string vector containing arr data
   **/
   std::vector<std::string> jsonArrayToVector(nlohmann::json arr);


  /**
    * @brief Returns std::vector<string> interpretation of a json array.
    *
    * Attempts to convert the json array to a C++ array. Also handles
    * strings in cases where one element arrays are stored as scalars.
    * Throws exception if the json obj is not an array.
    *
    * @param arr input json arr
    *
    * @returns string vector containing arr data
   **/
   std::string getDataDirectory();
}
