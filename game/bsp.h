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
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file bsp.h
/// @details

#include "bbox.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
struct ego_BSP_aabb
{
    size_t     dim;
    float_ary mins;
    float_ary mids;
    float_ary maxs;

    ego_BSP_aabb()             { dim = size_t( -1 ); };
    ego_BSP_aabb( size_t dim ) { ctor( this, dim ); };
    ~ego_BSP_aabb()            { dtor( this ); };

    static ego_BSP_aabb * ctor( ego_BSP_aabb * pbb, size_t dim );
    static ego_BSP_aabb * dtor( ego_BSP_aabb * pbb );

    static bool_t         reset( ego_BSP_aabb * psrc );
    static bool_t         empty( ego_BSP_aabb * psrc1 );
    static bool_t         lhs_contains_rhs( ego_BSP_aabb * psrc1, ego_BSP_aabb * psrc2 );
    static bool_t         do_overlap( ego_BSP_aabb * psrc1, ego_BSP_aabb * psrc2 );

    static bool_t         from_oct_bb( ego_BSP_aabb * pdst, ego_oct_bb   * psrc );

private:
    static ego_BSP_aabb * clear( ego_BSP_aabb * ptr )
    {
        if ( NULL == ptr ) return ptr;

        ptr->dim = 0;

        return ptr;
    }
};

#define BSP_AABB_INIT_VALUES { 0, DYNAMIC_ARY_INIT_VALS, DYNAMIC_ARY_INIT_VALS }

//--------------------------------------------------------------------------------------------
struct ego_BSP_leaf
{
    bool_t         inserted;

    ego_BSP_leaf * next;
    int            data_type;
    void         * data;
    size_t         index;

    ego_BSP_aabb   bbox;

    ego_BSP_leaf() { clear( this ); } ;
    ego_BSP_leaf( int dim, void * data, int type ) { clear( this ); ctor( this, dim, data, type ); } ;
    ~ego_BSP_leaf() { dtor( this ); };

    static ego_BSP_leaf * create( int dim, void * data, int type );
    static bool_t         destroy( ego_BSP_leaf ** ppleaf );
    static ego_BSP_leaf * ctor( ego_BSP_leaf * t, int dim, void * data, int type );
    static ego_BSP_leaf * dtor( ego_BSP_leaf * t );

private:
    static ego_BSP_leaf * clear( ego_BSP_leaf * ptr )
    {
        if ( NULL == ptr ) return ptr;

        ptr->inserted = bfalse;

        ptr->next = NULL;
        ptr->data_type = -1;
        ptr->data = NULL;
        ptr->index = size_t( -1 );

        return ptr;
    }
};

//--------------------------------------------------------------------------------------------
DECLARE_DYNAMIC_ARY( ego_BSP_leaf_ary, ego_BSP_leaf );
DECLARE_DYNAMIC_ARY( ego_BSP_leaf_pary, ego_BSP_leaf * );

//--------------------------------------------------------------------------------------------
struct ego_BSP_branch
{
    friend struct ego_BSP_tree;

    ego_BSP_branch  * parent;

    size_t            child_count;
    ego_BSP_branch ** child_lst;

    size_t            node_count;
    ego_BSP_leaf    * node_lst;

    ego_BSP_aabb      bbox;
    int               depth;

    ego_BSP_branch()             { clear( this ); }
    ego_BSP_branch( size_t dim ) { clear( this ); ctor( this, dim ); }
    ~ego_BSP_branch()            { dtor( this ); }

    static ego_BSP_branch * create( size_t dim );
    static bool_t           destroy( ego_BSP_branch ** ppbranch );
    static ego_BSP_branch * create_ary( size_t ary_size, size_t dim );
    static bool_t           destroy_ary( size_t ary_size, ego_BSP_branch ** ppbranch );
    static ego_BSP_branch * ctor( ego_BSP_branch * B, size_t dim );
    static ego_BSP_branch * dtor( ego_BSP_branch * B );
    static bool_t           empty( ego_BSP_branch * pbranch );

    static bool_t           insert_leaf( ego_BSP_branch * B, ego_BSP_leaf * n );
    static bool_t           insert_branch( ego_BSP_branch * B, int index, ego_BSP_branch * B2 );
    static bool_t           clear_nodes( ego_BSP_branch * B, bool_t recursive );
    static bool_t           free_nodes( ego_BSP_branch * B, bool_t recursive );
    static bool_t           unlink( ego_BSP_branch * B );
    static bool_t           add_all_nodes( ego_BSP_branch * pbranch, ego_BSP_leaf_pary * colst );

private:
    static ego_BSP_branch * clear( ego_BSP_branch * ptr )
    {
        if ( NULL == ptr ) return ptr;

        ptr->parent = NULL;

        ptr->child_count = 0;
        ptr->child_lst = NULL;

        ptr->node_count = 0;
        ptr->node_lst = NULL;

        ptr->depth = -1;

        return ptr;
    }

    static bool_t             collide( ego_BSP_branch * pbranch, ego_BSP_aabb * paabb, ego_BSP_leaf_pary * colst );
    static bool_t             dealloc_nodes( ego_BSP_branch * B, bool_t recursive );
};

//--------------------------------------------------------------------------------------------
DECLARE_DYNAMIC_ARY( ego_BSP_branch_ary, ego_BSP_branch );
DECLARE_DYNAMIC_ARY( ego_BSP_branch_pary, ego_BSP_branch * );

//--------------------------------------------------------------------------------------------
struct ego_BSP_tree
{
    size_t dimensions;
    int    depth;

    ego_BSP_branch_ary  branch_all;
    ego_BSP_branch_pary branch_used;
    ego_BSP_branch_pary branch_free;

    ego_BSP_branch      * root;

    size_t                infinite_count;
    ego_BSP_leaf        * infinite;

    ego_BSP_aabb          bbox;

    ego_BSP_tree()                           { clear( this ); };

    ego_BSP_tree( Sint32 dim, Sint32 depth ) { clear( this ); ctor( this, dim, depth ); }
    ~ego_BSP_tree()                          { dtor( this ); }

    static ego_BSP_tree   * create( size_t count );
    static bool_t           destroy( ego_BSP_tree   ** ptree );

    static ego_BSP_tree   * ctor( ego_BSP_tree   * t, Sint32 dim, Sint32 depth );
    static ego_BSP_tree   * dtor( ego_BSP_tree   * t );
    static bool_t           alloc( ego_BSP_tree   * t, size_t count, size_t dim );
    static bool_t           dealloc( ego_BSP_tree   * t );
    static bool_t           init_0( ego_BSP_tree   * t );
    //static ego_BSP_tree   * init_1( ego_BSP_tree   * t, Sint32 dim, Sint32 depth );

    static bool_t             clear_nodes( ego_BSP_tree   * t, bool_t recursive );
    static bool_t             dealloc_nodes( ego_BSP_tree   * t, bool_t recursive );
    static bool_t             dealloc_all( ego_BSP_tree   * t );
    static bool_t             prune( ego_BSP_tree   * t );
    static ego_BSP_branch * get_free( ego_BSP_tree   * t );
    static bool_t             add_free( ego_BSP_tree   * t, ego_BSP_branch * B );
    static ego_BSP_branch * ensure_root( ego_BSP_tree   * t );
    static ego_BSP_branch * ensure_branch( ego_BSP_tree   * t, ego_BSP_branch * B, int index );
    static Sint32             count_nodes( Sint32 dim, Sint32 depth );
    static bool_t             insert( ego_BSP_tree   * t, ego_BSP_branch * B, ego_BSP_leaf * n, int index );
    static bool_t             insert_leaf( ego_BSP_tree   * ptree, ego_BSP_leaf * pleaf );
    static bool_t             prune_branch( ego_BSP_tree   * t, size_t cnt );
    static bool_t             prune_one_branch( ego_BSP_tree   * t, ego_BSP_branch * B, bool_t recursive );

    static bool_t             generate_aabb_child( ego_BSP_aabb * psrc, int index, ego_BSP_aabb * pdst );
    static int                collide( ego_BSP_tree   * tree, ego_BSP_aabb * paabb, ego_BSP_leaf_pary * colst );

protected:
    static ego_BSP_branch * alloc_branch( ego_BSP_tree   * t );
    static bool_t             dealloc_branch( ego_BSP_tree   * t, ego_BSP_branch * B );
    static bool_t             remove_used( ego_BSP_tree   * t, ego_BSP_branch * B );

    static bool_t             insert_leaf_rec( ego_BSP_tree   * ptree, ego_BSP_branch * pbranch, ego_BSP_leaf * pleaf, int depth );
    static bool_t             dealloc_branches( ego_BSP_tree   * t );

    static bool_t             insert_infinite( ego_BSP_tree   * ptree, ego_BSP_leaf * pleaf );

private:
    static ego_BSP_tree * clear( ego_BSP_tree * ptr )
    {
        if ( NULL == ptr ) return ptr;

        ptr->dimensions = 0;
        ptr->depth      = -1;
        ptr->root = NULL;

        ptr->infinite_count = 0;
        ptr->infinite = NULL;

        return ptr;
    }

};

#define BSP_TREE_INIT_VALS                                               \
    {                                                                    \
        0,                     /* size_t              dimensions     */  \
        0,                     /* int                 depth          */  \
        DYNAMIC_ARY_INIT_VALS, /* ego_BSP_branch_ary::t    branch_all     */  \
        DYNAMIC_ARY_INIT_VALS, /* ego_BSP_branch_pary * branch_all     */  \
        DYNAMIC_ARY_INIT_VALS, /* ego_BSP_branch_pary * branch_free    */  \
        NULL,                  /* ego_BSP_branch        * root           */  \
        0,                     /* size_t              infinite_count */  \
        NULL,                  /* ego_BSP_leaf          * infinite       */  \
        BSP_AABB_INIT_VALUES   /* ego_BSP_aabb   bbox                    */  \
    }

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define _bsp_h


