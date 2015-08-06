#ifndef CYCAMORE_SRC_SEPARATIONS_H_
#define CYCAMORE_SRC_SEPARATIONS_H_

#include <string>
#include <vector>

#include "cyclus.h"

namespace cycamore {

class Context;

/// SepMaterial returns a material object that represents the composition and
/// quantity resulting from the separation of material from mat using the given
/// mass-based efficiencies.  Each key in effs represents a nuclide or element
/// (canonical PyNE form), and each value is the corresponding mass-based
/// separations efficiency for that nuclide or element.  Note that this returns
/// an untracked material that should only be used for its composition and qty
/// - not in any real inventories, etc.
cyclus::Material::Ptr SepMaterial(std::map<int, double> effs,
                                  cyclus::Material::Ptr mat,
				  double ideal_eff);

// std::vector<double> AdjustEfficiencies();
 
/// Separations processes feed material into one or more streams containing
/// specific elements and/or nuclides.  It uses mass-based efficiencies.
///
/// User defined separations streams are specified as groups of
/// component-efficiency pairs where 'component' means either a particular
/// element or a particular nuclide.  Each component's paired efficiency
/// represents the mass fraction of that component in the feed that is
/// separated into that stream.  The efficiencies of a particular component
/// across all streams must sum up to less than or equal to one.  If less than
/// one, the remainining material is sent to a waste inventory and
/// (potentially) traded away from there.
///
/// The facility receives material into a feed inventory that it processes with
/// a specified throughput each time step.  Each output stream has a
/// corresponding output inventory size/limit.  If the facility is unable to
/// reduce its stocks by trading and hits this limit for any of its output
/// streams, further processing/separations of feed material will halt until
/// room is again available in the output streams.
class Separations : public cyclus::Facility {
 public:

  Separations(cyclus::Context* ctx);
  virtual ~Separations(){};

#pragma cyclus note {	    \
    "niche": "separations", \
    "doc": \
      "Separations processes feed material into one or more streams containing"\
      " specific elements and/or nuclides.  It uses mass-based efficiencies." \
      "\n\n" \
      "User defined separations streams are specified as groups of" \
      " component-efficiency pairs where 'component' means either a particular"\
      " element or a particular nuclide.  Each component's paired efficiency" \
      " represents the mass fraction of that component in the feed that is" \
      " separated into that stream.  The efficiencies of a particular " \
      " component across all streams must sum up to less than or equal to one."\
      " If less than one, the remainining material is sent to a waste" \
      " inventory and (potentially) traded away from there." \
      "\n\n" \
      "The facility receives material into a feed inventory that it processes"\
      " with a specified throughput each time step.  Each output stream has a" \
      " corresponding output inventory size/limit.  If the facility is unable"\
      " to reduce its stocks by trading and hits this limit for any of its" \
      " output streams, further processing/separations of feed material will" \
      " halt until room is again available in the output streams." \
      "", \
  }

  virtual void Tick();
  virtual void Tock();
  virtual void EnterNotify();

  virtual void AcceptMatlTrades(const std::vector<std::pair<
      cyclus::Trade<cyclus::Material>, cyclus::Material::Ptr> >& responses);

  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
  GetMatlRequests();

  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr> GetMatlBids(
      cyclus::CommodMap<cyclus::Material>::type& commod_requests);

  virtual void GetMatlTrades(
      const std::vector<cyclus::Trade<cyclus::Material> >& trades,
      std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                            cyclus::Material::Ptr> >& responses);

  virtual std::vector<double> AdjustEfficiencies();
  
  #pragma cyclus clone
  #pragma cyclus initfromcopy
  #pragma cyclus infiletodb
  #pragma cyclus initfromdb
  #pragma cyclus schema
  #pragma cyclus annotations
  #pragma cyclus snapshot
  // the following pragmas are ommitted and the functions are written
  // manually in order to handle the vector of resource buffers:
  //
  //     #pragma cyclus snapshotinv
  //     #pragma cyclus initinv

  virtual cyclus::Inventories SnapshotInv();
  virtual void InitInv(cyclus::Inventories& inv);

 private:
  #pragma cyclus var { \
    "doc": "Ordered list of commodities on which to request feed material to " \
           "separate. Order only matters for matching up with feed commodity " \
           "preferences if specified.", \
    "uilabel": "Feed Commodity List", \
    "uitype": ["oneormore", "incommodity"], \
  }
  std::vector<std::string> feed_commods;

  #pragma cyclus var { \
    "default": [], \
    "uilabel": "Feed Commodity Preference List", \
    "doc": "Feed commodity request preferences for each of the given feed " \
           "commodities (same order)." \
           " If unspecified, default is to use zero for all preferences.", \
  }
  std::vector<double> feed_commod_prefs;

  #pragma cyclus var { \
    "doc": "Name for recipe to be used in feed requests." \
           " Empty string results in use of a dummy recipe.", \
    "uilabel": "Feed Commodity Recipe List", \
    "uitype": "recipe", \
    "default": "", \
  }
  std::string feed_recipe;

  #pragma cyclus var { \
    "doc" : "Maximum amount of feed material to keep on hand.", \
    "uilabel": "Maximum Feed Inventory",                     \
    "units" : "kg", \
  }
  double feedbuf_size;

  #pragma cyclus var { \
    "capacity" : "feedbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> feed;

  #pragma cyclus var { \
    "doc" : "Maximum quantity of feed material that can be processed per time "\
            "step.", \
    "uilabel": "Maximum Separations Throughput", \
    "units": "kg", \
  }
  double throughput;

  #pragma cyclus var { \
    "doc": "Commodity on which to trade the leftover separated material " \
           "stream. This MUST NOT be the same as any commodity used to define "\
           "the other separations streams.", \
    "uitype": "outcommodity", \
    "uilabel": "Leftover Commodity", \
    "default": "default-waste-stream", \
  }
  std::string leftover_commod;

  #pragma cyclus var { \
    "doc" : "Maximum amount of leftover separated material (not included in" \
            " any other stream) that can be stored." \
            " If full, the facility halts operation until space becomes " \
            "available.", \
    "uilabel": "Maximum Leftover Inventory", \
    "default": 1e299, \
    "units": "kg", \
  }
  double leftoverbuf_size;

 #pragma cyclus var { \
    "capacity" : "leftoverbuf_size", \
  }
  cyclus::toolkit::ResBuf<cyclus::Material> leftover;

  #pragma cyclus var { \
    "alias": ["streams", "commod", ["info", "buf_size", ["efficiencies", "comp", "eff"]]], \
    "uitype": ["oneormore", "outcommodity", ["pair", "double", ["oneormore", "nuclide", "double"]]], \
    "uilabel": "Separations Streams and Efficiencies", \
    "doc": "Output streams for separations." \
           " Each stream must have a unique name identifying the commodity on "\
           " which its material is traded," \
           " a max buffer capacity in kg (neg values indicate infinite size)," \
           " and a set of component efficiencies." \
           " 'comp' is a component to be separated into the stream" \
           " (e.g. U, Pu, etc.) and 'eff' is the mass fraction of the" \
           " component that is separated from the feed into this output" \
           " stream. If any stream buffer is full, the facility halts" \
           " operation until space becomes available." \
           " The sum total of all component efficiencies across streams must" \
           " be less than or equal to 1" \
           " (e.g. sum of U efficiencies for all streams must be <= 1).", \
  }
  std::map<std::string, std::pair<double, std::map<int, double> > > streams_;

  /*
  #pragma cyclus var {"default": 0, "tooltip": "defines RNG seed",\
                        "doc": "seed on current system time if set to -1," \
                               " otherwise seed on number defined"}
  int rng_seed;

  #pragma cyclus var {"default": 0,					\
                      "tooltip": "time to being allowing trades",\
                          "doc": "At all timesteps before this value, the "   \
                                 "facility does make material requests. At " \
                                 "times at or beyond this value, requests are "\
                                 "made subject to the other behavioral " \
                                 "features available in this archetype"  }
  double t_trade;    

  #pragma cyclus var {\
    //    "alias": ["streams", ["info", ["avg_efficiency", "sigma", "frequency"]]], \
    "uitype": ["oneormore", "outcommodity", ["map", "string", ["double"]]], \
    "uilabel": "Variation in efficiency for each stream", \
    "doc": "Variation in efficiency for each stream" \
           " Stream names MUST match those defined in Streams_ and include " \
           " Fuel, Diverted, Losses (additional streams not variable)" \
           " Each stream can have a variable amplitude and variable time "  \
           " implementation for its efficiency (assumes all components of " \
           " a given stream have the same efficiency)" \
    " The efficiency listed in Streams_ is a placeholder and will be "	\
	   " adjusted at each timestep based on behavior defined here. " \
	   " Average efficiency is the amplitude around which oscillations " \
	   " occur. Sigma is the width of the distribution function. " \
	   " Frequency is the time-base for irregular changes in amplitude " \
	   " To apply Normal Distribution: [mean, sigma, 1] " \
	   "             Every X Timestep: [mean, 0, freq] " \
	   "        Every Random Timestep: [mean, -1, freq]" \
           " Total efficiency of all strings <= 1.  Covert efficiency is " \
	   " defined here. Loss is fixed [mean, 0, 1].  " \
	   " Natural variation in Fuel is defined, then adjusted to be" \
           " F <= 1-D-L. If F+D+L < 1 then remainder of stream goes to Waste", \
  }
  std::map<std::string, std::vector<double> > eff_variation;

  */ 
  // custom SnapshotInv and InitInv and EnterNotify are used to persist this
  // state var.
  std::map<std::string, cyclus::toolkit::ResBuf<cyclus::Material> > streambufs;
};

}  // namespace cycamore

#endif  // CYCAMORE_SRC_SEPARATIONS_H_
