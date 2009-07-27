//---------------------------------------------------------------------
// ModBaker - a module editor for Egoboo
//---------------------------------------------------------------------
// Copyright (C) 2009 Tobias Gall
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------

/// @file
/// @brief ModBaker implementation
/// @details Implementation of the logic between all core elements


#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_types.h>
#include <SDL_endian.h>

#include "general.h"
#include "modbaker.h"
#include "global.h"

#include <iostream>
#include <stdlib.h>
#include <string>

using namespace std;

//---------------------------------------------------------------------
///   The main function
///   \param argc the parameters
///   \param argv number of parameters
///   \return error code or 0 on success
//---------------------------------------------------------------------
int main(int argc, char **argv)
{
	cout << "entering main" << endl;
	c_modbaker modbaker;

	string modname;

	modname = "advent.mod";

	// Read the module name from the command line
	if (argc >= 2)
	{
		modname = argv[1];
	}

	modbaker.init(modname);
	modbaker.main_loop();

	return 0;
}


//---------------------------------------------------------------------
///   Class constructor
//---------------------------------------------------------------------
c_modbaker::c_modbaker()
{
	this->done          = false;
	this->active        = true;

	// Reset the global objects
	g_config            = NULL;
	g_renderer          = NULL;
	g_input_handler     = NULL;
//	g_module              = NULL;
//	g_frustum           = NULL;
	g_mouse_x           = 0;
	g_mouse_x           = 0;
}


//---------------------------------------------------------------------
///   Class destructor
//---------------------------------------------------------------------
c_modbaker::~c_modbaker()
{
}


//---------------------------------------------------------------------
///   Initializes everything
///   \param p_modname module name to start with
//---------------------------------------------------------------------
void c_modbaker::init(string p_modname)
{
	g_config   = new c_config();

	// Load the renderer
	g_renderer = new c_renderer();
	g_renderer->load_basic_textures(p_modname);

	// Create a new module
	g_module = new c_module();

	g_module->load_module(p_modname);

	g_renderer->m_renderlist.build();

	// Set the module name
	g_module->modname = p_modname;
}


//---------------------------------------------------------------------
///   Main loop
//---------------------------------------------------------------------
void c_modbaker::main_loop()
{
	g_renderer->getPCam()->reset();

	while ( !done )
	{
		if ( active )
		{
			g_renderer->begin_frame();

			// 3D mode
			g_renderer->begin_3D_mode();
			g_renderer->getPCam()->move();
			g_renderer->render_mesh();
			g_renderer->render_positions();
			g_renderer->render_models(g_module); // Render spawn.txt
			get_GL_pos(g_mouse_x, g_mouse_y);
			g_renderer->end_3D_mode();

			// 2D mode
			g_renderer->begin_2D_mode();
			g_renderer->end_2D_mode();

			g_renderer->end_frame();  // Finally render the scene
		}

		handle_events();
	}
}


//---------------------------------------------------------------------
///   Load a module
///   \param p_modname the module name to load
///   \return true on success
//---------------------------------------------------------------------
bool c_module::load_module(string p_modname)
{
	string filename;

	filename = g_config->get_egoboo_path() + "modules/" + p_modname + "/gamedat/level.mpd";

	this->getTileDictioary().load();
	this->load_mesh_mpd(p_modname);

	// Read the object and spawn.txt information
	this->load_all_for_module(g_config->get_egoboo_path(), p_modname);
	this->load_menu_txt(g_config->get_egoboo_path() + "modules/" + p_modname + "/gamedat/menu.txt");

	// Reset module specific windows
	g_renderer->get_wm()->destroy_object_window();
	g_renderer->get_wm()->create_object_window(p_modname);
	g_renderer->get_wm()->destroy_texture_window();
	g_renderer->get_wm()->create_texture_window(p_modname);

	return false;
}


//---------------------------------------------------------------------
///   Save a module
///   \param p_modname the module name to save
///   \return true on success
//---------------------------------------------------------------------
bool c_module::save_module(string p_modname)
{
	return false;
}


//---------------------------------------------------------------------
///   Create a new module
///   \param p_modname the module name to create
///   \return true on success
//---------------------------------------------------------------------
bool c_module::new_module(string p_modname)
{
	return false;
}
