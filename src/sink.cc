// Implements the Sink class
#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "sink.h"
#include "behavior_functions.h"

namespace cycamore {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sink::Sink(cyclus::Context* ctx)
    : cyclus::Facility(ctx),
      social_behav(""), //***
      behav_interval(0), //***
      user_pref(0), //***
      sigma(0), //***
      max_inv_size(1e299) {}  // actually only used in header file


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Sink::~Sink() {}

#pragma cyclus def schema cycamore::Sink

#pragma cyclus def annotations cycamore::Sink

#pragma cyclus def infiletodb cycamore::Sink

#pragma cyclus def snapshot cycamore::Sink

#pragma cyclus def snapshotinv cycamore::Sink

#pragma cyclus def initinv cycamore::Sink

#pragma cyclus def clone cycamore::Sink

#pragma cyclus def initfromdb cycamore::Sink

#pragma cyclus def initfromcopy cycamore::Sink

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string Sink::str() {
  using std::string;
  using std::vector;
  std::stringstream ss;
  ss << cyclus::Facility::str();

  string msg = "";
  msg += "accepts commodities ";
  for (vector<string>::iterator commod = in_commods.begin();
       commod != in_commods.end();
       commod++) {
    msg += (commod == in_commods.begin() ? "{" : ", ");
    msg += (*commod);
  }
  msg += "} until its inventory is full at ";
  ss << msg << inventory.capacity() << " kg.";
  return "" + ss.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
Sink::GetMatlRequests() {
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;
  using cyclus::Composition;

  std::set<RequestPortfolio<Material>::Ptr> ports;

  // Want opposite behavior of EF.  Return EMPTY port if
  // conditions are not met.
  if (social_behav == "Every" && behav_interval > 0) {
    int cur_time = context()->time();
    if (!EveryXTimestep(cur_time, behav_interval)) // HEU every X time
      {
	return ports; 
      }
  }
  else if (social_behav == "Random" && behav_interval > 0) {
    if (!EveryRandomXTimestep(behav_interval)) // HEU randomly one in X times
      {
	return ports; 
      }
  }
  
  // if NOT social behavior, then respond to all requests
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  double amt = RequestAmt();
  Material::Ptr mat;

  if (recipe_name.empty()) {
    mat = cyclus::NewBlankMaterial(amt);
  } else {
    Composition::Ptr rec = this->context()->GetRecipe(recipe_name);
    mat = cyclus::Material::CreateUntracked(amt, rec); 
  } 

  if (amt > cyclus::eps()) {
    std::vector<std::string>::const_iterator it;
    std::vector<Request<Material>*> mutuals;
    for (it = in_commods.begin(); it != in_commods.end(); ++it) {
      mutuals.push_back(port->AddRequest(mat, this, *it));
    }
    port->AddMutualReqs(mutuals);
    ports.insert(port);
  }  // if amt > eps

  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Product>::Ptr>
Sink::GetGenRsrcRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Product;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Product>::Ptr> ports;
  RequestPortfolio<Product>::Ptr
      port(new RequestPortfolio<Product>());
  double amt = RequestAmt();

  if (amt > cyclus::eps()) {
    CapacityConstraint<Product> cc(amt);
    port->AddConstraint(cc);

    std::vector<std::string>::const_iterator it;
    for (it = in_commods.begin(); it != in_commods.end(); ++it) {
      std::string quality = "";  // not clear what this should be..
      Product::Ptr rsrc = Product::CreateUntracked(amt, quality);
      port->AddRequest(rsrc, this, *it);
    }

    ports.insert(port);
  }  // if amt > eps

  return ports;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::AdjustMatlPrefs(
  cyclus::PrefMap<cyclus::Material>::type& prefs) {

  using cyclus::Bid;
  using cyclus::Material;
  using cyclus::Request;

  cyclus::PrefMap<cyclus::Material>::type::iterator reqit;

  for (reqit = prefs.begin(); reqit != prefs.end(); ++reqit) {
    std::map<Bid<Material>*, double>::iterator mit;
    for (mit = reqit->second.begin(); mit != reqit->second.end(); ++mit) {
      mit->second = user_pref;
    }
  }
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::AcceptMatlTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                                 cyclus::Material::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory.Push(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::AcceptGenRsrcTrades(
    const std::vector< std::pair<cyclus::Trade<cyclus::Product>,
                                 cyclus::Product::Ptr> >& responses) {
  std::vector< std::pair<cyclus::Trade<cyclus::Product>,
                         cyclus::Product::Ptr> >::const_iterator it;
  for (it = responses.begin(); it != responses.end(); ++it) {
    inventory.Push(it->second);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::Tick() {
  using std::string;
  using std::vector;
  LOG(cyclus::LEV_INFO3, "SnkFac") << prototype() << " is ticking {";

  double requestAmt = RequestAmt();
  // inform the simulation about what the sink facility will be requesting
  if (requestAmt > cyclus::eps()) {
    for (vector<string>::iterator commod = in_commods.begin();
         commod != in_commods.end();
         commod++) {
      LOG(cyclus::LEV_INFO4, "SnkFac") << " will request " << requestAmt
                                       << " kg of " << *commod << ".";
    }
  }
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Sink::Tock() {
  LOG(cyclus::LEV_INFO3, "SnkFac") << prototype() << " is tocking {";

  // On the tock, the sink facility doesn't really do much.
  // Maybe someday it will record things.
  // For now, lets just print out what we have at each timestep.
  LOG(cyclus::LEV_INFO4, "SnkFac") << "Sink " << this->id()
                                   << " is holding " << inventory.quantity()
                                   << " units of material at the close of month "
                                   << context()->time() << ".";
  LOG(cyclus::LEV_INFO3, "SnkFac") << "}";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructSink(cyclus::Context* ctx) {
  return new Sink(ctx);
}

}  // namespace cycamore
