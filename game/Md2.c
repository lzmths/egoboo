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

#include "Md2.h"
#include "egoboo.h"
#include "egoboo_endian.h"

#include "log.h"                //For error logging

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
float kMd2Normals[][3] =
{
#include "id_normals.c"
    ,
    {0, 0, 0}  // Spiky Mace
};

// TEMPORARY: Global list of Md2Models.  It's declared in egoboo.h, which
// is why I have to include it here at the moment.
struct Md2Model *md2_models[MAXMODEL];

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void md2_freeModel( Md2Model *model )
{
    if ( model != NULL )
    {
        if ( model->texCoords != NULL ) free( model->texCoords );
        if ( model->triangles != NULL ) free( model->triangles );
        if ( model->skins != NULL ) free( model->skins );
        if ( model->frames != NULL )
        {
            int i;

            for ( i = 0; i < model->numFrames; i++ )
            {
                if ( model->frames[i].vertices != NULL ) free( model->frames[i].vertices );
            }

            free( model->frames );
        }

        free( model );
    }
}

//--------------------------------------------------------------------------------------------
Md2Model* md2_loadFromFile( const char *filename )
{
    return NULL;
}

//---------------------------------------------------------------------------------------------
int rip_md2_header()
{
    // ZZ> This function makes sure an md2 is really an md2
    int iTmp;
    int* ipIntPointer;

    // Check the file type
    ipIntPointer = ( int* ) cLoadBuffer;

    iTmp = ENDIAN_INT32( ipIntPointer[0] );
    if ( iTmp != MD2START ) return bfalse;

    return btrue;
}

//---------------------------------------------------------------------------------------------
void fix_md2_normals( Uint16 modelindex )
{
    // ZZ> This function helps light not flicker so much
    int cnt, tnc;
    Uint16 indexofcurrent, indexofnext, indexofnextnext, indexofnextnextnext;
    Uint16 indexofnextnextnextnext;
    Uint32 frame;

    frame = madframestart[modelindex];
    cnt = 0;

    while ( cnt < madvertices[modelindex] )
    {
        tnc = 0;

        while ( tnc < madframes[modelindex] )
        {
            indexofcurrent = madvrta[frame][cnt];
            indexofnext = madvrta[frame+1][cnt];
            indexofnextnext = madvrta[frame+2][cnt];
            indexofnextnextnext = madvrta[frame+3][cnt];
            indexofnextnextnextnext = madvrta[frame+4][cnt];
            if ( indexofcurrent == indexofnextnext && indexofnext != indexofcurrent )
            {
                madvrta[frame+1][cnt] = indexofcurrent;
            }
            if ( indexofcurrent == indexofnextnextnext )
            {
                if ( indexofnext != indexofcurrent )
                {
                    madvrta[frame+1][cnt] = indexofcurrent;
                }
                if ( indexofnextnext != indexofcurrent )
                {
                    madvrta[frame+2][cnt] = indexofcurrent;
                }
            }
            if ( indexofcurrent == indexofnextnextnextnext )
            {
                if ( indexofnext != indexofcurrent )
                {
                    madvrta[frame+1][cnt] = indexofcurrent;
                }
                if ( indexofnextnext != indexofcurrent )
                {
                    madvrta[frame+2][cnt] = indexofcurrent;
                }
                if ( indexofnextnextnext != indexofcurrent )
                {
                    madvrta[frame+3][cnt] = indexofcurrent;
                }
            }

            tnc++;
        }

        cnt++;
    }
}

//---------------------------------------------------------------------------------------------
void rip_md2_commands( Uint16 modelindex )
{
    // ZZ> This function converts an md2's GL commands into our little command list thing
    int iTmp;
    float fTmpu, fTmpv;
    int iNumVertices;
    int tnc;
    bool_t command_error = bfalse, entry_error = bfalse;
    int    vertex_max = 0;

    // char* cpCharPointer = (char*) cLoadBuffer;
    int* ipIntPointer = ( int* ) cLoadBuffer;
    float* fpFloatPointer = ( float* ) cLoadBuffer;

    // Number of GL commands in the MD2
    int iCommandWords = ENDIAN_INT32( ipIntPointer[9] );

    // Offset (in DWORDS) from the start of the file to the gl command list.
    int iCommandOffset = ENDIAN_INT32( ipIntPointer[15] ) >> 2;

    // Read in each command
    // iCommandWords is the number of dwords in the command list.
    // iCommandCount is the number of GL commands
    int iCommandCount;
    int entry;
    int cnt;

    iCommandCount = 0;
    entry = 0;
    cnt = 0;
    while ( cnt < iCommandWords )
    {
        Uint32 command_type;

        iNumVertices = ENDIAN_INT32( ipIntPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
        if ( 0 == iNumVertices ) break;
        if ( iNumVertices < 0 )
        {
            // Fans start with a negative
            iNumVertices = -iNumVertices;
            command_type = GL_TRIANGLE_FAN;
        }
        else
        {
            // Strips start with a positive
            command_type = GL_TRIANGLE_STRIP;
        }

        command_error = (iCommandCount >= MAXCOMMAND);
        if (!command_error)
        {
            madcommandtype[modelindex][iCommandCount] = command_type;
            madcommandsize[modelindex][iCommandCount] = MIN(iNumVertices, MAXCOMMANDENTRIES);
        }

        // Read in vertices for each command
        entry_error = bfalse;
        for ( tnc = 0; tnc < iNumVertices; tnc++ )
        {
            fTmpu = ENDIAN_FLOAT( fpFloatPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
            fTmpv = ENDIAN_FLOAT( fpFloatPointer[iCommandOffset] );  iCommandOffset++;  cnt++;
            iTmp  = ENDIAN_INT32( ipIntPointer[iCommandOffset]   );  iCommandOffset++;  cnt++;

            entry_error = entry >= MAXCOMMANDENTRIES;
            if ( iTmp > vertex_max )
            {
                vertex_max = iTmp;
            }
            if ( iTmp > MAXVERTICES ) iTmp = MAXVERTICES - 1;
            if ( !command_error && !entry_error )
            {
                madcommandu[modelindex][entry]   = fTmpu - ( 0.5f / 64 ); // GL doesn't align correctly
                madcommandv[modelindex][entry]   = fTmpv - ( 0.5f / 64 ); // with D3D
                madcommandvrt[modelindex][entry] = iTmp;
            }

            entry++;
        }

        // count only fully valid commands
        if ( !entry_error )
        {
            iCommandCount++;
        }
    }

    if ( vertex_max >= MAXVERTICES )
    {
        log_warning("rip_md2_commands(\"%s\") - \n\tOpenGL command references vertices above preset maximum: %d of %d\n", globalparsename, vertex_max, MAXVERTICES );
    }
    if ( command_error )
    {
        log_warning("rip_md2_commands(\"%s\") - \n\tNumber of OpenGL commands exceeds preset maximum: %d of %d\n", globalparsename, iCommandCount, MAXCOMMAND );
    }
    if ( entry_error )
    {
        log_warning("rip_md2_commands(\"%s\") - \n\tNumber of OpenGL command entries exceeds preset maximum: %d of %d\n", globalparsename, entry, MAXCOMMAND );
    }

    madcommands[modelindex] = MIN(MAXCOMMAND, iCommandCount);
}

//---------------------------------------------------------------------------------------------
int rip_md2_frame_name( int frame )
{
    // ZZ> This function gets frame names from the load buffer, it returns
    //     btrue if the name in cFrameName[] is valid
    int iFrameOffset;
    int iNumVertices;
    int iNumFrames;
    int cnt;
    int* ipNamePointer;
    int* ipIntPointer;
    int foundname;

    // Jump to the Frames section of the md2 data
    ipNamePointer = ( int* ) cFrameName;
    ipIntPointer = ( int* ) cLoadBuffer;

    iNumVertices = ENDIAN_INT32( ipIntPointer[6] );
    iNumFrames   = ENDIAN_INT32( ipIntPointer[10] );
    iFrameOffset = ENDIAN_INT32( ipIntPointer[14] ) >> 2;

    // Chug through each frame
    foundname = bfalse;
    cnt = 0;

    while ( cnt < iNumFrames && !foundname )
    {
        iFrameOffset += 6;
        if ( cnt == frame )
        {
            ipNamePointer[0] = ipIntPointer[iFrameOffset]; iFrameOffset++;
            ipNamePointer[1] = ipIntPointer[iFrameOffset]; iFrameOffset++;
            ipNamePointer[2] = ipIntPointer[iFrameOffset]; iFrameOffset++;
            ipNamePointer[3] = ipIntPointer[iFrameOffset]; iFrameOffset++;
            foundname = btrue;
        }
        else
        {
            iFrameOffset += 4;
        }

        iFrameOffset += iNumVertices;
        cnt++;
    }

    cFrameName[15] = 0;  // Make sure it's null terminated
    return foundname;
}

//---------------------------------------------------------------------------------------------
void rip_md2_frames( Uint16 modelindex )
{
    // ZZ> This function gets frames from the load buffer and adds them to
    //     the indexed model
    Uint8 cTmpx, cTmpy, cTmpz;
    Uint8 cTmpa;
    float fRealx, fRealy, fRealz;
    float fScalex, fScaley, fScalez;
    float fTranslatex, fTranslatey, fTranslatez;
    int iFrameOffset;
    int iNumVertices;
    int iNumFrames;
    int cnt, tnc;
    char* cpCharPointer;
    int* ipIntPointer;
    float* fpFloatPointer;

    // Jump to the Frames section of the md2 data
    cpCharPointer  = ( char* ) cLoadBuffer;
    ipIntPointer   = ( int* ) cLoadBuffer;
    fpFloatPointer = ( float* ) cLoadBuffer;

    iNumVertices = ENDIAN_INT32( ipIntPointer[6] );
    iNumFrames   = ENDIAN_INT32( ipIntPointer[10] );
    iFrameOffset = ENDIAN_INT32( ipIntPointer[14] ) >> 2;

    // Read in each frame
    madframestart[modelindex] = madloadframe;
    madframes[modelindex] = iNumFrames;
    madvertices[modelindex] = iNumVertices;
    cnt = 0;

    while ( cnt < iNumFrames && madloadframe < MAXFRAME )
    {
        fScalex     = ENDIAN_FLOAT( fpFloatPointer[iFrameOffset] ); iFrameOffset++;
        fScaley     = ENDIAN_FLOAT( fpFloatPointer[iFrameOffset] ); iFrameOffset++;
        fScalez     = ENDIAN_FLOAT( fpFloatPointer[iFrameOffset] ); iFrameOffset++;
        fTranslatex = ENDIAN_FLOAT( fpFloatPointer[iFrameOffset] ); iFrameOffset++;
        fTranslatey = ENDIAN_FLOAT( fpFloatPointer[iFrameOffset] ); iFrameOffset++;
        fTranslatez = ENDIAN_FLOAT( fpFloatPointer[iFrameOffset] ); iFrameOffset++;

        iFrameOffset += 4;
        tnc = 0;

        while ( tnc < iNumVertices )
        {
            // This should work because it's reading a single character
            cTmpx = cpCharPointer[( iFrameOffset<<2 )+0];
            cTmpy = cpCharPointer[( iFrameOffset<<2 )+1];
            cTmpz = cpCharPointer[( iFrameOffset<<2 )+2];
            cTmpa = cpCharPointer[( iFrameOffset<<2 )+3];

            fRealx = ( cTmpx * fScalex ) + fTranslatex;
            fRealy = ( cTmpy * fScaley ) + fTranslatey;
            fRealz = ( cTmpz * fScalez ) + fTranslatez;

            madvrtx[madloadframe][tnc] = -fRealx * 3.5f;
            madvrty[madloadframe][tnc] =  fRealy * 3.5f;
            madvrtz[madloadframe][tnc] =  fRealz * 3.5f;
            madvrta[madloadframe][tnc] =  cTmpa;

            iFrameOffset++;
            tnc++;
        }

        madloadframe++;
        cnt++;
    }
}

//---------------------------------------------------------------------------------------------
int load_one_md2(  const char* szLoadname, Uint16 modelindex )
{
    // ZZ> This function loads an id md2 file, storing the converted data in the indexed model
    //    int iFileHandleRead;
    size_t iBytesRead = 0;
    int iReturnValue;

    // Read the input file
    FILE *file = fopen( szLoadname, "rb" );
    if ( !file )
    {
        log_warning( "Cannot load file! (\"%s\")\n", szLoadname );
        return bfalse;
    }

    // Read up to MD2MAXLOADSIZE bytes from the file into the cLoadBuffer array.
    iBytesRead = fread( cLoadBuffer, 1, MD2MAXLOADSIZE, file );
    if ( iBytesRead == 0 )
        return bfalse;

    // save the filename for debugging
    globalparsename = szLoadname;

    // Check the header
    // TODO: Verify that the header's filesize correspond to iBytesRead.
    iReturnValue = rip_md2_header();
    if ( !iReturnValue )
        return bfalse;

    // Get the frame vertices
    rip_md2_frames( modelindex );
    // Get the commands
    rip_md2_commands( modelindex );
    // Fix them normals
    fix_md2_normals( modelindex );
    // Figure out how many vertices to transform
    get_madtransvertices( modelindex );

    fclose( file );

    return btrue;
}

//---------------------------------------------------------------------------------------------
void get_madtransvertices( Uint16 modelindex )
{
    // ZZ> This function gets the number of vertices to transform for a model...
    //     That means every one except the grip ( unconnected ) vertices

    //if (modelindex == 0)
    //{
    //    for ( cnt = 0; cnt < madvertices[modelindex]; cnt++ )
    //    {
    //        printf("%d-%d\n", cnt, vertexconnected( modelindex, cnt ) );
    //    }
    //}

    madtransvertices[modelindex] = madvertices[modelindex];
}
