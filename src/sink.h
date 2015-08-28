#ifndef CYCAMORE_SRC_SINK_H_
#define CYCAMORE_SRC_SINK_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "cyclus.h"
#include "cycamore_version.h"
#include "behavior_functions.h"

namespace cycamore {

class Context;

/// This facility acts as a sink of materials and products with a fixed
/// throughput (per time step) capacity and a lifetime capacity defined by a
/// total inventory size.  The inventory size and throughput capacity both
/// default to infinite. If a recipe is provided, it will request material with
/// that recipe. Requests are made for any number of specified commodities.
class Sink : public cyclus::Facility  {
 public:
  Sink(cyclus::Context* ctx);

  virtual ~Sink();

  virtual std::string version() { return CYCAMORE_VERSION; }

  #pragma cyclus note { \
    "doc": \
    " A sink facility that accepts materials and products with a fixed\n"\
    " throughput (per time step) capacity and a lifetime capacity defined by\n"\
    " a total inventory size. The inventory size and throughput capacity\n"\
    " both default to infinite. If a recipe is provided, it will request\n"\
    " material with that recipe. Requests are made for any number of\n"\
    " specified commodities.\n" \
    }

  #pragma cyclus decl

  virtual std::string str();

  virtual void Tick();

  virtual void Tock();

  /// @brief SinkFacilities request Materials of their given commodity. Note
  /// that it is assumed the Sink operates on a single resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
      GetMatlRequests();

  /// @brief SinkFacilities request Products of their given
  /// commodity. Note that it is assumed the Sink operates on a single
  /// resource type!
  virtual std::set<cyclus::RequestPortfolio<cyclus::Product>::Ptr>
      GetGenRsrcRequests();

  /// @brief Change preference for requests from this facility as
  /// defined by input (user_prefs state var)
  virtual void AdjustMatlPrefs(cyclus::PrefMap<cyclus::Material>::type& prefs);
 
  /// @brief SinkFacilities place accepted trade Materials in their Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief SinkFacilities place accepted trade Materials in their Inventory
  virtual void AcceptGenRsrcTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Product>,
      cyclus::Product::Ptr> >& responses);

  ///  add a commodity to the set of input commodities
  ///  @param name the commodity name
  inline void AddCommodity(std::string name) { in_commods.push_back(name); }

  /// @return the current inventory storage size
  inline double InventorySize() const { return inventory.quantity(); }

  /// determines the amount to request
  double RequestAmt() const {
    double desired_amt = RNG_NormalDist(avg_qty, sigma, rng_seed);
    return std::min(desired_amt, std::max(0.0, inventory.space()));
  }
  

  /// @return the input commodities
  inline const std::vector<std::string>&
      input_commodities() const { return in_commods; }

 private:
  /// all facilities must have at least one input commodity
  #pragma cyclus var {"tooltip": "input commodities", \
                      "doc": "commodities that the sink facility accepts", \
                      "uilabel": "List of Input Commodities", \
                      "uitype": ["oneormore", "incommodity"]}
  std::vector<std::string> in_commods;

  #pragma cyclus var {"default": "", "tooltip": "requested composition", \
                      "doc": "name of recipe to use for material requests, " \
                             "where the default (empty string) is to accept " \
                             "everything", \
                       "uilabel": "Input Recipe", \
                      "uitype": "recipe"}
  std::string recipe_name;
 
  //***
  #pragma cyclus var {"default": "None", "tooltip": "social behavior",	\
                          "doc": "type of social behavior used in trade " \
                                 "decisions: None, Every, Random, Reference " \
                                 "where behav_interval describes the " \
                                 "time interval for behavior action." \
                                 "Reference queries the RNG to mimic Random, "\
                                 "but returns zero material qty to trade."}
  std::string social_behav;

  #pragma cyclus var {"default": 0, "tooltip": "interval for behavior" ,\
                      "doc": "interval of social behavior: Every or "\
                             "EveryRandom.  If 0 then behavior is not " \
                             "implemented"}
  double behav_interval;

  #pragma cyclus var {"default": 1, "tooltip": "user-defined preference" , \
                      "doc": "change the default preference for requests "\
                             "from this agent"}
  int user_pref;

#pragma cyclus var {"default": 0, "tooltip": "defines RNG seed",\
                        "doc": "seed on current system time if set to -1," \
                               " otherwise seed on number defined"}
  int rng_seed;

  #pragma cyclus var {"default": 1e299, "tooltip": "sink avg_qty",	\
                          "doc": "mean for the normal distribution that " \
                                 "is sampled to determine the amount of " \
                                 "material actually requested at each " \
                                 "time step"}
  double avg_qty;

  #pragma cyclus var {"default": 0, "tooltip": "standard deviation",	\
                          "doc": "standard deviation (FWHM) of the normal " \
                                 "distribution used to generate requested " \
                                 "amount of material (avg_qty)" }
  double sigma; 

  #pragma cyclus var {"default": 0,					\
                      "tooltip": "time to being allowing trades",\
                          "doc": "At all timesteps before this value, the "   \
                                 "facility does make material requests. At " \
                                 "times at or beyond this value, requests are "\
                                 "made subject to the other behavioral " \
                                 "features available in this archetype"  }
  double t_trade;   //*** 

  /// max inventory size
  #pragma cyclus var {"default": 1e299, \
                      "tooltip": "sink maximum inventory size", \
                      "uilabel": "Maximum Inventory", \
                      "doc": "total maximum inventory size of sink facility"}
  double max_inv_size;

  /// monthly acceptance capacity
  #pragma cyclus var {"default": 1e299, "tooltip": "sink capacity", \
                      "uilabel": "Maximum Throughput", \
                      "doc": "capacity the sink facility can " \
                             "accept at each time step"}
  double capacity;

  /// this facility holds material in storage.
  #pragma cyclus var {'capacity': 'max_inv_size'}
  cyclus::toolkit::ResBuf<cyclus::Resource> inventory;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SINK_H_
