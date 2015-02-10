//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file game/game_logic/GameObject.hpp
/// @details An object representing instances of in-game egoboo objects (GameObject)
/// @author Johan Jansen

#pragma once

#include "game/egoboo_typedef.h"
#include "game/physics.h"
#include "game/script.h"
#include "game/profiles/Profile.hpp"
#include "game/graphic_mad.h"
#include "game/graphic_billboard.h"
#include "egolib/IDSZ_map.h"

//Macros
#define PACK_BEGIN_LOOP(INVENTORY, PITEM, IT) { int IT##_internal; for(IT##_internal=0;IT##_internal<GameObject::MAXNUMINPACK;IT##_internal++) { CHR_REF IT; GameObject * PITEM = NULL; IT = (CHR_REF)INVENTORY[IT##_internal]; if(!_gameObjects.exists (IT)) continue; PITEM = _gameObjects.get( IT );
#define PACK_END_LOOP() } }

/// The possible methods for characters to determine what direction they are facing
enum turn_mode_t : uint8_t
{
    TURNMODE_VELOCITY = 0,                       ///< Character gets rotation from velocity (normal)
    TURNMODE_WATCH,                              ///< For watch towers, look towards waypoint
    TURNMODE_SPIN,                               ///< For spinning objects
    TURNMODE_WATCHTARGET,                        ///< For combat intensive AI
    TURNMODE_COUNT
};

/// Everything that is necessary to compute the character's interaction with the environment
struct chr_environment_t
{
    // floor stuff
    Uint8   grid_twist;           ///< The twist parameter of the current grid (what angle it it at)
    float   grid_level;           ///< Height relative to the current grid
    float   grid_lerp;

    float   water_level;           ///< Height relative to the current water level
    float   water_lerp;

    float  floor_level;           ///< Height of tile
    float  level;                 ///< Height of a tile or a platform
    float  fly_level;             ///< Height of tile, platform, or water, whever is highest.

    float  zlerp;

    fvec3_t floor_speed;

    // friction stuff
    bool is_slipping;
    bool is_slippy,    is_watery;
    float  air_friction, ice_friction;
    float  fluid_friction_hrz, fluid_friction_vrt;
    float  traction, friction_hrz;

    // misc states
    bool inwater;
    bool grounded;              ///< standing on something?

    // various motion parameters
    fvec3_t  new_v;
    fvec3_t  acc;
    fvec3_t  vel;
};

/// the data used to define the spawning of a character
struct chr_spawn_data_t
{
    fvec3_t     pos;
    PRO_REF     profile;
    TEAM_REF    team;
    int         skin;
    FACING_T    facing;
    STRING      name;
    CHR_REF     override;
};

/// The definition of the character object.
class GameObject
{
public:
	static const size_t MAXNUMINPACK = 6; ///< Max number of items to carry in pack

public:
    /**
    * @brief Default constructor
    * @param profile Which character profile this character should be spawned with
    * @param id The unique CHR_REF associated with this character
    **/
    GameObject(const PRO_REF profile, const CHR_REF id);

    /**
    * @brief Deconstructor
    **/
    virtual ~GameObject();

    /**
    * @brief Gets a shared_ptr to the current ObjectProfile associated with this character.
    *        The ObjectProfile can change for polymorphing objects.
    **/
    inline const std::shared_ptr<ObjectProfile>& getProfile() const {return _profile;}

    /**
    * @return the unique CHR_REF associated with this character
    **/
    inline CHR_REF getCharacterID() const {return _characterID;}

    /**
    * @return the current team this object is on. This can change in-game (mounts or pets for example)
    **/
    inline TEAM_REF getTeam() const {return team;}

    /**
    * @return This function returns true if the character is over a water tile
    **/
    bool isOverWater() const;

    /**
    * @return Get current X, Y, Z position of this GameObject
    **/
    inline const fvec3_t& getPosition() const {return _position;}

    /**
    * @return Get current X position of this GameObject
    **/
    inline float getPosX() const {return _position.x;}

    /**
    * @return Get current Y position of this GameObject
    **/
    inline float getPosY() const {return _position.y;}

    /**
    * @return Get current Z position of this GameObject
    **/
    inline float getPosZ() const {return _position.z;}

    /**
    * @brief Set current X, Y, Z position of this GameObject
    * @return true if the position of this object has changed
    **/
    bool setPosition(const fvec3_t &position);

    /**
    * @brief Set current X, Y, Z position of this GameObject
    * @return true if the position of this object has changed
    **/
    inline bool setPosition(const float x, const float y, const float z) {return setPosition(fvec3_t(x, y, z));}

    /**
    * @brief Translate the current X, Y, Z position of this object by the specified values
    **/
    void movePosition(const float x, const float y, const float z);

    /**
    * @brief Sets the transparency for this GameObject
    * @param alpha Transparency level between 0 (fully transparent) and 255 (fully opaque)
    **/
    void setAlpha(const int alpha);

    /**
    * @brief Sets the shininess of this GameObject
    * @param alpha Transparency level between 0 (no shine effect) and 255 (completely shiny)
    **/
    void setSheen(const int sheen);

    /**
    * @brief Sets the transparency for this GameObject
    * @param light Transparency level between 0 (fully transparent) and 255 (fully opaque)
    **/
    void setLight(const int light);

    /**
    * @brief Checks if this GameObject is able to mount (ride) another GameObject
    * @param mount Which GameObject we are trying to mount
    * @return true if we are able to mount the specified GameObject
    **/
	bool canMount(const std::shared_ptr<GameObject> mount) const;

	/**
	* @return true if this GameObject is mountable by other GameObjects
	**/
	bool isMount() const {return getProfile()->isMount();}

    /**
    * @brief 
    *   This function calculates and applies damage to a character.  It also
    *   sets alerts and begins actions.  Blocking and frame invincibility are done here too.  
    *
    * @param direction
    *   Direction is ATK_FRONT if the attack is coming head on, ATK_RIGHT if from the right, 
    *   ATK_BEHIND if from the back, ATK_LEFT if from the left.
    *
    * @param damage 
    *   is a random range of damage to deal
    *
    * @param damageType 
    *   indicates what kind of damage this is (ZAP, CRUSH, FIRE, etc.) which is again 
    *   affected by resistances immunities, etc.
    *
    * @param team 
    *   which team is dealing the damage
    *
    * @param attacker 
    *   The GameObject which is dealing the damage to this GameObject
    *
    * @param effects 
    *   is a BIT_FIELD of various flags which affect how we determine damage.
    *
    * @param ignore_invictus 
    *   if this is true, then we allow damaging this object even though it is normally immune to damage.
    **/
    int damage(const FACING_T direction, const IPair  damage, const DamageType damagetype, const TEAM_REF team,
            const std::shared_ptr<GameObject> &attacker, const BIT_FIELD effects, const bool ignore_invictus);

    /**
     * @brief
     *  This function gives some purelife points to the target, ignoring any resistances and so forth.
     * @param healer
     *  the healer
     * @param amount
     *  the amount to heal the character
     */
    bool heal(const std::shared_ptr<GameObject> &healer, const UFP8_T amount, const bool ignoreInvincibility);

    /**
    * @return true if this GameObject is currently doing an attack animation
    **/
    bool isAttacking() const;

    /**
    * @return true if this GameObject is controlled by a player
    **/
    bool isPlayer() const {return islocalplayer;}

    bool isAlive() const {return alive;}

private:

    /**
    * @brief This function should be used whenever a character gets attacked or healed. The function
    *        handles if the attacker is a held item (so that the holder becomes the attacker). The function also
    *        updates alerts, timers, etc. This function can trigger character cries like "That tickles!" or "Be careful!"
    **/
    void updateLastAttacker(const std::shared_ptr<GameObject> &attacker, bool healing);

public:
    bool terminateRequested;         ///< True if this character no longer exists in the game and should be destructed

    BSP_leaf_t     bsp_leaf;

    chr_spawn_data_t  spawn_data;

    // character state
    ai_state_t     ai;              ///< ai data
    latch_t        latch;

    // character stats
    STRING         Name;            ///< My name
    uint8_t          gender;          ///< Gender

    uint8_t          life_color;      ///< Bar color
	SFP8_T         life;            ///< (signed 8.8 fixed point)
	UFP8_T         life_max;        ///< (unsigned 8.8 fixed point) @inv life_max >= life
    SFP8_T         life_return;     ///< Regeneration/poison - (8.8 fixed point)

    uint8_t          mana_color;      ///< Bar color
	SFP8_T         mana;            ///< (signed 8.8 fixed point)
	UFP8_T         mana_max;        ///< (unsigned 8.8 fixed point) @inv mana_max >= mana
    SFP8_T         mana_return;     ///< (8.8 fixed point)

	SFP8_T         mana_flow;       ///< (8.8 fixed point)

    SFP8_T         strength;        ///< Strength     - (8.8 fixed point)
    SFP8_T         wisdom;          ///< Wisdom       - (8.8 fixed point)
    SFP8_T         intelligence;    ///< Intelligence - (8.8 fixed point)
    SFP8_T         dexterity;       ///< Dexterity    - (8.8 fixed point)

    uint32_t         experience;      ///< Experience
    uint8_t          experiencelevel; ///< Experience Level

    int16_t         money;            ///< Money
    uint16_t         ammomax;          ///< Ammo stuff
    uint16_t         ammo;

    // equipment and inventory
    std::array<CHR_REF, SLOT_COUNT> holdingwhich; ///< != INVALID_CHR_REF if character is holding something
    std::array<CHR_REF, INVEN_COUNT> equipment;   ///< != INVALID_CHR_REF if character has equipped something
    std::array<CHR_REF, MAXNUMINPACK> inventory;  ///< != INVALID_CHR_REF if character has something in the inventory

    // team stuff
    TEAM_REF       team;            ///< Character's team
    TEAM_REF       team_base;        ///< Character's starting team

    // enchant data
    ENC_REF        firstenchant;                  ///< Linked list for enchants
    ENC_REF        undoenchant;                   ///< Last enchantment spawned

    float          fat_stt;                       ///< Character's initial size
    float          fat;                           ///< Character's size
    float          fat_goto;                      ///< Character's size goto
    int16_t         fat_goto_time;                 ///< Time left in size change

    // jump stuff
    float          jump_power;                    ///< Jump power
    uint8_t          jump_timer;                      ///< Delay until next jump
    uint8_t          jumpnumber;                    ///< Number of jumps remaining
    uint8_t          jumpnumberreset;               ///< Number of jumps total, 255=Flying
    uint8_t          jumpready;                     ///< For standing on a platform character

    // attachments
    CHR_REF        attachedto;                    ///< != INVALID_CHR_REF if character is a held weapon
    slot_t         inwhich_slot;                  ///< SLOT_LEFT or SLOT_RIGHT
    CHR_REF        inwhich_inventory;             ///< != INVALID_CHR_REF if character is inside an inventory

    // platform stuff
    bool         platform;                      ///< Can it be stood on
    bool         canuseplatforms;               ///< Can use platforms?
    int            holdingweight;                 ///< For weighted buttons
    float          targetplatform_level;          ///< What is the height of the target platform?
    CHR_REF        targetplatform_ref;            ///< Am I trying to attach to a platform?
    CHR_REF        onwhichplatform_ref;           ///< Am I on a platform?
    uint32_t         onwhichplatform_update;        ///< When was the last platform attachment made?

    // combat stuff
    uint8_t          damagetarget_damagetype;       ///< Type of damage for AI DamageTarget
    uint8_t          reaffirm_damagetype;           ///< For relighting torches
    std::array<uint8_t, DAMAGE_COUNT> damage_modifier; ///< Damage inversion
    std::array<float, DAMAGE_COUNT> damage_resistance; ///< Damage Resistances
    uint8_t          defense;                       ///< Base defense rating
    SFP8_T         damage_boost;                  ///< Add to swipe damage (8.8 fixed point)
    SFP8_T         damage_threshold;              ///< Damage below this number is ignored (8.8 fixed point)

    // missle handling
    uint8_t          missiletreatment;              ///< For deflection, etc.
    uint8_t          missilecost;                   ///< Mana cost for each one
    CHR_REF        missilehandler;                ///< Who pays the bill for each one...

    // "variable" properties
    bool         is_hidden;
    bool         alive;                         ///< Is it alive?
    bool         waskilled;                     ///< Fix for network
    PLA_REF        is_which_player;               ///< true = player
    bool         islocalplayer;                 ///< true = local player
    bool         invictus;                      ///< Totally invincible?
    bool         iskursed;                      ///< Can't be dropped?
    bool         nameknown;                     ///< Is the name known?
    bool         ammoknown;                     ///< Is the ammo known?
    bool         hitready;                      ///< Was it just dropped?
    bool         isequipped;                    ///< For boots and rings and stuff

    // "constant" properties
    bool         isitem;                        ///< Is it grabbable?
    bool         cangrabmoney;                  ///< Picks up coins?
    bool         openstuff;                     ///< Can it open chests/doors?
    bool         stickybutt;                    ///< Rests on floor
    bool         isshopitem;                    ///< Spawned in a shop?
    //bool         ismount;                       ///< Can you ride it?
    bool         canbecrushed;                  ///< Crush in a door?
    bool         canchannel;                    ///< Can it convert life to mana?

    // misc timers
    int16_t         grog_timer;                    ///< Grog timer
    int16_t         daze_timer;                    ///< Daze timer
    int16_t         bore_timer;                    ///< Boredom timer
    uint8_t          careful_timer;                 ///< "You hurt me!" timer
    uint16_t         reload_timer;                  ///< Time before another shot
    uint8_t          damage_timer;                  ///< Invincibility timer

    // graphica info
    uint8_t          flashand;        ///< 1,3,7,15,31 = Flash, 255 = Don't
    bool         transferblend;   ///< Give transparency to weapons?
    bool         draw_icon;       ///< Show the icon?
    uint8_t          sparkle;         ///< Sparkle color or 0 for off
    bool         show_stats;      ///< Display stats?
    SFP8_T         uoffvel;         ///< Moving texture speed (8.8 fixed point)
    SFP8_T         voffvel;          ///< Moving texture speed (8.8 fixed point)
    float          shadow_size_stt;  ///< Initial shadow size
    uint32_t         shadow_size;      ///< Size of shadow
    uint32_t         shadow_size_save; ///< Without size modifiers
    BBOARD_REF     ibillboard;       ///< The attached billboard

    // model info
    bool         is_overlay;                    ///< Is this an overlay? Track aitarget...
    SKIN_T         skin;                          ///< Character's skin
    PRO_REF        profile_ref;                      ///< Character's profile
    PRO_REF        basemodel_ref;                     ///< The true form
    chr_instance_t inst;                          ///< the render data

    // Skills
    int           darkvision_level;
    int           see_kurse_level;
    int           see_invisible_level;
    IDSZ_node_t   skills[MAX_IDSZ_MAP_SIZE];

    // collision info

    /// @note - to make it easier for things to "hit" one another (like a damage particle from
    ///        a torch hitting a grub bug), Aaron sometimes made the bumper size much different
    ///        than the shape of the actual object.
    ///        The old bumper data that is read from the data.txt file will be kept in
    ///        the struct "bump". A new bumper that actually matches the size of the object will
    ///        be kept in the struct "collision"
    bumper_t     bump_stt;
    bumper_t     bump;
    bumper_t     bump_save;

    bumper_t     bump_1;       ///< the loosest collision volume that mimics the current bump
    oct_bb_t     chr_max_cv;   ///< a looser collision volume for chr-prt interactions
    oct_bb_t     chr_min_cv;   ///< the tightest collision volume for chr-chr interactions

    std::array<oct_bb_t, SLOT_COUNT>     slot_cv;  ///< the cv's for the object's slots

    uint8_t        stoppedby;                     ///< Collision mask

    // character location data
    fvec3_t        pos_stt;                       ///< Starting position
    fvec3_t        vel;                           ///< Character's velocity
    orientation_t  ori;                           ///< Character's orientation

    fvec3_t        pos_old;                       ///< Character's last position
    fvec3_t        vel_old;                       ///< Character's last velocity
    orientation_t  ori_old;                       ///< Character's last orientation

    uint32_t         onwhichgrid;                   ///< Where the char is
    uint32_t         onwhichblock;                  ///< The character's collision block
    CHR_REF        bumplist_next;                 ///< Next character on fanblock

    // movement properties
    bool         waterwalk;                     ///< Always above watersurfacelevel?
    turn_mode_t  turnmode;                      ///< Turning mode

    BIT_FIELD      movement_bits;                 ///< What movement modes are allowed?
    float          anim_speed_sneak;              ///< Movement rate of the sneak animation
    float          anim_speed_walk;               ///< Walking if above this speed
    float          anim_speed_run;                ///< Running if above this speed
    float          maxaccel;                      ///< The actual maximum acelleration
    float          maxaccel_reset;                ///< The current maxaccel_reset
    uint8_t          flyheight;                     ///< Height to stabilize at

    // data for doing the physics in bump_all_objects()
    phys_data_t       phys;
    chr_environment_t enviro;

    int               dismount_timer;                ///< a timer BB added in to make mounts and dismounts not so unpredictable
    CHR_REF           dismount_object;               ///< the object that you were dismounting from

    bool         safe_valid;                    ///< is the last "safe" position valid?
    fvec3_t        safe_pos;                      ///< the last "safe" position
    uint32_t         safe_time;                     ///< the last "safe" time
    uint32_t         safe_grid;                     ///< the last "safe" grid

    breadcrumb_list_t crumbs;                     ///< a list of previous valid positions that the object has passed through

private:
    CHR_REF _characterID;                               ///< Our unique CHR_REF id
    const std::shared_ptr<ObjectProfile> &_profile;     ///< Our Profile

    fvec3_t _position;                ///< Character's position
};