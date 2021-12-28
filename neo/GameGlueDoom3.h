// GAMEGLUE_START
#pragma once

// Common header for game glue includes, and utility functions for doom3

#include "GameGlueServer.h"
#include "idlib/precompiled.h"

// Doom3 is right-handed: https://modwiki.dhewm3.org/doom3world/index.php?thread_id=2275
// Unity is left-handed: https://www.techarthub.com/a-guide-to-unitys-coordinate-system-with-practical-examples/

// Doom3 is Z-up
// Unity is Y-up

// Doom X is forward
// Unity Z is forward


const idVec3 g_unityRight = idVec3(1, 0, 0);
const idVec3 g_unityUp = idVec3(0, 1, 0);
const idVec3 g_unityForward = idVec3(0, 0, 1);

const idVec3 g_doomRight = idVec3(0, -1, 0);
const idVec3 g_doomForward = idVec3(1, 0, 0);
const idVec3 g_doomUp = idVec3(0, 0, 1);

const idMat3 g_doomToUnity = idMat3(g_doomRight, g_doomUp, g_doomForward).Transpose() * idMat3(g_unityRight, g_unityUp, g_unityForward);

inline GameGlue::Vector3 ToGameGlue(const idVec3& v)
{
	return GameGlue::Vector3(v.x, v.y, v.z);
}

static GameGlue::Vector3 PackDirection(const idVec3& dir)
{
	return ToGameGlue(dir * g_doomToUnity);
}

static GameGlue::Vector3 PackPosition(const idVec3& pos)
{
	// Translate doom's map units to meters as per https://doom.fandom.com/wiki/Map_unit
	constexpr float posScale = 1.0f / 32.0f;

	return ToGameGlue(pos * g_doomToUnity * posScale);
}

static GameGlue::Transform PackTransform(const idVec3& pos, const idMat3& axis)
{
	const auto position = PackPosition(pos);
	const auto right = ToGameGlue((g_doomRight * axis) * g_doomToUnity);
	const auto up = ToGameGlue((g_doomUp * axis) * g_doomToUnity);
	const auto forward = ToGameGlue((g_doomForward * axis) * g_doomToUnity);

	return GameGlue::Transform(position, forward, up, right);
}

// GAMEGLUE_END