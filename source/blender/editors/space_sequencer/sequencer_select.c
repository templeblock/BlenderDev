/*
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * Contributor(s): Blender Foundation, 2003-2009, Campbell Barton
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/editors/space_sequencer/sequencer_select.c
 *  \ingroup spseq
 */


#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include <sys/types.h>

#include "BLI_blenlib.h"
#include "BLI_math.h"
#include "BLI_utildefines.h"

#include "DNA_scene_types.h"

#include "BKE_context.h"
#include "BKE_sequencer.h"

#include "WM_api.h"
#include "WM_types.h"

#include "RNA_define.h"

/* for menu/popup icons etc etc*/

#include "ED_types.h"
#include "ED_screen.h"

#include "UI_view2d.h"

/* own include */
#include "sequencer_intern.h"
static void *find_nearest_marker(int UNUSED(d1), int UNUSED(d2)) {return NULL;}
	
static void select_surrounding_handles(Scene *scene, Sequence *test) /* XXX BRING BACK */
{
	Sequence *neighbor;
	
	neighbor=find_neighboring_sequence(scene, test, SEQ_SIDE_LEFT, -1);
	if (neighbor) {
		neighbor->flag |= SELECT;
		recurs_sel_seq(neighbor);
		neighbor->flag |= SEQ_RIGHTSEL;
	}
	neighbor=find_neighboring_sequence(scene, test, SEQ_SIDE_RIGHT, -1);
	if (neighbor) {
		neighbor->flag |= SELECT;
		recurs_sel_seq(neighbor);
		neighbor->flag |= SEQ_LEFTSEL;
	}
	test->flag |= SELECT;
}

/* used for mouse selection and for SEQUENCER_OT_select_active_side() */
static void select_active_side(ListBase *seqbase, int sel_side, int channel, int frame)
{
	Sequence *seq;

	for(seq= seqbase->first; seq; seq=seq->next) {
		if(channel==seq->machine) {
			switch(sel_side) {
			case SEQ_SIDE_LEFT:
				if (frame > (seq->startdisp)) {
					seq->flag &= ~(SEQ_RIGHTSEL|SEQ_LEFTSEL);
					seq->flag |= SELECT;
				}
				break;
			case SEQ_SIDE_RIGHT:
				if (frame < (seq->startdisp)) {
					seq->flag &= ~(SEQ_RIGHTSEL|SEQ_LEFTSEL);
					seq->flag |= SELECT;
				}
				break;
			case SEQ_SIDE_BOTH:
				seq->flag &= ~(SEQ_RIGHTSEL|SEQ_LEFTSEL);
				break;
			}
		}
	}
}

/* used for mouse selection and for SEQUENCER_OT_select_active_side() */
static void select_linked_time(ListBase *seqbase, Sequence *seq_link)
{
	Sequence *seq;

	for(seq= seqbase->first; seq; seq=seq->next) {
		if(seq_link->machine != seq->machine) {
			int left_match = (seq->startdisp == seq_link->startdisp) ? 1:0;
			int right_match = (seq->enddisp == seq_link->enddisp) ? 1:0;

			if(left_match && right_match) {
				/* a direct match, copy the selection settinhs */
				seq->flag &= ~(SELECT|SEQ_LEFTSEL|SEQ_RIGHTSEL);
				seq->flag |= seq_link->flag & (SELECT|SEQ_LEFTSEL|SEQ_RIGHTSEL);

				recurs_sel_seq(seq);
			}
			else if(seq_link->flag & SELECT && (left_match || right_match)) {

				/* clear for reselection */
				seq->flag &= ~(SEQ_LEFTSEL|SEQ_RIGHTSEL);

				if(left_match && seq_link->flag & SEQ_LEFTSEL)
					seq->flag |= SELECT|SEQ_LEFTSEL;

				if(right_match && seq_link->flag & SEQ_RIGHTSEL)
					seq->flag |= SELECT|SEQ_RIGHTSEL;

				recurs_sel_seq(seq);
			}
		}
	}
}

#if 0 // BRING BACK
void select_surround_from_last(Scene *scene)
{
	Sequence *seq=get_last_seq(scene);
	
	if (seq==NULL)
		return;
	
	select_surrounding_handles(scene, seq);
}
#endif


static void UNUSED_FUNCTION(select_single_seq)(Scene *scene, Sequence *seq, int deselect_all) /* BRING BACK */
{
	Editing *ed= seq_give_editing(scene, FALSE);
	
	if(deselect_all)
		deselect_all_seq(scene);
	seq_active_set(scene, seq);

	if((seq->type==SEQ_IMAGE) || (seq->type==SEQ_MOVIE)) {
		if(seq->strip)
			strncpy(ed->act_imagedir, seq->strip->dir, FILE_MAXDIR-1);
	}
	else if(seq->type==SEQ_SOUND) {
		if(seq->strip)
			strncpy(ed->act_sounddir, seq->strip->dir, FILE_MAXDIR-1);
	}
	seq->flag|= SELECT;
	recurs_sel_seq(seq);
}

// remove this function, replace with invert operator
//void swap_select_seq(Scene *scene)
#if 0
static void select_neighbor_from_last(Scene *scene, int lr)
{
	Sequence *seq= seq_active_get(scene);
	Sequence *neighbor;
	int change = 0;
	if (seq) {
		neighbor=find_neighboring_sequence(scene, seq, lr, -1);
		if (neighbor) {
			switch (lr) {
			case SEQ_SIDE_LEFT:
				neighbor->flag |= SELECT;
				recurs_sel_seq(neighbor);
				neighbor->flag |= SEQ_RIGHTSEL;
				seq->flag |= SEQ_LEFTSEL;
				break;
			case SEQ_SIDE_RIGHT:
				neighbor->flag |= SELECT;
				recurs_sel_seq(neighbor);
				neighbor->flag |= SEQ_LEFTSEL;
				seq->flag |= SEQ_RIGHTSEL;
				break;
			}
		seq->flag |= SELECT;
		change = 1;
		}
	}
	if (change) {
	}
}
#endif

/* (de)select operator */
static int sequencer_deselect_exec(bContext *C, wmOperator *UNUSED(op))
{
	Scene *scene= CTX_data_scene(C);
	Editing *ed= seq_give_editing(scene, FALSE);
	Sequence *seq;
	int desel = 0;

	for(seq= ed->seqbasep->first; seq; seq=seq->next) {
		if(seq->flag & SEQ_ALLSEL) {
			desel= 1;
			break;
		}
	}

	for(seq= ed->seqbasep->first; seq; seq=seq->next) {
		if (desel) {
			seq->flag &= ~SEQ_ALLSEL;
		}
		else {
			seq->flag &= ~(SEQ_LEFTSEL+SEQ_RIGHTSEL);
			seq->flag |= SELECT;
		}
	}

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);
	
	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_all_toggle(struct wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select or Deselect All";
	ot->idname= "SEQUENCER_OT_select_all_toggle";
	ot->description="Select or deselect all strips";
	
	/* api callbacks */
	ot->exec= sequencer_deselect_exec;
	ot->poll= sequencer_edit_poll;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
}


/* (de)select operator */
static int sequencer_select_inverse_exec(bContext *C, wmOperator *UNUSED(op))
{
	Scene *scene= CTX_data_scene(C);
	Editing *ed= seq_give_editing(scene, FALSE);
	Sequence *seq;

	for(seq= ed->seqbasep->first; seq; seq=seq->next) {
		if (seq->flag & SELECT) {
			seq->flag &= ~SEQ_ALLSEL;
		}
		else {
			seq->flag &= ~(SEQ_LEFTSEL+SEQ_RIGHTSEL);
			seq->flag |= SELECT;
		}
	}

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);
	
	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_inverse(struct wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select Inverse";
	ot->idname= "SEQUENCER_OT_select_inverse";
	ot->description="Select unselected strips";
	
	/* api callbacks */
	ot->exec= sequencer_select_inverse_exec;
	ot->poll= sequencer_edit_poll;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
}

static int sequencer_select_invoke(bContext *C, wmOperator *op, wmEvent *event)
{
	View2D *v2d= UI_view2d_fromcontext(C);
	Scene *scene= CTX_data_scene(C);
	Editing *ed= seq_give_editing(scene, FALSE);
	short extend= RNA_boolean_get(op->ptr, "extend");
	short linked_handle= RNA_boolean_get(op->ptr, "linked_handle");
	short left_right= RNA_boolean_get(op->ptr, "left_right");
	short linked_time= RNA_boolean_get(op->ptr, "linked_time");
	
	Sequence *seq,*neighbor, *act_orig;
	int hand,sel_side;
	TimeMarker *marker;

	if(ed==NULL)
		return OPERATOR_CANCELLED;
	
	marker=find_nearest_marker(SCE_MARKERS, 1); //XXX - dummy function for now
	
	seq= find_nearest_seq(scene, v2d, &hand, event->mval);

	// XXX - not nice, Ctrl+RMB needs to do left_right only when not over a strip
	if(seq && linked_time && left_right)
		left_right= FALSE;


	if (marker) {
		int oldflag;
		/* select timeline marker */
		if (extend) {
			oldflag= marker->flag;
			if (oldflag & SELECT)
				marker->flag &= ~SELECT;
			else
				marker->flag |= SELECT;
		}
		else {
			/* deselect_markers(0, 0); */ /* XXX, in 2.4x, seq selection used to deselect all, need to re-thnik this for 2.5 */
			marker->flag |= SELECT;				
		}
		
	} else if (left_right) {
		/* use different logic for this */
		float x;
		deselect_all_seq(scene);
		UI_view2d_region_to_view(v2d, event->mval[0], event->mval[1], &x, NULL);

		SEQP_BEGIN(ed, seq) {
			if (x < CFRA) {
				if(seq->enddisp < CFRA) {
					seq->flag |= SELECT;
					recurs_sel_seq(seq);
				}
			}
			else {
				if(seq->startdisp > CFRA) {
					seq->flag |= SELECT;
					recurs_sel_seq(seq);
				}
			}
		}
		SEQ_END
		
		{
			SpaceSeq *sseq= CTX_wm_space_seq(C);
			if (sseq && sseq->flag & SEQ_MARKER_TRANS) {
				TimeMarker *tmarker;

				for (tmarker= scene->markers.first; tmarker; tmarker= tmarker->next) {
					if(	((x < CFRA) && tmarker->frame < CFRA) ||
						((x >= CFRA) && tmarker->frame >= CFRA)
					) {
						tmarker->flag |= SELECT;
					}
					else {
						tmarker->flag &= ~SELECT;
					}
				}
			}
		}
	} else {
		// seq= find_nearest_seq(scene, v2d, &hand, mval);

		act_orig= ed->act_seq;

		if(extend == 0 && linked_handle==0)
			deselect_all_seq(scene);
	
		if(seq) {
			seq_active_set(scene, seq);
	
			if ((seq->type == SEQ_IMAGE) || (seq->type == SEQ_MOVIE)) {
				if(seq->strip) {
					strncpy(ed->act_imagedir, seq->strip->dir, FILE_MAXDIR-1);
				}
			} else
			if (seq->type == SEQ_SOUND) {
				if(seq->strip) {
					strncpy(ed->act_sounddir, seq->strip->dir, FILE_MAXDIR-1);
				}
			}
	
			if(extend && (seq->flag & SELECT) && ed->act_seq == act_orig ) {
				switch(hand) {
				case SEQ_SIDE_NONE:
					if (linked_handle==0)
						seq->flag &= ~SEQ_ALLSEL;
					break;
				case SEQ_SIDE_LEFT:
					seq->flag ^= SEQ_LEFTSEL;
					break;
				case SEQ_SIDE_RIGHT:
					seq->flag ^= SEQ_RIGHTSEL;
					break;
				}
			}
			else {
				seq->flag |= SELECT;
				if(hand==SEQ_SIDE_LEFT)		seq->flag |= SEQ_LEFTSEL;
				if(hand==SEQ_SIDE_RIGHT)	seq->flag |= SEQ_RIGHTSEL;
			}
			
			/* On Alt selection, select the strip and bordering handles */
			if (linked_handle && !ELEM(hand, SEQ_SIDE_LEFT, SEQ_SIDE_RIGHT)) {
				if(extend==0) deselect_all_seq(scene);
				seq->flag |= SELECT;
				select_surrounding_handles(scene, seq);
			}
			else if (linked_handle && ELEM(hand, SEQ_SIDE_LEFT, SEQ_SIDE_RIGHT) && (seq->flag & SELECT)) {
				/*
				 * First click selects adjacent handles on that side.
				 * Second click selects all strips in that direction.
				 * If there are no adjacent strips, it just selects all in that direction.
				 */
				sel_side= hand;
				neighbor=find_neighboring_sequence(scene, seq, sel_side, -1);
				if (neighbor) {
					switch (sel_side) {
					case SEQ_SIDE_LEFT:
						if ((seq->flag & SEQ_LEFTSEL) && (neighbor->flag & SEQ_RIGHTSEL)) {
							if(extend==0) deselect_all_seq(scene);
							seq->flag |= SELECT;
							
							select_active_side(ed->seqbasep, SEQ_SIDE_LEFT, seq->machine, seq->startdisp);
						} else {
							if(extend==0) deselect_all_seq(scene);
							seq->flag |= SELECT;

							neighbor->flag |= SELECT;
							recurs_sel_seq(neighbor);
							neighbor->flag |= SEQ_RIGHTSEL;
							seq->flag |= SEQ_LEFTSEL;
						}
						break;
					case SEQ_SIDE_RIGHT:
						if ((seq->flag & SEQ_RIGHTSEL) && (neighbor->flag & SEQ_LEFTSEL)) {
							if(extend==0) deselect_all_seq(scene);
							seq->flag |= SELECT;

							select_active_side(ed->seqbasep, SEQ_SIDE_RIGHT, seq->machine, seq->startdisp);
						} else {
							if(extend==0) deselect_all_seq(scene);
							seq->flag |= SELECT;

							neighbor->flag |= SELECT;
							recurs_sel_seq(neighbor);
							neighbor->flag |= SEQ_LEFTSEL;
							seq->flag |= SEQ_RIGHTSEL;
						}
						break;
					}
				} else {
					if(extend==0) deselect_all_seq(scene);
					select_active_side(ed->seqbasep, sel_side, seq->machine, seq->startdisp);
				}
			}
			recurs_sel_seq(seq);

			if(linked_time) {
				select_linked_time(ed->seqbasep, seq);
			}
		}
	}
	
	/* marker transform */
#if 0 // XXX probably need to redo this differently for 2.5
	if (marker) {
		int mval[2], xo, yo;
//		getmouseco_areawin(mval);
		xo= mval[0]; 
		yo= mval[1];
		
		while(get_mbut()) {		
//			getmouseco_areawin(mval);
			if(abs(mval[0]-xo)+abs(mval[1]-yo) > 4) {
				transform_markers('g', 0);
				return;
			}
		}
	}
#endif
	
	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);

	/* allowing tweaks */
	return OPERATOR_FINISHED|OPERATOR_PASS_THROUGH;
}

void SEQUENCER_OT_select(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Activate/Select";
	ot->idname= "SEQUENCER_OT_select";
	ot->description="Select a strip (last selected becomes the \"active strip\")";
	
	/* api callbacks */
	ot->invoke= sequencer_select_invoke;
	ot->poll= ED_operator_sequencer_active;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* properties */
	RNA_def_boolean(ot->srna, "extend", 0, "Extend", "Extend the selection.");
	RNA_def_boolean(ot->srna, "linked_handle", 0, "Linked Handle", "Select handles next to the active strip.");
	/* for animation this is an enum but atm having an enum isnt useful for us */
	RNA_def_boolean(ot->srna, "left_right", 0, "Left/Right", "select based on the frame side the cursor is on.");
	RNA_def_boolean(ot->srna, "linked_time", 0, "Linked Time", "Select other strips at the same time.");
}




/* run recursivly to select linked */
static int select_more_less_seq__internal(Scene *scene, int sel, int linked) {
	Editing *ed= seq_give_editing(scene, FALSE);
	Sequence *seq, *neighbor;
	int change=0;
	int isel;
	
	if(ed==NULL) return 0;
	
	if (sel) {
		sel = SELECT;
		isel = 0;
	} else {
		sel = 0;
		isel = SELECT;
	}
	
	if (!linked) {
		/* if not linked we only want to touch each seq once, newseq */
		for(seq= ed->seqbasep->first; seq; seq= seq->next) {
			seq->tmp = NULL;
		}
	}
	
	for(seq= ed->seqbasep->first; seq; seq= seq->next) {
		if((int)(seq->flag & SELECT) == sel) {
			if ((linked==0 && seq->tmp)==0) {
				/* only get unselected nabours */
				neighbor = find_neighboring_sequence(scene, seq, SEQ_SIDE_LEFT, isel);
				if (neighbor) {
					if (sel) {neighbor->flag |= SELECT; recurs_sel_seq(neighbor);}
					else		neighbor->flag &= ~SELECT;
					if (linked==0) neighbor->tmp = (Sequence *)1;
					change = 1;
				}
				neighbor = find_neighboring_sequence(scene, seq, SEQ_SIDE_RIGHT, isel);
				if (neighbor) {
					if (sel) {neighbor->flag |= SELECT; recurs_sel_seq(neighbor);}
					else		neighbor->flag &= ~SELECT;
					if (linked==0) neighbor->tmp = (void *)1;
					change = 1;
				}
			}
		}
	}
	
	return change;
}



/* select more operator */
static int sequencer_select_more_exec(bContext *C, wmOperator *UNUSED(op))
{
	Scene *scene= CTX_data_scene(C);
	
	if(!select_more_less_seq__internal(scene, 0, 0))
		return OPERATOR_CANCELLED;

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);
	
	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_more(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select More";
	ot->idname= "SEQUENCER_OT_select_more";
	ot->description="Select more strips adjacent to the current selection";
	
	/* api callbacks */
	ot->exec= sequencer_select_more_exec;
	ot->poll= sequencer_edit_poll;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* properties */
}


/* select less operator */
static int sequencer_select_less_exec(bContext *C, wmOperator *UNUSED(op))
{
	Scene *scene= CTX_data_scene(C);
	
	if(!select_more_less_seq__internal(scene, 1, 0))
		return OPERATOR_CANCELLED;
 
	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);
	
	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_less(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select less";
	ot->idname= "SEQUENCER_OT_select_less";
	ot->description="Shrink the current selection of adjacent selected strips";
	
	/* api callbacks */
	ot->exec= sequencer_select_less_exec;
	ot->poll= sequencer_edit_poll;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* properties */
}


/* select pick linked operator (uses the mouse) */
static int sequencer_select_linked_pick_invoke(bContext *C, wmOperator *op, wmEvent *event)
{
	Scene *scene= CTX_data_scene(C);
	View2D *v2d= UI_view2d_fromcontext(C);
	
	short extend= RNA_boolean_get(op->ptr, "extend");
	
	Sequence *mouse_seq;
	int selected, hand;

	/* this works like UV, not mesh */
	mouse_seq= find_nearest_seq(scene, v2d, &hand, event->mval);
	if (!mouse_seq)
		return OPERATOR_FINISHED; /* user error as with mesh?? */
	
	if (extend==0)
		deselect_all_seq(scene);
	
	mouse_seq->flag |= SELECT;
	recurs_sel_seq(mouse_seq);
	
	selected = 1;
	while (selected) {
		selected = select_more_less_seq__internal(scene, 1, 1);
	}
	
	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);
	
	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_linked_pick(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select pick linked";
	ot->idname= "SEQUENCER_OT_select_linked_pick";
	ot->description="Select a chain of linked strips nearest to the mouse pointer";
	
	/* api callbacks */
	ot->invoke= sequencer_select_linked_pick_invoke;
	ot->poll= ED_operator_sequencer_active;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* properties */
	RNA_def_boolean(ot->srna, "extend", 0, "Extend", "extend the selection");
}


/* select linked operator */
static int sequencer_select_linked_exec(bContext *C, wmOperator *UNUSED(op))
{
	Scene *scene= CTX_data_scene(C);
	int selected;

	selected = 1;
	while (selected) {
		selected = select_more_less_seq__internal(scene, 1, 1);
	}

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);

	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_linked(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select linked";
	ot->idname= "SEQUENCER_OT_select_linked";
	ot->description="Select all strips adjacent to the current selection";
	
	/* api callbacks */
	ot->exec= sequencer_select_linked_exec;
	ot->poll= sequencer_edit_poll;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* properties */
}


/* select handles operator */
static int sequencer_select_handles_exec(bContext *C, wmOperator *op)
{
	Scene *scene= CTX_data_scene(C);
	Editing *ed= seq_give_editing(scene, 0);
	Sequence *seq;
	int sel_side= RNA_enum_get(op->ptr, "side");


	for(seq= ed->seqbasep->first; seq; seq=seq->next) {
		if (seq->flag & SELECT) {
			switch(sel_side) {
			case SEQ_SIDE_LEFT:
				seq->flag &= ~SEQ_RIGHTSEL;
				seq->flag |= SEQ_LEFTSEL;
				break;
			case SEQ_SIDE_RIGHT:
				seq->flag &= ~SEQ_LEFTSEL;
				seq->flag |= SEQ_RIGHTSEL;
				break;
			case SEQ_SIDE_BOTH:
				seq->flag |= SEQ_LEFTSEL+SEQ_RIGHTSEL;
				break;
			}
		}
	}

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);

	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_handles(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select Handles";
	ot->idname= "SEQUENCER_OT_select_handles";
	ot->description="Select manipulator handles on the sides of the selected strip";
	
	/* api callbacks */
	ot->exec= sequencer_select_handles_exec;
	ot->poll= sequencer_edit_poll;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* properties */
	RNA_def_enum(ot->srna, "side", prop_side_types, SEQ_SIDE_BOTH, "Side", "The side of the handle that is selected");
}

/* select side operator */
static int sequencer_select_active_side_exec(bContext *C, wmOperator *op)
{
	Scene *scene= CTX_data_scene(C);
	Editing *ed= seq_give_editing(scene, 0);
	Sequence *seq_act= seq_active_get(scene);

	if (ed==NULL || seq_act==NULL)
		return OPERATOR_CANCELLED;

	seq_act->flag |= SELECT;

	select_active_side(ed->seqbasep, RNA_enum_get(op->ptr, "side"), seq_act->machine, seq_act->startdisp);

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);

	return OPERATOR_FINISHED;
}

void SEQUENCER_OT_select_active_side(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Select Active Side";
	ot->idname= "SEQUENCER_OT_select_active_side";
	ot->description="Select strips on the nominated side of the active strip";
	
	/* api callbacks */
	ot->exec= sequencer_select_active_side_exec;
	ot->poll= sequencer_edit_poll;

	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;

	/* properties */
	RNA_def_enum(ot->srna, "side", prop_side_types, SEQ_SIDE_BOTH, "Side", "The side of the handle that is selected");
}


/* borderselect operator */
static int sequencer_borderselect_exec(bContext *C, wmOperator *op)
{
	Scene *scene= CTX_data_scene(C);
	Editing *ed= seq_give_editing(scene, FALSE);
	View2D *v2d= UI_view2d_fromcontext(C);
	
	Sequence *seq;
	rcti rect;
	rctf rectf, rq;
	short selecting = (RNA_int_get(op->ptr, "gesture_mode")==GESTURE_MODAL_SELECT);
	int mval[2];

	if(ed==NULL)
		return OPERATOR_CANCELLED;

	rect.xmin= RNA_int_get(op->ptr, "xmin");
	rect.ymin= RNA_int_get(op->ptr, "ymin");
	rect.xmax= RNA_int_get(op->ptr, "xmax");
	rect.ymax= RNA_int_get(op->ptr, "ymax");
	
	mval[0]= rect.xmin;
	mval[1]= rect.ymin;
	UI_view2d_region_to_view(v2d, mval[0], mval[1], &rectf.xmin, &rectf.ymin);
	mval[0]= rect.xmax;
	mval[1]= rect.ymax;
	UI_view2d_region_to_view(v2d, mval[0], mval[1], &rectf.xmax, &rectf.ymax);

	for(seq= ed->seqbasep->first; seq; seq= seq->next) {
		seq_rectf(seq, &rq);
		
		if(BLI_isect_rctf(&rq, &rectf, NULL)) {
			if(selecting)		seq->flag |= SELECT;
			else				seq->flag &= ~SEQ_ALLSEL;
			recurs_sel_seq(seq);
		}
	}

	WM_event_add_notifier(C, NC_SCENE|ND_SEQUENCER|NA_SELECTED, scene);

	return OPERATOR_FINISHED;
} 


/* ****** Border Select ****** */
void SEQUENCER_OT_select_border(wmOperatorType *ot)
{
	/* identifiers */
	ot->name= "Border Select";
	ot->idname= "SEQUENCER_OT_select_border";
	ot->description="Enable border select mode";
	
	/* api callbacks */
	ot->invoke= WM_border_select_invoke;
	ot->exec= sequencer_borderselect_exec;
	ot->modal= WM_border_select_modal;
	ot->cancel= WM_border_select_cancel;
	
	ot->poll= ED_operator_sequencer_active;
	
	/* flags */
	ot->flag= OPTYPE_REGISTER|OPTYPE_UNDO;
	
	/* rna */
	WM_operator_properties_gesture_border(ot, FALSE);
}
