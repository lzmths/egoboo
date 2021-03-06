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

#define EGOLIB_PROFILES_PRIVATE 1
#include "egolib/Profiles/EnchantProfileReader.hpp"

#include "egolib/Audio/AudioSystem.hpp"
#include "egolib/FileFormats/template.h"
#include "egolib/strutil.h"
#include "egolib/fileutil.h"
#include "egolib/Core/StringUtilities.hpp"
#include "egolib/vfs.h"
#include "egolib/_math.h"

bool EnchantProfileReader::read(std::shared_ptr<eve_t> profile, const std::string& pathname)
{
    char cTmp;

    if (!profile) return false;

    profile->reset();

    ReadContext ctxt(pathname);
    if (!ctxt.ensureOpen())
    {
        return false;
    }

    // true/false values
    profile->retarget = vfs_get_next_bool(ctxt);
    profile->_override = vfs_get_next_bool(ctxt);
    profile->remove_overridden = vfs_get_next_bool(ctxt);
    profile->killtargetonend = vfs_get_next_bool(ctxt);

    profile->poofonend = vfs_get_next_bool(ctxt);

    // More stuff
    profile->lifetime = vfs_get_next_int(ctxt);
    profile->endmessage = vfs_get_next_int(ctxt);

    // Drain stuff
    profile->_owner._manaDrain = vfs_get_next_float(ctxt);
    profile->_target._manaDrain = vfs_get_next_float(ctxt);
    profile->endIfCannotPay = vfs_get_next_bool(ctxt);
    profile->_owner._lifeDrain = vfs_get_next_float(ctxt);
    profile->_target._lifeDrain = vfs_get_next_float(ctxt);

    // Specifics
    profile->required_damagetype = vfs_get_next_damage_type(ctxt);
    profile->require_damagetarget_damagetype = vfs_get_next_damage_type(ctxt);
    profile->removedByIDSZ = vfs_get_next_idsz(ctxt);

    // Now the set values
    profile->_set[eve_t::SETDAMAGETYPE].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETDAMAGETYPE].value = vfs_get_damage_type(ctxt);

    profile->_set[eve_t::SETNUMBEROFJUMPS].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETNUMBEROFJUMPS].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETLIFEBARCOLOR].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETLIFEBARCOLOR].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETMANABARCOLOR].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETMANABARCOLOR].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETSLASHMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETSLASHMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDSLASHRESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETCRUSHMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETCRUSHMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDCRUSHRESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETPOKEMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETPOKEMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDPOKERESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETHOLYMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETHOLYMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDHOLYRESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETEVILMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETEVILMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDEVILRESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETFIREMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETFIREMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDFIRERESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETICEMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETICEMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDICERESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETZAPMODIFIER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETZAPMODIFIER].value = vfs_get_damage_modifier(ctxt);
    profile->_add[eve_t::ADDZAPRESIST].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETFLASHINGAND].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETFLASHINGAND].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETLIGHTBLEND].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETLIGHTBLEND].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETALPHABLEND].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETALPHABLEND].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETSHEEN].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETSHEEN].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETFLYTOHEIGHT].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETFLYTOHEIGHT].value = ctxt.readIntegerLiteral();

    profile->_set[eve_t::SETWALKONWATER].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETWALKONWATER].value = ctxt.readBool();

    profile->_set[eve_t::SETCANSEEINVISIBLE].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETCANSEEINVISIBLE].value = ctxt.readBool();

    profile->_set[eve_t::SETMISSILETREATMENT].apply = vfs_get_next_bool(ctxt);
    cTmp = ctxt.readPrintable();
    if ('R' == Ego::toupper(cTmp)) profile->_set[eve_t::SETMISSILETREATMENT].value = MISSILE_REFLECT;
    else if ('D' == Ego::toupper(cTmp)) profile->_set[eve_t::SETMISSILETREATMENT].value = MISSILE_DEFLECT;
    else profile->_set[eve_t::SETMISSILETREATMENT].value = MISSILE_NORMAL;

    profile->_set[eve_t::SETCOSTFOREACHMISSILE].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETCOSTFOREACHMISSILE].value = ctxt.readRealLiteral();

    profile->_set[eve_t::SETMORPH].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETMORPH].value = true;  // vfs_get_bool( fileread );        //ZF> huh? why always channel and morph?

    profile->_set[eve_t::SETCHANNEL].apply = vfs_get_next_bool(ctxt);
    profile->_set[eve_t::SETCHANNEL].value = true;  // vfs_get_bool( fileread );

    // Now read in the add values
    profile->_add[eve_t::ADDJUMPPOWER].value = vfs_get_next_float(ctxt);
    profile->_add[eve_t::ADDBUMPDAMPEN].value = vfs_get_next_int(ctxt) / 256.0f;    // Stored as 8.8-fixed, used as float
    profile->_add[eve_t::ADDBOUNCINESS].value = vfs_get_next_int(ctxt) / 256.0f;    // Stored as 8.8-fixed, used as float
    profile->_add[eve_t::ADDDAMAGE].value = vfs_get_next_float(ctxt);            // Stored as float, used as 8.8-fixed
    profile->_add[eve_t::ADDSIZE].value = vfs_get_next_float(ctxt);           // Stored as float, used as float
    profile->_add[eve_t::ADDACCEL].value = vfs_get_next_int(ctxt) / 80.0f;   // Stored as int, used as float
    profile->_add[eve_t::ADDRED].value = vfs_get_next_int(ctxt);
    profile->_add[eve_t::ADDGRN].value = vfs_get_next_int(ctxt);
    profile->_add[eve_t::ADDBLU].value = vfs_get_next_int(ctxt);
    profile->_add[eve_t::ADDDEFENSE].value = -vfs_get_next_int(ctxt);    // Defense is backwards
    profile->_add[eve_t::ADDMANA].value = vfs_get_next_float(ctxt);    // Stored as float, used as 8.8-fixed
    profile->_add[eve_t::ADDLIFE].value = vfs_get_next_float(ctxt);    // Stored as float, used as 8.8-fixed
    profile->_add[eve_t::ADDSTRENGTH].value = vfs_get_next_float(ctxt);    // Stored as float, used as 8.8-fixed
    profile->_add[eve_t::ADDWISDOM].value = vfs_get_next_float(ctxt);    // Deprecated (not used)
    profile->_add[eve_t::ADDINTELLIGENCE].value = vfs_get_next_float(ctxt);    // Stored as float, used as 8.8-fixed
    profile->_add[eve_t::ADDDEXTERITY].value = vfs_get_next_float(ctxt);    // Stored as float, used as 8.8-fixed

    // Determine which entries are not important
    for (size_t cnt = 0; cnt < eve_t::MAX_ENCHANT_ADD; cnt++)
    {
        profile->_add[cnt].apply = (0.0f != profile->_add[cnt].value);
    }
    profile->_add[eve_t::ADDFIRERESIST].apply = profile->_set[eve_t::SETFIREMODIFIER].apply;
    profile->_add[eve_t::ADDEVILRESIST].apply = profile->_set[eve_t::SETEVILMODIFIER].apply;
    profile->_add[eve_t::ADDZAPRESIST].apply = profile->_set[eve_t::SETZAPMODIFIER].apply;
    profile->_add[eve_t::ADDICERESIST].apply = profile->_set[eve_t::SETICEMODIFIER].apply;
    profile->_add[eve_t::ADDHOLYRESIST].apply = profile->_set[eve_t::SETHOLYMODIFIER].apply;
    profile->_add[eve_t::ADDPOKERESIST].apply = profile->_set[eve_t::SETPOKEMODIFIER].apply;
    profile->_add[eve_t::ADDSLASHRESIST].apply = profile->_set[eve_t::SETSLASHMODIFIER].apply;
    profile->_add[eve_t::ADDCRUSHRESIST].apply = profile->_set[eve_t::SETCRUSHMODIFIER].apply;

    // Read expansions
    while (ctxt.skipToColon(true))
    {
        switch(ctxt.readIDSZ())
        {
            case MAKE_IDSZ('A', 'M', 'O', 'U'): profile->contspawn._amount = ctxt.readIntegerLiteral(); break;
            case MAKE_IDSZ('T', 'Y', 'P', 'E'): profile->contspawn._lpip = vfs_get_local_particle_profile_ref(ctxt); break;
            case MAKE_IDSZ('T', 'I', 'M', 'E'): profile->contspawn._delay = ctxt.readIntegerLiteral(); break;
            case MAKE_IDSZ('F', 'A', 'C', 'E'): profile->contspawn._facingAdd = ctxt.readIntegerLiteral(); break;
            case MAKE_IDSZ('S', 'E', 'N', 'D'): profile->endsound_index = ctxt.readIntegerLiteral(); break;
            case MAKE_IDSZ('S', 'T', 'A', 'Y'): profile->_owner._stay = (0 != ctxt.readIntegerLiteral()); break;
            case MAKE_IDSZ('O', 'V', 'E', 'R'): profile->spawn_overlay = (0 != ctxt.readIntegerLiteral()); break;
            case MAKE_IDSZ('D', 'E', 'A', 'D'): profile->_target._stay = (0 != ctxt.readIntegerLiteral()); break;
            case MAKE_IDSZ('C', 'K', 'U', 'R'): profile->seeKurses = ctxt.readIntegerLiteral(); break;
            case MAKE_IDSZ('D', 'A', 'R', 'K'): profile->darkvision = ctxt.readIntegerLiteral(); break;
            case MAKE_IDSZ('N', 'A', 'M', 'E'): profile->setEnchantName(ctxt.readName()); break;
            default: /*TODO: log error*/ break;
        }
    }

    profile->_name = pathname;
    profile->_loaded = true;

    // Limit the endsound_index.
    profile->endsound_index = Ego::Math::constrain<Sint16>(profile->endsound_index, INVALID_SOUND_ID, MAX_WAVE);

    return true;
}
