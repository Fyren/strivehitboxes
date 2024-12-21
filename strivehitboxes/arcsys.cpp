#include "arcsys.h"
#include "sigscan.h"

//AREDGameState_Battle::StaticClass()
using StaticClass_t = UClass*(*)();
const auto AREDGameState_Battle_StaticClass = (StaticClass_t)(get_rip_relative(
	sigscan::get().scan("\x30\x01\x00\x00\x48\x85\xFF\x74\x31", "xxxxxxxxx") + 10));

//BATTLE_CScreenManager::ApplyOffsetMatrix(FVector *result, FVector *location, FRotator *rotation)
using asw_scene_camera_transform_t = void(*)(const BATTLE_CScreenManager*, FVector*, FVector*, FVector*);
const auto asw_scene_camera_transform = (asw_scene_camera_transform_t)(
	sigscan::get().scan("\x4D\x85\xC0\x74\x15\xF2\x41\x0F", "xxxxxxxx") - 0x56);

//OBJ_CBase::IsAttackAvailable(bool forThrow)
using asw_entity_is_active_t = bool(*)(const OBJ_CBase*, bool);
const auto asw_entity_is_active = (asw_entity_is_active_t)(
	sigscan::get().scan("\xF7\x81\x00\x00\x00\x00\x00\x00\x00\x04\x0F", "xx????xxxxx") - 0x12);

//OBJ_CBase::IsObjectToOjbectPushCollisionAvailable()
using asw_entity_is_pushbox_active_t = bool(*)(const OBJ_CBase*);
const auto asw_entity_is_pushbox_active = (asw_entity_is_pushbox_active_t)(
	sigscan::get().scan("\xF7\x80\xCC\x5D\x00\x00\x00\x00\x02\x00", "xx????xxxx") - 0x1A);

//OBJ_CBase::GetPosX(), only differs from GetPosZ slightly
using asw_entity_get_pos_x_t = int(*)(const OBJ_CBase*);
const auto asw_entity_get_pos_x = (asw_entity_get_pos_x_t)(
	sigscan::get().scan("\xF3\x0F\x59\xC2\xF3\x0F\x5C\xD8\xF3\x0F\x2C\xFB", "xxxxxxxxxxxx") - 0x15D);

//OBJ_CBase::GetPosY()
using asw_entity_get_pos_y_t = int(*)(const OBJ_CBase*);
const auto asw_entity_get_pos_y = (asw_entity_get_pos_y_t)(
	sigscan::get().scan("\x3D\x00\x08\x04\x00\x75\x18", "xxxxxxx") - 0x3D);

/*
	The following three functions all start out the same, besides the offset to the member they first try to read:
	40 53             push rbx
	48 83 EC 20       sub rsp, 20h
	8B 81 EC 04 00 00 mov eax, [rcx+offset]
	The members are m_PushColWidth, m_PushColHeight, and m_PushColHeightLow, which are adjacent 32-bit ints in the object and in that order.

	The first two functions are exactly the same besides offsets, except for the hardcoded fallback return value.
*/

/*
	OBJ_CBase::GetPushColW()
	Ends with:
	B8 78 00 00 00 mov eax, 78h
	48 83 C4 20    add rsp, 20h
	5B             pop rbx
	C3             retn
*/
using asw_entity_pushbox_width_t = int(*)(const OBJ_CBase*);
const auto asw_entity_pushbox_width = (asw_entity_pushbox_width_t)(
	sigscan::get().scan("\xB8\x78\x00\x00\x00\x48\x83\xC4\x20\x5B\xC3", "xxxxxxxxxxx") - 0x77);

/*
	OBJ_CBase::GetPushColH()
	Ends with:
	B8 64 00 00 00 mov eax, 64h
	48 83 C4 20    add rsp, 20h
	5B             pop rbx
	C3             retn
*/
using asw_entity_pushbox_height_t = int(*)(const OBJ_CBase*);
const auto asw_entity_pushbox_height = (asw_entity_pushbox_height_t)(
	sigscan::get().scan("\xB8\x64\x00\x00\x00\x48\x83\xC4\x20\x5B\xC3", "xxxxxxxxxxx") - 0x77);

/*
	OBJ_CBase::GetPushColHLow()
	Ends with:
	48 0F 44 D8       cmovz rbx, rax
	8B 83 ?? ?? ?? ?? mov eax, [rbx+offset]
	48 83 C4 20       add rsp, 20h
	5B                pop     rbx
	C3                retn
	33 C0             xor eax, eax
	48 83 C4 20       add rsp, 20h
	5B                pop rbx
	C3                retn
*/
using asw_entity_pushbox_bottom_t = int(*)(const OBJ_CBase*);
const auto asw_entity_pushbox_bottom = (asw_entity_pushbox_bottom_t)(
	sigscan::get().scan("\x48\x0F\x44\xD8\x8B\x83\x00\x00\x00\x00\x48\x83\xC4\x20\x5B\xC3\x33\xC0\x48\x83\xC4\x20\x5B\xC3", "xxxxxx????xxxxxxxxxxxxxx") - 0x33);

//OBJ_CBase::GetPushWorldRect(int *L, int *T, int *R, int *B)
using asw_entity_get_pushbox_t = void(*)(const OBJ_CBase*, int*, int*, int*, int*);
const auto asw_entity_get_pushbox = (asw_entity_get_pushbox_t)(
	sigscan::get().scan("\x99\x48\x8B\xCB\x2B\xC2\xD1\xF8\x44", "xxxxxxxxx") - 0x5B);

UClass *AREDGameState_Battle::StaticClass()
{
	return AREDGameState_Battle_StaticClass();
}

BATTLE_CObjectManager *BATTLE_CObjectManager::get()
{
	if (*GWorld == nullptr)
		return nullptr;

	const auto *GameState = (*GWorld)->GameState;
	if (GameState == nullptr || !GameState->IsA<AREDGameState_Battle>())
		return nullptr;

	return ((AREDGameState_Battle*)GameState)->Engine;
}

BATTLE_CScreenManager *BATTLE_CScreenManager::get()
{
	if (*GWorld == nullptr)
		return nullptr;

	const auto *GameState = (*GWorld)->GameState;
	if (GameState == nullptr || !GameState->IsA<AREDGameState_Battle>())
		return nullptr;

	return ((AREDGameState_Battle*)GameState)->Scene;
}

void BATTLE_CScreenManager::camera_transform(FVector *delta, FVector *position, FVector *angle) const
{
	asw_scene_camera_transform(this, delta, position, angle);
}

void BATTLE_CScreenManager::camera_transform(FVector *position, FVector *angle) const
{
	FVector delta;
	asw_scene_camera_transform(this, &delta, position, angle);
}

bool OBJ_CBase::is_active() const
{
	// Otherwise returns false during COUNTER
	return cinematic_counter || asw_entity_is_active(this, !is_player);
}

bool OBJ_CBase::is_pushbox_active() const
{
	return asw_entity_is_pushbox_active(this);
}

bool OBJ_CBase::is_strike_invuln() const
{
	return strike_invuln || wakeup || backdash_invuln > 0;
}

bool OBJ_CBase::is_throw_invuln() const
{
	return throw_invuln || wakeup || backdash_invuln > 0;
}

int OBJ_CBase::get_pos_x() const
{
	return asw_entity_get_pos_x(this);
}

int OBJ_CBase::get_pos_y() const
{
	return asw_entity_get_pos_y(this);
}

int OBJ_CBase::pushbox_width() const
{
	return asw_entity_pushbox_width(this);
}

int OBJ_CBase::pushbox_height() const
{
	return asw_entity_pushbox_height(this);
}

int OBJ_CBase::pushbox_bottom() const
{
	return asw_entity_pushbox_bottom(this);
}

void OBJ_CBase::get_pushbox(int *left, int *top, int *right, int *bottom) const
{
	asw_entity_get_pushbox(this, left, top, right, bottom);
}
