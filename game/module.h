//********************************************************************************************
//* Egoboo - module,h
//*
//* A manager for egoboo modules
//*
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

#pragma once

#include "ogl_texture.h"
#include "egoboo_types.h"


//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------


#define RANKSIZE 8
#define SUMMARYLINES 8
#define SUMMARYSIZE  80

#define MAXMODULE           100                     // Number of modules

//--------------------------------------------------------------------------------------------

struct GameState_t;

//--------------------------------------------------------------------------------------------
typedef struct import_info_t
{
  bool_t valid;                // Can it import?

  int    object;
  int    player;

  int    amount;
  int    min_pla;
  int    max_pla;

  Sint16 slot_lst[1024];

} IMPORT_INFO;

bool_t import_info_clear(IMPORT_INFO * ii);
bool_t import_info_add(IMPORT_INFO * ii, int obj);

//--------------------------------------------------------------------------------------------

typedef struct mod_data_t
{
  char            rank[RANKSIZE];               // Number of stars
  char            longname[32];                 // Module names
  char            loadname[32];                 // Module load names
  Uint8           importamount;                 // # of import characters
  bool_t          allowexport;                  // Export characters?
  Uint8           minplayers;                   // Number of players
  Uint8           maxplayers;                   //
  bool_t          monstersonly;                 // Only allow monsters
  RESPAWN_MODE    respawnmode;                // What kind of respawn is allowed?
  bool_t          rts_control;

  STRING host;                         // what is the host of this module? blank if network not being used.
  Uint32 tx_title_idx;                  // which texture do we use?
  bool_t is_hosted;                    // is this module here, or on the server?
  bool_t is_verified;
} MOD_INFO;

MOD_INFO * ModInfo_new( MOD_INFO * pmi );
bool_t ModInfo_delete( MOD_INFO * pmi );
MOD_INFO * ModInfo_renew( MOD_INFO * pmi );

//--------------------------------------------------------------------------------------------
typedef struct module_summary_t
{
  int    numlines;                                   // Lines in summary
  char   val;
  char   summary[SUMMARYLINES][SUMMARYSIZE];      // Quest description
} MOD_SUMMARY;

MOD_SUMMARY * ModSummary_new( MOD_SUMMARY * ms );
bool_t        ModSummary_delete( MOD_SUMMARY * ms );
MOD_SUMMARY * ModSummary_renew( MOD_SUMMARY * ms );

//--------------------------------------------------------------------------------------------
typedef struct ModState_t
{
  bool_t initialized;

  bool_t Active;                     // Is the control loop still going?
  bool_t Paused;                     // Is the control loop paused?

  bool_t       loaded;
  Uint32       seed;                       // the seed for the module
  bool_t       respawnvalid;               // Can players respawn with Spacebar?
  bool_t       respawnanytime;             // True if it's a small level...
  bool_t       exportvalid;                // Can it export?
  bool_t       rts_control;                // Play as a real-time stragedy? BAD REMOVE
  bool_t       beat;                       // Show Module Ended text?

  IMPORT_INFO import;

} ModState;

ModState * ModState_new(ModState * ms, MOD_INFO * mi, Uint32 seed);
bool_t     ModState_delete(ModState * ms);
ModState * ModState_renew(ModState * ms, MOD_INFO * mi, Uint32 seed);

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

extern GLtexture TxTitleImage[MAXMODULE];      /* title images */

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

bool_t module_load( struct GameState_t * gs, char *smallname );
void   module_release(  struct GameState_t * gs  );
void   module_quit( struct GameState_t * gs );

bool_t module_reference_matches( char *szLoadName, IDSZ idsz );
void   module_add_idsz( char *szLoadName, IDSZ idsz );
int    module_find( char *smallname, MOD_INFO * mi_ary, size_t mi_size );
Uint32 module_load_one_image( int titleimage, char *szLoadName );
bool_t module_read_data( struct mod_data_t * pmod, char *szLoadName );
bool_t module_read_summary( char *szLoadName, MOD_SUMMARY * ms );
size_t module_load_all_data(MOD_INFO * mi, size_t sz);