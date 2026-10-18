// Background thread that plays a Route into the sensor store.
#pragma once
#include "route.h"
#include "store.h"

namespace andros {

void PlayRoute(Store* store, std::vector<Pt> pts, double speed_mps, bool loop);
void StopRoute();
Motion CurrentMotion();  // last state published by the player

}  // namespace andros
