// Copyright (c) 2022 Connor Mellon
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#version 450

layout(location = 0)in vec3 pass_colour;

layout(location = 0) out vec4 out_colour;

void main() {
    out_colour = vec4(pass_colour.xyz, 1.0);
}