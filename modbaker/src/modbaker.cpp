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
//-   The main function
//---------------------------------------------------------------------
int main(int argc, char **argv)
{
	cout << "entering main" << endl;
	c_modbaker modbaker;

	string modname;

	modname = "advent.mod";

	// TODO: Read from argv
	if (argc >= 2)
	{
		modname = argv[1];
	}

	modbaker.init(modname);
	modbaker.main_loop();

	return 0;
}


//---------------------------------------------------------------------
//-   Constructor
//---------------------------------------------------------------------
c_modbaker::c_modbaker()
{
	this->done          = false;
	this->active        = true;

	// Reset the global objects
	g_config            = NULL;
	g_renderer          = NULL;
	g_input_handler     = NULL;
	g_mesh              = NULL;
//	g_frustum           = NULL;
	g_mouse_x           = 0;
	g_mouse_x           = 0;
}

/*
c_modbaker::~c_modbaker()
{
	delete input;
}
*/


//---------------------------------------------------------------------
//-   Init everything
//---------------------------------------------------------------------
void c_modbaker::init(string p_modname)
{
	g_config   = new c_config();

	// Load the renderer
	g_renderer = new c_renderer();
	g_renderer->load_basic_textures(p_modname);

	// Load the mesh
	g_mesh = new c_mesh();
	g_mesh->getTileDictioary().load();

	g_object_manager = new c_object_manager();
	load_module(p_modname);

	g_renderer->m_renderlist.build();

	// Set the module name
	// TODO: Remove the global modname variable
	g_mesh->modname = p_modname;
}


//---------------------------------------------------------------------
//-   The main loop
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
			g_renderer->render_models(g_object_manager); // Render spawn.txt
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