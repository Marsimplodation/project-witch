#include "JoltImpl.h"
using namespace JPH;
using namespace JPH::literals;

/// Class that determines if two object layers can collide
bool ObjectLayerPairFilterImpl::ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const {
	switch (inObject1) {
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
	}
}


// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
BPLayerInterfaceImpl::BPLayerInterfaceImpl() {
	// Create a mapping table from object to broad phase layer
	mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
}

uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const {
	return BroadPhaseLayers::NUM_LAYERS;
}
BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(ObjectLayer inLayer) const {
	JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
	return mObjectToBroadPhase[inLayer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char * BPLayerInterfaceImpl::GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const {
	switch ((BroadPhaseLayer::Type)inLayer) {
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
				return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
				return "MOVING";
		default:
			JPH_ASSERT(false); return "INVALID";
	}
}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const {
	switch (inLayer1) {
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
	}
}
