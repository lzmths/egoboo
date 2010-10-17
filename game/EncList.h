#pragma once

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

/// @file EncList.h
/// @brief Routines for enchant list management

#include "egoboo_object.h"
#include "enchant.h"

#include "egoboo_object_list.inl"

//--------------------------------------------------------------------------------------------
// MACROS
//--------------------------------------------------------------------------------------------

#define VALID_ENC_IDX( INDX )      ( ((INDX) >= 0) && ((INDX) < MAX_ENC) )
#define VALID_ENC_REF( IENC )      EncList.validate_ref(IENC)
#define ALLOCATED_ENC( IENC )      ( VALID_ENC_REF( IENC ) && ALLOCATED_PBASE ( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )
#define VALID_ENC( IENC )          ( VALID_ENC_REF( IENC ) && VALID_PBASE     ( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )
#define DEFINED_ENC( IENC )        ( VALID_ENC_REF( IENC ) && VALID_PBASE     ( POBJ_GET_PBASE(EncList.lst + (IENC)) ) && !TERMINATED_PBASE ( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )
#define ACTIVE_ENC( IENC )         ( VALID_ENC_REF( IENC ) && ACTIVE_PBASE    ( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )
#define WAITING_ENC( IENC )        ( VALID_ENC_REF( IENC ) && WAITING_PBASE   ( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )
#define TERMINATED_ENC( IENC )     ( VALID_ENC_REF( IENC ) && TERMINATED_PBASE( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )

#define GET_IDX_PENC( PENC )        ((size_t)GET_IDX_POBJ( PENC, MAX_ENC ))
#define GET_REF_PENC( PENC )        ((ENC_REF)GET_REF_POBJ( PENC, MAX_ENC ))
#define VALID_ENC_PTR( PENC )       ( (NULL != (PENC)) && VALID_ENC_REF( GET_REF_PENC( PENC ) ) )
#define ALLOCATED_PENC( PENC )      ( VALID_ENC_PTR( PENC ) && ALLOCATED_PBASE( POBJ_GET_PBASE(PENC) ) )
#define VALID_PENC( PENC )          ( VALID_ENC_PTR( PENC ) && VALID_PBASE( POBJ_GET_PBASE(PENC) ) )
#define DEFINED_PENC( PENC )        ( VALID_ENC_PTR( PENC ) && VALID_PBASE ( POBJ_GET_PBASE(PENC) ) && !TERMINATED_PBASE ( POBJ_GET_PBASE(PENC) ) )
#define ACTIVE_PENC( PENC )         ( VALID_ENC_PTR( PENC ) && ACTIVE_PBASE( POBJ_GET_PBASE(PENC) ) )
#define TERMINATED_PENC( PENC )     ( VALID_ENC_PTR( PENC ) && TERMINATED_PBASE( POBJ_GET_PBASE(PENC) ) )

// Macros automate looping through the EncList. This hides code which defers the creation and deletion of
// objects until the loop terminates, so that the length of the list will not change during the loop.
#define ENC_BEGIN_LOOP_ACTIVE(IT, PENC)  {size_t IT##_internal; int enc_loop_start_depth = EncList.loop_depth; EncList.loop_depth++; for(IT##_internal=0;IT##_internal<EncList.used_count;IT##_internal++) { ENC_REF IT; ego_enc * PENC = NULL; IT = (ENC_REF)EncList.used_ref[IT##_internal]; if(!ACTIVE_ENC (IT)) continue; PENC =  EncList.get_valid_ptr(IT);
#define ENC_BEGIN_LOOP_BSP(IT, PENC)     {size_t IT##_internal; int enc_loop_start_depth = EncList.loop_depth; EncList.loop_depth++; for(IT##_internal=0;IT##_internal<EncList.used_count;IT##_internal++) { ENC_REF IT; ego_enc * PENC = NULL; IT = (ENC_REF)EncList.used_ref[IT##_internal]; if(!ACTIVE_ENC (IT)) continue; PENC =  EncList.get_valid_ptr(IT); if( !PENC->bsp_leaf.inserted ) continue;
#define ENC_END_LOOP() } EncList.loop_depth--; if(enc_loop_start_depth != EncList.loop_depth) EGOBOO_ASSERT(bfalse); EncList.cleanup(); }


// Macros to determine whether the enchant is in the game or not.
// If objects are being spawned, then any object that is just "defined" is treated as "in game"
#define INGAME_ENC_BASE(IENC)       ( VALID_ENC_REF( IENC ) && ACTIVE_PBASE( POBJ_GET_PBASE(EncList.lst + (IENC)) ) && ON_PBASE( POBJ_GET_PBASE(EncList.lst + (IENC)) ) )
#define INGAME_PENC_BASE(PENC)      ( VALID_ENC_PTR( PENC ) && ACTIVE_PBASE( POBJ_GET_PBASE(PENC) ) && ON_PBASE( POBJ_GET_PBASE(PENC) ) )

#define INGAME_ENC(IENC)            ( (ego_object::spawn_depth) > 0 ? DEFINED_ENC(IENC) : INGAME_ENC_BASE(IENC) )
#define INGAME_PENC(PENC)           ( (ego_object::spawn_depth) > 0 ? DEFINED_PENC(PENC) : INGAME_PENC_BASE(PENC) )

//--------------------------------------------------------------------------------------------
// list dclaration
//--------------------------------------------------------------------------------------------
extern t_ego_obj_lst<ego_enc,MAX_ENC> EncList;

//--------------------------------------------------------------------------------------------
// Function prototypes
//--------------------------------------------------------------------------------------------

#define _EncList_h