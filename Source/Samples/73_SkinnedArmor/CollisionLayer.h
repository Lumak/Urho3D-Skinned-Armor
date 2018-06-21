//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

//=============================================================================
//=============================================================================
enum CollisionLayerType
{
    ColLayer_None       = (0),

    ColLayer_Static     = (1<<0),
    ColLayer_InvisWall  = (1<<1),

    ColLayer_Character  = (1<<2),
    ColLayer_Dummy      = (1<<3),
    ColLayer_Weapon     = (1<<4),

    ColLayer_All        = (0xffff)
};

enum CollisionMaskType
{
    ColMask_Character  = ~(ColLayer_Weapon),                     
    ColMask_Weapon     = ~(ColLayer_Static | ColLayer_Character),  

    ColMask_Camera     = ~(ColLayer_Character | ColLayer_Dummy | ColLayer_Weapon)
};


