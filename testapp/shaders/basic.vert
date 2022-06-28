// Copyright (c) 2022 Connor Mellon
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#version 450

layout(location = 0)in vec2 pos;
layout(location = 1)in vec3 colour;

layout(location = 0)out vec3 pass_colour;

void main() {
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    pass_colour = colour;
}