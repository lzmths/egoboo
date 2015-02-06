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

/// @file game/gamestates/MainMenuState.cpp
/// @details The Main Menu of the game, the first screen presented to the players
/// @author Johan Jansen

#include "game/gamestates/MainMenuState.hpp"
#include "game/gamestates/SelectModuleState.hpp"
#include "game/gamestates/SelectPlayersState.hpp"
#include "game/core/GameEngine.hpp"
#include "game/audio/AudioSystem.hpp"
#include "egolib/platform.h"
#include "game/ui.h"
#include "game/menu.h"
#include "game/game.h"
#include "game/gui/Button.hpp"
#include "game/gui/Image.hpp"
#include "game/gui/Label.hpp"

MainMenuState::MainMenuState() :
	_slidyButtons()
{
	std::shared_ptr<Image> background;
	std::shared_ptr<Image> gameLogo;

	//Special xmas theme
	if (check_time(SEASON_CHRISTMAS))
	{
	    background = std::make_shared<Image>("mp_data/menu/menu_xmas");
	    gameLogo = std::make_shared<Image>("mp_data/menu/snowy_logo");
	}

	//Special Halloween theme
	else if (check_time(SEASON_HALLOWEEN))
	{
	    background = std::make_shared<Image>("mp_data/menu/menu_halloween");
	    gameLogo = std::make_shared<Image>("mp_data/menu/creepy_logo");
	}

	//Default egoboo theme
	else
	{
	    background = std::make_shared<Image>("mp_data/menu/menu_main");
	    gameLogo = std::make_shared<Image>("mp_data/menu/menu_logo");
	}

	// calculate the centered position of the background
	float fminw = std::min<float>(GFX_WIDTH, background->getTextureWidth()) / static_cast<float>(background->getTextureWidth());
	float fminh = std::min<float>(GFX_HEIGHT, background->getTextureHeight()) / static_cast<float>(background->getTextureWidth());
	float fminb  = std::min(fminw, fminh);
	background->setSize(background->getTextureWidth() * fminb, background->getTextureHeight() * fminb);
	background->setPosition((GFX_WIDTH  - background->getWidth()) * 0.5f, (GFX_HEIGHT - background->getHeight()) * 0.5f);
	addComponent(background);

	// calculate the position of the logo
	fminb = std::min(background->getWidth() * 0.5f / gameLogo->getTextureWidth(), background->getHeight() * 0.5f / gameLogo->getTextureHeight());
	gameLogo->setPosition(background->getX(), background->getY());
	gameLogo->setSize(gameLogo->getTextureWidth() * fminb, gameLogo->getTextureHeight() * fminb);
	addComponent(gameLogo);

	//Add the buttons
	int yOffset = GFX_HEIGHT-80;
	std::shared_ptr<Button> exitButton = std::make_shared<Button>("Exit Game", SDLK_ESCAPE);
	exitButton->setPosition(20, yOffset);
	exitButton->setSize(200, 30);
	exitButton->setOnClickFunction(
	[]{
		_gameEngine->shutdown();
	});
	addComponent(exitButton);
	_slidyButtons.push_front(exitButton);

	yOffset -= exitButton->getHeight() + 10;

	std::shared_ptr<Button> optionsButton = std::make_shared<Button>("Options", SDLK_o);
	optionsButton->setPosition(20, yOffset);
	optionsButton->setSize(200, 30);
	addComponent(optionsButton);
	_slidyButtons.push_front(optionsButton);

	yOffset -= optionsButton->getHeight() + 10;

	std::shared_ptr<Button> loadGameButton = std::make_shared<Button>("Load Game", SDLK_l);
	loadGameButton->setPosition(20, yOffset);
	loadGameButton->setSize(200, 30);
	loadGameButton->setOnClickFunction(
	[]{
		_gameEngine->pushGameState(std::make_shared<SelectPlayersState>());
	});
	addComponent(loadGameButton);
	_slidyButtons.push_front(loadGameButton);

	yOffset -= loadGameButton->getHeight() + 10;

	std::shared_ptr<Button> newGameButton = std::make_shared<Button>("New Game", SDLK_n);
	newGameButton->setPosition(20, yOffset);
	newGameButton->setSize(200, 30);
	newGameButton->setOnClickFunction(
	[]{
		_gameEngine->pushGameState(std::make_shared<SelectModuleState>());
	});
	addComponent(newGameButton);
	_slidyButtons.push_front(newGameButton);

	yOffset -= newGameButton->getHeight() + 10;

	//Add version label and copyright text
	std::shared_ptr<Label> welcomeLabel = std::make_shared<Label>("Welcome to Egoboo!");
	welcomeLabel->setPosition(exitButton->getX() + exitButton->getWidth() + 40,
		GFX_HEIGHT - 80 - welcomeLabel->getHeight());
	addComponent(welcomeLabel);

	std::shared_ptr<Label> websiteText = std::make_shared<Label>("http://egoboo.sourceforge.net");
	websiteText->setPosition(welcomeLabel->getX(), welcomeLabel->getY() + welcomeLabel->getHeight());
	addComponent(websiteText);

	std::shared_ptr<Label> versionText = std::make_shared<Label>("Version 2.9.0");
	versionText->setPosition(websiteText->getX(), websiteText->getY() + websiteText->getHeight());
	addComponent(versionText);


}

void MainMenuState::update()
{
}

void MainMenuState::drawContainer()
{
	ui_beginFrame(0);
	{
	    draw_mouse_cursor();
	}
	ui_endFrame();
}

void MainMenuState::beginState()
{
	// menu settings
    SDL_WM_GrabInput( SDL_GRAB_OFF );

    _audioSystem.playMusic(AudioSystem::MENU_SONG);

    float offset = 0;
    for(const std::shared_ptr<Button> &button : _slidyButtons)
    {
		button->beginSlidyButtonEffect(button->getWidth() + offset);
		offset += 20;
    }
}