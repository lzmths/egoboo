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

/// @file ui.c
/// @brief The Egoboo GUI
/// @details A basic library for implementing user interfaces, based off of Casey Muratori's
/// IMGUI.  (https://mollyrocket.com/forums/viewtopic.php?t=134)

#include "ui.h"
#include "graphic.h"
#include "font_ttf.h"
#include "texture.h"
#include "log.h"

#include "extensions/ogl_debug.h"
#include "extensions/SDL_extensions.h"

#include "egoboo_display_list.h"

#include <string.h>
#include <SDL_opengl.h>

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

struct UiContext;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

const ego_ui_Just ui_just_nothing    = { UI_JUST_NON, UI_JUST_NON };
const ego_ui_Just ui_just_topleft    = { UI_JUST_LOW, UI_JUST_LOW };
const ego_ui_Just ui_just_topcenter  = { UI_JUST_LOW, UI_JUST_MID };
const ego_ui_Just ui_just_topright   = { UI_JUST_LOW, UI_JUST_HGH };
const ego_ui_Just ui_just_centered   = { UI_JUST_MID, UI_JUST_MID };
const ego_ui_Just ui_just_centerleft = { UI_JUST_MID, UI_JUST_LOW };

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define UI_MAX_JOYSTICKS 8
#define UI_CONTROL_TIMER 100
struct ego_ui_control_info
{
    int    timer;
    float  scr[2];
    float  vrt[2];
};

static void ui_joy_init( UiContext * pctxt );
static void ui_cursor_update();
static bool_t ui_joy_set( SDL_JoyAxisEvent * evt_ptr );

/// The data to describe the UI state
struct UiContext
{
    // Tracking control focus stuff
    ui_id_t active;
    ui_id_t hot;

    // info on the mouse control
    ego_ui_control_info mouse;
    ego_ui_control_info joy;
    ego_ui_control_info joys[UI_MAX_JOYSTICKS];

    // Basic cursor state
    float cursor_X, cursor_Y;
    int   cursor_Released;
    int   cursor_Pressed;

    STRING defaultFontName;
    float  defaultFontSize;
    TTF_Font  *defaultFont;
    TTF_Font  *activeFont;

    // virtual window
    float vw, vh, ww, wh;

    // define the forward transform
    float aw, ah, bw, bh;

    // define the inverse transform
    float iaw, iah, ibw, ibh;

    UiContext() { clear( this ); }

    static UiContext * clear( UiContext * ptr )
    {
        memset( ptr, 0, sizeof( *ptr ) );

        ptr->active = UI_Nothing;
        ptr->hot    = UI_Nothing;

        // define the forward transform
        ptr->aw = ptr->ah = 1.0f;

        // define the inverse transform
        ptr->iaw = ptr->iah = 1.0f;

        ui_joy_init( ptr );

        return ptr;
    }

};

static struct UiContext ui_context;

static const GLfloat ui_white_color[]  = {1.00f, 1.00f, 1.00f, 1.00f};

static const GLfloat ui_active_color[]  = {0.00f, 0.00f, 0.90f, 0.60f};
static const GLfloat ui_hot_color[]     = {0.54f, 0.00f, 0.00f, 1.00f};
static const GLfloat ui_normal_color[]  = {0.66f, 0.00f, 0.00f, 0.60f};

static const GLfloat ui_active_color2[] = {0.00f, 0.45f, 0.45f, 0.60f};
static const GLfloat ui_hot_color2[]    = {0.00f, 0.28f, 0.28f, 1.00f};
static const GLfloat ui_normal_color2[] = {0.33f, 0.00f, 0.33f, 0.60f};

static void ui_virtual_to_screen_abs( float vx, float vy, float *rx, float *ry );
static void ui_screen_to_virtual_abs( float rx, float ry, float *vx, float *vy );

static void ui_virtual_to_screen_rel( float vx, float vy, float *rx, float *ry );
static void ui_screen_to_virtual_rel( float rx, float ry, float *vx, float *vy );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// Core functions
int ui_begin( const char *default_font, int default_font_size )
{
    // initialize the font handler
    fnt_init();

    UiContext::clear( &ui_context );

    ui_context.defaultFontSize = default_font_size;
    strncpy( ui_context.defaultFontName, default_font, SDL_arraysize( ui_context.defaultFontName ) );

    ui_set_virtual_screen( sdl_scr.x, sdl_scr.y, sdl_scr.x, sdl_scr.y );

    return 1;
}

//--------------------------------------------------------------------------------------------
void ui_end()
{
    // clear out the default font
    if ( NULL != ui_context.defaultFont )
    {
        fnt_freeFont( ui_context.defaultFont );
        ui_context.defaultFont = NULL;
    }

    // clear out the active font
    ui_context.activeFont = NULL;

    UiContext::clear( &ui_context );
}

//--------------------------------------------------------------------------------------------
void ui_Reset()
{
    ui_context.active = ui_context.hot = UI_Nothing;
}

//--------------------------------------------------------------------------------------------
bool_t ui_handleSDLEvent( SDL_Event *evt )
{
    bool_t handled;

    if ( NULL == evt ) return bfalse;

    handled = btrue;
    switch ( evt->type )
    {
        case SDL_JOYBUTTONDOWN:
        case SDL_MOUSEBUTTONDOWN:
            ui_context.cursor_Released = 0;
            ui_context.cursor_Pressed = 1;
            break;

        case SDL_JOYBUTTONUP:
        case SDL_MOUSEBUTTONUP:
            ui_context.cursor_Pressed = 0;
            ui_context.cursor_Released = 1;
            break;

        case SDL_MOUSEMOTION:
            // convert the screen coordinates to our "virtual coordinates"
            ui_context.mouse.scr[0] = evt->motion.x;
            ui_context.mouse.vrt[1] = evt->motion.y;
            ui_screen_to_virtual_abs( ui_context.mouse.scr[0], ui_context.mouse.vrt[1], &( ui_context.mouse.vrt[0] ), &( ui_context.mouse.vrt[1] ) );
            ui_context.mouse.timer  = 2 * UI_CONTROL_TIMER;
            break;

        case SDL_JOYAXISMOTION:
            ui_joy_set( &( evt->jaxis ) );
            break;

        case SDL_VIDEORESIZE:
            if ( SDL_VIDEORESIZE == evt->resize.type )
            {
                // the video has been re-sized, if the game is active, the
                // view matrix needs to be recalculated and possibly the
                // auto-formatting for the menu system and the ui system must be
                // recalculated

                // grab all the new SDL screen info
                SDLX_Get_Screen_Info( &sdl_scr, SDL_FALSE );

                // set the ui's virtual screen size based on the graphic system's
                // configuration
                gfx_set_virtual_screen( &gfx );
            }
            break;

        default:
            handled = bfalse;
    }

    return handled;
}

//--------------------------------------------------------------------------------------------
void ui_beginFrame( float deltaTime )
{
    ATTRIB_PUSH( "ui_beginFrame", GL_ENABLE_BIT );
    GL_DEBUG( glDisable )( GL_DEPTH_TEST );
    GL_DEBUG( glDisable )( GL_CULL_FACE );
    GL_DEBUG( glEnable )( GL_TEXTURE_2D );

    GL_DEBUG( glEnable )( GL_BLEND );
    GL_DEBUG( glBlendFunc )( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    GL_DEBUG( glViewport )( 0, 0, sdl_scr.x, sdl_scr.y );

    // Set up an ortho projection for the gui to use.  Controls are free to modify this
    // later, but most of them will need this, so it's done by default at the beginning
    // of a frame
    GL_DEBUG( glMatrixMode )( GL_PROJECTION );
    GL_DEBUG( glPushMatrix )();
    GL_DEBUG( glLoadIdentity )();
    GL_DEBUG( glOrtho )( 0, sdl_scr.x, sdl_scr.y, 0, -1, 1 );

    GL_DEBUG( glMatrixMode )( GL_MODELVIEW );
    GL_DEBUG( glLoadIdentity )();

    // hotness gets reset at the start of each frame
    ui_context.hot = UI_Nothing;

    // update the cursor position
    ui_cursor_update();
}

//--------------------------------------------------------------------------------------------
void ui_draw_cursor_icon( TX_REF icon_ref )
{
    float x, y;

    ui_virtual_to_screen_abs( ui_context.cursor_X - 1, ui_context.cursor_Y - 1, &x, &y );

    // Draw the cursor
    draw_one_icon( icon_ref, x, y, NOSPARKLE );
}

//--------------------------------------------------------------------------------------------
void ui_draw_cursor_ogl()
{
    const float cursor_h = 15.0f;
    const float cursor_w  = cursor_h * INV_SQRT_TWO;

    const float cursor_d1  = 0.92387953251128675612818318939679f;
    const float cursor_d2  = 0.3826834323650897717284599840304f;

    float x1, y1, x2, y2, x3, y3;

    ATTRIB_PUSH( "ui_draw_cursor_ogl", GL_ENABLE_BIT | GL_CURRENT_BIT | GL_HINT_BIT | GL_COLOR_BUFFER_BIT );
    {
        // mmm.... anti-aliased cursor....

        GL_DEBUG( glDisable )( GL_TEXTURE_2D );                           // GL_ENABLE_BIT

        GL_DEBUG( glEnable )( GL_LINE_SMOOTH );                           // GL_ENABLE_BIT
        GL_DEBUG( glEnable )( GL_POLYGON_SMOOTH );                        // GL_ENABLE_BIT

        GL_DEBUG( glEnable )( GL_BLEND );                                 // GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT
        GL_DEBUG( glBlendFunc )( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );  // GL_COLOR_BUFFER_BIT

        GL_DEBUG( glHint )( GL_LINE_SMOOTH_HINT, GL_NICEST );             // GL_HINT_BIT
        GL_DEBUG( glHint )( GL_POLYGON_SMOOTH_HINT, GL_NICEST );          // GL_HINT_BIT

        ui_virtual_to_screen_abs( ui_context.cursor_X,          ui_context.cursor_Y,          &x1, &y1 );
        ui_virtual_to_screen_abs( ui_context.cursor_X + cursor_w, ui_context.cursor_Y + cursor_w, &x2, &y2 );
        ui_virtual_to_screen_abs( ui_context.cursor_X,          ui_context.cursor_Y + cursor_h, &x3, &y3 );

        // Draw the head
        GL_DEBUG( glColor4f )( 1, 1, 1, 1 );                   // GL_CURRENT_BIT
        GL_DEBUG( glBegin )( GL_POLYGON );
        {
            GL_DEBUG( glVertex2f )( x1, y1 );
            GL_DEBUG( glVertex2f )( x2, y2 );
            GL_DEBUG( glVertex2f )( x3, y3 );
        }
        GL_DEBUG_END();

        // Draw the tail
        {
            float dx1 = SQRT_TWO * cursor_h * cursor_d2;
            float dy1 = SQRT_TWO * cursor_h * cursor_d1;

            float dx2 = cursor_h / 10.0f * cursor_d1;
            float dy2 = -cursor_h / 10.0f * cursor_d2;

            ui_virtual_to_screen_abs( ui_context.cursor_X + dx1 + dx2, ui_context.cursor_Y + dy1 + dy2, &x2, &y2 );
            ui_virtual_to_screen_abs( ui_context.cursor_X + dx1 - dx2, ui_context.cursor_Y + dy1 - dy2, &x3, &y3 );
        }

        GL_DEBUG( glBegin )( GL_POLYGON );
        {
            GL_DEBUG( glVertex2f )( x1, y1 );
            GL_DEBUG( glVertex2f )( x2, y2 );
            GL_DEBUG( glVertex2f )( x3, y3 );
        }
        GL_DEBUG_END();
    }
    ATTRIB_POP( "ui_draw_cursor_ogl" );
}

//--------------------------------------------------------------------------------------------
void ui_draw_cursor()
{
    // see if the icon cursor is available
    TX_REF           ico_ref = TX_REF( TX_CURSOR );
    oglx_texture_t * ptex    = TxTexture_get_ptr( ico_ref );

    if ( NULL == ptex || INVALID_GL_ID == ptex->base.binding )
    {
        ui_draw_cursor_ogl();
    }
    else
    {
        ui_draw_cursor_icon( ico_ref );
    }
}

//--------------------------------------------------------------------------------------------
void ui_endFrame()
{
    // Draw the cursor last
    ui_draw_cursor();

    // Restore the OpenGL matrices to what they were
    GL_DEBUG( glMatrixMode )( GL_PROJECTION );
    GL_DEBUG( glPopMatrix )();

    GL_DEBUG( glMatrixMode )( GL_MODELVIEW );
    GL_DEBUG( glLoadIdentity )();

    // Re-enable any states disabled by gui_beginFrame
    ATTRIB_POP( "ui_endFrame" );

    // Clear input states at the end of the frame
    ui_context.cursor_Pressed = ui_context.cursor_Released = 0;
}

//--------------------------------------------------------------------------------------------
// Utility functions
int ui_mouseInside( float vx, float vy, float vwidth, float vheight )
{
    float vright, vbottom;

    vright  = vx + vwidth;
    vbottom = vy + vheight;
    if ( vx <= ui_context.cursor_X && vy <= ui_context.cursor_Y && ui_context.cursor_X <= vright && ui_context.cursor_Y <= vbottom )
    {
        return 1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------
void ui_setactive( ui_id_t id )
{
    ui_context.active = id;
}

//--------------------------------------------------------------------------------------------
void ui_sethot( ui_id_t id )
{
    ui_context.hot = id;
}

//--------------------------------------------------------------------------------------------
void ui_Widget::setActive( ui_Widget * pw )
{
    if ( NULL == pw )
    {
        ui_context.active = UI_Nothing;
    }
    else
    {
        ui_context.active = pw->id;

        pw->timeout = egoboo_get_ticks() + 100;
        if ( ui_Widget::LatchMask_Test( pw, UI_LATCH_CLICKED ) )
        {
            // use exclusive or to flip the bit
            pw->latch_state ^= UI_LATCH_CLICKED;
        };
    };
}

//--------------------------------------------------------------------------------------------
void ui_Widget::setHot( ui_Widget * pw )
{
    if ( NULL == pw )
    {
        ui_context.hot = UI_Nothing;
    }
    else if (( ui_context.active == pw->id || ui_context.active == UI_Nothing ) )
    {
        if ( pw->timeout < egoboo_get_ticks() )
        {
            pw->timeout = egoboo_get_ticks() + 100;

            if ( ui_Widget::LatchMask_Test( pw, UI_LATCH_MOUSEOVER ) && ui_context.hot != pw->id )
            {
                pw->latch_state |= UI_LATCH_MOUSEOVER;
            };
        };

        // Only allow hotness to be set if this control, or no control is active
        ui_context.hot = pw->id;
    }
}

//--------------------------------------------------------------------------------------------
TTF_Font * ui_getFont()
{
    return ( NULL != ui_context.activeFont ) ? ui_context.activeFont : ui_context.defaultFont;
}

//--------------------------------------------------------------------------------------------
TTF_Font* ui_setFont( TTF_Font * font )
{
    ui_context.activeFont = font;

    return ui_context.activeFont;
}

//--------------------------------------------------------------------------------------------
// Behaviors
ui_buttonValues ui_buttonBehavior( ui_id_t id, float vx, float vy, float vwidth, float vheight )
{
    ui_buttonValues result = BUTTON_NOCHANGE;

    if ( id == UI_Nothing )
        return result;

    // If the mouse is over the button, try and set hotness so that it can be cursor_clicked
    if ( ui_mouseInside( vx, vy, vwidth, vheight ) )
    {
        ui_sethot( id );
    }

    // Check to see if the button gets cursor_clicked on
    if ( id == ui_context.active )
    {
        if ( 1 == ui_context.cursor_Released )
        {
            ui_setactive( UI_Nothing );
            result = BUTTON_UP;
        }
    }
    else if ( id == ui_context.hot )
    {
        if ( 1 == ui_context.cursor_Pressed )
        {
            ui_setactive( id );
            result = BUTTON_DOWN;
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------
ui_buttonValues ui_Widget::Behavior( ui_Widget * pWidget )
{
    ui_buttonValues result = BUTTON_NOCHANGE;

    if ( NULL == pWidget ) return result;

    if ( UI_Nothing == pWidget->id ) return result;

    // If the mouse is over the button, try and set hotness so that it can be cursor_clicked
    if ( ui_mouseInside( pWidget->vx, pWidget->vy, pWidget->vwidth, pWidget->vheight ) )
    {
        ui_Widget::setHot( pWidget );
    }

    // Check to see if the button gets cursor_clicked on
    if ( ui_context.active == pWidget->id )
    {
        if ( 1 == ui_context.cursor_Released )
        {
            // mouse button up
            ui_Widget::setActive( NULL );
            result = BUTTON_UP;
        }
    }
    else if ( ui_context.hot == pWidget->id )
    {
        if ( 1 == ui_context.cursor_Pressed )
        {
            // mouse button down
            ui_Widget::setActive( pWidget );
            result = BUTTON_DOWN;
        }
    }
    else
    {
        pWidget->latch_state &= ~UI_LATCH_MOUSEOVER;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
// Drawing
void ui_drawButton( ui_id_t id, float vx, float vy, float vwidth, float vheight, const GLXvector4f pcolor )
{
    float x1, x2, y1, y2;

    GLXvector4f color_1 = { 0.0f, 0.0f, 0.9f, 0.6f };
    GLXvector4f color_2 = { 0.54f, 0.0f, 0.0f, 1.0f };
    GLXvector4f color_3 = { 0.66f, 0.0f, 0.0f, 0.6f };

    // Draw the button
    GL_DEBUG( glDisable )( GL_TEXTURE_2D );

    if ( NULL == pcolor )
    {
        if ( ui_context.active != UI_Nothing && ui_context.active == id && ui_context.hot == id )
        {
            pcolor = color_1;
        }
        else if ( ui_context.hot != UI_Nothing && ui_context.hot == id )
        {
            pcolor = color_2;
        }
        else
        {
            pcolor = color_3;
        }
    }

    // convert the virtual coordinates to screen coordinates
    ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );
    ui_virtual_to_screen_abs( vx + vwidth, vy + vheight, &x2, &y2 );

    GL_DEBUG( glColor4fv )( pcolor );
    GL_DEBUG( glBegin )( GL_QUADS );
    {
        GL_DEBUG( glVertex2f )( x1, y1 );
        GL_DEBUG( glVertex2f )( x1, y2 );
        GL_DEBUG( glVertex2f )( x2, y2 );
        GL_DEBUG( glVertex2f )( x2, y1 );
    }
    GL_DEBUG_END();

    GL_DEBUG( glEnable )( GL_TEXTURE_2D );
}

//--------------------------------------------------------------------------------------------
void ui_drawImage( ui_id_t id, oglx_texture_t *img, float vx, float vy, float vwidth, float vheight, const GLXvector4f image_tint )
{
    GLXvector4f tmp_tint = {1, 1, 1, 1};

    float vw, vh;
    float tx, ty;
    float x1, x2, y1, y2;

    // handle optional parameters
    if ( NULL == image_tint ) image_tint = tmp_tint;

    if ( img )
    {
        if ( vwidth == 0 || vheight == 0 )
        {
            vw = img->imgW;
            vh = img->imgH;
        }
        else
        {
            vw = vwidth;
            vh = vheight;
        }

        tx = ( float ) oglx_texture_GetImageWidth( img )  / ( float ) oglx_texture_GetTextureWidth( img );
        ty = ( float ) oglx_texture_GetImageHeight( img ) / ( float ) oglx_texture_GetTextureHeight( img );

        // convert the virtual coordinates to screen coordinates
        ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );
        ui_virtual_to_screen_abs( vx + vw, vy + vh, &x2, &y2 );

        // Draw the image
        oglx_texture_Bind( img );

        GL_DEBUG( glColor4fv )( image_tint );

        GL_DEBUG( glBegin )( GL_QUADS );
        {
            GL_DEBUG( glTexCoord2f )( 0,  0 );  GL_DEBUG( glVertex2f )( x1, y1 );
            GL_DEBUG( glTexCoord2f )( tx,  0 );  GL_DEBUG( glVertex2f )( x2, y1 );
            GL_DEBUG( glTexCoord2f )( tx, ty );  GL_DEBUG( glVertex2f )( x2, y2 );
            GL_DEBUG( glTexCoord2f )( 0, ty );  GL_DEBUG( glVertex2f )( x1, y2 );
        }
        GL_DEBUG_END();
    }
}

//--------------------------------------------------------------------------------------------
egoboo_rv ui_Widget::drawButton( ui_Widget * pw )
{
    const GLfloat * pcolor = NULL;
    bool_t ui_active, ui_hot;

    if ( NULL == pw ) return rv_error;

    if ( !pw->on || 0 == DisplayMask_Test( pw, UI_DISPLAY_BUTTON ) ) return rv_fail;

    ui_active = ui_context.active == pw->id && ui_context.hot == pw->id;
    ui_hot    = ui_context.hot == pw->id;

    if ( UI_Nothing == pw->id )
    {
        // this is a label
        pcolor = ui_normal_color;
    }
    else if ( 0 != pw->latch_mask )
    {
        // this is a "normal" button

        bool_t st_active, st_hot;

        st_active = 0 != ( ui_Widget::LatchMask_Test( pw, UI_LATCH_CLICKED ) & pw->latch_state );
        st_hot    = 0 != ( ui_Widget::LatchMask_Test( pw, UI_LATCH_MOUSEOVER ) & pw->latch_state );

        if ( ui_active || st_active )
        {
            pcolor = ui_normal_color2;
        }
        else if ( ui_hot || st_hot )
        {
            pcolor = ui_hot_color;
        }
        else
        {
            pcolor = ui_normal_color;
        }
    }
    else
    {
        // this is a "presistent" button

        if ( ui_active )
        {
            pcolor = ui_active_color;
        }
        else if ( ui_hot )
        {
            pcolor = ui_hot_color;
        }
        else
        {
            pcolor = ui_normal_color;
        }
    }

    ui_drawButton( pw->id, pw->vx, pw->vy, pw->vwidth, pw->vheight, pcolor );

    return rv_success;
}

//--------------------------------------------------------------------------------------------
egoboo_rv ui_Widget::drawImage( ui_Widget * pw )
{
    if ( NULL == pw ) return rv_error;

    if ( !pw->on || 0 == ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_IMAGE ) )
    {
        return rv_fail;
    }

    if ( NULL == pw->img )
    {
        return rv_success;
    }

    ui_Widget wtmp;

    ui_Widget::shrink( &wtmp, pw, 5 );
    wtmp.vwidth = wtmp.vheight;

    ui_drawImage( wtmp.id, wtmp.img, wtmp.vx, wtmp.vy, wtmp.vwidth, wtmp.vheight, NULL );

    return rv_success;
}

//--------------------------------------------------------------------------------------------
egoboo_rv ui_Widget::drawText( ui_Widget * pw )
{
    if ( NULL == pw ) return rv_error;

    if ( !pw->on || 0 == ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_TEXT ) )
    {
        return rv_fail;
    }

    if ( NULL == pw->tx_lst )
    {
        return rv_success;
    }

    GL_DEBUG( glColor3f )( 1, 1, 1 );
    display_list_draw( pw->tx_lst );

    return rv_success;
}

//--------------------------------------------------------------------------------------------
/** ui_vupdateTextBox
 * Draws a text string into a box, splitting it into lines according to newlines in the string.
 * @warning Doesn't pay attention to the width/height arguments yet.
 *
 * text    - The text to draw
 * x       - The x position to start drawing at
 * y       - The y position to start drawing at
 * width   - Maximum width of the box (not implemented)
 * height  - Maximum height of the box (not implemented)
 * spacing - Amount of space to move down between lines. (usually close to your font size)
 */

//--------------------------------------------------------------------------------------------
float ui_drawTextBoxImmediate( TTF_Font * ttf_ptr, float vx, float vy, float spacing, const char *format, ... )
{
    display_list_t * tmp_tx_lst = NULL;
    va_list args;

    // render the text to a display_list
    va_start( args, format );
    tmp_tx_lst = ui_vupdateTextBox( tmp_tx_lst, ttf_ptr, vx, vy, spacing, format, args );
    va_end( args );

    if ( NULL != tmp_tx_lst )
    {
        float x1, y1;

        frect_t bound;
        int line_count;

        // set the texture position
        display_list_pbound( tmp_tx_lst, &bound );
        ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );
        display_list_adjust_bound( tmp_tx_lst, x1 - bound.x, y1 - bound.y );

        // output the lines
        line_count = display_list_draw( tmp_tx_lst );

        // adjust the vertical position
        vy += spacing * line_count;

        // get rid of the temporary list
        tmp_tx_lst = display_list_dtor( tmp_tx_lst, btrue );
    }

    // return the new vertical position
    return vy;
}

//--------------------------------------------------------------------------------------------
display_item_t * ui_vupdateText( display_item_t * tx_ptr, TTF_Font * ttf_ptr, float vx, float vy, const char *format, va_list args )
{
    float  x1, y1;
    bool_t local_dspl;
    display_item_t * retval = NULL;

    // use the default ui font?
    if ( NULL == ttf_ptr )
    {
        ttf_ptr = ui_getFont();
    }

    // make sure we have a list of some kind
    local_dspl = bfalse;
    if ( NULL == tx_ptr )
    {
        tx_ptr = display_item_new();
        local_dspl = btrue;
    }

    // convert the virtual coordinates to screen coordinates
    ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );

    // convert the text to a display_item using screen coordinates
    retval = fnt_vconvertText( tx_ptr, ttf_ptr, format, args );
    display_item_set_pos( tx_ptr, x1, y1 );

    // an error
    if ( NULL == retval )
    {
        // delete the display list data
        tx_ptr = display_item_free( tx_ptr, local_dspl );
    }

    return tx_ptr;
}

//--------------------------------------------------------------------------------------------
display_item_t * ui_updateText( display_item_t * tx_ptr, TTF_Font * ttf_ptr, float vx, float vy, const char *format, ... )
{
    va_list args;

    // use the default ui font?
    if ( NULL == ttf_ptr )
    {
        ttf_ptr = ui_getFont();
    }

    va_start( args, format );
    tx_ptr = ui_vupdateText( tx_ptr, ttf_ptr, vx, vy, format, args );
    va_end( args );

    return tx_ptr;
}

//--------------------------------------------------------------------------------------------
bool_t ui_drawText( display_item_t * tx_ptr, float vx, float vy )
{
    float x1, y1;

    if ( NULL == tx_ptr ) return bfalse;

    ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );

    display_item_set_pos( tx_ptr, x1, y1 );

    return GL_TRUE == display_item_draw( tx_ptr );
}

//--------------------------------------------------------------------------------------------
display_list_t * ui_updateTextBox_literal( display_list_t * tx_lst, TTF_Font * ttf_ptr, float vx, float vy, float vspacing, const char * text )
{
    float   x1, y1;
    float   spacing;
    GLsizei line_count;
    bool_t  local_dspl_lst;

    if ( NULL == ttf_ptr || NULL == text || '\0' == text[0] )
    {
        return display_list_dtor( tx_lst, bfalse );
    }

    // use the default ui font?
    if ( NULL == ttf_ptr ) ttf_ptr = ui_getFont();

    // since tx_lst is not returned from fnt_convertTextBox_literal,
    // we must have a valid one to pass it
    local_dspl_lst = bfalse;
    if ( NULL == tx_lst )
    {
        tx_lst = display_list_ctor( tx_lst, MAX_WIDGET_TEXT );
        local_dspl_lst = btrue;
    }
    else if ( 0 == display_list_used( tx_lst ) )
    {
        tx_lst = display_list_ctor( tx_lst, MAX_WIDGET_TEXT );
        local_dspl_lst = bfalse;
    }

    // convert the virtual coordinates to screen coordinates
    ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );
    spacing = ui_context.ah * vspacing;

    // convert the text to a display_list using screen coordinates
    line_count = fnt_convertTextBox_literal( tx_lst, ttf_ptr, x1, y1, spacing, text );

    if ( line_count <= 0 )
    {
        tx_lst = display_list_dtor( tx_lst, local_dspl_lst );
    }
    else if ( line_count > display_list_used( tx_lst ) )
    {
        log_warning( "The following text was too long for the requested text box\n****start****\n%s\n****end****\n", text );
    }

    return tx_lst;
}

//--------------------------------------------------------------------------------------------
display_list_t * ui_vupdateTextBox( display_list_t * tx_lst, TTF_Font * ttf_ptr, float vx, float vy, float vspacing, const char * format, va_list args )
{
    display_list_t * retval;
    int vsnprintf_rv;

    char text[4096];

    if ( NULL == format || '\0' == format[0] )
    {
        return display_list_dtor( tx_lst, bfalse );
    }

    // convert the text string
    vsnprintf_rv = vsnprintf( text, SDL_arraysize( text ) - 1, format, args );

    // some problem printing the text?
    if ( vsnprintf_rv < 0 )
    {
        retval = display_list_dtor( tx_lst, bfalse );
    }
    else
    {
        retval = ui_updateTextBox_literal( tx_lst, ttf_ptr, vx, vy, vspacing, text );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
display_list_t * ui_updateTextBox( display_list_t * tx_lst, TTF_Font * ttf_ptr, float vx, float vy, float vspacing, const char * format, ... )
{
    // the normal entry function for ui_vupdateTextBox()

    va_list args;

    va_start( args, format );
    tx_lst = ui_vupdateTextBox( tx_lst, ttf_ptr, vx, vy, vspacing, format, args );
    va_end( args );

    return tx_lst;
}

//--------------------------------------------------------------------------------------------
int ui_drawTextBox( display_list_t * tx_lst, float vx, float vy, float vwidth, float vheight )
{
    int rv = 0;
    float x1, x2, y1, y2;
    size_t size;

    if ( NULL == tx_lst ) return 0;

    size = display_list_used( tx_lst );
    if ( 0 == size ) return 0;

    ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );

    // default size?
    x2 = y2 = -1.0f;
    if ( vwidth < 0.0f || vheight < 0.0f )
    {
        frect_t tmp;

        if ( display_list_pbound( tx_lst, &tmp ) )
        {
            x2 = tmp.x + tmp.w;
            y2 = tmp.y + tmp.h;
        }
    }
    else
    {
        ui_virtual_to_screen_abs( vx + vwidth, vy + vheight, &x2, &y2 );
    }

    ATTRIB_PUSH( "ui_vupdateTextBox", GL_SCISSOR_BIT | GL_ENABLE_BIT );
    {
        //if( x2 > 0.0f && y2 > 0.0f )
        //{
        //    GL_DEBUG( glEnable )( GL_SCISSOR_TEST );
        //    GL_DEBUG( glScissor )( x1, y1, x2-x1, y2-y1 );
        //}

        rv = display_list_draw( tx_lst );
    }
    ATTRIB_POP( "ui_vupdateTextBox" );

    return rv;
}

//--------------------------------------------------------------------------------------------
// Controls
ui_buttonValues ui_doButton( ui_id_t id, display_item_t * tx_ptr, const char *text, float vx, float vy, float vwidth, float vheight )
{
    ui_buttonValues result;
    bool_t          convert_rv = bfalse;

    // Do all the logic type work for the button
    result = ui_buttonBehavior( id, vx, vy, vwidth, vheight );

    // Draw the button part of the button
    ui_drawButton( id, vx, vy, vwidth, vheight, NULL );

    // And then draw the text that goes on top of the button
    convert_rv = bfalse;
    if ( NULL != text && '\0' != text[0] )
    {
        int text_w, text_h;
        int text_x, text_y;
        float x1, x2, y1, y2;

        // convert the virtual coordinates to screen coordinates
        ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );
        ui_virtual_to_screen_abs( vx + vwidth, vy + vheight, &x2, &y2 );

        // find the vwidth & vheight of the text to be drawn, so that it can be centered inside
        // the button
        fnt_getTextSize( ui_getFont(), &text_w, &text_h, text );

        text_x = (( x2 - x1 ) - text_w ) / 2 + x1;
        text_y = (( y2 - y1 ) - text_h ) / 2 + y1;

        GL_DEBUG( glColor3f )( 1, 1, 1 );

        if ( NULL == tx_ptr )
        {
            float new_y = ui_drawTextBoxImmediate( ui_getFont(), text_x, text_y, 20, text );

            convert_rv = ( new_y != text_y );
        }
        else
        {
            tx_ptr = fnt_convertText( tx_ptr, ui_getFont(), text );
            if ( NULL != tx_ptr )
            {
                display_item_set_bound( tx_ptr, text_x, text_y, text_w, text_h );
            }

            convert_rv = GL_TRUE == display_item_draw( tx_ptr );
        }
    }

    // if there was an error, delete any existing tx_ptr
    if ( !convert_rv )
    {
        tx_ptr = display_item_free( tx_ptr, bfalse );
    }

    return result;
}

//--------------------------------------------------------------------------------------------
ui_buttonValues ui_doImageButton( ui_id_t id, oglx_texture_t *img, float vx, float vy, float vwidth, float vheight, GLXvector3f image_tint )
{
    ui_buttonValues result;

    // Do all the logic type work for the button
    result = ui_buttonBehavior( id, vx, vy, vwidth, vheight );

    // Draw the button part of the button
    ui_drawButton( id, vx, vy, vwidth, vheight, NULL );

    // And then draw the image on top of it
    ui_drawImage( id, img, vx + 5, vy + 5, vwidth - 10, vheight - 10, image_tint );

    return result;
}

//--------------------------------------------------------------------------------------------
ui_buttonValues ui_doImageButtonWithText( ui_id_t id, display_item_t * tx_ptr, oglx_texture_t *img, const char *text, float vx, float vy, float vwidth, float vheight )
{
    ui_buttonValues result;

    float text_x, text_y;
    int   text_w, text_h;
    bool_t loc_display = bfalse, retval = bfalse;

    //are we going to create a display here?
    loc_display = bfalse;
    if ( NULL == tx_ptr )
    {
        loc_display = ( NULL == tx_ptr );
    }

    // Do all the logic type work for the button
    result = ui_buttonBehavior( id, vx, vy, vwidth, vheight );

    // Draw the button part of the button
    ui_drawButton( id, vx, vy, vwidth, vheight, NULL );

    // Draw the image part
    ui_drawImage( id, img, vx + 5, vy + 5, 0, 0, NULL );

    // And draw the text next to the image
    // And then draw the text that goes on top of the button
    retval = bfalse;
    if ( NULL != tx_ptr )
    {
        float x1, x2, y1, y2;

        // convert the virtual coordinates to screen coordinates
        ui_virtual_to_screen_abs( vx, vy, &x1, &y1 );
        ui_virtual_to_screen_abs( vx + vwidth, vy + vheight, &x2, &y2 );

        // find the vwidth & vheight of the text to be drawn, so that it can be centered inside
        // the button
        fnt_getTextSize( ui_getFont(), &text_w, &text_h, text );

        text_x = ( img->imgW + 10 ) * ui_context.aw + x1;
        text_y = (( y2 - y1 ) - text_h ) / 2         + y1;

        GL_DEBUG( glColor3f )( 1, 1, 1 );

        tx_ptr = fnt_convertText( tx_ptr, ui_getFont(), text );
        if ( NULL != tx_ptr )
        {
            display_item_set_bound( tx_ptr, text_x, text_y, text_w, text_h );
        }

        retval = GL_TRUE == display_item_draw( tx_ptr );
    }

    if ( !retval && loc_display )
    {
        tx_ptr = display_item_free( tx_ptr, btrue );
    }

    return result;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
ui_buttonValues ui_Widget::Run( ui_Widget * pw )
{
    ui_buttonValues result;

    // is the widget even on?
    if ( !pw->on ) return BUTTON_NOCHANGE;

    // Do all the logic type work for the button
    result = ui_Widget::Behavior( pw );

    // Draw the button part of the button
    ui_Widget::drawButton( pw );

    // draw any image on the left hand side of the button
    ui_Widget::drawImage( pw );

    // And draw the text on the right hand side of any image
    ui_Widget::drawText( pw );

    return result;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::copy( ui_Widget * pdst, ui_Widget * psrc )
{
    void * rv_ptr = NULL;

    if ( NULL == pdst || NULL == psrc ) return bfalse;

    rv_ptr = memcpy( pdst, psrc, sizeof( ui_Widget ) );

    // we do not own these, even if the source does
    pdst->img_own = bfalse;
    pdst->tx_own = bfalse;

    return NULL != rv_ptr;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::shrink( ui_Widget * pw2, ui_Widget * pw1, float pixels )
{
    if ( NULL == pw2 || NULL == pw1 ) return bfalse;

    if ( !ui_Widget::copy( pw2, pw1 ) ) return bfalse;

    pw2->vx += pixels;
    pw2->vy += pixels;
    pw2->vwidth  -= 2 * pixels;
    pw2->vheight -= 2 * pixels;

    if ( pw2->vwidth < 0 )  pw2->vwidth   = 0;
    if ( pw2->vheight < 0 ) pw2->vheight = 0;

    return pw2->vwidth > 0 && pw2->vheight > 0;
}

//--------------------------------------------------------------------------------------------
//bool_t ui_Widget::init( ui_Widget * pw, ui_id_t id, TTF_Font * ttf_ptr, const char *text, oglx_texture_t *img, float vx, float vy, float vwidth, float vheight )
//{
//    if ( NULL == pw ) return bfalse;
//
//    // use the default ui font?
//    if ( NULL == ttf_ptr )
//    {
//        ttf_ptr = ui_getFont();
//    }
//
//    pw->id            = id;
//    pw->vx            = vx;
//    pw->vy            = vy;
//    pw->vwidth        = vwidth;
//    pw->vheight       = vheight;
//    pw->latch_state   = 0;
//    pw->timeout       = 0;
//    pw->latch_mask    = 0;
//    pw->display_mask  = UI_DISPLAY_BUTTON;
//
//    // set any image
//    ui_Widget::set_img( pw, ui_just_centered, img );
//
//    // set any text
//    ui_Widget::set_text( pw, ui_just_topcenter, ttf_ptr, text );
//
//    return btrue;
//}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::LatchMask_Add( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    ADD_BITS( pw->latch_mask, mbits );
    REMOVE_BITS( pw->latch_state, mbits );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::LatchMask_Remove( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    REMOVE_BITS( pw->latch_mask, mbits );
    REMOVE_BITS( pw->latch_state, mbits );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::LatchMask_Set( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    pw->latch_mask  = mbits;
    REMOVE_BITS( pw->latch_state, mbits );

    return btrue;
}

//--------------------------------------------------------------------------------------------
BIT_FIELD ui_Widget::LatchMask_Test( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    return pw->latch_mask & mbits;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::DisplayMask_Add( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    ADD_BITS( pw->display_mask, mbits );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::DisplayMask_Remove( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    REMOVE_BITS( pw->display_mask, mbits );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::DisplayMask_Set( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    pw->display_mask  = mbits;

    return btrue;
}

//--------------------------------------------------------------------------------------------
BIT_FIELD ui_Widget::DisplayMask_Test( ui_Widget * pw, BIT_FIELD mbits )
{
    if ( NULL == pw ) return bfalse;

    return pw->display_mask & mbits;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::update_bound( ui_Widget * pw, frect_t * pbound )
{
    int   cnt_w = 0;
    float img_w, img_h;
    float txt_w, txt_h;
    float but_w, but_h;

    if ( NULL == pw || NULL == pbound ) return bfalse;

    // grab the existing image pbound
    img_w = 0.0f;
    img_h = 0.0f;
    if ( ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_IMAGE ) && NULL != pw->img )
    {
        img_w = pw->img->imgW;
        img_h = pw->img->imgH;

        cnt_w++;
    }

    // grab the existing text pbound
    txt_w = 0.0f;
    txt_h = 0.0f;
    if ( ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_TEXT ) && NULL != pw->tx_lst )
    {
        frect_t tmp;

        display_list_pbound( pw->tx_lst, &tmp );

        txt_w = tmp.w;
        txt_h = tmp.h;

        cnt_w++;
    }

    // grab the existing button bound, if there is nothing else in the widget
    but_w = 30.0f;
    but_h = 30.0f;
    if ( 0 == cnt_w && ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_BUTTON ) )
    {
        but_w = MAX( but_w, pw->vwidth );
        but_h = MAX( but_h, pw->vheight );
    }

    // set the position
    pbound->x = pw->vx;
    pbound->y = pw->vy;

    // find the basic size
    if ( 0 == cnt_w )
    {
        // just copy the button size
        pbound->w = but_w;
        pbound->h = but_h;
    }
    else
    {
        // place a 5 pix border around the objects and a 5 pixel strip between
        pbound->w = 10 + img_w + txt_w + ( cnt_w - 1 ) * 5;
        pbound->h = 10 + img_h + txt_h;

        // make sure that it bigger than the default size
        pbound->w = MAX( but_w, pbound->w );
        pbound->h = MAX( but_h, pbound->h );
    }

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_bound( ui_Widget * pw, float x, float y, float w, float h )
{
    /// @details BB@> render the text string to a SDL_Surface

    frect_t size = {0, 0, 0, 0};

    if ( NULL == pw ) return bfalse;

    // set the basic size
    size.x = x;
    size.y = y;
    size.w = w;
    size.h = h;

    // get the "default" size
    if ( w <= 0 || h <= 0 )
    {
        frect_t tmp;

        ui_Widget::update_bound( pw, &tmp );

        if ( w <= 0 ) size.w = tmp.w;
        if ( h <= 0 ) size.h = tmp.h;
    }

    // store the button position
    pw->vx      = size.x;
    pw->vy      = size.y;
    pw->vwidth  = MAX( 30, size.w );
    pw->vheight = MAX( 30, size.h );

    // update the text position
    ui_Widget::update_text_pos( pw );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_button( ui_Widget * pw, float x, float y, float w, float h )
{
    bool_t rv;

    rv = bfalse;
    if ( ui_Widget::set_bound( pw, x, y, w, h ) )
    {
        // tell the button to display
        ui_Widget::DisplayMask_Add( pw, UI_DISPLAY_BUTTON );

        rv = btrue;
    }

    return rv;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_pos( ui_Widget * pw, float x, float y )
{
    float  dx, dy;

    if ( NULL == pw ) return bfalse;

    dx = x - pw->vx;
    dy = y - pw->vy;

    pw->vx = x;
    pw->vy = y;

    return GL_TRUE == display_list_adjust_bound( pw->tx_lst, dx, dy );
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_id( ui_Widget * pw, ui_id_t id )
{
    /// @details BB@>

    if ( NULL == pw ) return bfalse;

    pw->id = id;

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_img( ui_Widget * pw, const ego_ui_Just & just, oglx_texture_t *img, bool_t own )
{
    if ( NULL == pw ) return bfalse;

    // tell the widget not to dosplay an image
    ui_Widget::DisplayMask_Remove( pw, UI_DISPLAY_IMAGE );

    if ( NULL == img ) return btrue;

    // set the image
    pw->img      = img;
    pw->img_just = just;
    pw->img_own  = own;

    // tell the ui to display the image
    ui_Widget::DisplayMask_Add( pw, UI_DISPLAY_IMAGE );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::update_text_pos( ui_Widget * pw )
{
    int   just;
    float diff, new_x, new_y;
    float x1, x2, y1, y2;
    frect_t tx_bound, w_bound, w_vbound;

    if ( NULL == pw ) return bfalse;

    // if there is no justification info, just return
    if ( UI_JUST_NON == pw->tx_just.horz && UI_JUST_NON == pw->tx_just.vert ) return btrue;

    // get the widget bound in virtual coordinates
    w_vbound.x = pw->vx;
    w_vbound.y = pw->vy;
    w_vbound.w = pw->vwidth;
    w_vbound.h = pw->vheight;

    // adjust the available region so that it is to the left of any image
    if ( 0 != ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_IMAGE ) && NULL != pw->img )
    {
        w_vbound.x += 5 + pw->img->imgW;
        w_vbound.w -= 5 + pw->img->imgW;
    }

    // shrink this region to make a 5 pixel border
    if ( 0 != ui_Widget::DisplayMask_Test( pw, UI_DISPLAY_IMAGE ) && NULL != pw->img )
    {
        w_vbound.x +=  5;
        w_vbound.w -= 10;
        w_vbound.y +=  5;
        w_vbound.h -= 10;
    }

    // get the screen coordinates of the button
    ui_virtual_to_screen_abs( w_vbound.x, w_vbound.y, &x1, &y1 );
    ui_virtual_to_screen_abs( w_vbound.x + w_vbound.w, w_vbound.y + w_vbound.h, &x2, &y2 );

    // get the widget bound in screen coordinates
    w_bound.x = x1;
    w_bound.y = y1;
    w_bound.w = x2 - x1;
    w_bound.h = y2 - y1;

    // get the button bound in screen coordinates
    display_list_pbound( pw->tx_lst, &tx_bound );

    //---- do the x coordinate

    // make an alias of this
    just = pw->tx_just.horz;

    // test whether the text will fit inside the allowed region
    diff = w_bound.w - tx_bound.w;

    // if not, default to left justified
    if ( diff < 0 ) just = UI_JUST_LOW;

    switch ( just )
    {
        default:
        case UI_JUST_NON:
        case UI_JUST_LOW:
            new_x = w_bound.x;
            break;

        case UI_JUST_MID:
            new_x = w_bound.x + diff * 0.5f;
            break;

        case UI_JUST_HGH:
            new_x = ( w_bound.x + w_bound.w ) - tx_bound.w;
            break;
    }

    new_x = MAX( new_x, pw->vx );

    //---- do the y coordinate

    // make an alias of this
    just = pw->tx_just.vert;

    // test whether the text will fit inside the allowed region
    diff = w_bound.h - tx_bound.h;

    // if not, default to top justified
    if ( diff < 0 ) just = UI_JUST_LOW;

    switch ( just )
    {
        default:
        case UI_JUST_NON:
        case UI_JUST_LOW:
            new_y = w_bound.y;
            break;

        case UI_JUST_MID:
            new_y = w_bound.y + diff * 0.5f;
            break;

        case UI_JUST_HGH:
            new_y = ( w_bound.y + w_bound.h ) - tx_bound.h;
            break;
    }

    new_y = MAX( new_y, pw->vy );

    display_list_adjust_bound( pw->tx_lst, new_x - tx_bound.x, new_y - tx_bound.y );

    return btrue;
}

//--------------------------------------------------------------------------------------------
ui_Widget * ui_Widget::clear( ui_Widget * pw )
{
    if ( NULL == pw ) return pw;

    // blank the data
    memset( pw, 0, sizeof( *pw ) );

    // set the id (probably UI_Nothing)
    pw->id = UI_Nothing;
    pw->on = btrue;

    return pw;
}

//--------------------------------------------------------------------------------------------
ui_Widget * ui_Widget::ctor_this( ui_Widget * pw, ui_id_t id )
{
    // set the id (probably UI_Nothing)
    pw->id = id;

    // assume it is a button
    pw->display_mask  = ( UI_Nothing != id ) ? UI_DISPLAY_BUTTON : 0;

    return pw;
}

//--------------------------------------------------------------------------------------------
ui_Widget * ui_Widget::dtor_this( ui_Widget * pw )
{
    if ( NULL == pw ) return pw;

    dealloc( pw );

    return ui_Widget::clear( pw );
}

//--------------------------------------------------------------------------------------------
ui_Widget * ui_Widget::reset( ui_Widget * pw, ui_id_t id )
{
    if ( NULL == pw ) return pw;

    ui_Widget::dtor_this( pw );
    ui_Widget::ctor_this( pw, id );

    return pw;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::dealloc( ui_Widget * pw )
{
    if ( NULL == pw ) return bfalse;

    if ( NULL != pw->tx_lst && pw->tx_own )
    {
        pw->tx_lst = display_list_dtor( pw->tx_lst, btrue );
        pw->tx_own = bfalse;
    }

    if ( NULL != pw->img && pw->img_own )
    {
        oglx_texture_Release( pw->img );
        pw->img     = NULL;
        pw->img_own = bfalse;
    }

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_vtext( ui_Widget * pw, const ego_ui_Just just, TTF_Font * ttf_ptr, const char * format, va_list args )
{
    int lines = 0;

    if ( NULL == pw ) return bfalse;

    // use the default ui font?
    if ( NULL == ttf_ptr )
    {
        ttf_ptr = ui_getFont();
    }

    // remove any existing text data
    pw->tx_lst = display_list_dtor( pw->tx_lst, btrue );

    // tell the widget not to dosplay text
    ui_Widget::DisplayMask_Remove( pw, UI_DISPLAY_TEXT );
    pw->tx_just.horz = UI_JUST_NON;
    pw->tx_just.vert = UI_JUST_NON;

    // Create the image and position it on the right hand side of any image
    if ( NULL != format && '\0' != format[0] )
    {
        float x1, y1;

        // convert the virtual coordinates to screen coordinates
        ui_virtual_to_screen_abs( pw->vx, pw->vy, &x1, &y1 );

        // ensure that we hae a display list
        pw->tx_lst = display_list_ctor( pw->tx_lst, MAX_WIDGET_TEXT );

        // actually convert the text_ptr ( use the text_ptr returned from fnt_vgetTextBoxSize() )
        GL_DEBUG( glColor3f )( 1, 1, 1 );
        lines = fnt_vconvertTextBox( pw->tx_lst, ttf_ptr, x1, y1, 20, format, args );

        // set the justification
        pw->tx_just = just;

        // adjust the text position
        ui_Widget::update_text_pos( pw );
    }

    // set the visibility of this component
    if ( lines > 0 )
    {
        ui_Widget::DisplayMask_Add( pw, UI_DISPLAY_TEXT );
    }

    return lines > 0;
}

//--------------------------------------------------------------------------------------------
bool_t ui_Widget::set_text( ui_Widget * pw, const ego_ui_Just & just, TTF_Font * ttf_ptr, const char * format, ... )
{
    /// @details BB@> render the text string to a ogl texture

    va_list args;
    bool_t retval;

    // turn the old text off
    DisplayMask_Remove( pw, UI_DISPLAY_TEXT );

    // use the default ui font?
    if ( NULL == ttf_ptr )
    {
        ttf_ptr = ui_getFont();
    }

    va_start( args, format );
    retval = ui_Widget::set_vtext( pw, just, ttf_ptr, format, args );
    va_end( args );

    ui_Widget::update_text_pos( pw );

    // we own this text
    pw->tx_own = btrue;

    // turn any new text on
    if ( NULL != pw->tx_lst )
    {
        DisplayMask_Add( pw, UI_DISPLAY_TEXT );
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ui_virtual_to_screen_abs( float vx, float vy, float * rx, float * ry )
{
    /// @details BB@> convert "virtual" screen positions into "real" space

    *rx = ui_context.aw * vx + ui_context.bw;
    *ry = ui_context.ah * vy + ui_context.bh;
}

//--------------------------------------------------------------------------------------------
void ui_screen_to_virtual_abs( float rx, float ry, float *vx, float *vy )
{
    /// @details BB@> convert "real" mouse positions into "virtual" space

    *vx = ui_context.iaw * rx + ui_context.ibw;
    *vy = ui_context.iah * ry + ui_context.ibh;
}

//--------------------------------------------------------------------------------------------
void ui_virtual_to_screen_rel( float vx, float vy, float * rx, float * ry )
{
    /// @details BB@> convert "virtual" screen positions into "real" space

    *rx = ui_context.aw * vx;
    *ry = ui_context.ah * vy;
}

//--------------------------------------------------------------------------------------------
void ui_screen_to_virtual_rel( float rx, float ry, float *vx, float *vy )
{
    /// @details BB@> convert "real" mouse positions into "virtual" space

    *vx = ui_context.iaw * rx;
    *vy = ui_context.iah * ry;
}

//--------------------------------------------------------------------------------------------
void ui_set_virtual_screen( float vw, float vh, float ww, float wh )
{
    /// @details BB@> set up the ui's virtual screen

    float k;
    TTF_Font * old_defaultFont;

    // define the virtual screen
    ui_context.vw = vw;
    ui_context.vh = vh;
    ui_context.ww = ww;
    ui_context.wh = wh;

    // define the forward transform
    k = MIN( sdl_scr.x / ww, sdl_scr.y / wh );
    ui_context.aw = k;
    ui_context.ah = k;
    ui_context.bw = ( sdl_scr.x - k * ww ) * 0.5f;
    ui_context.bh = ( sdl_scr.y - k * wh ) * 0.5f;

    // define the inverse transform
    ui_context.iaw = 1.0f / ui_context.aw;
    ui_context.iah = 1.0f / ui_context.ah;
    ui_context.ibw = -ui_context.bw * ui_context.iaw;
    ui_context.ibh = -ui_context.bh * ui_context.iah;

    // make sure the font is sized right for the virtual screen
    old_defaultFont = ui_context.defaultFont;
    if ( NULL != ui_context.defaultFont )
    {
        fnt_freeFont( ui_context.defaultFont );
    }
    ui_context.defaultFont = ui_loadFont( ui_context.defaultFontName, ui_context.defaultFontSize );

    // fix the active font. in general, we do not own it, so do not delete
    if ( NULL == ui_context.activeFont || old_defaultFont == ui_context.activeFont )
    {
        ui_context.activeFont = ui_context.defaultFont;
    }
}

//--------------------------------------------------------------------------------------------
TTF_Font * ui_loadFont( const char * font_name, float vpointSize )
{
    float pointSize;

    pointSize = vpointSize * ui_context.aw;

    return fnt_loadFont( font_name, pointSize );
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void ui_joy_init( UiContext * pctxt )
{
    int cnt;

    if ( NULL == pctxt ) return;

    for ( cnt = 0; cnt < UI_MAX_JOYSTICKS; cnt++ )
    {
        memset( pctxt->joys + cnt, 0, sizeof( ego_ui_control_info ) );
    }

    memset( &( pctxt->joy ), 0, sizeof( ego_ui_control_info ) );

    memset( &( pctxt->mouse ), 0, sizeof( ego_ui_control_info ) );
}

//--------------------------------------------------------------------------------------------
void ui_cursor_update()
{
    int   cnt;

    ego_ui_control_info * pctrl = NULL;

    // assume no one is in control
    pctrl = NULL;

    // find the best most controlling joystick
    for ( cnt = 0; cnt < UI_MAX_JOYSTICKS; cnt++ )
    {
        ego_ui_control_info * pinfo = ui_context.joys + cnt;

        if ( pinfo->timer <= 0 ) continue;

        if (( NULL == pctrl ) || ( pinfo->timer > pctrl->timer ) )
        {
            pctrl = pinfo;
        }
    }

    // update the ui_context.joy device
    if ( NULL != pctrl )
    {
        ui_context.joy.timer   = pctrl->timer;

        ui_context.joy.vrt[0] += pctrl->vrt[0];
        ui_context.joy.vrt[1] += pctrl->vrt[1];
        ui_virtual_to_screen_abs( ui_context.joy.vrt[0], ui_context.joy.vrt[1], &( ui_context.joy.scr[0] ), &( ui_context.joy.scr[1] ) );

        pctrl = &( ui_context.joy );
    }

    // find out whether the mouse or the joystick is the better controller
    if ( NULL == pctrl )
    {
        pctrl = &( ui_context.mouse );
    }
    else if ( ui_context.mouse.timer >  pctrl->timer )
    {
        pctrl = &( ui_context.mouse );
    }

    if ( NULL != pctrl )
    {
        ui_context.cursor_X = 0.5f * ui_context.cursor_X + 0.5f * pctrl->vrt[0];
        ui_context.cursor_Y = 0.5f * ui_context.cursor_Y + 0.5f * pctrl->vrt[1];
    }

    // decrement the joy timers
    for ( cnt = 0; cnt < UI_MAX_JOYSTICKS; cnt++ )
    {
        ego_ui_control_info * pinfo = ui_context.joys + cnt;

        if ( pinfo->timer > 0 )
        {
            pinfo->timer--;
        }
    }

    // decrement the mouse timer
    if ( ui_context.mouse.timer > 0 )
    {
        ui_context.mouse.timer--;
    }

    // decrement the joy timer
    if ( ui_context.joy.timer > 0 )
    {
        ui_context.joy.timer--;
    }

}

//--------------------------------------------------------------------------------------------
bool_t ui_joy_set( SDL_JoyAxisEvent * evt_ptr )
{
    const int   dead_zone = 0x8000 >> 4;
    const float sensitivity = 10.0f;

    ego_ui_control_info * pctrl   = NULL;
    bool_t          updated = bfalse;
    int             value   = 0;

    if ( NULL == evt_ptr || SDL_JOYAXISMOTION != evt_ptr->type ) return bfalse;
    value   = evt_ptr->value;

    // check the correct range of the events
    if ( evt_ptr->which >= UI_MAX_JOYSTICKS ) return btrue;
    pctrl = ui_context.joys + evt_ptr->which;

    updated = bfalse;
    if ( evt_ptr->axis < 2 )
    {
        float old_diff, new_diff;

        // make a dead zone
        if ( value > dead_zone ) value -= dead_zone;
        else if ( value < -dead_zone ) value += dead_zone;
        else value = 0;

        // update the info
        old_diff = pctrl->scr[evt_ptr->axis];
        new_diff = ( float )value / ( float )( 0x8000 - dead_zone ) * sensitivity;
        pctrl->scr[evt_ptr->axis] = new_diff;

        updated = ( old_diff != new_diff );
    }

    if ( updated )
    {
        ui_screen_to_virtual_rel( pctrl->scr[0], pctrl->scr[1], &( pctrl->vrt[0] ), &( pctrl->vrt[1] ) );

        pctrl->timer = UI_CONTROL_TIMER;
    }

    return ( NULL != pctrl ) && ( pctrl->timer > 0 );
}
