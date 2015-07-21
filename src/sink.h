#ifndef CYCAMORE_SRC_SINK_H_
#define CYCAMORE_SRC_SINK_H_

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "cyclus.h"
#include "behavior_functions.h"

namespace cycamore {

class Context;

///   @class Sink
///   This cyclus::Facility requests a finite amount of its input commodity.
///  It offers nothing.
///
///  The Sink class inherits from the cyclus::Facility class and is
///  dynamically loaded by the Agent class when requested.
///
///  @section intro Introduction
///  The Sink is a facility type in *Cyclus* capable of accepting
///  a finite or infinite quantity of some commodity produced in the
///  simulation. A Sink requests an amount of that commodity from
///  the appropriate market. It then receives that commodity when the
///  market issues an order that the request has been matched with a
///  corresponding offer.
///  @section agentparams Agent Parameters
///  Sink behavior is comprehensively defined by the following
///  parameters:
///  - double capacity: The acceptance avg_qty of the facility (units
///  vary, but typically kg/month). Avg_Qty is infinite if a positive
///  value is provided.
///  - int startDate: The date on which the facility begins to operate
///  (months). - int lifeTime: The length of time that the facility
///  operates (months). - std::string inCommod: The commodity type this
///  facility accepts.
///  @section optionalparams Optional Parameters
///  Sink behavior may also be specified with the following
///  optional parameters which have default values listed here.
///  - double avg_qtyFactor: The ratio of actual acceptance avg_qty to
///  the rated acceptance avg_qty. Default is 1 (actual/rated).
///  - double AvailFactor: The percent of time the facility operates at
///  its avg_qty factor. Default is 100%.
///  - double capitalCost: The cost of constructing and commissioning this
///  facility. Default is 0 ($).
///  - double opCost: The annual cost of operation and maintenance of this
///  facility. Default is 0 ($/year).
///  - int constrTime: The number of months it takes to construct and
///  commission this facility. Default is 0 (months).
///  - int decomTime: The number of months it takes to deconstruct and
///  decommission this facility. Default is 0 (months).
///  - Inst* inst: The institution responsible for this facility.
///  - string name: A non-generic name for this facility.
///
///  @section detailed Detailed Behavior
///  @subsection finite If Finite Avg_Qty:
///  The Sink starts operation when the simulation reaches the
///  month specified as the startDate. It immediately begins to request
///  the inCommod commodity type at the rate defined by the Sink
///  avg_qty. If a request is matched with an offer from another
///  facility, the Sink executes that order by adding that
///  quantity to its stocks. When the simulation time equals the startDate
///  plus the lifeTime, the facility ceases to operate.
///
///  @subsection infinite If Infinite Avg_Qty:
///  The Sink starts operation when the simulation reaches the
///  month specified as the startDate. Each month the Sink
///  requests an infinite amount of the inCommod commodity from the
///  appropriate market. If there is a corresponding offer for that
///  commodity type from another facility, the Sink executes that
///  order by adding that quantity to its stocks. When the simulation time
///  equals the startDate plus the lifeTime, the facility ceases to
///  operate.
///  @subsection question Question:
///  What is the best way to allow requests of an infinite amount of
///  material on a market?
class Sink : public cyclus::Facility  {
 public:
  // --- Module Members ---
  ///  Constructor for the Sink class.
  ///  @param ctx the cyclus context for access to simulation-wide parameters
  Sink(cyclus::Context* ctx);

  ///   Destructor for the Sink class.
  virtual ~Sink();

  #pragma cyclus decl

  #pragma cyclus note {"doc": "A sink facility that accepts specified " \
                              "amounts of commodities from other agents"}

  ///   A verbose printer for the Sink Facility.
  virtual std::string str();
  // ---

  // --- Agent Members ---
  /// The Sink can handle the Tick.

  /// @param time the current simulation time.
  virtual void Tick();

  /// The Sink can handle the Tock.

  /// @param time the current simulation time.
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
  // ---

  // --- Sink Members ---
  ///  add a commodity to the set of input commodities
  ///  @param name the commodity name
  inline void AddCommodity(std::string name) { in_commods.push_back(name); }

  /// @return the current inventory storage size
  inline double InventorySize() const { return inventory.quantity(); }

  // Amount of material to be requested. Re-assessed at each timestep
  // in the Tick
  double amt ;

  /// @return the input commodities
  inline const std::vector<std::string>&
      input_commodities() const { return in_commods; }

 private:
  /// all facilities must have at least one input commodity
  #pragma cyclus var {"tooltip": "input commodities", \
                      "doc": "commodities that the sink facility accepts", \
                      "uitype": ["oneormore", "incommodity"]}
  std::vector<std::string> in_commods;

  #pragma cyclus var {"default": "", "tooltip": "requested composition", \
                      "doc": "name of recipe to use for material " \
                             "requests"}
  std::string recipe_name;

  //***
  #pragma cyclus var {"default": "None", "tooltip": "social behavior",	\
                          "doc": "type of social behavior used in trade " \
                                 "decisions: None, Every, Random " \
                                 "where behav_interval describes the " \
                                 "time interval for behavior action"}
  std::string social_behav;

  #pragma cyclus var {"default": 0, "tooltip": "interval for behavior" ,\
                      "doc": "interval of social behavior: Every or "\
                             "EveryRandom.  If 0 then behavior is not " \
                             "implemented"}
  double behav_interval;

  #pragma cyclus var {"default": 0, "tooltip": "user-defined preference" , \
                      "doc": "change the default preference for requests "\
                             "from this agent"}
  int user_pref;

#pragma cyclus var {"default": 0, "tooltip": "defines RNG seed as constant ",\
                      "doc": "if set to zero or seeded on Time if set to 1."}
  bool time_seed;

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
                      "doc": "total maximum inventory size of sink facility"}
  double max_inv_size;

  /// this facility holds material in storage.
  #pragma cyclus var {'capacity': 'max_inv_size'}
  cyclus::toolkit::ResourceBuff inventory;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SINK_H_
