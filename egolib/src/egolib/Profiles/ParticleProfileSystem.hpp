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
#if !defined(EGOLIB_PROFILES_PRIVATE) || EGOLIB_PROFILES_PRIVATE != 1
#error(do not include directly, include `egolib/profiles/_Include.hpp` instead)
#endif

#include "egolib/Profiles/_AbstractProfileSystem.hpp"
#include "egolib/Profiles/ParticleProfile.hpp"
#include "egolib/Profiles/ParticleProfileReader.hpp"

class ParticleProfileSystem : public AbstractProfileSystem<pip_t, PIP_REF, INVALID_PIP_REF, MAX_PIP, ParticleProfileReader>, public Ego::Core::Singleton<ParticleProfileSystem>
{
protected:

    // Befriend with singleton to grant access to ParticleProfileSystem::ParticleProfileSystem and ParticleProfileSystem::~ParticleProfileSystem.
    using TheSingleton = Ego::Core::Singleton<ParticleProfileSystem>;
    friend TheSingleton;

    ParticleProfileSystem();
    ~ParticleProfileSystem();

public:
    //Explicit member to avoid ambiguous inheritance with Singleton
    void initialize() { AbstractProfileSystem<pip_t, PIP_REF, INVALID_PIP_REF, MAX_PIP, ParticleProfileReader>::initialize(); }
    
    //Explicit member to avoid ambiguous inheritance with Singleton
    void unintialize() { AbstractProfileSystem<pip_t, PIP_REF, INVALID_PIP_REF, MAX_PIP, ParticleProfileReader>::unintialize(); }
};

inline bool LOADED_PIP(PIP_REF ref) {
    return ParticleProfileSystem::get().isLoaded(ref);
}
