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

/// @file  egolib/Graphics/TextureManager.cpp
/// @brief the texture manager.

#include "egolib/_math.h"
#include "egolib/fileutil.h"
#include "egolib/Graphics/TextureManager.hpp"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

TextureManager::TextureManager() :
    _deferredLoadingMutex(),
    _requestedLoadDeferredTextures(),
    _notifyDeferredLoadingComplete()
{
    Ego::OpenGL::initializeErrorTextures();
}

TextureManager::~TextureManager()
{
    _textureCache.clear();
	_unload.clear();
    Ego::OpenGL::uninitializeErrorTextures();
}

void TextureManager::release_all()
{
	if (SDL_GL_GetCurrentContext() != nullptr) {
		// We are the main OpenGL context thread so we can destroy textures.
		_textureCache.clear();
		_unload.clear();
	} else {
		// We are not the main OpenGL context thread so we can not destroy textures.
		for (auto it = std::begin(_textureCache); it != std::end(_textureCache);)
		{
			_unload.push_front(it->second);
			it = _textureCache.erase(it);
		}
	}
}

void TextureManager::reupload()
{
    // TODO
}

void TextureManager::updateDeferredLoading()
{
    //If nothing to do, exit function immeadiately
    if(_requestedLoadDeferredTextures.empty()) return;

    //Load each texture that is required by another thread
    {
        std::lock_guard<std::mutex> lock(_deferredLoadingMutex);
        for(const std::string &filePath : _requestedLoadDeferredTextures) {
            std::shared_ptr<Ego::Texture> loadTexture = std::make_shared<Ego::OpenGL::Texture>();
            ego_texture_load_vfs(loadTexture.get(), filePath.c_str(), TRANSCOLOR);
            _textureCache[filePath] = loadTexture;
			Log::get().debug("Deferred texture load: %s\n", filePath.c_str());
        }
        _requestedLoadDeferredTextures.clear();
    }

    //Notify all waiting threads that loading is complete
    _notifyDeferredLoadingComplete.notify_all();  
}

const std::shared_ptr<Ego::Texture>& TextureManager::getTexture(const std::string &filePath)
{
    //Not loaded yet?
    auto result = _textureCache.find(filePath);
    if(result == _textureCache.end()) {

        if(SDL_GL_GetCurrentContext() != nullptr) {
            //We are the main OpenGL context thread so we can load textures
            std::shared_ptr<Ego::Texture> loadTexture = std::make_shared<Ego::OpenGL::Texture>();
            ego_texture_load_vfs(loadTexture.get(), filePath.c_str(), TRANSCOLOR);
            _textureCache[filePath] = loadTexture;
        }
        else {
            //We cannot load textures, wait blocking for main thread to load it for us
            std::unique_lock<std::mutex> lock(_deferredLoadingMutex);
            _requestedLoadDeferredTextures.push_front(filePath);
			Log::get().debug("Wait for deferred texture: %s\n", filePath.c_str());
            _notifyDeferredLoadingComplete.wait(lock, [this]{return _requestedLoadDeferredTextures.empty();});
        }

        return _textureCache[filePath];
    }

    //Get cached texture
    else {
        return (*result).second;
    }
}
