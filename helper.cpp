#include "helper.h"

float heXon::Distance(const Urho3D::Vector3 from, const Urho3D::Vector3 to){
    return (to - from).Length();
}

unsigned heXon::IntVector2ToHash(Urho3D::IntVector2 vec) { return (Urho3D::MakeHash(vec.x_) & 0xffff) | (Urho3D::MakeHash(vec.y_) << 16); }
