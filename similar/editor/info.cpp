/*
 * Portions of this file are copyright Rebirth contributors and licensed as
 * described in COPYING.txt.
 * Portions of this file are copyright Parallax Software and licensed
 * according to the Parallax license below.
 * See COPYING.txt for license details.

THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

/*
 *
 * Print debugging info in ui.
 *
 */

#include <cinttypes>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inferno.h"
#include "window.h"
#include "segment.h"
#include "event.h"
#include "gr.h"
#include "ui.h"
#include "editor.h"
#include "editor/esegment.h"
#include "dxxerror.h"
#include "textures.h"
#include "object.h"
#include "ai.h"
#include "texpage.h"		// Textue selection paging stuff
#include "objpage.h"		// Object selection paging stuff
#include "wall.h"
#include "switch.h"
#include "info.h"
#include "d_levelstate.h"

int init_info;

typedef const char object_type_name[13];
typedef const char control_type_name[15];
typedef const char movement_type_name[15];
typedef const char ai_type_name[13];

static object_type_name &get_object_type(int num)
{
	switch (num) {
		case OBJ_NONE:
			return "OBJ_NONE    ";
		case OBJ_WALL:
			return "OBJ_WALL    ";
		case OBJ_FIREBALL:
			return "OBJ_FIREBALL";
		case OBJ_ROBOT:
			return "OBJ_ROBOT   ";
		case OBJ_HOSTAGE:
			return "OBJ_HOSTAGE ";
		case OBJ_PLAYER:
			return "OBJ_PLAYER  ";
		case OBJ_WEAPON:
			return "OBJ_WEAPON  ";
		case OBJ_CAMERA:
			return "OBJ_CAMERA  ";
		case OBJ_POWERUP:
			return "OBJ_POWERUP ";
		default:
			return " (unknown)  ";
	}
}

static control_type_name &get_control_type(const typename object::control_type num)
{
	switch (num) {
		case object::control_type::None:
			return "CT_NONE       ";
		case object::control_type::ai:
			return "CT_AI         ";
		case object::control_type::explosion:
			return "CT_EXPLOSION  ";
		case object::control_type::flying:
			return "CT_FLYING     ";
		case object::control_type::slew:
			return "CT_SLEW       ";
		case object::control_type::flythrough:
			return "CT_FLYTHROUGH ";
		case object::control_type::weapon:
			return "CT_WEAPON     ";
		default:
			return " (unknown)    ";
	}
}

static movement_type_name &get_movement_type(const typename object::movement_type num)
{
	switch (num) {
		case object::movement_type::None:
			return "MT_NONE       ";
		case object::movement_type::physics:
			return "MT_PHYSICS    ";
		case object::movement_type::spinning:
			return "MT_SPINNING   ";
		default:
			return " (unknown)    ";
	}
}

static ai_type_name &get_ai_behavior(ai_behavior num)
{
	switch (num) {
		case ai_behavior::AIB_STILL:
			return "STILL       ";
		case ai_behavior::AIB_NORMAL:
			return "NORMAL      ";
#if defined(DXX_BUILD_DESCENT_I)
		case ai_behavior::AIB_HIDE:
			return "HIDE        ";
		case ai_behavior::AIB_FOLLOW_PATH:
			return "FOLLOW_PATH ";
#elif defined(DXX_BUILD_DESCENT_II)
		case ai_behavior::AIB_BEHIND:
			return "BEHIND      ";
		case ai_behavior::AIB_SNIPE:
			return "SNIPE       ";
		case ai_behavior::AIB_FOLLOW:
			return "FOLLOW      ";
#endif
		case ai_behavior::AIB_RUN_FROM:
			return "RUN_FROM    ";
		default:
			return " (unknown)  ";
	}
}

//	---------------------------------------------------------------------------------------------------
static void info_display_object_placement(grs_canvas &canvas, int show_all)
{
	auto &Objects = LevelUniqueObjectState.Objects;
	static	int	old_Cur_object_index;
	static	int	old_type;
	static typename object::movement_type old_movement_type;
	static typename object::control_type old_control_type;
	static ai_behavior old_mode;
	if (init_info || show_all ||
		( Cur_object_index != old_Cur_object_index) || 
			( Objects[Cur_object_index].type != old_type) || 
			( Objects[Cur_object_index].movement_source != old_movement_type) || 
			( Objects[Cur_object_index].control_source != old_control_type) || 
			( Objects[Cur_object_index].ctype.ai_info.behavior != old_mode) ) {

		gr_uprintf(canvas, *canvas.cv_font, 0, 0, "Object id: %4d\n", Cur_object_index);
		gr_uprintf(canvas, *canvas.cv_font, 0, 16, "Type: %s\n", get_object_type(Objects[Cur_object_index].type));
		gr_uprintf(canvas, *canvas.cv_font, 0, 32, "Movmnt: %s\n", get_movement_type(Objects[Cur_object_index].movement_source));
		gr_uprintf(canvas, *canvas.cv_font, 0, 48, "Cntrl: %s\n", get_control_type(Objects[Cur_object_index].control_source));
		gr_uprintf(canvas, *canvas.cv_font, 0, 64, "Mode: %s\n", get_ai_behavior(Objects[Cur_object_index].ctype.ai_info.behavior));

		old_Cur_object_index = Cur_object_index;
		old_type = Objects[Cur_object_index].type;
		old_movement_type = Objects[Cur_object_index].movement_source;
		old_control_type = Objects[Cur_object_index].control_source;
		old_mode = Objects[Cur_object_index].ctype.ai_info.behavior;
	}

}

//	---------------------------------------------------------------------------------------------------
static void info_display_segsize(grs_canvas &canvas, int show_all)
{
	static	int	old_SegSizeMode;
	if (init_info | show_all) {
		old_SegSizeMode = -2;
	}

	if (old_SegSizeMode != SegSizeMode  ) {
		const char *name;
		switch (SegSizeMode) {
			case SEGSIZEMODE_FREE:		name = "free   ";	break;
			case SEGSIZEMODE_ALL:		name = "all    ";	break;
			case SEGSIZEMODE_CURSIDE:	name = "curside";	break;
			case SEGSIZEMODE_EDGE:		name = "edge   ";	break;
			case SEGSIZEMODE_VERTEX:	name = "vertex ";	break;
			default:
				Error("Illegal value for SegSizeMode in info.c/info_display_segsize\n");
		}
		old_SegSizeMode = SegSizeMode;
		gr_uprintf(canvas, *canvas.cv_font, 0, 0, "Mode: %s\n", name);
	}

}

//	---------------------------------------------------------------------------------------------------
static void info_display_default(grs_canvas &canvas, int show_all)
{
	auto &LevelSharedVertexState = LevelSharedSegmentState.get_vertex_state();
	auto &TmapInfo = LevelUniqueTmapInfoState.TmapInfo;
	static int old_Num_segments = -1;
	static int old_Num_vertices = -1;
	static int old_Num_objects = -1;
	static int old_Cursegp_num = -1;
	static int old_Curside = -1;
	static int old_Cursegp_num_for_verts = -1;
	static int old_CurrentTexture = -1;
	static int old_Num_walls = -1;
	static int old_Num_triggers = -1;

	if (init_info | show_all) {
		init_info = 0;
		old_Num_segments = -1;
		old_Num_vertices = -1;
		old_Num_objects = -1;
		old_Cursegp_num = -1;
		old_Cursegp_num_for_verts = -1;
		old_Curside = -1;
		old_CurrentTexture = -1;
		old_Num_walls = -1;
		old_Num_triggers = -1;
	}

	gr_set_fontcolor(canvas, CBLACK, CWHITE);

	//--------------- Number of segments ----------------

	if (old_Num_segments != LevelSharedSegmentState.Num_segments)
	{
		old_Num_segments = LevelSharedSegmentState.Num_segments;
		gr_uprintf(canvas, *canvas.cv_font, 0, 0, "Segments: %4d/%4" PRIuFAST32, LevelSharedSegmentState.Num_segments, static_cast<uint_fast32_t>(MAX_SEGMENTS));
	}

	//---------------- Number of vertics -----------------
	
	if (old_Num_vertices != LevelSharedVertexState.Num_vertices)
	{
		old_Num_vertices = LevelSharedVertexState.Num_vertices;
		gr_uprintf(canvas, *canvas.cv_font, 0, 16, "Vertices: %4d/%4" PRIuFAST32, LevelSharedVertexState.Num_vertices, static_cast<uint_fast32_t>(MAX_VERTICES));
	}

	//---------------- Number of objects -----------------
	
	if (old_Num_objects != LevelUniqueObjectState.num_objects)
	{
		const auto num_objects = LevelUniqueObjectState.num_objects;
		old_Num_objects = num_objects;
		gr_uprintf(canvas, *canvas.cv_font, 0, 32, "Objs: %3d/%3" DXX_PRI_size_type, num_objects, MAX_OBJECTS.value);
	}

  	//--------------- Current_segment_number -------------
	//--------------- Current_side_number -------------

	if (old_Cursegp_num != Cursegp || old_Curside != Curside)
	{
		old_Cursegp_num = Cursegp;
		old_Curside = Curside;
		gr_uprintf(canvas, *canvas.cv_font, 0, 48, "Cursegp/side: %3hu/%1d", static_cast<segnum_t>(Cursegp), Curside);
		unique_segment &useg = *Cursegp;
		auto &uside = useg.sides[Curside];
		gr_uprintf(canvas, *canvas.cv_font, 0, 128, " tmap1,2,o: %3d/%3dx%1u", uside.tmap_num, get_texture_index(uside.tmap_num2), static_cast<unsigned>(get_texture_rotation_low(uside.tmap_num2)));
	}

	//--------------- Current_vertex_numbers -------------

	if (old_Cursegp_num_for_verts != Cursegp)
	{
		old_Cursegp_num_for_verts = Cursegp;
		gr_uprintf(canvas, *canvas.cv_font, 0, 64, "{%3d,%3d,%3d,%3d,", Cursegp->verts[0],Cursegp->verts[1],
																							 Cursegp->verts[2],Cursegp->verts[3] );
		gr_uprintf(canvas, *canvas.cv_font, 0, 80," %3d,%3d,%3d,%3d}", Cursegp->verts[4],Cursegp->verts[5],
																							 Cursegp->verts[6],Cursegp->verts[7] );
	}

	//--------------- Num walls/links/triggers -------------------------

	auto &Walls = LevelUniqueWallSubsystemState.Walls;
	if ( old_Num_walls != Walls.get_count() ) {
		old_Num_walls = Walls.get_count();
		gr_uprintf(canvas, *canvas.cv_font, 0, 96, "Walls %3d", Walls.get_count());
	}

	//--------------- Num triggers ----------------------

	{
	auto &Triggers = LevelUniqueWallSubsystemState.Triggers;
	if ( old_Num_triggers != Triggers.get_count() ) {
		old_Num_triggers = Triggers.get_count();
		gr_uprintf(canvas, *canvas.cv_font, 0, 112, "Num_triggers %2d", Triggers.get_count());
	}
	}

	//--------------- Current texture number -------------

	if ( old_CurrentTexture != CurrentTexture )	{
		old_CurrentTexture = CurrentTexture;
		gr_uprintf(canvas, *canvas.cv_font, 0, 144, "Tex/Light: %3d %5.2f", CurrentTexture, f2fl(TmapInfo[CurrentTexture].lighting));
	}
}

//	------------------------------------------------------------------------------------
static void clear_pad_display(grs_canvas &canvas)
{
	gr_clear_canvas(canvas, CWHITE);
	gr_set_fontcolor(canvas, CBLACK, CWHITE);
}

//	------------------------------------------------------------------------------------
static window_event_result info_display_all(window *wind,const d_event &event, const unused_window_userdata_t *)
{
	static int old_padnum = -1;
	int        padnum,show_all = 1;		// always redraw
	grs_canvas *save_canvas = grd_curcanv;

	switch (event.type)
	{
		case EVENT_WINDOW_DRAW:
		{
			gr_set_current_canvas(wind->w_canv);
			auto &canvas = *grd_curcanv;

			padnum = ui_pad_get_current();
			Assert(padnum <= MAX_PAD_ID);

			if (padnum != old_padnum) {
				old_padnum = padnum;
				clear_pad_display(canvas);
				//show_all = 1;
			}

			switch (padnum) {
				case OBJECT_PAD_ID:			// Object placement
					info_display_object_placement(canvas, show_all);
					break;
				case SEGSIZE_PAD_ID:			// Segment sizing
					info_display_segsize(canvas, show_all);
					break;
				default:
					info_display_default(canvas, show_all);
					break;
			}
			grd_curcanv = save_canvas;
			return window_event_result::handled;
		}
		case EVENT_WINDOW_CLOSE:
			Pad_info = NULL;
			break;
			
		default:
			break;
	}
	return window_event_result::ignored;
}

//	------------------------------------------------------------------------------------
window *info_window_create(void)
{
	const auto wind = window_create(*Canv_editor, PAD_X + 250, PAD_Y + 8, 180, 160, info_display_all, unused_window_userdata);
	if (wind)
		wind->set_modal(0);
	return wind;
}


