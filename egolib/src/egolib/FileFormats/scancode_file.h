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

/// @file egolib/FileFormats/scancode_file.h
/// @details routines for reading and writing the file "scancode.txt"

#pragma once

#include "egolib/typedef.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

    struct control_t;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

/// A mapping between the state of an input device and an internal game latch
    struct scantag_t
    {
        const char *name;      ///< tag name
        uint32_t value;        ///< tag value
    };

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
    size_t scantag_get_count( void );
    int scantag_find_index( const char *string );
    const scantag_t  *scantag_get_tag( int index );
	bool scantag_get_value(int index, Uint32 * pvalue);
    const char *scantag_get_name( int index );

    const char *scantag_get_string( int device, const control_t &control, char * buffer, size_t buffer_size );
    void scantag_parse_control( const char * tag_string, control_t &control );

    Uint32 scancode_get_kmod( Uint32 scancode );

    const scantag_t *scantag_find_bits( const scantag_t * ptag_src, char device_char, Uint32 tag_bits );
    const scantag_t *scantag_find_value( const scantag_t * ptag_src, char device_char, Uint32 tag_value );
