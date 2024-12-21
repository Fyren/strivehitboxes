#pragma once

#include "ue4.h"
#include "struct_util.h"

//probably actually AREDGameState_BattleAdv now
class AREDGameState_Battle : public AGameState {
public:
	static UClass *StaticClass();

	FIELD(0xBB8, class BATTLE_CObjectManager*, Engine);
	FIELD(0xBC0, class BATTLE_CScreenManager*, Scene);
};

class BATTLE_TeamManager {
public:
	FIELD(0x8, class OBJ_CBase*, mainPlayer); //OBJ_CCharBase* m_pMainPlayerObject
};


// Used by the shared GG/BB/DBFZ engine code
class BATTLE_CObjectManager {
public:
	static constexpr auto COORD_SCALE = .458f;

	static BATTLE_CObjectManager *get();

	ARRAY_FIELD(0x0, BATTLE_TeamManager[2], m_TeamManagers);
	FIELD(0x8A0, int, entity_count); //int m_ActiveObjectCount
	ARRAY_FIELD(0xC10, class OBJ_CBase*[107], entities); //OBJ_CBase* m_SortedObjPtrVector[107]
};


class BATTLE_CScreenManager {
public:
	static BATTLE_CScreenManager *get();

	// "delta" is the difference between input and output position
	// position gets written in place
	// position/angle can be null
	void camera_transform(FVector *delta, FVector *position, FVector *angle) const;
	void camera_transform(FVector *position, FVector *angle) const;
};

class hitbox {
public:
	enum class box_type : int {
		hurt = 0,
		hit = 1,
		grab = 2 // Not used by the game
	};

	box_type type;
	float x, y;
	float w, h;
};

enum class direction : int {
	right = 0,
	left = 1
};

class OBJ_CBase {
public:
	FIELD(0x18, bool, is_player);
	//FIELD(0x44, unsigned char, player_index);
	FIELD(0x78, hitbox*, hitboxes);
	FIELD(0x10C, int, hurtbox_count);
	FIELD(0x110, int, hitbox_count);
	//   _____    ____    _    _   _   _   _______   ______   _____  
	//  / ____|  / __ \  | |  | | | \ | | |__   __| |  ____| |  __ \ 
	// | |      | |  | | | |  | | |  \| |    | |    | |__    | |__) |
	// | |      | |  | | | |  | | | . ` |    | |    |  __|   |  _  / 
	// | |____  | |__| | | |__| | | |\  |    | |    | |____  | | \ \ 
	//  \_____|  \____/   \____/  |_| \_|    |_|    |______| |_|  \_\ 
	BIT_FIELD(0x1A8, 0x4000000, cinematic_counter);
	//FIELD(0x1B0, int, state_frames);
	FIELD(0x2C0, OBJ_CBase*, opponent);
	//FIELD(0x2C8, asw_entity*, parent);
	FIELD(0x318, OBJ_CBase*, attached);
	//BIT_FIELD(0x380, 1, airborne);
	BIT_FIELD(0x390, 256, counterhit);
	BIT_FIELD(0x394, 16, strike_invuln);
	BIT_FIELD(0x394, 32, throw_invuln);
	BIT_FIELD(0x394, 64, wakeup);
	FIELD(0x3A4, direction, facing);
	FIELD(0x3A8, int, pos_x);
	FIELD(0x3AC, int, pos_y);
	FIELD(0x3B0, int, pos_z);
	FIELD(0x3B4, int, angle_x);
	FIELD(0x3B8, int, angle_y);
	FIELD(0x3BC, int, angle_z);
	FIELD(0x3C4, int, scale_x);
	FIELD(0x3C8, int, scale_y);
	FIELD(0x3CC, int, scale_z);
	//FIELD(0x4B8, int, vel_x);
	//FIELD(0x4BC, int, vel_y);
	//FIELD(0x4C0, int, gravity);
	FIELD(0x4FC, int, pushbox_front_offset);
	FIELD(0x794, int, throw_box_top); //OBJ_CCharObj::m_AtkParam 0x750 + CAtkParam::m_AtkRangeMaxY 0x44
	FIELD(0x79C, int, throw_box_bottom); //OBJ_CCharObj::m_AtkParam 0x750 + CAtkParam::m_AtkRangeMinY 0x4c
	FIELD(0x7A0, int, throw_range); //OBJ_CCharObj::m_AtkParam offset 0x750 + CAtkParam::m_AtkPushRangeX 0x50
	FIELD(0x11CC, int, backdash_invuln);
	FIELD(0x9ADC, int, ply_PushColHeightLowAir);
	FIELD(0xF638, int, afro); //OBJ_CCharObj::m_IsAfro
	FIELD(0xF670, int, afroW); //OBJ_CCharOBJ::m_ExtendJon[0] 0xF640 + ExtendJonParam::m_ColW 0x30
	FIELD(0xF674, int, afroH); //OBJ_CCharOBJ::m_ExtendJon[0] 0xF640 + ExtendJonParam::m_ColH 0x34

	bool is_active() const;
	bool is_pushbox_active() const;
	bool is_strike_invuln() const;
	bool is_throw_invuln() const;
	int get_pos_x() const;
	int get_pos_y() const;
	int pushbox_width() const;
	int pushbox_height() const;
	int pushbox_bottom() const;
	void get_pushbox(int *left, int *top, int *right, int *bottom) const;
};
