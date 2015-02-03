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
/// @author Johan Jansen

#include "game/core/GameEngine.hpp"
#include "game/audio/AudioSystem.hpp"
#include "game/graphics/CameraSystem.hpp"
#include "game/gamestates/MainMenuState.hpp"
#include "game/profiles/ProfileSystem.hpp"
#include "game/graphic.h"
#include "game/renderer_2d.h"
#include "game/ui.h"
#include "game/game.h"
#include "game/collision.h"
#include "egolib/egolib.h"
#include "game/PrtList.h"

//Declaration of class constants
const uint32_t GameEngine::GAME_TARGET_FPS;
const uint32_t GameEngine::GAME_TARGET_UPS;

const uint32_t GameEngine::DELAY_PER_RENDER_FRAME;
const uint32_t GameEngine::DELAY_PER_UPDATE_FRAME;

GameEngine::GameEngine() :
	_isInitialized(false),
	_terminateRequested(false),
	_updateTimeout(0),
	_renderTimeout(0),
	_gameStateStack(),
	_currentGameState(nullptr),
	_config()
{
	//ctor
}

void GameEngine::shutdown()
{
	_terminateRequested = true;
}

void GameEngine::start()
{
	initialize();

	//Initialize clock timeout	
	_updateTimeout = SDL_GetTicks() + DELAY_PER_UPDATE_FRAME;
	_renderTimeout = SDL_GetTicks() + DELAY_PER_RENDER_FRAME;

	while(!_terminateRequested)
	{
		//Check if it is time to update everything
		uint32_t currentTick = SDL_GetTicks();
		if(currentTick >= _updateTimeout)
		{
			updateOneFrame();
			_updateTimeout = SDL_GetTicks() + DELAY_PER_UPDATE_FRAME + (SDL_GetTicks()-currentTick);
		}

		//Check if it is time to draw everything
		currentTick = SDL_GetTicks();
		if(currentTick >= _renderTimeout)
		{
			renderOneFrame();
			_renderTimeout = SDL_GetTicks() + DELAY_PER_RENDER_FRAME + (SDL_GetTicks()-currentTick);
		}

		//Don't hog CPU if we have nothing to do
		currentTick = SDL_GetTicks();
		uint32_t delay = std::min(_renderTimeout-currentTick, _updateTimeout-currentTick);
		if(delay > 1) {
			SDL_Delay(delay);
		}

		// Test the panic button
	    if ( SDL_KEYDOWN( keyb, SDLK_q ) && SDL_KEYDOWN( keyb, SDLK_LCTRL ) )
	    {
	        //terminate the program
	        shutdown();
	    }
	}

	uninitialize();
}

void GameEngine::updateOneFrame()
{
	//Fall through to next state if needed
    if(_currentGameState->isEnded())
    {
        _gameStateStack.pop_front();

        //No more states? Default back to main menu
        if(_gameStateStack.empty())
        {
            pushGameState(std::make_shared<MainMenuState>());
        }
        else
        {
            _currentGameState = _gameStateStack.front();
            _currentGameState->beginState();
        }
    }
    
    // read the input values
    input_read_all_devices();

	_currentGameState->update();
}

void GameEngine::renderOneFrame()
{
    // clear the screen
    gfx_request_clear_screen();
	gfx_do_clear_screen();

	_currentGameState->drawAll();

	// flip the graphics page
    gfx_request_flip_pages();
	gfx_do_flip_pages();
}

bool GameEngine::initialize()
{
	//Initialize logging next, so that we can use it everywhere.
    log_init("/debug/log.txt", LOG_DEBUG);

    // start initializing the various subsystems
    log_message("Starting Egoboo " VERSION " ...\n");
    log_info("PhysFS file system version %s has been initialized...\n", vfs_getVersion());

    //Initialize OS specific stuff
    sys_initialize();

    // read the "setup.txt" file
    setup_read_vfs();

    // download the "setup.txt" values into the cfg struct
    loadConfiguration(true);

    //Initialize SDL
    log_info( "Initializing SDL version %d.%d.%d... ", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL );
    if ( SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD) < 0 )
    {
        log_message( "Failure!\n" );
        log_error( "Unable to initialize SDL: %s\n", SDL_GetError() );
    }
    else
    {
        log_message( "Success!\n" );
    }

    // do basic system initialization
    input_system_init();

    //Initialize graphics
    log_info("Initializing SDL_Image version %d.%d.%d... ", SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL);
    gfx_system_begin();
    GLSetup_SupportedFormats();
    gfx_system_init_all_graphics();
    gfx_do_clear_screen();
    gfx_do_flip_pages();

	// synchronize the config values with the various game subsystems
    // do this after the ego_init_SDL() and gfx_system_init_OpenGL() in case the config values are clamped
    // to valid values
    loadConfiguration(true);

    // read all the scantags
    scantag_read_all_vfs("mp_data/scancode.txt");

    // load input
    input_settings_load_vfs("/controls.txt", -1);

    // initialize the random treasure system
    init_random_treasure_tables_vfs("mp_data/randomtreasure.txt");

    // initialize the console
    egolib_console_begin();

    // initialize the sound system
    _audioSystem.initialize(cfg);
    _audioSystem.loadAllMusic();
    _audioSystem.playMusic(AudioSystem::MENU_SONG);

    // make sure that a bunch of stuff gets initialized properly
    particle_system_begin();
    enchant_system_begin();
    model_system_begin();
    ego_mesh_ctor(PMesh);
    _profileSystem.begin();

    // setup the system gui
    ui_begin( "mp_data/Bo_Chen.ttf", 24 );

    // clear out the import and remote directories
    vfs_empty_temp_directories();

    //Start the main menu
    pushGameState(std::make_shared<MainMenuState>());

    return true;
}

void GameEngine::uninitialize()
{
    log_info( "memory_cleanUp() - Attempting to clean up loaded things in memory... " );

    // synchronize the config values with the various game subsystems
    config_synch(&cfg, true);

    // quit the setup system, making sure that the setup file is written
    setup_write_vfs();
    setup_end();

    // delete all the graphics allocated by SDL and OpenGL
    gfx_system_delete_all_graphics();

    // make sure that the current control configuration is written
    input_settings_save_vfs( "controls.txt", -1 );

	//shut down the ui
    ui_end();

	// deallocate any dynamically allocated collision memory
    collision_system_end();

    // deallocate any dynamically allocated scripting memory
    scripting_system_end();

    // deallocate all dynamically allocated memory for characters, particles, enchants, and models
    particle_system_end();
    enchant_system_end();
    _gameObjects.clear();
    model_system_end();

    // shut down the log services
    log_message( "Success!\n" );
    log_info( "Exiting Egoboo " VERSION " the good way...\n" );
    log_shutdown();

    //Shutdown SDL last
	SDL_Quit();
}

void GameEngine::setGameState(std::shared_ptr<GameState> gameState)
{
    _gameStateStack.clear();
    pushGameState(gameState);
}

void GameEngine::pushGameState(std::shared_ptr<GameState> gameState)
{
    _gameStateStack.push_front(gameState);
    _currentGameState = _gameStateStack.front();
    _currentGameState->beginState();
}

bool GameEngine::loadConfiguration(bool syncFromFile)
{
    size_t tmp_maxparticles;

    // synchronize settings from a pre-loaded setup.txt? (this will load setup.txt into *pcfg)
    if (syncFromFile)
    {
        if ( !setup_download(&cfg) ) return false;
    }

    // status display
    StatusList.on = cfg.show_stats;

    // fps display
    fpson = cfg.fps_allowed;

    // message display
    DisplayMsg_count = Math::constrain(cfg.message_count_req, EGO_MESSAGE_MIN, EGO_MESSAGE_MAX);
    DisplayMsg_on    = cfg.message_count_req > 0;
    wraptolerance 	 = cfg.show_stats ? 90 : 32;

    // Get the particle limit
    // if the particle limit has changed, make sure to make not of it
    // number of particles
    tmp_maxparticles = Math::constrain<uint16_t>(cfg.particle_count_req, 256, MAX_PRT);
    if (maxparticles != tmp_maxparticles)
    {
        maxparticles = tmp_maxparticles;
        maxparticles_dirty = true;
    }

    // camera options
    _cameraSystem.getCameraOptions().turnMode = cfg.autoturncamera;

    // sound options
    _audioSystem.setConfiguration(cfg);

    // renderer options
    gfx_config_download_from_egoboo_config(&gfx, &cfg);

    // texture options
    oglx_texture_parameters_download_gfx(&tex_params, &cfg);

    return true;
}


int SDL_main(int argc, char **argv)
{
    /// @details This is where the program starts and all the high level stuff happens

    // initialize the virtual filesystem first
    vfs_init(argv[0]);
    setup_init_base_vfs_paths();

    std::unique_ptr<GameEngine> gameEngine = std::unique_ptr<GameEngine>(new GameEngine());

    gameEngine->start();

    return EXIT_SUCCESS;
}
