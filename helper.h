#ifndef HEXON_HELPER_H
#define HEXON_HELPER_H

#include <Urho3D/Urho3D.h>
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Container/HashBase.h>

namespace heXon {

template <class T>
T Cycle(T x, T min, T max){
    return (x < min) ?
                x + (max - min) * abs(ceil((min - x) / (max - min)))
              : (x > max) ?
                x - (max - min) * abs(ceil((x - max) / (max - min)))
                  : x;
}

float Distance(const Urho3D::Vector3 from, const Urho3D::Vector3 to);
unsigned IntVector2ToHash(Urho3D::IntVector2 vec);
}

#endif // HEXON_HELPER_H
