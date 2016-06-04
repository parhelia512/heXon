/*
// Copyright (C) 2016 LucKey Productions (luckeyproductions.nl)
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

#include "luckey.h"

float LucKey::Distance(const Vector3 from, const Vector3 to){
    return (to - from).Length();
}

unsigned LucKey::IntVector2ToHash(IntVector2 vec) { return (MakeHash(vec.x_) & 0xffff) | (MakeHash(vec.y_) << 16); }

Vector3 LucKey::Scale(const Vector3 lhs, const Vector3 rhs) {
    return Vector3(lhs.x_ * rhs.x_, lhs.y_ * rhs.y_, lhs.z_ * rhs.z_);
}

Urho3D::IntVector2 LucKey::Scale(const Urho3D::IntVector2 lhs, const Urho3D::IntVector2 rhs) {
    return Urho3D::IntVector2(lhs.x_ * rhs.x_, lhs.y_ * rhs.y_);
}

Vector2 LucKey::Rotate(const Vector2 vec2, const float angle){
    float x{vec2.x_};
    float y{vec2.y_};

    float theta{M_DEGTORAD * angle};

    float cs{cos(theta)};
    float sn{sin(theta)};

    return Vector2(x * cs - y * sn, x * sn + y * cs);
}

float LucKey::RandomSign()
{
    return static_cast<float>(Random(2)*2-1);
}

Color LucKey::RandomHairColor(bool onlyNatural)
{
    bool grey{!Random(23)};
    bool dyed{!Random(23)};
    Color hairColor{};
    if (onlyNatural || (!dyed && !grey)){
        //Natural
        hairColor.FromHSV(Random(0.034f, 0.15f),
                          Random(0.34f, 0.65f),
                          Random(0.05f, 0.9f));
    } else if (!dyed && grey){
        //Grey
        hairColor.FromHSV(Random(0.034f, 0.16f),
                          Random(0.0f, 0.05f),
                          Random(0.23f, 0.86f));
    } else if (dyed && !grey){
        //Bright dye
        hairColor.FromHSV(Random(6)*0.1666f,
                          Random(0.6f, 0.9f),
                          Random(0.5f, 0.71f));
    } else if (dyed && grey){
        //Fake black
        hairColor.FromHSV(0.666f,
                          0.05f,
                          Random(0.01f, 0.05f));
    }
    return hairColor;
}

Color LucKey::RandomSkinColor()
{
    Color skinColor{};
    skinColor.FromHSV(Random(0.05f, 0.13f), Random(0.6f, 0.8f), Random(0.23f, 0.8f));
    return skinColor;
}
Color LucKey::RandomColor()
{
    Color color{};
    color.FromHSV(Random(), Random(), Random());
    return color;
}

float LucKey::Sine(float x)
{
    if (x < -M_PI){
        while (x < -M_PI) {
            x += 2.0f * M_PI;
        }
    } else while (x > M_PI) {
        x -= 2.0f * M_PI;
    }


    float sin{};

    if (x < 0.0f)
        sin = 1.27323954f * x + 0.405284735f * x * x;
    else
        sin = 1.27323954f * x - 0.405284735f * x * x;

    if (sin < 0)
        sin = 0.225f * (sin *-sin - sin) + sin;
    else
        sin = 0.225f * (sin * sin - sin) + sin;

    return sin;
}
float LucKey::Cosine(float x)
{
    return Sine(x + M_PI * 0.5f);
}
