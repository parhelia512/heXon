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

#include "helper.h"

float heXon::Distance(const Urho3D::Vector3 from, const Urho3D::Vector3 to){
    return (to - from).Length();
}

unsigned heXon::IntVector2ToHash(Urho3D::IntVector2 vec) { return (Urho3D::MakeHash(vec.x_) & 0xffff) | (Urho3D::MakeHash(vec.y_) << 16); }

Urho3D::Vector3 heXon::Scale(const Urho3D::Vector3 lhs, const Urho3D::Vector3 rhs) {
    return Urho3D::Vector3(lhs.x_ * rhs.x_, lhs.y_ * rhs.y_, lhs.z_ * rhs.z_);
}
