#include "separations.h"
#include "behavior_functions.h"

using cyclus::Material;
using cyclus::Composition;
using cyclus::toolkit::ResBuf;
using cyclus::toolkit::MatVec;
using cyclus::KeyError;
using cyclus::ValueError;
using cyclus::Request;
using cyclus::CompMap;

namespace cycamore {

Separations::Separations(cyclus::Context* ctx) : cyclus::Facility(ctx) {
  cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>(
      "the Separations archetype "
      "is experimental");
}

cyclus::Inventories Separations::SnapshotInv() {
  cyclus::Inventories invs;

  // these inventory names are intentionally convoluted so as to not clash
  // with the user-specified stream commods that are used as the separations
  // streams inventory names.
  invs["leftover-inv-name"] = leftover.PopNRes(leftover.count());
  leftover.Push(invs["leftover-inv-name"]);
  invs["feed-inv-name"] = feed.PopNRes(feed.count());
  feed.Push(invs["feed-inv-name"]);

  std::map<std::string, ResBuf<Material> >::iterator it;
  for (it = streambufs.begin(); it != streambufs.end(); ++it) {
    invs[it->first] = it->second.PopNRes(it->second.count());
    it->second.Push(invs[it->first]);
  }

  return invs;
}

void Separations::InitInv(cyclus::Inventories& inv) {
  leftover.Push(inv["leftover-inv-name"]);
  feed.Push(inv["feed-inv-name"]);

  cyclus::Inventories::iterator it;
  for (it = inv.begin(); it != inv.end(); ++it) {
    streambufs[it->first].Push(it->second);
  }
}

typedef std::pair<double, std::map<int, double> > Stream;
typedef std::map<std::string, Stream> StreamSet;

void Separations::EnterNotify() {
  cyclus::Facility::EnterNotify();
  StreamSet::iterator it;
  for (it = streams_.begin(); it != streams_.end(); ++it) {
    std::string name = it->first;
    Stream stream = it->second;
    double cap = stream.first;
    if (cap >= 0) {
      streambufs[name].capacity(cap);
    }
  }

  if (feed_commod_prefs.size() == 0) {
    for (int i = 0; i < feed_commods.size(); i++) {
      feed_commod_prefs.push_back(0);
    }
  }
}

void Separations::Tick() {
  if (feed.count() == 0) {
    return;
  }

   // determine efficiency value for the timestep
   std::vector<double> updated_eff = AdjustEfficiencies();
  
  Material::Ptr mat = feed.Pop(std::min(throughput, feed.quantity()));
  double orig_qty = mat->quantity();

  // Finally overwrite the Stream efficiencies in the original stream structure
  StreamSet::iterator it;
  double maxfrac = 1;
  std::map<std::string, Material::Ptr> stagedsep;
  for (it = streams_.begin(); it != streams_.end(); ++it) {
    double curr_eff = -1;
    Stream info = it->second;
    std::string name = it->first;
    if (name == "Fuel") {
      curr_eff = updated_eff[0];
    }
    else if (name == "Diverted") {
      curr_eff = updated_eff[1];
    }
    else if (name == "Losses") {
      curr_eff = updated_eff[2];
    }
    stagedsep[name] = SepMaterial(info.second, mat, curr_eff);
    double frac = streambufs[name].space() / stagedsep[name]->quantity();
    if (frac < maxfrac) {
      maxfrac = frac;
    }
  }

  std::map<std::string, Material::Ptr>::iterator itf;
  for (itf = stagedsep.begin(); itf != stagedsep.end(); ++itf) {
    std::string name = itf->first;
    Material::Ptr m = itf->second;
    if (m->quantity() > 0) {
      streambufs[name].Push(
          mat->ExtractComp(m->quantity() * maxfrac, m->comp()));
    }
  }

  if (maxfrac == 1) {
    if (mat->quantity() > 0) {
      // unspecified separations fractions go to leftovers
      leftover.Push(mat);
    }
  } else {  // maxfrac is < 1
    // push back any leftover feed due to separated stream inv size constraints
    feed.Push(mat->ExtractQty((1 - maxfrac) * orig_qty));
    if (mat->quantity() > 0) {
      // unspecified separations fractions go to leftovers
      leftover.Push(mat);
    }
  }
}


//----------------------------------------------------------------------
// Add behavior to separation efficiencies to affect how much reprocessed
// material is diverted, and when.
std::vector<double> AdjustEfficiencies() {
    
  int cur_time = cyclus::context()->time();
  double ideal_fuel = 0;
  double ideal_diverted = 0;
  double ideal_loss = 0;

  std::map<std::string, std::vector<double> >::iterator eff_it;
  for (eff_it = eff_variation.begin(); eff_it != eff_variation.end(); ++eff_it){
    std::vector eff_params = eff_it->second;
    std::string stream_name = eff_it->first;
    double avg_qty = eff_params[0];
    double sigma = eff_params[1];
    double freq = eff_params[2];
    double desired_eff;
    // determine the amount to request (if sigma=0 then RNG is not queried)
    // if freq = 1 then trade on every timestep 
    desired_eff = RNG_NormalDist(avg_qty, sigma, rng_seed);
    // make sure result is within bounds of 0-1
    if (desired_eff > 1) {
      desired_eff = 1;
    }
    else if (desired_eff < 0) {
      desired_eff = 0;
    }
    // Now set efficiency to zero if it's not the right timestep
    if (freq > 1) {
      desired_eff *= EveryXTimestep(cur_time, freq);
    }
    // if frequency is negative, use Random
    else if (freq < 0) {
	desired_eff *= EveryRandomXTimestep(freq, rng_seed);
    }
    // if frequency is 0, no trading occurs
    else {
      desired_eff = 0;
    }
    // now save the desired eff outside the loop
    if (stream_name == "Fuel") {
      ideal_fuel = desired_eff;
    }
    else if (stream_name == "Diverted") {
      if   (cur_time < t_trade) {
	ideal_diverted = 0;
      }
      else {
	ideal_diverted = desired_eff;
      }
    }
    else if (stream_name == "Losses") {
      ideal_loss = desired_eff;
    }
    else {
      LOG(cyclus::LEV_INFO1, "SepFac") << stream_name << " stream is non-standard and will be considered as waste";
    }
  }

  // Now recalculate all efficiencies to make sure net is > 0
  double net_eff = ideal_loss + ideal_diverted + ideal_fuel ;
  if (net_eff > 1) {
    ideal_fuel -= (net_eff - 1);
    if (ideal_fuel < 0) {
      throw cyclus::ValueError("Total efficiency of Sep streams " \
			       "is greater than 1" );
    }
  }
  std::vector<double> updated_eff = [ideal_fuel, ideal_diverted, ideal_loss];
  return updated_eff;
  }
    
//----------------------------------------------------------------------


// Note that this returns an untracked material that should just be used for
// its composition and qty - not in any real inventories, etc.
  Material::Ptr SepMaterial(std::map<int, double> effs, Material::Ptr mat,
			    double ideal_eff) {
  CompMap cm = mat->comp()->mass();
  cyclus::compmath::Normalize(&cm, mat->quantity());
  double tot_qty = 0;
  CompMap sepcomp;

  CompMap::iterator it;
  for (it = cm.begin(); it != cm.end(); ++it) {
    int nuc = it->first;
    int elem = (nuc / 10000000) * 10000000;
    double eff = 0;
    if (effs.count(nuc) > 0) {
      if (effs[nuc] != 0) {
	if (ideal_eff == -1) {
	  eff = effs[nuc];
	} else {
	  eff = ideal_eff;
	}
      }
    } else if (effs.count(elem) > 0) {
      if (effs[elem] != 0){
	if (ideal_eff == -1) {
	  eff = effs[elem];
	} else {
	  eff = ideal_eff;
	}
      }
    } else {
      continue;
    }

    double qty = it->second;
    double sepqty = qty * eff;
    sepcomp[nuc] = sepqty;
    tot_qty += sepqty;
  }

  Composition::Ptr c = Composition::CreateFromMass(sepcomp);
  return Material::CreateUntracked(tot_qty, c);
};

std::set<cyclus::RequestPortfolio<Material>::Ptr>
Separations::GetMatlRequests() {
  using cyclus::RequestPortfolio;
  std::set<RequestPortfolio<Material>::Ptr> ports;
  bool exclusive = false;

  if (feed.space() > cyclus::eps()) {
    RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());

    Material::Ptr m = cyclus::NewBlankMaterial(feed.space());
    if (!feed_recipe.empty()) {
      Composition::Ptr c = context()->GetRecipe(feed_recipe);
      m = Material::CreateUntracked(feed.space(), c);
    }

    std::vector<cyclus::Request<Material>*> reqs;
    for (int i = 0; i < feed_commods.size(); i++) {
      std::string commod = feed_commods[i];
      double pref = feed_commod_prefs[i];
      reqs.push_back(port->AddRequest(m, this, commod, pref, exclusive));
    }
    port->AddMutualReqs(reqs);
    ports.insert(port);
  }

  return ports;
}

void Separations::GetMatlTrades(
    const std::vector<cyclus::Trade<Material> >& trades,
    std::vector<std::pair<cyclus::Trade<Material>, Material::Ptr> >&
        responses) {
  using cyclus::Trade;

  std::vector<cyclus::Trade<cyclus::Material> >::const_iterator it;
  for (int i = 0; i < trades.size(); i++) {
    std::string commod = trades[i].request->commodity();
    if (commod == leftover_commod) {
      double amt = std::min(leftover.quantity(), trades[i].amt);
      Material::Ptr m = leftover.Pop(amt);
      responses.push_back(std::make_pair(trades[i], m));
    } else if (streambufs.count(commod) > 0) {
      double amt = std::min(streambufs[commod].quantity(), trades[i].amt);
      Material::Ptr m = streambufs[commod].Pop(amt);
      responses.push_back(std::make_pair(trades[i], m));
    } else {
      throw ValueError("invalid commodity " + commod +
                       " on trade matched to prototype " + prototype());
    }
  }
}

void Separations::AcceptMatlTrades(const std::vector<
    std::pair<cyclus::Trade<Material>, Material::Ptr> >& responses) {
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
                        cyclus::Material::Ptr> >::const_iterator trade;

  for (trade = responses.begin(); trade != responses.end(); ++trade) {
    feed.Push(trade->second);
  }
}

std::set<cyclus::BidPortfolio<Material>::Ptr> Separations::GetMatlBids(
    cyclus::CommodMap<Material>::type& commod_requests) {
  using cyclus::BidPortfolio;

  bool exclusive = false;
  std::set<BidPortfolio<Material>::Ptr> ports;

  // bid streams
  std::map<std::string, ResBuf<Material> >::iterator it;
  for (it = streambufs.begin(); it != streambufs.end(); ++it) {
    std::string commod = it->first;
    std::vector<Request<Material>*>& reqs = commod_requests[commod];
    if (reqs.size() == 0) {
      continue;
    } else if (streambufs[commod].quantity() < cyclus::eps()) {
      continue;
    }

    MatVec mats = streambufs[commod].PopN(streambufs[commod].count());
    streambufs[commod].Push(mats);

    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    for (int j = 0; j < reqs.size(); j++) {
      Request<Material>* req = reqs[j];
      double tot_bid = 0;
      for (int k = 0; k < mats.size(); k++) {
        Material::Ptr m = mats[k];
        tot_bid += m->quantity();
        port->AddBid(req, m, this, exclusive);
        if (tot_bid >= req->target()->quantity()) {
          break;
        }
      }
    }

    double tot_qty = streambufs[commod].quantity();
    cyclus::CapacityConstraint<Material> cc(tot_qty);
    port->AddConstraint(cc);
    ports.insert(port);
  }

  // bid leftovers
  std::vector<Request<Material>*>& reqs = commod_requests[leftover_commod];
  if (reqs.size() > 0 && leftover.quantity() >= cyclus::eps()) {
    MatVec mats = leftover.PopN(leftover.count());
    leftover.Push(mats);

    BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

    for (int j = 0; j < reqs.size(); j++) {
      Request<Material>* req = reqs[j];
      double tot_bid = 0;
      for (int k = 0; k < mats.size(); k++) {
        Material::Ptr m = mats[k];
        tot_bid += m->quantity();
        port->AddBid(req, m, this, exclusive);
        if (tot_bid >= req->target()->quantity()) {
          break;
        }
      }
    }

    cyclus::CapacityConstraint<Material> cc(leftover.quantity());
    port->AddConstraint(cc);
    ports.insert(port);
  }

  return ports;
}

void Separations::Tock() {}

extern "C" cyclus::Agent* ConstructSeparations(cyclus::Context* ctx) {
  return new Separations(ctx);
}

}  // namespace cycamore
