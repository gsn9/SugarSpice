/**
  * @file
  *
  *
 **/

#include <fmt/format.h>
#include <SpiceUsr.h>

#include <nlohmann/json.hpp>

#include <ghc/fs_std.hpp>

#include "spice_types.h"
#include "query.h"
#include "utils.h"
#include "config.h"

using namespace std;
using json = nlohmann::json;

namespace SpiceQL {

  /**
   * @brief Used here to do reverse lookups of enum stringss
   **/
  template < typename T> pair<bool, int > findInVector(const std::vector<T>  & vecOfElements, const T  & element) {
    pair<bool, int > result;
    auto it = find(vecOfElements.begin(), vecOfElements.end(), element);
    if (it != vecOfElements.end()) {
      result.second = distance(vecOfElements.begin(), it);
      result.first = true;
    }
    else {
      result.first = false;
      result.second = -1;
    }
    return result;
  }


  const std::vector<std::string> Kernel::TYPES =  { "na", "ck", "spk", "tspk",
                                                    "lsk", "mk", "sclk",
                                                    "iak", "ik", "fk",
                                                    "dsk", "pck", "ek"};

  const std::vector<std::string> Kernel::QUALITIES = { "na",
                                                       "predicted",
                                                       "nadir",
                                                       "reconstructed",
                                                       "smithed"};


  string Kernel::translateType(Kernel::Type type) {
    return Kernel::TYPES[static_cast<int>(type)];
  }


  Kernel::Type Kernel::translateType(string type) {
    auto res = findInVector<string>(Kernel::TYPES, type);
    if (res.first) {
      return static_cast<Kernel::Type>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type", type));
  };


  string Kernel::translateQuality(Kernel::Quality qa) {
    return Kernel::QUALITIES[static_cast<int>(qa)];
  }


  Kernel::Quality Kernel::translateQuality(string qa) {
    auto res = findInVector<string>(Kernel::QUALITIES, qa);
    if (res.first) {
      return static_cast<Kernel::Quality>(res.second);
    }

    throw invalid_argument(fmt::format("{} is not a valid kernel type", qa));
  }


  int Kernel::translateFrame(string frame) {
    KernelPool::getInstance();    
    
    SpiceInt code;
    SpiceBoolean found;

    // Get FKs 
    // load all latest quntil we can get smarter about it
    Config c;
    json j = c.getLatestRecursive("fk");
    KernelSet kset(j);

    bodn2c_c(frame.c_str(), &code, &found);

    if (!found) {
      throw invalid_argument(fmt::format("Frame code for frame name \"{}\" not Found", frame));
    }

    return code;
  }


  string Kernel::translateFrame(int frame) {
    KernelPool::getInstance(); 

    SpiceChar name[128];
    SpiceBoolean found;

    bodc2n_c(frame, 128, name, &found);

    if(!found) {
      throw invalid_argument(fmt::format("Frame name for code {} not Found", frame));
    }

    return string(name);
  }


  Kernel::Kernel(string path) {
    this->path = path;
    KernelPool::getInstance().load(path, true);
  }


  Kernel::Kernel(Kernel &other) {
    KernelPool::getInstance().load(other.path);
    this->path = other.path;
  }


  Kernel::~Kernel() {
    KernelPool::getInstance().unload(this->path);
  }


  double utcToEt(string utc) {
      // get lsk kernel
      json conf = getMissionConfig("base");
      conf = globKernels(getDataDirectory(), conf, "lsk");
      Kernel lsk(getLatestKernel(conf.at("base").at("lsk").at("kernels")));

      SpiceDouble et;
      utc2et_c(utc.c_str(), &et);
      return et;
  }


  double sclkToEt(string mission, string sclk) {
      // get lsk kernel
      Config missionConf;
      missionConf = missionConf[mission];

      json sclks = missionConf.getLatestRecursive("sclk");
      KernelSet sclkSet(sclks);

      SpiceDouble et;
      scs2e_c(Kernel::translateFrame(mission), sclk.c_str(), &et);
      return et;
  }


  int KernelPool::load(string path, bool force_refurnsh) {
    int refCount; 

    auto it = refCounts.find(path);

    if (it != refCounts.end()) {
      // it's been furnished before, increment ref count
      it->second += 1;
      refCount = it->second; 

      if (force_refurnsh) {
        furnsh_c(path.c_str());
      } 
    }
    else {  
      // load the kernel and register in onto the kernel map 
      furnsh_c(path.c_str());
      refCounts.emplace(path, 1);
    }

    return refCount;
  }


  int KernelPool::unload(string path) {
    try { 
      int &refcount = refCounts.at(path);
      
      // if the map contains the last copy of the kernel, delete it
      if (refcount == 1) {
        // unfurnsh the kernel
        unload_c(path.c_str());
        refCounts.erase(path);
        return 0;
      }
      else {
        unload_c(path.c_str());
        refcount--;
        
        return refcount;
      }
    }
    catch(out_of_range &e) {
      throw out_of_range(path + " is not a kernel that has been loaded."); 
    }
  }


  unsigned int KernelPool::getRefCount(std::string key) {
    try {
      return refCounts.at(key);
    } catch(out_of_range &e) {
      return 0;
    }
  }


  unordered_map<string, int> KernelPool::getRefCounts() {
    return refCounts;
  }


  KernelPool &KernelPool::getInstance() {
    static KernelPool pool;
    return pool;
  }


  KernelPool::KernelPool() : refCounts() { 
    loadLeapSecondKernel();

    // create aliases for spacecrafts 
    boddef_c("mess", -236); // NAIF uses MESSENGER, we use mess for short
  }


  vector<string> KernelPool::getLoadedKernels() {
    vector<string> res;

    for( const auto& [key, value] : refCounts ) {
      res.emplace_back(key);
    }
    return res;
  }

  void KernelPool::loadClockKernels() { 
    json clocks;

    // if data dir not set, should raise an exception 
    fs::path dataDir = getDataDirectory();

    vector<json> confs = getAvailableConfigs();
    // get SCLKs
    for(auto &j : confs) {
      vector<json::json_pointer> p = findKeyInJson(j, "sclk", true);
      
      if (!p.empty()) {
        json sclks = j[p.at(0)];
        clocks[p.at(0)] = sclks;
      }
    }
  
    clocks = searchMissionKernels(dataDir, clocks);
    clocks = getLatestKernels(clocks);

    vector<json::json_pointer> kpointers = findKeyInJson(clocks, "kernels", true);
    for (auto &p : kpointers) {
        json sclks = clocks[p];
        
        for (auto &e : sclks) { 
          load(e.get<string>());
        }
    }
  }


  void KernelPool::loadLeapSecondKernel() {
    // get the distribution's LSK
    fs::path dbPath = getConfigDirectory();
    string lskPath = dbPath / "kernels" / "naif0011.tls";
    load(lskPath);
  }


  KernelSet::KernelSet(json kernels) {
    this->kernels = kernels; 

    vector<json::json_pointer> pointers = findKeyInJson(kernels, "kernels", true);

    for(auto &p : pointers) { 
      json jkernels = kernels[p]; 
      vector<SharedKernel> res; 
      for(auto & k : jkernels) {
        SharedKernel sk(new Kernel(k));
        res.emplace_back(sk);
      }
      loadedKernels.emplace(p, res);
    } 
  }

  
} 
 