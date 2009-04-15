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
//*    along with Egoboo.  If not, see <http:// www.gnu.org/licenses/>.
//*
//********************************************************************************************

/* Egoboo - egoboo.h
 * Disgusting, hairy, way too monolithic header file for the whole darn
 * project.  In severe need of cleaning up.  Venture here with extreme
 * caution, and bring one of those canaries with you to make sure you
 * don't run out of oxygen.
 */

/* Typedefs for various platforms */
#include "egobootypedef.h"

#include "proto.h"
#include "sound.h"

#include "gltexture.h"  /* OpenGL texture loader */
#include "egoboo_math.h"  /* vector and matrix math */
#include "configfile.h"
#include "Md2.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#include <SDL_opengl.h>

//OPENGL VERTEX
typedef struct
{
    GLfloat x, y, z, w;
    GLfloat r, g, b, a;
    Uint32   color; // should replace r,g,b,a and be called by glColor4ubv
    GLfloat s, t; // u and v in D3D I guess
} GLVERTEX;

// The following magic allows this include to work in multiple files
#ifdef DECLARE_GLOBALS
#define EXTERN
#define EQ(x) =x;
#else
#define EXTERN extern
#define EQ(x)
#endif

#define VERSION "2.7.0c"                         // Version of the game

EXTERN bool_t        gamepaused EQ( bfalse );    // Is the game paused?
EXTERN bool_t        pausekeyready EQ( btrue );  // Is the game paused?
EXTERN bool_t    overrideslots EQ( bfalse );     //Override existing slots?
EXTERN bool_t    screenshotkeyready EQ( btrue );    // Ready to take screenshot?
#define EXPKEEP 0.85f                                // Experience to keep when respawning

#define MAXMODULE           100                     // Number of modules
#define TITLESIZE           128                     // Size of a title image
#define MAXINVENTORY        7
#define MAXIMPORTPERPLAYER  (MAXINVENTORY + 2)
#define MAXIMPORT           (4*MAXIMPORTPERPLAYER)          // Number of subdirs in IMPORT directory

#define NOSPARKLE           255
#define ANYTIME             0xFF          // Code for respawnvalid...

#define SIZETIME            50                      // Time it takes to resize a character

#define NOSKINOVERRIDE      -1                      // For import

EXTERN Uint16    endtextindex[8];

#define DISMOUNTZVEL        16
#define DISMOUNTZVELFLY     4

#define EDGE                128                     // Camera bounds/edge of the map

#define NOHIDE              127                     // Don't hide

// Stats
#define MANARETURNSHIFT     22                       //
#define LOWSTAT             256                     // Worst...
#define PERFECTSTAT         (70*256)                // Maximum stat without magic effects
#define PERFECTBIG          (100*256)               // Perfect life or mana...
#define HIGHSTAT            (100*256)                // Absolute max adding enchantments as well

EXTERN int wraptolerance  EQ( 80 );        // Status bar

#define DAMFXNONE           0                       // Damage effects
#define DAMFXARMO           1                       // Armor piercing
#define DAMFXBLOC           2                       // Cannot be blocked by shield
#define DAMFXARRO           4                       // Only hurts the one it's attached to
#define DAMFXTURN           8                       // Turn to attached direction
#define DAMFXTIME           16                      //

#define HURTDAMAGE           256                     // Minimum damage for hurt animation

#define ULTRABLUDY         2                       // This makes any damage draw blud

#define SPELLBOOK           127                     // The spellbook model

// Geneder stuff
#define GENFEMALE           0                       // Gender
#define GENMALE             1                       //
#define GENOTHER            2                       //
#define GENRANDOM           3                       //

#define MAXPASS             256                     // Maximum number of passages ( mul 32 )
#define MAXSTAT             16                      // Maximum status displays

#define JOYBUTTON           32                      // Maximum number of joystick buttons

// Messaging stuff
#define MAXMESSAGE          6                       // Number of messages
#define MAXTOTALMESSAGE     4096                    //
#define MESSAGESIZE         80                      //
#define MESSAGEBUFFERSIZE   (MAXTOTALMESSAGE*40)
EXTERN Uint16 messagetime   EQ(200);                     // Time to keep the message alive
#define TABAND              31                      // Tab size

// Model tags
#define MADFXINVICTUS       1                       // I Invincible
#define MADFXACTLEFT        2                       // AL Activate left item
#define MADFXACTRIGHT       4                       // AR Activate right item
#define MADFXGRABLEFT       8                       // GL GO Grab left/Grab only item
#define MADFXGRABRIGHT      16                      // GR Grab right item
#define MADFXDROPLEFT       32                      // DL DO Drop left/Drop only item
#define MADFXDROPRIGHT      64                      // DR Drop right item
#define MADFXSTOP           128                     // S Stop movement
#define MADFXFOOTFALL       256                     // F Footfall sound
#define MADFXCHARLEFT       512                     // CL Grab left/Grab only character
#define MADFXCHARRIGHT      1024                    // CR Grab right character
#define MADFXPOOF           2048                    // P Poof

// Animation walking
#define LIPDA               0                       // For smooth transitions 'tween
#define LIPWA               1                       //   walking rates
#define LIPWB               2                       //
#define LIPWC               3                       //

#define NOACTION            0xffff                     // Action not valid for this character
#define MAXACTION           76                         // Number of action types
EXTERN char                 cActionName[MAXACTION][2]; // Two letter name code

#define GRABSIZE            90.0f                      // Grab tolerance
#define SEEINVISIBLE        128                        // Cutoff for invisible characters

#define DAMAGENULL          255                        //

enum e_damage_type
{
    DAMAGE_SLASH = 0,                        //
    DAMAGE_CRUSH,                            //
    DAMAGE_POKE,                             //
    DAMAGE_HOLY,                             // (Most invert Holy damage )
    DAMAGE_EVIL,                             //
    DAMAGE_FIRE,                             //
    DAMAGE_ICE,                              //
    DAMAGE_ZAP,                              //
    DAMAGE_COUNT                             // Damage types
};

#define DAMAGECHARGE        8                       // 0000x000 Converts damage to mana
#define DAMAGEINVERT        4                       // 00000x00 Makes damage heal
#define DAMAGESHIFT         3                       // 000000xx Resistance ( 1 is common )
#define DAMAGETILETIME      32                      // Invincibility time
#define DAMAGETIME          16                      // Invincibility time
#define DEFENDTIME          16                      // Invincibility time
#define DROPXYVEL           8                       //
#define DROPZVEL            7                       //
#define JUMPATTACKVEL       -2                      //
#define WATERJUMP           12                      //

#define TURNMODEVELOCITY    0                       // Character gets rotation from velocity
#define TURNMODEWATCH       1                       // For watch towers
#define TURNMODESPIN        2                       // For spinning objects
#define TURNMODEWATCHTARGET 3                       // For combat intensive AI

#define MAXCAPNAMESIZE      32                      // Character class names
#define MAXLEVEL            6                       // Basic Levels 0-5

enum e_idsz_type
{
    IDSZ_PARENT = 0,                             // Parent index
    IDSZ_TYPE,                                   // Self index
    IDSZ_SKILL,                                  // Skill index
    IDSZ_SPECIAL,                                // Special index
    IDSZ_HATE,                                   // Hate index
    IDSZ_VULNERABILITY,                          // Vulnerability index
    IDSZ_COUNT                                   // ID strings per character
};

// XP stuff
enum e_xp_type
{
    XP_FINDSECRET = 0,                          // Finding a secret
    XP_WINQUEST,                                // Beating a module or a subquest
    XP_USEDUNKOWN,                              // Used an unknown item
    XP_KILLENEMY,                               // Killed an enemy
    XP_KILLSLEEPY,                              // Killed a sleeping enemy
    XP_KILLHATED,                               // Killed a hated enemy
    XP_TEAMKILL,                                // Team has killed an enemy
    XP_TALKGOOD,                                // Talk good, er...  I mean well
    XP_COUNT                                    // Number of ways to get experience
};

#define XPDIRECT            255                     // No modification

// Teams
#define MAXTEAM             27                      // Teams A-Z, +1 more for damage tiles
#define DAMAGETEAM          26                      // For damage tiles
#define EVILTEAM            4                       // E
#define GOODTEAM            6                       // G
#define NULLTEAM            13                      // N

#define LATCHBUTTON_LEFT      ( 1 << 0 )                      // Character button presses
#define LATCHBUTTON_RIGHT     ( 1 << 1 )                      //
#define LATCHBUTTON_JUMP      ( 1 << 2 )                      //
#define LATCHBUTTON_ALTLEFT   ( 1 << 3 )                      // ( Alts are for grab/drop )
#define LATCHBUTTON_ALTRIGHT  ( 1 << 4 )                     //
#define LATCHBUTTON_PACKLEFT  ( 1 << 5 )                     // ( Packs are for inventory cycle )
#define LATCHBUTTON_PACKRIGHT ( 1 << 6 )                     //
#define LATCHBUTTON_RESPAWN   ( 1 << 7 )                    //

// Z velocity stuff
#define JUMPDELAY           20                      // Time between jumps

#define MAPID 0x4470614d                        // The string... MapD

#define RAISE 12 //25                               // Helps correct z level
#define SHADOWRAISE 5                               //
#define DAMAGERAISE 25                              //

#define MAXWATERLAYER 2                             // Maximum water layers
#define MAXWATERFRAME 512                           // Maximum number of wave frames
#define WATERFRAMEAND (MAXWATERFRAME-1)             //
#define WATERPOINTS 4                               // Points in a water fan
#define WATERMODE 4                                 // Ummm...  For making it work, yeah...

#define ROTMESHTOPSIDE                  55          // For figuring out what to draw
#define ROTMESHBOTTOMSIDE               65          //
#define ROTMESHUP                       40 //35          //
#define ROTMESHDOWN                     60          //
EXTERN int rotmeshtopside;                                 // The ones that get used
EXTERN int rotmeshbottomside;                              //
EXTERN int rotmeshup;                                      //
EXTERN int rotmeshdown;                                    //

#define NUMFONTX                        16          // Number of fonts in the bitmap
#define NUMFONTY                        6           //
#define NUMFONT                         (NUMFONTX*NUMFONTY)
#define FONTADD                         4           // Gap between letters
#define NUMBAR                          6           // Number of status bars

#define TABX                            32//16      // Size of little name tag on the bar
#define BARX                            112//216         // Size of bar
#define BARY                            16//8           //
#define NUMTICK                         10//50          // Number of ticks per row
#define TICKX                           8//4           // X size of each tick
#define MAXTICK                         (NUMTICK*10) // Max number of ticks to draw

#define TURNSPD                         0.01f         // Cutoff for turning or same direction

#define MD2START                        0x32504449  // MD2 files start with these four bytes
#define MD2MAXLOADSIZE                  (512*1024)  // Don't load any models bigger than 512k
#define MD2LIGHTINDICES                 163//162    // MD2's store vertices as x,y,z,normal
#define EQUALLIGHTINDEX                 162         // I added an extra index to do the
// spikey mace...

#define MAXTEXTURE                      768         // Max number of textures
#define MAXVERTICES                     2048        // Max number of points in a model
#define MAXCOMMAND                      512         // Max number of commands
#define MAXCOMMANDSIZE                  128          // Max number of points in a command
#define MAXCOMMANDENTRIES               512         // Max entries in a command list ( trigs )
#define MAXMODEL                        256         // Max number of models
#define MAXEVE                          MAXMODEL    // One enchant type per model
#define MAXEVESETVALUE                  24          // Number of sets
#define MAXEVEADDVALUE                  16          // Number of adds
#define MAXFRAME                        (128*32)    // Max number of frames in all models
#define MAXCHR                          512         // Max number of characters

//Object positions
#define MAXSLOT                          2
#define SLOT_LEFT                        0
#define SLOT_RIGHT                       1

#define ATTACH_NONE                      0
#define ATTACH_INVENTORY                 1
#define ATTACH_LEFT                      2
#define ATTACH_RIGHT                     3

#define GRIP_VERTS                       4
#define GRIP_RIGHT                   (2 * GRIP_VERTS)  // Right weapon grip starts 8 from last
#define GRIP_LEFT                    (1 * GRIP_VERTS)  // Left weapon grip starts 4 from last
#define GRIP_ONLY                        GRIP_LEFT     // Only weapon grip starts 4 from last
#define GRIP_LAST                        1             // Spawn particles at the last vertex
#define GRIP_ORIGIN                      0             // Spawn attachments at the center
#define GRIP_INVENTORY                   0             //

#define CHOPPERMODEL                    32          //
#define MAXSECTION                      4           // T-wi-n-k...  Most of 4 sections
#define MAXCHOP                         (MAXMODEL*CHOPPERMODEL)
#define CHOPSIZE                        8
#define CHOPDATACHUNK                   (MAXCHOP*CHOPSIZE)

#define MAXMESHFAN                      (512*512)   // Terrain mesh size
#define MAXMESHTILEY                    1024        // Max tiles in y direction
#define MAXMESHBLOCKY                   (( MAXMESHTILEY >> 2 )+1)  // max blocks in the y direction
#define BYTESFOREACHVERTEX              14            // 14 bytes each
#define MAXMESHVERTICES                 16            // Fansquare vertices
#define MAXMESHTYPE                     64            // Number of fansquare command types
#define MAXMESHCOMMAND                  4             // Draw up to 4 fans
#define MAXMESHCOMMANDENTRIES           32            // Fansquare command list size
#define MAXMESHCOMMANDSIZE              32            // Max trigs in each command
#define MAXTILETYPE                     256           // Max number of tile images
#define MAXMESHRENDER                   1024          // Max number of tiles to draw
#define FANOFF                          0xffff        // Don't draw the fansquare if tile = this

#define MESHFX_REF                       0           // 0 This tile is drawn 1st
#define MESHFX_SHA                       1           // 0 This tile is drawn 2nd
#define MESHFX_DRAWREF                   2           // 1 Draw reflection of characters
#define MESHFX_ANIM                      4           // 2 Animated tile ( 4 frame )
#define MESHFX_WATER                     8           // 3 Render water above surface ( Water details are set per module )
#define MESHFX_WALL                      16          // 4 Wall ( Passable by ghosts, particles )
#define MESHFX_IMPASS                    32          // 5 Impassable
#define MESHFX_DAMAGE                    64          // 6 Damage
#define MESHFX_SLIPPY                    128         // 7 Ice or normal

// Physics
#define SLOPE                           800         // Slope increments for terrain normals
#define SLIDE                           0.04f         // Acceleration for steep hills
#define SLIDEFIX                        0.08f         // To make almost flat surfaces flat

#define STOPBOUNCING                    0.1f //1.0f         // To make objects stop bouncing
#define STOPBOUNCINGPART                5.0f         // To make particles stop bouncing

// Particles
#define PRTLIGHTSPRITE                  0           // Magic effect particle
#define PRTSOLIDSPRITE                  1           // Sprite particle
#define PRTALPHASPRITE                  2           // Smoke particle
#define MAXPARTICLEIMAGE                256         // Number of particle images ( frames )
#define DYNAFANS  12

/* SDL_GetTicks() always returns milli seconds */
#define TICKS_PER_SEC                   1000
EXTERN Sint32 framelimit                EQ(30);
#define UPDATES_PER_SEC                 50
#define UPDATE_SKIP                     ((float)TICKS_PER_SEC/(float)UPDATES_PER_SEC)
#define ONESECOND                       TICKS_PER_SEC

#define TRANSCOLOR                      0           // Transparent color

// Debug option
EXTERN bool_t  gGrabMouse EQ( btrue );
EXTERN bool_t gHideMouse EQ( bfalse );
EXTERN bool_t gDevMode EQ( bfalse );
// Debug option

EXTERN int     animtileupdateand  EQ( 7 );                        // New tile every 7 frames
EXTERN Uint16  animtileframeand  EQ( 3 );              // Only 4 frames
EXTERN Uint16  animtilebaseand  EQ( 0xfffc );          //
EXTERN Uint16  biganimtileframeand  EQ( 7 );           // For big tiles
EXTERN Uint16  biganimtilebaseand  EQ( 0xfff8 );       //
EXTERN Uint16  animtileframeadd  EQ( 0 );              // Current frame

#define NORTH 16384                                 // Character facings
#define SOUTH 49152                                 //
#define EAST 32768                                  //
#define WEST 0                                      //
#define FRONT 0                                     // Attack directions
#define BEHIND 32768                                //
#define LEFT 49152                                  //
#define RIGHT 16384                                 //

#define MAXXP 200000                               // Maximum experience
#define MAXMONEY 9999                               // Maximum money
#define NOLEADER 0xFFFF                              // If the team has no leader...

EXTERN bool_t                   teamhatesteam[MAXTEAM][MAXTEAM];     // Don't damage allies...
EXTERN Uint16                    teammorale[MAXTEAM];                 // Number of characters on team
EXTERN Uint16                    teamleader[MAXTEAM];                 // The leader of the team
EXTERN Uint16                    teamsissy[MAXTEAM];                  // Whoever called for help last
EXTERN Sint16                   damagetileparttype;
EXTERN short                   damagetilepartand;
EXTERN short                   damagetilesound;
EXTERN short                   damagetilesoundtime;
EXTERN Uint16                  damagetilemindistance;

#define TILESOUNDTIME 16
#define TILEREAFFIRMAND  3

// Minimap stuff
#define MAXBLIP 128
#define NUMBLIP 6             //Blip textures
EXTERN Uint16                  numblip  EQ( 0 );
EXTERN Uint16                  blipx[MAXBLIP];
EXTERN Uint16                  blipy[MAXBLIP];
EXTERN Uint8                   blipc[MAXBLIP];
EXTERN Uint8                   mapon  EQ( bfalse );
EXTERN Uint8                   mapvalid  EQ( bfalse );
EXTERN Uint8                   youarehereon  EQ( bfalse );

// Display
EXTERN Uint8                   timeron  EQ( bfalse );          // Game timer displayed?
EXTERN Uint32                  timervalue  EQ( 0 );           // Timer time ( 50ths of a second )
EXTERN char                    szfpstext[]  EQ( "000 FPS" );
EXTERN Uint8                   fpson  EQ( btrue );             // FPS displayed?

// Timers
EXTERN Sint32          clock_stt;                   // GetTickCount at start
EXTERN Sint32          clock_all  EQ( 0 );             // The total number of ticks so far
EXTERN Sint32          clock_lst  EQ( 0 );             // The last total of ticks so far
EXTERN Sint32          clock_wld  EQ( 0 );             // The sync clock
EXTERN Sint32          clock_fps  EQ( 0 );             // The number of ticks this second
EXTERN Uint32          frame_wld  EQ( 0 );             // The number of frames that should have been drawn
EXTERN Uint32          frame_all  EQ( 0 );             // The total number of frames drawn so far
EXTERN Uint32          frame_fps  EQ( 0 );             // The number of frames drawn this second
EXTERN Uint32          clock_stat  EQ( 0 );            // For stat regeneration
EXTERN Uint32          pitclock  EQ( 0 );             // For pit kills
EXTERN Uint32          outofsync  EQ( 0 );
EXTERN Uint8           parseerror  EQ( bfalse );

//Pitty stuff
EXTERN bool_t          pitskill  EQ( bfalse );          // Do they kill?
EXTERN bool_t          pitsfall  EQ( bfalse );          // Do they teleport?
EXTERN Uint32          pitx;
EXTERN Uint32          pity;
EXTERN Uint32          pitz;

EXTERN bool_t                    fullscreen EQ( bfalse );        // Start in fullscreen?
EXTERN bool_t                    clearson  EQ( btrue );             // Do we clear every time?
EXTERN bool_t                    gameactive  EQ( bfalse );       // Stay in game or quit to windows?
EXTERN bool_t                    moduleactive  EQ( bfalse );     // Is the control loop still going?
EXTERN bool_t                    soundon  EQ( btrue );              // Is the sound alive?
EXTERN bool_t                    staton  EQ( btrue );               // Draw the status bars?
EXTERN bool_t                    phongon  EQ( btrue );              // Do phong overlay?
EXTERN bool_t                    networkon  EQ( btrue );            // Try to connect?
EXTERN bool_t                    serviceon  EQ( bfalse );        // Do I need to free the interface?
EXTERN bool_t                    twolayerwateron  EQ( btrue );      // Two layer water?
EXTERN bool_t                    menuactive  EQ( bfalse );       // Menu running?
EXTERN int             selectedPlayer EQ( 0 );           // Which player is currently selected to play
EXTERN bool_t                    hostactive  EQ( bfalse );       // Hosting?
EXTERN bool_t                    readytostart;               // Ready to hit the Start Game button?
EXTERN bool_t                    waitingforplayers;          // Has everyone talked to the host?
EXTERN bool_t                    respawnvalid;               // Can players respawn with Spacebar?
EXTERN bool_t                    respawnanytime;             // True if it's a small level...
EXTERN bool_t                    importvalid;                // Can it import?
EXTERN bool_t                    exportvalid;                // Can it export?
EXTERN bool_t                    rtscontrol;                 // Play as a real-time stragedy? BAD REMOVE
EXTERN bool_t                    backgroundvalid;            // Allow large background?
EXTERN bool_t                    overlayvalid;               // Allow large overlay?
EXTERN bool_t                    local_noplayers;             // Are there any local players?
EXTERN bool_t                    usefaredge;                 // Far edge maps? (Outdoor)
EXTERN bool_t                    beatmodule;                 // Show Module Ended text?
EXTERN Uint8                   autoturncamera;             // Type of camera control...
EXTERN Uint8                   doturntime;                 // Time for smooth turn
EXTERN Uint8                     importamount;               // Number of imports for this module
EXTERN Uint8                     playeramount;               //
EXTERN Uint32                   seed  EQ( 0 );                 // The module seed
EXTERN char                    pickedmodule[64];           // The module load name
EXTERN int                     pickedindex;                // The module index number
EXTERN int                     playersready;               // Number of players ready to start
EXTERN int                     playersloaded;              //

//Respawning
EXTERN bool_t                   local_allpladead;            // Has everyone died?
EXTERN Uint16                   revivetimer EQ(0);

// Networking
EXTERN int                     local_machine  EQ( 0 );        // 0 is host, 1 is 1st remote, 2 is 2nd...
EXTERN int                     numimport;                     // Number of imports from this machine
EXTERN Uint32                  local_control[16];             // Input bits for each imported player
EXTERN short                   local_slot[16];                // For local imports

// Setup values
EXTERN int                     maxmessage  EQ( MAXMESSAGE );  //
EXTERN int                     scrd  EQ( 8 );                 // Screen bit depth
EXTERN int                     scrz  EQ( 16 );                // Screen z-buffer depth ( 8 unsupported )
EXTERN int                     scrx  EQ( 320 );               // Screen X size
EXTERN int                     scry  EQ( 200 );               // Screen Y size
EXTERN Uint8                   reffadeor  EQ( 0 );            // 255 = Don't fade reflections
EXTERN Uint8                   messageon  EQ( btrue );         // Messages?
EXTERN bool_t                  overlayon  EQ( bfalse );        // Draw overlay?
EXTERN bool_t                  perspective  EQ( bfalse );      // Perspective correct textures?
EXTERN bool_t                  dither  EQ( bfalse );           // Dithering?
EXTERN GLuint                  shading  EQ( GL_SMOOTH );           // Gourad shading?
EXTERN Uint8                   antialiasing  EQ( bfalse );     // Antialiasing?
EXTERN bool_t                  refon  EQ( bfalse );            // Reflections?
EXTERN bool_t                  shaon  EQ( bfalse );            // Shadows?
EXTERN Uint8                   texturefilter  EQ( 1 );     // Texture filtering?
EXTERN bool_t                  wateron  EQ( btrue );           // Water overlays?
EXTERN bool_t                  shasprite  EQ( bfalse );        // Shadow sprites?
EXTERN bool_t                  zreflect  EQ( bfalse );         // Reflection z buffering?

EXTERN int              cursorx  EQ( 0 );              // Cursor position
EXTERN int              cursory  EQ( 0 );              //
EXTERN bool_t           pressed EQ( bfalse );                //
EXTERN bool_t           clicked EQ( bfalse );                //
EXTERN bool_t           pending_click EQ( bfalse );
EXTERN bool_t           mouse_wheel_event EQ( bfalse );
// EWWWW. GLOBALS ARE EVIL.


// KEYBOARD
EXTERN Uint8  scancode_to_ascii[SDLK_LAST];
EXTERN Uint8  scancode_to_ascii_shift[SDLK_LAST];
EXTERN bool_t console_mode EQ( bfalse );                   // Input text from keyboard?
EXTERN bool_t console_done EQ( bfalse );                   // Input text from keyboard finished?


#define SDLKEYDOWN(k) ( !console_mode &&  (NULL != keyb.state_ptr) &&  ((k) < keyb.count) && ( 0 != keyb.state_ptr[k] ) )


// Weather and water gfx
EXTERN int                     weatheroverwater EQ( bfalse );       // Only spawn over water?
EXTERN int                     weathertimereset EQ( 10 );          // Rate at which weather particles spawn
EXTERN int                     weathertime EQ( 0 );                // 0 is no weather
EXTERN int                     weatherplayer;
EXTERN int                     numwaterlayer EQ( 0 );              // Number of layers
EXTERN float                   watersurfacelevel EQ( 0 );          // Surface level for water striders
EXTERN float                   waterdouselevel EQ( 0 );            // Surface level for torches
EXTERN Uint8                   waterlight EQ( 0 );                 // Is it light ( default is alpha )
EXTERN Uint8                   waterspekstart EQ( 128 );           // Specular begins at which light value
EXTERN Uint8                   waterspeklevel EQ( 128 );           // General specular amount (0-255)
EXTERN Uint8                   wateriswater  EQ( btrue );          // Is it water?  ( Or lava... )
EXTERN Uint8                   waterlightlevel[MAXWATERLAYER]; // General light amount (0-63)
EXTERN Uint8                   waterlightadd[MAXWATERLAYER];   // Ambient light amount (0-63)
EXTERN float                   waterlayerz[MAXWATERLAYER];     // Base height of water
EXTERN Uint8                   waterlayeralpha[MAXWATERLAYER]; // Transparency
EXTERN float                   waterlayeramp[MAXWATERLAYER];   // Amplitude of waves
EXTERN float                   waterlayeru[MAXWATERLAYER];     // Coordinates of texture
EXTERN float                   waterlayerv[MAXWATERLAYER];     //
EXTERN float                   waterlayeruadd[MAXWATERLAYER];  // Texture movement
EXTERN float                   waterlayervadd[MAXWATERLAYER];  //
EXTERN float                   waterlayerzadd[MAXWATERLAYER][MAXWATERFRAME][WATERMODE][WATERPOINTS];
EXTERN Uint8                   waterlayercolor[MAXWATERLAYER][MAXWATERFRAME][WATERMODE][WATERPOINTS];
EXTERN Uint16           waterlayerframe[MAXWATERLAYER]; // Frame
EXTERN Uint16           waterlayerframeadd[MAXWATERLAYER];      // Speed
EXTERN float                   waterlayerdistx[MAXWATERLAYER];         // For distant backgrounds
EXTERN float                   waterlayerdisty[MAXWATERLAYER];         //
EXTERN Uint32                   waterspek[256];             // Specular highlights
EXTERN float                   foregroundrepeat  EQ( 1 );     //
EXTERN float                   backgroundrepeat  EQ( 1 );     //

// Fog stuff
EXTERN bool_t                  fogallowed  EQ( btrue );        //
EXTERN bool_t                  fogon  EQ( bfalse );            // Do ground fog?
EXTERN float                   fogbottom  EQ( 0.0f );          //
EXTERN float                   fogtop  EQ( 100 );             //
EXTERN float                   fogdistance  EQ( 100 );        //
EXTERN Uint8                   fogred  EQ( 255 );             //  Fog collour
EXTERN Uint8                   foggrn  EQ( 255 );             //
EXTERN Uint8                   fogblu  EQ( 255 );             //
EXTERN Uint8                   fogaffectswater;

EXTERN int                     fontoffset;                 // Line up fonts from top of screen

/*OpenGL Textures*/
EXTERN  STRING      TxFormatSupported[50]; // OpenGL icon surfaces
EXTERN  Uint8       maxformattypes EQ(0);

EXTERN  GLTexture       TxIcon[MAXTEXTURE+1];       // OpenGL icon surfaces
EXTERN  GLTexture       TxTitleImage[MAXMODULE];    // OpenGL title image surfaces
EXTERN  GLTexture       TxFont;                     // OpenGL font surface
EXTERN  GLTexture       TxBars;                     // OpenGL status bar surface
EXTERN  GLTexture       TxBlip;                     // OpenGL you are here surface
EXTERN  GLTexture       TxMap;                      // OpenGL map surface
EXTERN  GLTexture       txTexture[MAXTEXTURE];      // All textures
EXTERN  bool_t          use_sdl_image EQ(btrue);    //Allow advanced SDL_Image functions?

// Anisotropic filtering - yay! :P
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
EXTERN float maxAnisotropy;                    // Max anisotropic filterings (Between 1.00f and 16.00f)

//Texture filtering
typedef enum e_tx_filters
{
    TX_UNFILTERED,
    TX_LINEAR,
    TX_MIPMAP,
    TX_BILINEAR,
    TX_TRILINEAR_1,
    TX_TRILINEAR_2,
    TX_ANISOTROPIC
} TX_FILTERS;

EXTERN glMatrix mWorld;                      // World Matrix
EXTERN glMatrix mView;                      // View Matrix
EXTERN glMatrix mViewSave;                    // View Matrix initial state
EXTERN glMatrix mProjection;                  // Projection Matrix

// Input player control
EXTERN int                     nullicon  EQ( 0 );
EXTERN int                     keybicon  EQ( 0 );
EXTERN int                     mousicon  EQ( 0 );
EXTERN int                     joyaicon  EQ( 0 );
EXTERN int                     joybicon  EQ( 0 );

// Interface stuff
EXTERN rect_t                    iconrect;                   // The 32x32 icon rectangle
EXTERN SDL_Rect           fontrect[NUMFONT];          // The font rectangles
EXTERN Uint8       fontxspacing[NUMFONT];      // The spacing stuff
EXTERN Uint8       fontyspacing;               //
EXTERN rect_t                    tabrect[NUMBAR];            // The tab rectangles
EXTERN rect_t                    barrect[NUMBAR];            // The bar rectangles
EXTERN rect_t                    bliprect[NUMBAR];           // The blip rectangles
EXTERN rect_t                    maprect;                    // The map rectangle

#define SPARKLESIZE 28
#define SPARKLEADD 2
#define MAPSIZE 96
#define BLIPSIZE 6

// Lightning effects

#define MAXDYNADIST                     2700        // Leeway for offscreen lights
#define TOTALMAXDYNA                    64          // Absolute max number of dynamic lights
EXTERN int                     maxlights EQ( 8 ); // Max number of lights to draw
EXTERN int                     numdynalight;               // Number of dynamic lights
EXTERN int                     dynadistancetobeat;         // The number to beat
EXTERN int                     dynadistance[TOTALMAXDYNA];      // The distances
EXTERN float                   dynalightlistx[TOTALMAXDYNA];    // Light position
EXTERN float                   dynalightlisty[TOTALMAXDYNA];    //
EXTERN float                   dynalightlevel[TOTALMAXDYNA];    // Light level
EXTERN float                   dynalightfalloff[TOTALMAXDYNA];  // Light falloff

EXTERN Uint8                   cLoadBuffer[MD2MAXLOADSIZE];// Where to put an MD2

//Mesh
EXTERN Uint32 maplrtwist[256];            // For surface normal of mesh
EXTERN Uint32 mapudtwist[256];            //
EXTERN float                   vellrtwist[256];            // For sliding down steep hills
EXTERN float                   veludtwist[256];            //
EXTERN Uint8 flattwist[256];             //

// Camera stuff
#define TRACKXAREALOW     100
#define TRACKXAREAHIGH    180
#define TRACKYAREAMINLOW  320
#define TRACKYAREAMAXLOW  460
#define TRACKYAREAMINHIGH 460
#define TRACKYAREAMAXHIGH 600

// Values for manipulating AI
#define MAXWAY              8                       // Waypoints
#define WAYTHRESH           64                      // Threshold for reaching waypoint
#define MAXSTOR             16                      // Storage data (Used in SetXY)
#define STORAND             15                      //

// AI stuff
struct s_ai_state
{
    // which script to run
    Uint16         type;

    // the execution pointer(s)
    size_t         exe_stt;
    size_t         exe_end;
    size_t         exe_pos;
    Uint32         opcode;

    // some script states
    Sint32         poof_time;
    bool_t         changed;
    bool_t         terminate;
    Uint32         indent;
    Uint32         indent_last;

    // who are we related to?
    Uint16         index;         // what is the index value of this character
    Uint16         target;        // Who the AI is after
    Uint16         owner;         // The character's owner
    Uint16         child;         // The character's child

    // some local storage
    Uint32         alert;         // Alerts for AI script
    int            state;         // Short term memory for AI
    int            content;       // More short term memory
    int            passage;       // The passage associated with this character
    int            timer;         // AI Timer
    int            x[MAXSTOR];    // Temporary values...  SetXY
    int            y[MAXSTOR];    //

    // ai memory from the last event
    Uint16         bumplast;        // Last character it was bumped by
    Uint16         attacklast;      // Last character it was attacked by
    Uint16         hitlast;         // Last character it hit
    Uint16         directionlast;   // Direction of last attack/healing
    Uint16         damagetypelast;  // Last damage type
    Uint16         lastitemused;    // The last item the character used
    Uint16         target_old;

    // message handling
    Uint32         order;           // The last order given the character
    Uint16         rank;           // The rank of the character on the order chain

    // waypoints
    Uint8          wp_tail;          // Which waypoint
    Uint8          wp_head;          // Where to stick next
    float          wp_pos_x[MAXWAY]; // Waypoint
    float          wp_pos_y[MAXWAY]; // Waypoint
};
typedef struct s_ai_state ai_state_t;

// Character stuff
#define MAXSKIN   4

EXTERN int            numfreechr EQ( 0 );             // For allocation
EXTERN Uint16         freechrlist[MAXCHR];        //

//This is the character definition
struct s_chr
{
    glMatrix       matrix;          // Character's matrix
    char           matrixvalid;     // Did we make one yet?
    char           name[MAXCAPNAMESIZE];  // Character name
    bool_t         on;              // Does it exist?
    Uint8          onold;           // Network fix
    Uint8          alive;           // Is it alive?
    Uint8          waskilled;       // Fix for network
    Uint8          inpack;          // Is it in the inventory?
    Uint8          wasinpack;       // Temporary thing...
    Uint16         nextinpack;    // Link to the next item
    Uint8          numinpack;       // How many
    Uint8          openstuff;       // Can it open chests/doors?
    Uint8          lifecolor;       // Bar color
    Sint16         life;            // Basic character stats
    Sint16         lifemax;         //   All 8.8 fixed point
    Uint16         lifeheal;        //
    Uint8          manacolor;       // Bar color
    Uint8          ammomax;         // Ammo stuff
    Uint16         ammo;            //
    Uint8          gender;          // Gender
    Sint16         mana;            // Mana stuff
    Sint16         manamax;         //
    Sint16         manaflow;        //
    Sint16         manareturn;      //
    Sint16         strength;        // Strength
    Sint16         wisdom;          // Wisdom
    Sint16         intelligence;    // Intelligence
    Sint16         dexterity;       // Dexterity
    bool_t         icon;            // Show the icon?
    Uint8          sparkle;         // Sparkle color or 0 for off
    bool_t         cangrabmoney;    // Picks up coins?
    bool_t         isplayer;        // btrue = player
    bool_t         islocalplayer;   // btrue = local player
    ai_state_t     ai;
    Uint8          stickybutt;      // Rests on floor
    Uint8          enviro;          // Environment map?
    float          oldx;            // Character's last position
    float          oldy;            //
    float          oldz;            //
    Uint8          inwater;         //
    Uint16         oldturn;         //
    Uint8          flyheight;       // Height to stabilize at
    Uint8          team;            // Character's team
    Uint8          baseteam;        // Character's starting team
    Uint8          staton;          // Display stats?
    float          xstt;            // Starting position
    float          ystt;            //
    float          zstt;            //
    float          xpos;            // Character's position
    float          ypos;            //
    float          zpos;            //
    float          xvel;            // Character's velocity
    float          yvel;            //
    float          zvel;            //
    float          latchx;          // Character latches
    float          latchy;          //
    Uint32         latchbutton;     // Button latches
    Uint16         reloadtime;      // Time before another shot
    float          maxaccel;        // Maximum acceleration
    float          fat;             // Character's size
    float          sizegoto;        // Character's size goto
    Uint8          sizegototime;    // Time left in size change
    float          dampen;          // Bounciness
    float          level;           // Height of tile
    float          jump;            // Jump power
    Uint8          jumptime;        // Delay until next jump
    Uint8          jumpnumber;      // Number of jumps remaining
    Uint8          jumpnumberreset; // Number of jumps total, 255=Flying
    Uint8          jumpready;       // For standing on a platform character
    Uint32         onwhichfan;      // Where the char is
    Uint32         onwhichblock;    // The character's collision block
    Uint8          indolist;        // Has it been added yet?
    Uint16         uoffset;         // For moving textures
    Uint16         voffset;         //
    Uint16         uoffvel;         // Moving texture speed
    Uint16         voffvel;         //
    Uint16         turnleftright;   // Character's rotation 0 to 0xFFFF
    Uint16         lightturnleftright;// Character's light rotation 0 to 0xFFFF
    Uint16         turnmaplr;       //
    Uint16         turnmapud;       //
    Uint16         skin;            // Character's skin
    Uint16         texture;         // The texture id of the character's skin
    Uint8          model;           // Character's model
    Uint8          basemodel;       // The true form
    Uint8          actionready;     // Ready to play a new one
    Uint8          action;          // Character's action
    bool_t         keepaction;      // Keep the action playing
    bool_t         loopaction;      // Loop it too
    Uint8          nextaction;      // Character's action to play next
    Uint16         frame;           // Character's frame
    Uint16         lastframe;       // Character's last frame
    Uint8          lip;             // Character's frame in betweening
    Uint8          vrta[MAXVERTICES];// Lighting hack ( Ooze )
    Uint16         holdingwhich[MAXSLOT]; // !=MAXCHR if character is holding something
    Uint16         attachedto;      // !=MAXCHR if character is a held weapon
    Uint16         weapongrip[GRIP_VERTS];   // Vertices which describe the weapon grip
    Uint8          alpha;           // 255 = Solid, 0 = Invisible
    Uint8          basealpha;
    Uint8          light;           // 1 = Light, 0 = Normal
    Uint8          flashand;        // 1,3,7,15,31 = Flash, 255 = Don't
    Uint8          lightlevel;      // 0-255, terrain light
    Uint8          sheen;           // 0-15, how shiny it is
    Uint8          transferblend;   // Give transparency to weapons?
    Uint8          isitem;          // Is it grabbable?
    Uint8          invictus;        // Totally invincible?
    Uint8          ismount;         // Can you ride it?
    Uint8          redshift;        // Color channel shifting
    Uint8          grnshift;        //
    Uint8          blushift;        //
    Uint8          shadowsize;      // Size of shadow
    Uint8          bumpsize;        // Size of bumpers
    Uint8          bumpsizebig;     // For octagonal bumpers
    Uint8          bumpheight;      // Distance from head to toe
    Uint8          shadowsizesave;  // Without size modifiers
    Uint8          bumpsizesave;    //
    Uint8          bumpsizebigsave; //
    Uint8          bumpheightsave;  //
    Uint16         bumpnext;        // Next character on fanblock
    float          bumpdampen;      // Character bump mass
    Uint8          platform;        // Can it be stood on
    Uint8          waterwalk;       // Always above watersurfacelevel?
    Uint8          turnmode;        // Turning mode
    Uint8          sneakspd;        // Sneaking if above this speed
    Uint8          walkspd;         // Walking if above this speed
    Uint8          runspd;          // Running if above this speed
    Uint8          damagetargettype;// Type of damage for AI DamageTarget
    Uint8          reaffirmdamagetype; // For relighting torches
    Uint8          damagemodifier[DAMAGE_COUNT];  // Resistances and inversion
    Uint8          damagetime;      // Invincibility timer
    Uint8          defense;         // Base defense rating
    Uint16         weight;          // Weight ( for pressure plates )
    Uint16         holdingweight;   // For weighted buttons
    Sint16         money;           // Money
    Sint16         lifereturn;      // Regeneration/poison
    Sint16         manacost;        // Mana cost to use
    Uint8          stoppedby;       // Collision mask
    Uint32         experience;      // Experience
    Uint8          experiencelevel; // Experience Level
    Sint16         grogtime;        // Grog timer
    Sint16         dazetime;        // Daze timer
    Uint8          iskursed;        // Can't be dropped?
    Uint8          nameknown;       // Is the name known?
    Uint8          ammoknown;       // Is the ammo known?
    Uint8          hitready;        // Was it just dropped?
    Sint16         boretime;        // Boredom timer
    Uint8          carefultime;     // "You hurt me!" timer
    bool_t         canbecrushed;    // Crush in a door?
    Uint8          inwhichhand;     // GRIP_LEFT or GRIP_RIGHT
    Uint8          isequipped;      // For boots and rings and stuff
    Uint16         firstenchant;    // Linked list for enchants
    Uint16         undoenchant;     // Last enchantment spawned
    bool_t         canchannel;      //
    bool_t         overlay;         // Is this an overlay?  Track aitarget...
    Uint8          missiletreatment;// For deflection, etc.
    Uint8          missilecost;     // Mana cost for each one
    Uint16         missilehandler;  // Who pays the bill for each one...
    Uint16         damageboost;     // Add to swipe damage
    bool_t         isshopitem;     // Spawned in a shop?
    Sint8          soundindex[SOUND_COUNT];       // a map for soundX.wav to sound types

    //Skills
    Sint8           shieldproficiency;  // Can it use shields?
    bool_t          canjoust; //
    bool_t          canuseadvancedweapons; //
    bool_t          canseeinvisible; //
    bool_t          canseekurse;     //
    bool_t          canusedivine;
    bool_t          canusearcane;
    bool_t          canusetech;
    bool_t          candisarm;
    bool_t          canbackstab;
    bool_t          canusepoison;
    bool_t          canread;

    // Accumulators for doing the physics in bump_characters(). should prevent you from being bumped into a wall
    float          phys_pos_x;
    float          phys_pos_y;
    float          phys_pos_z;
    float          phys_vel_x;
    float          phys_vel_y;
    float          phys_vel_z;
};

typedef struct s_chr chr_t;

EXTERN chr_t chr[MAXCHR];

#define INVISIBLE           20                      // The character can't be detected

//AI targeting
#define NEARBY		3*TILESIZE    //3 tiles
#define WIDE		6*TILESIZE    //8 tiles
#define NEAREST		0           //unlimited range
#define TILESIZE    128       //Size of one texture tile in egoboo

EXTERN bool_t                    local_seeinvisible   EQ( bfalse );
EXTERN bool_t                    local_seekurse       EQ( bfalse );
EXTERN Uint16                    local_senseenemies   EQ( MAXCHR );
EXTERN IDSZ                      local_senseenemiesID;
EXTERN bool_t                    local_listening      EQ( bfalse );  // Players with listen skill?

//------------------------------------
// Enchantment variables
//------------------------------------
#define MAXENCHANT                      200         // Number of enchantments
EXTERN Uint16       numfreeenchant;             // For allocating new ones
EXTERN Uint16       freeenchant[MAXENCHANT];    //

EXTERN bool_t           evevalid[MAXEVE];                       // Enchant.txt loaded?
EXTERN bool_t           eveoverride[MAXEVE];                    // Override other enchants?
EXTERN bool_t           everemoveoverridden[MAXEVE];            // Remove other enchants?
EXTERN bool_t           evesetyesno[MAXEVE][MAXEVESETVALUE];    // Set this value?
EXTERN Uint8            evesetvalue[MAXEVE][MAXEVESETVALUE];    // Value to use
EXTERN Sint32           eveaddvalue[MAXEVE][MAXEVEADDVALUE];    // The values to add
EXTERN bool_t           everetarget[MAXEVE];                    // Pick a weapon?
EXTERN bool_t           evekillonend[MAXEVE];                   // Kill the target on end?
EXTERN bool_t           evepoofonend[MAXEVE];                   // Spawn a poof on end?
EXTERN bool_t           eveendifcantpay[MAXEVE];                // End on out of mana
EXTERN bool_t           evestayifnoowner[MAXEVE];               // Stay if owner has died?
EXTERN Sint16           evetime[MAXEVE];                        // Time in seconds
EXTERN Sint32           eveendmessage[MAXEVE];                  // Message for end -1 for none
EXTERN Sint16           eveownermana[MAXEVE];                   // Boost values
EXTERN Sint16           eveownerlife[MAXEVE];                   //
EXTERN Sint16           evetargetmana[MAXEVE];                  //
EXTERN Sint16           evetargetlife[MAXEVE];                  //
EXTERN Uint8            evedontdamagetype[MAXEVE];              // Don't work if ...
EXTERN Uint8            eveonlydamagetype[MAXEVE];              // Only work if ...
EXTERN IDSZ             everemovedbyidsz[MAXEVE];               // By particle or [NONE]
EXTERN Uint16           evecontspawntime[MAXEVE];               // Spawn timer
EXTERN Uint8            evecontspawnamount[MAXEVE];             // Spawn amount
EXTERN Uint16           evecontspawnfacingadd[MAXEVE];          // Spawn in circle
EXTERN Uint16           evecontspawnpip[MAXEVE];                // Spawn type ( local )
EXTERN Sint16           evewaveindex[MAXEVE];                   // Sound on end (-1 for none)
EXTERN Uint16           evefrequency[MAXEVE];                   // Sound frequency
EXTERN Uint8            eveoverlay[MAXEVE];                     // Spawn an overlay?
EXTERN Uint16           eveseekurse[MAXEVE];                     // Spawn an overlay?

EXTERN bool_t           encon[MAXENCHANT];                      // Enchantment on
EXTERN Uint16           enceve[MAXENCHANT];                     // The type
EXTERN Uint16           enctarget[MAXENCHANT];                  // Who it enchants
EXTERN Uint16           encnextenchant[MAXENCHANT];             // Next in the list
EXTERN Uint16           encowner[MAXENCHANT];                   // Who cast the enchant
EXTERN Uint16           encspawner[MAXENCHANT];                 // The spellbook character
EXTERN Uint16           encoverlay[MAXENCHANT];                 // The overlay character
EXTERN Sint16           encownermana[MAXENCHANT];               // Boost values
EXTERN Sint16           encownerlife[MAXENCHANT];               //
EXTERN Sint16           enctargetmana[MAXENCHANT];              //
EXTERN Sint16           enctargetlife[MAXENCHANT];              //
EXTERN bool_t           encsetyesno[MAXENCHANT][MAXEVESETVALUE];// Was it set?
EXTERN bool_t           encsetsave[MAXENCHANT][MAXEVESETVALUE]; // The value to restore
EXTERN Sint16           encaddsave[MAXENCHANT][MAXEVEADDVALUE]; // The value to take away
EXTERN Sint16           enctime[MAXENCHANT];                    // Time before end
EXTERN Uint16           encspawntime[MAXENCHANT];               // Time before spawn

//missile treatments
#define MISNORMAL               0                  // Treat missiles normally
#define MISDEFLECT              1                  // Deflect incoming missiles
#define MISREFLECT              2                  // Reflect them back!

#define LEAVEALL                0
#define LEAVEFIRST              1
#define LEAVENONE               2

EXTERN float            textureoffset[256];         // For moving textures

//------------------------------------
// Particle variables
//------------------------------------
#define SPAWNNOCHARACTER        255                                      // For particles that spawn characters...
#define TOTALMAXPRT            2048                                      // True max number of particles
#define TOTALMAXPRTPIP         1024                                      // Particle templates
#define MAXPRTPIPPEROBJECT       13                                      // Max part*.txt per object

EXTERN Uint16           maxparticles EQ(512);                            // max number of particles
EXTERN int              numfreeprt   EQ( 0 );                            // For allocation
EXTERN Uint16           freeprtlist[TOTALMAXPRT];                        //

EXTERN Uint8            prton[TOTALMAXPRT];                              // Does it exist?
EXTERN Uint16           prtpip[TOTALMAXPRT];                             // The part template
EXTERN Uint16           prtmodel[TOTALMAXPRT];                           // Pip spawn model
EXTERN Uint16           prtattachedtocharacter[TOTALMAXPRT];             // For torch flame
EXTERN Uint16           prtgrip[TOTALMAXPRT];                            // The vertex it's on
EXTERN Uint8            prttype[TOTALMAXPRT];                            // Transparency mode, 0-2
EXTERN Uint16           prtfacing[TOTALMAXPRT];                          // Direction of the part
EXTERN Uint8            prtteam[TOTALMAXPRT];                            // Team
EXTERN float            prtxpos[TOTALMAXPRT];                            // Position
EXTERN float            prtypos[TOTALMAXPRT];                            //
EXTERN float            prtzpos[TOTALMAXPRT];                            //
EXTERN float            prtxvel[TOTALMAXPRT];                            // Velocity
EXTERN float            prtyvel[TOTALMAXPRT];                            //
EXTERN float            prtzvel[TOTALMAXPRT];                            //
EXTERN float            prtlevel[TOTALMAXPRT];                           // Height of tile
EXTERN Uint8            prtspawncharacterstate[TOTALMAXPRT];             //
EXTERN Uint16           prtrotate[TOTALMAXPRT];                          // Rotation direction
EXTERN Sint16           prtrotateadd[TOTALMAXPRT];                       // Rotation rate
EXTERN Uint32           prtonwhichfan[TOTALMAXPRT];                      // Where the part is
EXTERN Uint32           prtonwhichblock[TOTALMAXPRT];                         // The particle's collision block
EXTERN Uint16           prtsize[TOTALMAXPRT];                            // Size of particle>>8
EXTERN Sint16           prtsizeadd[TOTALMAXPRT];                         // Change in size
EXTERN bool_t           prtinview[TOTALMAXPRT];                          // Render this one?
EXTERN Uint16           prtimage[TOTALMAXPRT];                           // Which image ( >> 8 )
EXTERN Uint16           prtimageadd[TOTALMAXPRT];                        // Animation rate
EXTERN Uint16           prtimagemax[TOTALMAXPRT];                        // End of image loop
EXTERN Uint16           prtimagestt[TOTALMAXPRT];                        // Start of image loop
EXTERN Uint8            prtlight[TOTALMAXPRT];                           // Light level
EXTERN Uint16           prttime[TOTALMAXPRT];                            // Duration of particle
EXTERN Uint16           prtspawntime[TOTALMAXPRT];                       // Time until spawn
EXTERN Uint8            prtbumpsize[TOTALMAXPRT];                        // Size of bumpers
EXTERN Uint8            prtbumpsizebig[TOTALMAXPRT];                     //
EXTERN Uint8            prtbumpheight[TOTALMAXPRT];                      // Bounding box height
EXTERN Uint16           prtbumpnext[TOTALMAXPRT];                        // Next particle on fanblock
EXTERN Uint16           prtdamagebase[TOTALMAXPRT];                      // For strength
EXTERN Uint16           prtdamagerand[TOTALMAXPRT];                      // For fixes...
EXTERN Uint8            prtdamagetype[TOTALMAXPRT];                      // Damage type
EXTERN Uint16           prtchr[TOTALMAXPRT];                             // The character that is attacking
EXTERN float            prtdynalightfalloff[TOTALMAXPRT];                // Dyna light...
EXTERN float            prtdynalightlevel[TOTALMAXPRT];                  //
EXTERN float            particleimageu[MAXPARTICLEIMAGE][2];        // Texture coordinates
EXTERN float            particleimagev[MAXPARTICLEIMAGE][2];        //
EXTERN Uint16           particletexture;                            // All in one bitmap
EXTERN bool_t           prtdynalighton[TOTALMAXPRT];                     // Dynamic light?
EXTERN Uint16           prttarget[TOTALMAXPRT];                          // Who it's chasing

//particle templates
EXTERN int                     numpip  EQ( 0 );
EXTERN bool_t                  pipforce[TOTALMAXPRTPIP];                        // Force spawn?

EXTERN Uint8                   piptype[TOTALMAXPRTPIP];                         // Transparency mode
EXTERN Uint8                   pipnumframes[TOTALMAXPRTPIP];                    // Number of frames
EXTERN Uint8                   pipimagebase[TOTALMAXPRTPIP];                    // Starting image
EXTERN Uint16                  pipimageadd[TOTALMAXPRTPIP];                     // Frame rate
EXTERN Uint16                  pipimageaddrand[TOTALMAXPRTPIP];                 // Frame rate randomness
EXTERN Uint16                  piptime[TOTALMAXPRTPIP];                         // Time until end
EXTERN Uint16                  piprotatebase[TOTALMAXPRTPIP];                   // Rotation
EXTERN Uint16                  piprotaterand[TOTALMAXPRTPIP];                   // Rotation
EXTERN Sint16                  piprotateadd[TOTALMAXPRTPIP];                    // Rotation rate
EXTERN Uint16                  pipsizebase[TOTALMAXPRTPIP];                     // Size
EXTERN Sint16                  pipsizeadd[TOTALMAXPRTPIP];                      // Size rate
EXTERN float                   pipspdlimit[TOTALMAXPRTPIP];                     // Speed limit
EXTERN float                   pipdampen[TOTALMAXPRTPIP];                       // Bounciness
EXTERN Sint8                   pipbumpmoney[TOTALMAXPRTPIP];                    // Value of particle
EXTERN Uint8                   pipbumpsize[TOTALMAXPRTPIP];                     // Bounding box size
EXTERN Uint8                   pipbumpheight[TOTALMAXPRTPIP];                   // Bounding box height
EXTERN bool_t                  pipendwater[TOTALMAXPRTPIP];                     // End if underwater
EXTERN bool_t                  pipendbump[TOTALMAXPRTPIP];                      // End if bumped
EXTERN bool_t                  pipendground[TOTALMAXPRTPIP];                    // End if on ground
EXTERN bool_t                  pipendwall[TOTALMAXPRTPIP];                      // End if hit a wall
EXTERN bool_t                  pipendlastframe[TOTALMAXPRTPIP];                 // End on last frame
EXTERN Uint16                  pipdamagebase[TOTALMAXPRTPIP];                   // Damage
EXTERN Uint16                  pipdamagerand[TOTALMAXPRTPIP];                   // Damage
EXTERN Uint8                   pipdamagetype[TOTALMAXPRTPIP];                   // Damage type
EXTERN Sint16                  pipfacingbase[TOTALMAXPRTPIP];                   // Facing
EXTERN Uint16                  pipfacingadd[TOTALMAXPRTPIP];                    // Facing
EXTERN Uint16                  pipfacingrand[TOTALMAXPRTPIP];                   // Facing
EXTERN Sint16                  pipxyspacingbase[TOTALMAXPRTPIP];                // Spacing
EXTERN Uint16                  pipxyspacingrand[TOTALMAXPRTPIP];                // Spacing
EXTERN Sint16                  pipzspacingbase[TOTALMAXPRTPIP];                 // Altitude
EXTERN Uint16                  pipzspacingrand[TOTALMAXPRTPIP];                 // Altitude
EXTERN Sint8                   pipxyvelbase[TOTALMAXPRTPIP];                    // Shot velocity
EXTERN Uint8                   pipxyvelrand[TOTALMAXPRTPIP];                    // Shot velocity
EXTERN Sint8                   pipzvelbase[TOTALMAXPRTPIP];                     // Up velocity
EXTERN Uint8                   pipzvelrand[TOTALMAXPRTPIP];                     // Up velocity
EXTERN Uint16                  pipcontspawntime[TOTALMAXPRTPIP];                // Spawn timer
EXTERN Uint8                   pipcontspawnamount[TOTALMAXPRTPIP];              // Spawn amount
EXTERN Uint16                  pipcontspawnfacingadd[TOTALMAXPRTPIP];           // Spawn in circle
EXTERN Uint16                  pipcontspawnpip[TOTALMAXPRTPIP];                 // Spawn type ( local )
EXTERN Uint8                   pipendspawnamount[TOTALMAXPRTPIP];               // Spawn amount
EXTERN Uint16                  pipendspawnfacingadd[TOTALMAXPRTPIP];            // Spawn in circle
EXTERN Uint8                   pipendspawnpip[TOTALMAXPRTPIP];                  // Spawn type ( local )
EXTERN Uint8                   pipbumpspawnamount[TOTALMAXPRTPIP];              // Spawn amount
EXTERN Uint8                   pipbumpspawnpip[TOTALMAXPRTPIP];                 // Spawn type ( global )
EXTERN Uint8                   pipdynalightmode[TOTALMAXPRTPIP];                // Dynamic light on?
EXTERN float                   pipdynalevel[TOTALMAXPRTPIP];                    // Intensity
EXTERN Uint16                  pipdynafalloff[TOTALMAXPRTPIP];                  // Falloff
EXTERN Uint16                  pipdazetime[TOTALMAXPRTPIP];                     // Daze
EXTERN Uint16                  pipgrogtime[TOTALMAXPRTPIP];                     // Drunkeness
EXTERN Sint8                   pipsoundspawn[TOTALMAXPRTPIP];                   // Beginning sound
EXTERN Sint8                   pipsoundend[TOTALMAXPRTPIP];                     // Ending sound
EXTERN Sint8                   pipsoundfloor[TOTALMAXPRTPIP];                   // Floor sound
EXTERN Sint8                   pipsoundwall[TOTALMAXPRTPIP];                    // Ricochet sound
EXTERN bool_t                  pipfriendlyfire[TOTALMAXPRTPIP];                 // Friendly fire
EXTERN bool_t                  piphateonly[TOTALMAXPRTPIP];                     //Only hit hategroup
EXTERN bool_t                  piprotatetoface[TOTALMAXPRTPIP];                 // Arrows/Missiles
EXTERN bool_t                  pipnewtargetonspawn[TOTALMAXPRTPIP];             // Get new target?
EXTERN bool_t                  piphoming[TOTALMAXPRTPIP];                       // Homing?
EXTERN Uint16                  piptargetangle[TOTALMAXPRTPIP];                  // To find target
EXTERN float                   piphomingaccel[TOTALMAXPRTPIP];                  // Acceleration rate
EXTERN float                   piphomingfriction[TOTALMAXPRTPIP];               // Deceleration rate
EXTERN float                   pipdynalightleveladd[TOTALMAXPRTPIP];            // Dyna light changes
EXTERN float                   pipdynalightfalloffadd[TOTALMAXPRTPIP];          //
EXTERN bool_t                  piptargetcaster[TOTALMAXPRTPIP];                 // Target caster?
EXTERN bool_t                  pipspawnenchant[TOTALMAXPRTPIP];                 // Spawn enchant?
EXTERN bool_t                  pipneedtarget[TOTALMAXPRTPIP];                   // Need a target?
EXTERN bool_t                  piponlydamagefriendly[TOTALMAXPRTPIP];           // Only friends?
EXTERN bool_t                  pipstartontarget[TOTALMAXPRTPIP];                // Start on target?
EXTERN int                     pipzaimspd[TOTALMAXPRTPIP];                      // [ZSPD] For Z aiming
EXTERN Uint16                  pipdamfx[TOTALMAXPRTPIP];                        // Damage effects
EXTERN bool_t                  pipallowpush[TOTALMAXPRTPIP];                    // Allow particle to push characters around
EXTERN bool_t                  pipintdamagebonus[TOTALMAXPRTPIP];               // Add intelligence as damage bonus
EXTERN bool_t                  pipwisdamagebonus[TOTALMAXPRTPIP];               // Add wisdom as damage bonus

#define COIN1               0                       // Coins are the first particles loaded
#define COIN5               1                       //
#define COIN25              2                       //
#define COIN100             3                       //
#define WEATHER4            4                       // Weather particles
#define WEATHER5            5                       // Weather particle finish
#define SPLASH              6                       // Water effects are next
#define RIPPLE              7                       //
#define DEFEND              8                       // Defend particle

//------------------------------------
// Module variables
//------------------------------------
#define RANKSIZE 12
#define SUMMARYLINES 8
#define SUMMARYSIZE  80

EXTERN int                     globalnummodule;                            // Number of modules
EXTERN char                    modrank[MAXMODULE][RANKSIZE];               // Number of stars
EXTERN char                    modlongname[MAXMODULE][MAXCAPNAMESIZE];     // Module names
EXTERN char                    modloadname[MAXMODULE][MAXCAPNAMESIZE];     // Module load names
EXTERN Uint8                   modimportamount[MAXMODULE];                 // # of import characters
EXTERN Uint8                   modallowexport[MAXMODULE];                  // Export characters?
EXTERN Uint8                   modminplayers[MAXMODULE];                   // Number of players
EXTERN Uint8                   modmaxplayers[MAXMODULE];                   //
EXTERN bool_t                  modmonstersonly[MAXMODULE];                 // Only allow monsters
EXTERN bool_t                  modrtscontrol[MAXMODULE];                   // Real Time Stragedy?
EXTERN Uint8                   modrespawnvalid[MAXMODULE];                 // Allow respawn
EXTERN int                     numlines;                                   // Lines in summary
EXTERN char                    modsummary[SUMMARYLINES][SUMMARYSIZE];      // Quest description

//------------------------------------
// Model stuff
//------------------------------------
// TEMPORARY: Needs to be moved out of egoboo.h eventually
extern struct Md2Model *md2_models[MAXMODEL];                 // Md2 models

EXTERN int             globalicon_count;                              // Number of icons
EXTERN Uint16          madloadframe;                               // Where to load next

EXTERN bool_t          madused[MAXMODEL];                          // Model slot
EXTERN char            madname[MAXMODEL][128];                     // Model name
EXTERN Uint16          madskintoicon[MAXTEXTURE];                  // Skin to icon
EXTERN Uint16          madskins[MAXMODEL];                         // Number of skins
EXTERN Uint16          madskinstart[MAXMODEL];                     // Starting skin of model
EXTERN Uint16          madframes[MAXMODEL];                        // Number of frames
EXTERN Uint16          madframestart[MAXMODEL];                    // Starting frame of model
EXTERN Uint16          madmsgstart[MAXMODEL];                      // The first message
EXTERN Uint16          madvertices[MAXMODEL];                      // Number of vertices
EXTERN Uint16          madtransvertices[MAXMODEL];                 // Number to transform
EXTERN Uint16          madcommands[MAXMODEL];                      // Number of commands
EXTERN GLenum          madcommandtype[MAXMODEL][MAXCOMMAND];       // Fan or strip
EXTERN Uint16          madcommandsize[MAXMODEL][MAXCOMMAND];       // Entries used by command
EXTERN Uint16          madcommandvrt[MAXMODEL][MAXCOMMANDENTRIES]; // Which vertex
EXTERN float           madcommandu[MAXMODEL][MAXCOMMANDENTRIES];   // Texture position
EXTERN float           madcommandv[MAXMODEL][MAXCOMMANDENTRIES];   //
EXTERN float           madvrtx[MAXFRAME][MAXVERTICES];             // Vertex position
EXTERN float           madvrty[MAXFRAME][MAXVERTICES];             //
EXTERN float           madvrtz[MAXFRAME][MAXVERTICES];             //
EXTERN Uint8           madvrta[MAXFRAME][MAXVERTICES];             // Light index of vertex
EXTERN Uint8           madframelip[MAXFRAME];                      // 0-15, How far into action is each frame
EXTERN Uint16          madframefx[MAXFRAME];                       // Invincibility, Spawning
EXTERN Uint16          madframeliptowalkframe[MAXMODEL][4][16];    // For walk animations
EXTERN Uint16          madai[MAXMODEL];                            // AI for each model
EXTERN Uint8           madactionvalid[MAXMODEL][MAXACTION];        // bfalse if not valid
EXTERN Uint16          madactionstart[MAXMODEL][MAXACTION];        // First frame of anim
EXTERN Uint16          madactionend[MAXMODEL][MAXACTION];          // One past last frame
EXTERN Uint16          madprtpip[MAXMODEL][MAXPRTPIPPEROBJECT];    // Local particles

EXTERN Uint16  bookicon_count    EQ(0);
EXTERN Uint16  bookicon[MAXSKIN];                      // The first book icon

// Character profiles
EXTERN int             importobject;
EXTERN short           capimportslot[MAXMODEL];
EXTERN char            capclassname[MAXMODEL][MAXCAPNAMESIZE];     // Class name
EXTERN char            capskinname[MAXMODEL][MAXSKIN][MAXCAPNAMESIZE];   // Skin name
EXTERN Sint8           capskinoverride[MAXMODEL];                  // -1 or 0-3.. For import
EXTERN Uint8           capleveloverride[MAXMODEL];                 // 0 for normal
EXTERN int             capstateoverride[MAXMODEL];                 // 0 for normal
EXTERN int             capcontentoverride[MAXMODEL];               // 0 for normal
EXTERN Uint16          capskincost[MAXMODEL][MAXSKIN];                   // Store prices
EXTERN Uint8           capskindressy[MAXMODEL];                    // Dressy
EXTERN float           capstrengthdampen[MAXMODEL];                // Strength damage factor
EXTERN Uint8           capstoppedby[MAXMODEL];                     // Collision Mask
EXTERN bool_t          capuniformlit[MAXMODEL];                    // Bad lighting?
EXTERN Uint8           caplifecolor[MAXMODEL];                     // Bar colors
EXTERN Uint8           capmanacolor[MAXMODEL];                     //
EXTERN Uint8           capammomax[MAXMODEL];                       // Ammo stuff
EXTERN Uint8           capammo[MAXMODEL];                          //
EXTERN Uint8           capgender[MAXMODEL];                        // Gender
EXTERN Uint16          caplifebase[MAXMODEL];                      // Life
EXTERN Uint16          capliferand[MAXMODEL];                      //
EXTERN Uint16          caplifeperlevelbase[MAXMODEL];              //
EXTERN Uint16          caplifeperlevelrand[MAXMODEL];              //
EXTERN Sint16          caplifereturn[MAXMODEL];                    //
EXTERN Sint16          capmoney[MAXMODEL];                         // Money
EXTERN Uint16          caplifeheal[MAXMODEL];                      //
EXTERN Uint16          capmanabase[MAXMODEL];                      // Mana
EXTERN Uint16          capmanarand[MAXMODEL];                      //
EXTERN Sint16          capmanacost[MAXMODEL];                      //
EXTERN Uint16          capmanaperlevelbase[MAXMODEL];              //
EXTERN Uint16          capmanaperlevelrand[MAXMODEL];              //
EXTERN Uint16          capmanareturnbase[MAXMODEL];                //
EXTERN Uint16          capmanareturnrand[MAXMODEL];                //
EXTERN Uint16          capmanareturnperlevelbase[MAXMODEL];        //
EXTERN Uint16          capmanareturnperlevelrand[MAXMODEL];        //
EXTERN Uint16          capmanaflowbase[MAXMODEL];                  //
EXTERN Uint16          capmanaflowrand[MAXMODEL];                  //
EXTERN Uint16          capmanaflowperlevelbase[MAXMODEL];          //
EXTERN Uint16          capmanaflowperlevelrand[MAXMODEL];          //
EXTERN Uint16          capstrengthbase[MAXMODEL];                  // Strength
EXTERN Uint16          capstrengthrand[MAXMODEL];                  //
EXTERN Uint16          capstrengthperlevelbase[MAXMODEL];          //
EXTERN Uint16          capstrengthperlevelrand[MAXMODEL];          //
EXTERN Uint16          capwisdombase[MAXMODEL];                    // Wisdom
EXTERN Uint16          capwisdomrand[MAXMODEL];                    //
EXTERN Uint16          capwisdomperlevelbase[MAXMODEL];            //
EXTERN Uint16          capwisdomperlevelrand[MAXMODEL];            //
EXTERN Uint16          capintelligencebase[MAXMODEL];              // Intlligence
EXTERN Uint16          capintelligencerand[MAXMODEL];              //
EXTERN Uint16          capintelligenceperlevelbase[MAXMODEL];      //
EXTERN Uint16          capintelligenceperlevelrand[MAXMODEL];      //
EXTERN Uint16          capdexteritybase[MAXMODEL];                 // Dexterity
EXTERN Uint16          capdexterityrand[MAXMODEL];                 //
EXTERN Uint16          capdexterityperlevelbase[MAXMODEL];         //
EXTERN Uint16          capdexterityperlevelrand[MAXMODEL];         //
EXTERN float           capsize[MAXMODEL];                          // Scale of model
EXTERN float           capsizeperlevel[MAXMODEL];                  // Scale increases
EXTERN float           capdampen[MAXMODEL];                        // Bounciness
EXTERN Uint8           capshadowsize[MAXMODEL];                    // Shadow size
EXTERN Uint8           capbumpsize[MAXMODEL];                      // Bounding octagon
EXTERN Uint8           capbumpsizebig[MAXMODEL];                   // For octagonal bumpers
EXTERN Uint8           capbumpheight[MAXMODEL];                    //
EXTERN float           capbumpdampen[MAXMODEL];                    // Mass
EXTERN Uint8           capweight[MAXMODEL];                        // Weight
EXTERN float           capjump[MAXMODEL];                          // Jump power
EXTERN Uint8           capjumpnumber[MAXMODEL];                    // Number of jumps ( Ninja )
EXTERN Uint8           capsneakspd[MAXMODEL];                      // Sneak threshold
EXTERN Uint8           capwalkspd[MAXMODEL];                       // Walk threshold
EXTERN Uint8           caprunspd[MAXMODEL];                        // Run threshold
EXTERN Uint8           capflyheight[MAXMODEL];                     // Fly height
EXTERN Uint8           capflashand[MAXMODEL];                      // Flashing rate
EXTERN Uint8           capalpha[MAXMODEL];                         // Transparency
EXTERN Uint8           caplight[MAXMODEL];                         // Light blending
EXTERN bool_t          captransferblend[MAXMODEL];                 // Transfer blending to rider/weapons
EXTERN bool_t          capsheen[MAXMODEL];                         // How shiny it is ( 0-15 )
EXTERN bool_t          capenviro[MAXMODEL];                        // Phong map this baby?
EXTERN Uint16          capuoffvel[MAXMODEL];                       // Texture movement rates
EXTERN Uint16          capvoffvel[MAXMODEL];                       //
EXTERN bool_t          capstickybutt[MAXMODEL];                    // Stick to the ground?
EXTERN Uint16          capiframefacing[MAXMODEL];                  // Invincibility frame
EXTERN Uint16          capiframeangle[MAXMODEL];                   //
EXTERN Uint16          capnframefacing[MAXMODEL];                  // Normal frame
EXTERN Uint16          capnframeangle[MAXMODEL];                   //
EXTERN Uint8           capresistbumpspawn[MAXMODEL];               // Don't catch fire
EXTERN Uint8           capdefense[MAXMODEL][MAXSKIN];                    // Defense for each skin
EXTERN Uint8           capdamagemodifier[MAXMODEL][DAMAGE_COUNT][MAXSKIN];
EXTERN float           capmaxaccel[MAXMODEL][MAXSKIN];                   // Acceleration for each skin
EXTERN Uint32          capexperienceforlevel[MAXMODEL][MAXLEVEL];  // Experience needed for next level
EXTERN Uint32          capexperiencebase[MAXMODEL];                // Starting experience
EXTERN Uint16          capexperiencerand[MAXMODEL];                //
EXTERN Uint16          capexperienceworth[MAXMODEL];               // Amount given to killer/user
EXTERN float           capexperienceexchange[MAXMODEL];            // Adds to worth
EXTERN float           capexperiencerate[MAXMODEL][XP_COUNT];
EXTERN IDSZ            capidsz[MAXMODEL][IDSZ_COUNT];                 // ID strings
EXTERN bool_t          capisitem[MAXMODEL];                        // Is it an item?
EXTERN bool_t          capinvictus[MAXMODEL];                      // Is it invincible?
EXTERN bool_t          capismount[MAXMODEL];                       // Can you ride it?
EXTERN bool_t          capisstackable[MAXMODEL];                   // Is it arrowlike?
EXTERN bool_t          capnameknown[MAXMODEL];                     // Is the class name known?
EXTERN bool_t          capusageknown[MAXMODEL];                    // Is its usage known
EXTERN bool_t          capcancarrytonextmodule[MAXMODEL];          // Take it with you?
EXTERN bool_t          capneedskillidtouse[MAXMODEL];              // Check IDSZ first?
EXTERN bool_t          capwaterwalk[MAXMODEL];                     // Walk on water?
EXTERN bool_t          capplatform[MAXMODEL];                      // Can be stood on?
EXTERN bool_t          capcanuseplatforms[MAXMODEL];               // Can use platforms?
EXTERN bool_t          capcangrabmoney[MAXMODEL];                  // Collect money?
EXTERN bool_t          capcanopenstuff[MAXMODEL];                  // Open chests/doors?
EXTERN bool_t          capicon[MAXMODEL];                          // Draw icon
EXTERN bool_t          capforceshadow[MAXMODEL];                   // Draw a shadow?
EXTERN bool_t          capripple[MAXMODEL];                        // Spawn ripples?
EXTERN Uint8           capdamagetargettype[MAXMODEL];              // For AI DamageTarget
EXTERN Uint8           capweaponaction[MAXMODEL];                  // Animation needed to swing
EXTERN bool_t          capgripvalid[MAXMODEL][MAXSLOT];            // Left/Right hands valid
EXTERN Uint8           capattackattached[MAXMODEL];                //
EXTERN Sint8           capattackprttype[MAXMODEL];                 //
EXTERN Uint8           capattachedprtamount[MAXMODEL];             // Sticky particles
EXTERN Uint8           capattachedprtreaffirmdamagetype[MAXMODEL]; // Relight that torch...
EXTERN Uint16          capattachedprttype[MAXMODEL];               //
EXTERN Uint8           capgopoofprtamount[MAXMODEL];               // Poof effect
EXTERN Sint16          capgopoofprtfacingadd[MAXMODEL];            //
EXTERN Uint16          capgopoofprttype[MAXMODEL];                 //
EXTERN bool_t          capbludvalid[MAXMODEL];                    // Blud ( yuck )
EXTERN Uint8           capbludprttype[MAXMODEL];                  //
EXTERN bool_t          capridercanattack[MAXMODEL];                // Rider attack?
EXTERN bool_t          capcanbedazed[MAXMODEL];                    // Can it be dazed?
EXTERN bool_t          capcanbegrogged[MAXMODEL];                  // Can it be grogged?
EXTERN Uint8           capkursechance[MAXMODEL];                   // Chance of being kursed
EXTERN bool_t          capistoobig[MAXMODEL];                      // Can't be put in pack
EXTERN bool_t          capreflect[MAXMODEL];                       // Draw the reflection
EXTERN bool_t          capalwaysdraw[MAXMODEL];                    // Always render
EXTERN bool_t          capisranged[MAXMODEL];                      // Flag for ranged weapon
EXTERN Sint8           caphidestate[MAXMODEL];                       // Don't draw when...
EXTERN bool_t          capisequipment[MAXMODEL];                     // Behave in silly ways
EXTERN Sint8           capisvaluable[MAXMODEL];                      // Force to be valuable
EXTERN Sint8           capsoundindex[MAXMODEL][SOUND_COUNT];       // a map for soundX.wav to sound types
EXTERN Mix_Chunk *     capwavelist[MAXMODEL][MAXWAVE];             // sounds in a object

//skill system
EXTERN Sint8         capshieldproficiency[MAXMODEL];               // Can it use shields?
EXTERN bool_t        capcanjoust[MAXMODEL];                        // Can it use advanced weapons?
EXTERN bool_t        capcanuseadvancedweapons[MAXMODEL];           // Can it use advanced weapons?
EXTERN bool_t        capcanseeinvisible[MAXMODEL];                 // Can it see invisible?
EXTERN bool_t        capcanseekurse[MAXMODEL];                     // Can it see kurses?
EXTERN bool_t        capcanusedivine[MAXMODEL];
EXTERN bool_t        capcanusearcane[MAXMODEL];
EXTERN bool_t        capcanusetech[MAXMODEL];
EXTERN bool_t        capcandisarm[MAXMODEL];
EXTERN bool_t        capcanbackstab[MAXMODEL];
EXTERN bool_t        capcanusepoison[MAXMODEL];
EXTERN bool_t        capcanread[MAXMODEL];

// Network stuff
#define NETREFRESH          1000                    // Every second
#define NONETWORK           numservice              //
#define MAXSERVICE          16
#define NETNAMESIZE         16
#define MAXSESSION          16
#define MAXNETPLAYER         8

EXTERN Uint32                  randsave;                  // Used in network timer
EXTERN int                     networkservice;
EXTERN char                    nethostname[64];                            // Name for hosting session
EXTERN char                    netmessagename[64];                         // Name for messages
EXTERN int                     numservice  EQ( 0 );                           // How many we found
EXTERN char                    netservicename[MAXSERVICE][NETNAMESIZE];    // Names of services
EXTERN int                     numsession  EQ( 0 );                           // How many we found
EXTERN char                    netsessionname[MAXSESSION][NETNAMESIZE];    // Names of sessions
EXTERN int                     numplayer  EQ( 0 );                            // How many we found
EXTERN char                    netplayername[MAXNETPLAYER][NETNAMESIZE];   // Names of machines

EXTERN const char *parse_filename  EQ( NULL );  // For debuggin' goto_colon
EXTERN const char *globalparsename  EQ( NULL ); // The SCRIPT.TXT filename
EXTERN FILE *globalnetworkerr  EQ( NULL ); // For debuggin' network


EXTERN float           hillslide  EQ( 1.00f );                                 //
EXTERN float           slippyfriction  EQ( 1.00f );  //1.05f for Chevron          // Friction
EXTERN float           airfriction  EQ( 0.95f );                                //
EXTERN float           waterfriction  EQ( 0.85f );                              //
EXTERN float           noslipfriction  EQ( 0.95f );                            //
EXTERN float           platstick  EQ( 0.040f );                                 //
EXTERN float           gravity  EQ( ( float ) - 1.0f );                        // Gravitational accel
EXTERN int             damagetileamount  EQ( 256 );                           // Amount of damage
EXTERN Uint8           damagetiletype  EQ( DAMAGE_FIRE );                      // Type of damage

EXTERN char            cFrameName[16];                                     // MD2 Frame Name

EXTERN Uint16          globesttarget;                                      // For find_target
EXTERN Uint16          globestangle;                                       //
EXTERN Uint16          glouseangle;                                        //
EXTERN int             globestdistance;

EXTERN Uint16          meshbumplistchr[MAXMESHFAN/16];                     // For character collisions
EXTERN Uint16          meshbumplistchrnum[MAXMESHFAN/16];                  // Number on the block
EXTERN Uint16          meshbumplistprt[MAXMESHFAN/16];                     // For particle collisions
EXTERN Uint16          meshbumplistprtnum[MAXMESHFAN/16];                  // Number on the block
EXTERN Uint8           meshexploremode  EQ( bfalse );                      // Explore mode?
EXTERN int             maxtotalmeshvertices  EQ( 256*256*6 );              // of vertices
EXTERN int             meshtilesx  EQ(0);                                  // Size in tiles
EXTERN int             meshtilesy  EQ(0);                                  //
EXTERN Uint32          meshtiles   EQ(0);                                  // Number of tiles
EXTERN int             meshblocksx EQ(0);                                  // Size in blocks
EXTERN int             meshblocksy EQ(0);                                  //
EXTERN Uint32          meshblocks  EQ(0);                                  // Number of blocks (collision areas)
EXTERN float           meshedgex;                                          // Limits
EXTERN float           meshedgey;                                          //
EXTERN Uint8           meshtype[MAXMESHFAN];                               // Command type
EXTERN Uint8           meshfx[MAXMESHFAN];                                 // Special effects flags
EXTERN Uint8           meshtwist[MAXMESHFAN];                              //
EXTERN bool_t          meshinrenderlist[MAXMESHFAN];                       //
EXTERN Uint16          meshtile[MAXMESHFAN];                               // Get texture from this
EXTERN Uint32          meshvrtstart[MAXMESHFAN];                           // Which vertex to start at
EXTERN Uint32          meshblockstart[MAXMESHBLOCKY];
EXTERN Uint32          meshfanstart[MAXMESHTILEY];                         // Which fan to start a row with
EXTERN size_t          meshvertcount EQ(0);                                                 // For malloc
EXTERN float*          meshvrtx EQ(NULL);                                           // Vertex position
EXTERN float*          meshvrty EQ(NULL);                                           //
EXTERN float*          meshvrtz EQ(NULL);                                           // Vertex elevation
EXTERN Uint8*          meshvrta EQ(NULL);                                           // Vertex base light
EXTERN Uint8*          meshvrtl EQ(NULL);                                           // Vertex light
EXTERN Uint8           meshcommands[MAXMESHTYPE];                          // Number of commands
EXTERN Uint8           meshcommandsize[MAXMESHTYPE][MAXMESHCOMMAND];       // Entries in each command
EXTERN Uint16          meshcommandvrt[MAXMESHTYPE][MAXMESHCOMMANDENTRIES]; // Fansquare vertex list
EXTERN Uint8           meshcommandnumvertices[MAXMESHTYPE];                // Number of vertices
EXTERN float           meshcommandu[MAXMESHTYPE][MAXMESHVERTICES];         // Vertex texture posi
EXTERN float           meshcommandv[MAXMESHTYPE][MAXMESHVERTICES];         //
EXTERN float           meshtileoffu[MAXTILETYPE];                          // Tile texture offset
EXTERN float           meshtileoffv[MAXTILETYPE];                          //


#define INVALID_BLOCK ((Uint32)(~0))
#define INVALID_TILE  ((Uint32)(~0))

Uint32 mesh_get_block( float pos_x, float pos_y );
Uint32 mesh_get_tile ( float pos_x, float pos_y );
bool_t mesh_allocate_memory( int meshvertices );
void   mesh_free_memory();

EXTERN Uint8 asciitofont[256];                                   // Conversion table

// Display messages
EXTERN Uint16          msgtimechange EQ( 0 );                                  //
EXTERN Uint16          msgstart EQ( 0 );                                       // The message queue
EXTERN Sint16          msgtime[MAXMESSAGE];                                //
EXTERN char            msgtextdisplay[MAXMESSAGE][MESSAGESIZE];            // The displayed text

// Message files
EXTERN Uint16          msgtotal EQ( 0 );                                       // The number of messages
EXTERN Uint32          msgtotalindex EQ( 0 );                                  // Where to put letter
EXTERN Uint32          msgindex[MAXTOTALMESSAGE];                          // Where it is
EXTERN char            msgtext[MESSAGEBUFFERSIZE];                         // The text buffer

// My lil' random number table
#define MAXRAND 4096
EXTERN Uint16  randie[MAXRAND];
EXTERN Uint16  randindex;
#define RANDIE randie[randindex];  randindex=(randindex+1)&(MAXRAND-1)

//End text
#define MAXENDTEXT 1024
EXTERN char endtext[MAXENDTEXT];     // The end-module text
EXTERN int  endtextwrite;

// This is for random naming
EXTERN Uint16           numchop  EQ( 0 );              // The number of name parts
EXTERN Uint32           chopwrite  EQ( 0 );            // The data pointer
EXTERN char             chopdata[CHOPDATACHUNK];    // The name parts
EXTERN Uint16           chopstart[MAXCHOP];         // The first character of each part
EXTERN Uint16           capsectionsize[MAXMODEL][MAXSECTION];   // Number of choices, 0
EXTERN Uint16           capsectionstart[MAXMODEL][MAXSECTION];  //
EXTERN char             namingnames[MAXCAPNAMESIZE];// The name returned by the function

// These are for FindPath function only
#define   MOVE_MELEE  300
#define   MOVE_RANGED  600
#define   MOVE_DISTANCE 175
#define   MOVE_RETREAT  900
#define   MOVE_CHARGE  111
#define   MOVE_FOLLOW  0

// Character AI alerts
#define ALERTIF_SPAWNED                      ( 1 <<  0 )          // 0
#define ALERTIF_HITVULNERABLE                ( 1 <<  1 )          // 1
#define ALERTIF_ATWAYPOINT                   ( 1 <<  2 )           // 2
#define ALERTIF_ATLASTWAYPOINT               ( 1 <<  3 )           // 3
#define ALERTIF_ATTACKED                     ( 1 <<  4 )          // 4
#define ALERTIF_BUMPED                       ( 1 <<  5 )          // 5
#define ALERTIF_ORDERED                      ( 1 <<  6 )          // 6
#define ALERTIF_CALLEDFORHELP                ( 1 <<  7 )         // 7
#define ALERTIF_KILLED                       ( 1 <<  8 )         // 8
#define ALERTIF_TARGETKILLED                 ( 1 <<  9 )         // 9
#define ALERTIF_DROPPED                      ( 1 << 10 )        // 10
#define ALERTIF_GRABBED                      ( 1 << 11 )        // 11
#define ALERTIF_REAFFIRMED                   ( 1 << 12 )        // 12
#define ALERTIF_LEADERKILLED                 ( 1 << 13 )        // 13
#define ALERTIF_USED                         ( 1 << 14 )       // 14
#define ALERTIF_CLEANEDUP                    ( 1 << 15 )       // 15
#define ALERTIF_SCOREDAHIT                   ( 1 << 16 )       // 16
#define ALERTIF_HEALED                       ( 1 << 17 )      // 17
#define ALERTIF_DISAFFIRMED                  ( 1 << 18 )      // 18
#define ALERTIF_CHANGED                      ( 1 << 19 )      // 19
#define ALERTIF_INWATER                      ( 1 << 20 )     // 20
#define ALERTIF_BORED                        ( 1 << 21 )     // 21
#define ALERTIF_TOOMUCHBAGGAGE               ( 1 << 22 )     // 22
#define ALERTIF_GROGGED                      ( 1 << 23 )     // 23
#define ALERTIF_DAZED                        ( 1 << 24 )    // 24
#define ALERTIF_HITGROUND                    ( 1 << 25 )    // 25
#define ALERTIF_NOTDROPPED                   ( 1 << 26 )    // 26
#define ALERTIF_BLOCKED                      ( 1 << 27 )   // 27
#define ALERTIF_THROWN                       ( 1 << 28 )   // 28
#define ALERTIF_CRUSHED                      ( 1 << 29 )   // 29
#define ALERTIF_NOTPUTAWAY                   ( 1 << 30 )  // 30
#define ALERTIF_TAKENOUT                     ( 1 << 31 ) // 31

// This stuff is for actions
#define ACTIONDA            0
#define ACTIONDB            1
#define ACTIONDC            2
#define ACTIONDD            3
#define ACTIONUA            4
#define ACTIONUB            5
#define ACTIONUC            6
#define ACTIONUD            7
#define ACTIONTA            8
#define ACTIONTB            9
#define ACTIONTC            10
#define ACTIONTD            11
#define ACTIONCA            12
#define ACTIONCB            13
#define ACTIONCC            14
#define ACTIONCD            15
#define ACTIONSA            16
#define ACTIONSB            17
#define ACTIONSC            18
#define ACTIONSD            19
#define ACTIONBA            20
#define ACTIONBB            21
#define ACTIONBC            22
#define ACTIONBD            23
#define ACTIONLA            24
#define ACTIONLB            25
#define ACTIONLC            26
#define ACTIONLD            27
#define ACTIONXA            28
#define ACTIONXB            29
#define ACTIONXC            30
#define ACTIONXD            31
#define ACTIONFA            32
#define ACTIONFB            33
#define ACTIONFC            34
#define ACTIONFD            35
#define ACTIONPA            36
#define ACTIONPB            37
#define ACTIONPC            38
#define ACTIONPD            39
#define ACTIONEA            40
#define ACTIONEB            41
#define ACTIONRA            42
#define ACTIONZA            43
#define ACTIONZB            44
#define ACTIONZC            45
#define ACTIONZD            46
#define ACTIONWA            47
#define ACTIONWB            48
#define ACTIONWC            49
#define ACTIONWD            50
#define ACTIONJA            51
#define ACTIONJB            52
#define ACTIONJC            53
#define ACTIONHA            54
#define ACTIONHB            55
#define ACTIONHC            56
#define ACTIONHD            57
#define ACTIONKA            58
#define ACTIONKB            59
#define ACTIONKC            60
#define ACTIONKD            61
#define ACTIONMA            62
#define ACTIONMB            63
#define ACTIONMC            64
#define ACTIONMD            65
#define ACTIONME            66
#define ACTIONMF            67
#define ACTIONMG            68
#define ACTIONMH            69
#define ACTIONMI            70
#define ACTIONMJ            71
#define ACTIONMK            72
#define ACTIONML            73
#define ACTIONMM            74
#define ACTIONMN            75

// For damage/stat pair reads/writes
EXTERN int pairbase, pairrand;
EXTERN float pairfrom, pairto;

// Passages
EXTERN int numpassage;              // Number of passages in the module
EXTERN int passtlx[MAXPASS];          // Passage positions
EXTERN int passtly[MAXPASS];
EXTERN int passbrx[MAXPASS];
EXTERN int passbry[MAXPASS];
EXTERN int passagemusic[MAXPASS];        // Music track appointed to the specific passage
EXTERN Uint8 passmask[MAXPASS];
EXTERN Uint8 passopen[MAXPASS];      // Is the passage open?
#define CLOSETOLERANCE      2                       // For closing doors

// For shops
EXTERN int numshoppassage;
EXTERN Uint16  shoppassage[MAXPASS];  // The passage number
EXTERN Uint16  shopowner[MAXPASS];    // Who gets the gold?
#define NOOWNER 0xFFFF        // Shop has no owner
#define STOLEN  0xFFFF        // Someone stole a item
#define BUY  0
#define SELL 1

// Status displays
EXTERN int numstat  EQ( 0 );
EXTERN Uint16  statlist[MAXSTAT];
EXTERN int statdelay  EQ( 25 );

// Network Stuff
#define SHORTLATCH 1024.0f
#define CHARVEL 5.0f
#define MAXSENDSIZE 8192
#define COPYSIZE    4096
#define TOTALSIZE   2097152
#define MAXPLAYER   8                               // 2 to a power...  2^3
#define MAXLAG      64                              //
#define LAGAND      63                              //
#define STARTTALK   10                              //
EXTERN bool_t                  plavalid[MAXPLAYER];                    // Player used?
EXTERN Uint16                  plaindex[MAXPLAYER];                    // Which character?
EXTERN float                   plalatchx[MAXPLAYER];                   // Local latches
EXTERN float                   plalatchy[MAXPLAYER];                   //
EXTERN Uint32                   plalatchbutton[MAXPLAYER];              //

EXTERN Uint32                  numplatimes EQ( 0 );
EXTERN float                   platimelatchx[MAXPLAYER][MAXLAG];       // Timed latches
EXTERN float                   platimelatchy[MAXPLAYER][MAXLAG];       //
EXTERN Uint32                   platimelatchbutton[MAXPLAYER][MAXLAG];  //
EXTERN Uint32                  platimetimes[MAXPLAYER];         // Timed latches
EXTERN Uint32                  platimetime[MAXPLAYER][MAXLAG];         // Timed latches

EXTERN Uint8                   pladevice[MAXPLAYER];                   // Input device
EXTERN int                     numpla;                                 // Number of players
EXTERN int                     local_numlpla;                            //
EXTERN int                     lag  EQ( 3 );                              // Lag tolerance
EXTERN int                     numfile;                                // For network copy
EXTERN int                     numfilesent;                            // For network copy
EXTERN int                     numfileexpected;                        // For network copy
EXTERN int                     numplayerrespond;                       //

#define TO_ANY_TEXT         25935                               // Message headers
#define TO_HOST_MODULEOK    14951                               //
#define TO_HOST_LATCH       33911                               //
#define TO_HOST_RTS         30376                               //
#define TO_HOST_IM_LOADED   40192                               //
#define TO_HOST_FILE        20482                               //
#define TO_HOST_DIR         49230                               //
#define TO_HOST_FILESENT    13131                               //
#define TO_REMOTE_MODULE    56025                               //
#define TO_REMOTE_LATCH     12715                               //
#define TO_REMOTE_FILE      62198                               //
#define TO_REMOTE_DIR       11034                               //
#define TO_REMOTE_RTS        5143                               //
#define TO_REMOTE_START     51390                               //
#define TO_REMOTE_FILESENT  19903                               //
extern Uint32  nexttimestamp;                // Expected timestamp

// Key/Control input defenitions
#define MAXTAG              128                     // Number of tags in scancode.txt
#define TAGSIZE             32                      // Size of each tag
EXTERN int    numscantag;
EXTERN char   tagname[MAXTAG][TAGSIZE];                      // Scancode names
EXTERN Sint32 tagvalue[MAXTAG];                     // Scancode values

enum e_order
{
    ORDER_NONE  = 0,
    ORDER_ATTACK,
    ORDER_ASSIST,
    ORDER_STAND,
    ORDER_TERRAIN,
    ORDER_COUNT

};

//Quest system
#define QUEST_BEATEN         -1
#define QUEST_NONE             -2

#define  _EGOBOO_H_
