/**
  * @file
  *
  *
  *
  *
 **/
#include <fstream>
#include <algorithm>

#include <SpiceUsr.h>

#include <ghc/fs_std.hpp>

#include "query.h"
#include "spice_types.h"
#include "utils.h"

using json = nlohmann::json;
using namespace std;

namespace SpiceQL {


 std::string getKernelStringValue(std::string key) {
   // check to make sure the key exists when calling findKeyWords(key)
   if (findKeywords(key).contains(key)){
      json results = findKeywords(key);
      return results[key] ;
    }
    // throw exception
    else{
      throw std::invalid_argument("key not in results");
    }
  }

  std::vector<string> getKernelVectorValue(std::string key) {

    // check to make sure the key exists when calling findKeyWords(key)
    if (findKeywords(key).contains(key)){

      // get json results of key
      json results = findKeywords(key);
      vector<string> kernelValues;

      // iterate over results @ key
      for(auto i : results[key]){
        // push values to vector
        kernelValues.push_back(to_string(i));
      }
      return kernelValues;
    }
    // throw exception
    else{
      throw std::invalid_argument("key not in results");
    }
  }



  string getLatestKernel(vector<string> kernels) {
    if(kernels.empty()) {
      throw invalid_argument("Can't get latest kernel from empty vector");
    }

    string extension = static_cast<fs::path>(kernels.at(0)).extension();
    // ensure everything is different versions of the same file
    for(const fs::path &k : kernels) {
      if (k.extension() != extension) {
        throw invalid_argument("The input paths do are not different versions of the same file");
      }
    }

    return *(max_element(kernels.begin(), kernels.end()));
  }


  json getLatestKernels(json kernels) {
    // the kernels group is now the conf with
    for(auto &kernelType: {"ck", "spk", "tspk", "fk", "ik", "iak", "pck", "lsk"}) {
        vector<json::json_pointer> catPointers = findKeyInJson(kernels, kernelType, true);
        for(auto &p : catPointers) {
          for(auto qual: Kernel::QUALITIES) {
            if(!kernels[p].contains(qual)){
              continue;
            }

            std::vector<string> l = jsonArrayToVector(kernels[p][qual]["kernels"]);
            fs::path latest;

            if (!l.empty()) {
              latest = getLatestKernel(l);
              kernels[p][qual]["kernels"] = latest;
            }
          }

          if(kernels[p].contains("kernels")) {
            vector<string> k = jsonArrayToVector(kernels[p]["kernels"]);
            if(!k.empty()) {
              fs::path latest = getLatestKernel(k);
              kernels[p]["kernels"] = latest;
            }
          }
        }
    }

    vector<json::json_pointer> pointers = findKeyInJson(kernels, "sclk", true);
    for(auto &p : pointers) {
      if(kernels.at(p).contains("kernels")) {
        p /= "kernels";
      }

      vector<string> k = jsonArrayToVector(kernels[p]);
      if(!k.empty()) {
        fs::path latest = getLatestKernel(k);
        kernels[p] = latest;
      }
    }

    return kernels;
  }


  json globKernels(string root, json conf, string kernelType) {
    vector<json::json_pointer> pointers = findKeyInJson(conf, kernelType, true);

    json ret;

    // iterate pointers
    for(auto pointer : pointers) {
      json category = conf[pointer];

      if (category.contains("kernels")) {
        ret[pointer]["kernels"] = getPathsFromRegex(root, category.at("kernels"));
      }

      if (category.contains("deps")) {
        if (category.at("deps").contains("sclk")) {
          ret[pointer]["deps"]["sclk"] = getPathsFromRegex(root, category.at("deps").at("sclk"));
        }
        if (category.at("deps").contains("pck")) {
          ret[pointer]["deps"]["pck"] = getPathsFromRegex(root, category.at("deps").at("pck"));
        }
        if (category.at("deps").contains("objs")) {
          ret[pointer]["deps"]["objs"] = category.at("deps").at("objs");
        }
      }

      // iterate over potential qualities
      for(auto qual: Kernel::QUALITIES) {
        if(!category.contains(qual)) {
          continue;
        }

        ret[pointer][qual]["kernels"] = getPathsFromRegex(root, category[qual].at("kernels"));

        if (category[qual].contains("deps")) {
          if (category[qual].at("deps").contains("sclk")) {
            ret[pointer][qual]["deps"]["sclk"] = getPathsFromRegex(root, category[qual].at("deps").at("sclk"));
          }
          if (category[qual].at("deps").contains("pck")) {
            ret[pointer][qual]["deps"]["pck"] = getPathsFromRegex(root, category[qual].at("deps").at("pck"));
          }
          if (category[qual].at("deps").contains("objs")) {
            ret[pointer][qual]["deps"]["objs"] = category[qual].at("deps").at("objs");
          }
        }
      }
    }

    return  ret.empty() ? "{}"_json : ret;
  }


  json searchMissionKernels(string root, json conf) {
    json kernels;

    // the kernels group is now the conf with
    for(auto &kernelType: {"ck", "spk", "tspk", "fk", "ik", "iak", "pck", "lsk", "sclk"}) {
        kernels.merge_patch(globKernels(root, conf, kernelType));
    }
    return kernels;
  }


  json searchMissionKernels(json kernels, std::vector<double> times, bool isContiguous)  {
    json reducedKernels;

    // Load any SCLKs in the config
    vector<KernelSet> sclkKernels;
    for (auto &p : findKeyInJson(kernels, "sclk", true)) {
      sclkKernels.push_back(KernelSet(kernels[p]));
    }

    vector<json::json_pointer> ckpointers = findKeyInJson(kernels, "ck", true);
    vector<json::json_pointer> spkpointers = findKeyInJson(kernels, "spk", true);
    vector<json::json_pointer> pointers(ckpointers.size() + spkpointers.size());
    merge(ckpointers.begin(), ckpointers.end(), spkpointers.begin(), spkpointers.end(), pointers.begin());

    json newKernels = json::array();

    // refine cks for every instrument/category
    for (auto &p : pointers) {
      json cks = kernels[p];
      if(cks.is_null() ) {
        continue;
      }

      for(auto qual: Kernel::QUALITIES) {
        if(!cks.contains(qual)) {
          continue;
        }

        json ckQual = cks[qual]["kernels"];
        newKernels = json::array();

        for(auto &kernel : ckQual) {
          vector<pair<double, double>> intervals = getTimeIntervals(kernel);
          for(auto &interval : intervals) {
            auto isInRange = [&interval](double d) -> bool {return d >= interval.first && d <= interval.second;};

            if (isContiguous && all_of(times.cbegin(), times.cend(), isInRange)) {
              newKernels.push_back(kernel);
            }
            else if (any_of(times.cbegin(), times.cend(), isInRange)) {
              newKernels.push_back(kernel);
            }
          } // end of searching intervals
        } // end  of searching kernels

        reducedKernels[p/qual/"kernels"] = newKernels;
        reducedKernels[p]["deps"] = kernels[p]["deps"];
      }
    }
    kernels.merge_patch(reducedKernels);
    return kernels;
  }

  json searchMissionKernels(json conf) {
    fs::path root = getDataDirectory();
    return searchMissionKernels(root, conf);
  }


  vector<string> getKernelList(json kernels) {
    vector<json::json_pointer> pointers = findKeyInJson(kernels, "kernels");

    vector<string> kernelVect;

    for (auto & p : pointers) {
      vector<string> k = jsonArrayToVector(kernels[p]);
      kernelVect.insert(kernelVect.end(), k.begin(), k.end());
    }    

    return kernelVect;
  }


  set<string> getKernelSet(json kernels) {
    vector<json::json_pointer> pointers = findKeyInJson(kernels, "kernels");

    set<string> kernelSet;

    for (auto & p : pointers) {
      vector<string> ks = jsonArrayToVector(kernels[p]);
      for (auto &k : ks) {
        kernelSet.emplace(k);
      }
    }    

    return kernelSet;
  }

}
