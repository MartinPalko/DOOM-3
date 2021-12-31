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

// Translate doom's map units to meters as per https://doom.fandom.com/wiki/Map_unit
constexpr float g_doomToUnityWorldScale = 1.0f / 32.0f;

inline GameGlue::Vector2 ToGameGlue(const idVec2& v)
{
	return GameGlue::Vector2(v.x, v.y);
}

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
	return ToGameGlue(pos * g_doomToUnity * g_doomToUnityWorldScale);
}

static GameGlue::Transform PackTransform(const idVec3& pos, const idMat3& axis)
{
	const auto position = PackPosition(pos);
	const auto right = ToGameGlue((g_doomRight * axis) * g_doomToUnity);
	const auto up = ToGameGlue((g_doomUp * axis) * g_doomToUnity);
	const auto forward = ToGameGlue((g_doomForward * axis) * g_doomToUnity);

	return GameGlue::Transform(position, forward, up, right);
}

static void SendEntityCreated(int entityHandle)
{
	flatbuffers::FlatBufferBuilder builder(16);

	auto dataBuilder = GameGlue::EntityCreatedBuilder(builder);
	dataBuilder.add_entity_handle(entityHandle);
	const auto data = dataBuilder.Finish();

	GameGlue::ServerMessageBuilder messageBuilder(builder);
	messageBuilder.add_data_type(GameGlue::ServerMessageData_EntityCreated);
	messageBuilder.add_data(data.o);
	builder.Finish(messageBuilder.Finish());

	common->GetGameGlueServer()->writeMessage(builder);
}

static void SendEntityUpdated(int entityHandle, const idVec3& origin, const idMat3& axis, const idRenderModel* model)
{
	flatbuffers::FlatBufferBuilder builder(16);

	const auto transform = PackTransform(origin, axis);

	std::vector<int> materialHandles(model->NumSurfaces());
	for (int i = 0; i < model->NumSurfaces(); i++)
	{
		materialHandles[i] = (int)model->Surface(i)->shader;
	}

	const auto data = GameGlue::CreateEntityUpdatedDirect(builder, entityHandle, &transform, (int32_t)model, &materialHandles);

	GameGlue::ServerMessageBuilder messageBuilder(builder);
	messageBuilder.add_data_type(GameGlue::ServerMessageData_EntityUpdated);
	messageBuilder.add_data(data.o);
	builder.Finish(messageBuilder.Finish());

	common->GetGameGlueServer()->writeMessage(builder);
}

static void SendEntityDestroyed(int entityHandle)
{
	flatbuffers::FlatBufferBuilder builder(16);

	auto dataBuilder = GameGlue::EntityDestroyedBuilder(builder);
	dataBuilder.add_entity_handle(entityHandle);
	const auto data = dataBuilder.Finish();

	GameGlue::ServerMessageBuilder messageBuilder(builder);
	messageBuilder.add_data_type(GameGlue::ServerMessageData_EntityDestroyed);
	messageBuilder.add_data(data.o);
	builder.Finish(messageBuilder.Finish());

	common->GetGameGlueServer()->writeMessage(builder);
}

// GAMEGLUE_END