/* heXon
// Copyright (C) 2015 LucKey Productions (luckeyproductions.nl)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

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
Urho3D::Vector3 Scale(const Urho3D::Vector3 lhs, const Urho3D::Vector3 rhs);
}

#endif // HEXON_HELPER_H
