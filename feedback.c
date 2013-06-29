/*
 The contents of this file are subject to the Mozilla Public License
 Version 1.1 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 License for the specific language governing rights and limitations
 under the License.

 Alternatively, the contents of this file may be used under the terms
 of the GNU Lesser General Public license (the  "LGPL License"), in which case the
 provisions of LGPL License are applicable instead of those
 above.

 For feedback and questions about our Files and Projects please mail us,
 Janek Schäfer (foxblock) , foxblock_gmail_com
 Alexander Matthes (Ziz) , zizsdl_at_googlemail.com
*/

#include "feedback.h"

pLevel level;

pLevel* getLevelOverPointer()
{
	return &level;
}

void do_control_stuff()
{
	//Controls and some Logic (more in feedback.c)
	if (spGetInput()->button[SP_BUTTON_R])
	{
		spGetInput()->button[SP_BUTTON_R] = 0;
		level->choosenPlayer = level->choosenPlayer->prev;
	}
	if (spGetInput()->button[SP_BUTTON_L])
	{
		spGetInput()->button[SP_BUTTON_L] = 0;
		level->choosenPlayer = level->choosenPlayer->next;
	}
}

void setSpeed( pPhysicsElement element )
{
	if (element->levelObject == NULL)
		return;
	if (element->type == PLATFORM)
	{
		if (element->levelObject->direction == 0)
		{
			element->speed.x += element->levelObject->speed.v1.x;
			element->speed.y += element->levelObject->speed.v1.y;
		}
		else
		{
			element->speed.x = element->levelObject->speed.v2.x;
			element->speed.y = element->levelObject->speed.v2.y;
		}	
	}
	else
	if (element->levelObject == level->choosenPlayer)
	{
		//Half of the Y-Movement stuff
		if ( !spGetInput()->button[SP_BUTTON_LEFT] )
		{
			if ( element->specific.player.in_jump >= JUMP_MIN_TIME && element->specific.player.can_jump ) // start turn-around
			{
				element->specific.player.in_jump = JUMP_UPWARDS_TIME;
				element->specific.player.can_jump = 0;
			}
			if ( element->specific.player.in_jump == 0 ) // allow another jump only after another press of the button
				element->specific.player.can_jump = 1;
		}
		if (element->specific.player.in_jump)
		{
			if (element->specific.player.in_jump < JUMP_UPWARDS_TIME) // moving upwards
			{
				element->specific.player.in_jump++;
				element->speed.y-=JUMP_FORCE;
				element->freeFallCounter = 0;
			}
			else
			if (element->specific.player.in_jump < JUMP_END_TIME) // smooth turn-around (peak of jump)
			{
				element->specific.player.in_jump++;
				element->speed.y-=GRAVITY_MAX*(JUMP_END_TIME-element->specific.player.in_jump)/(JUMP_END_TIME-JUMP_UPWARDS_TIME);
				element->freeFallCounter = 0;
				element->specific.player.can_jump = 0;
			}
			else // end jump
			{
				element->specific.player.in_jump = 0;
				element->specific.player.can_jump = 0;
			}
		}
		//Moving the player X
		if (spGetInput()->axis[0] < 0)
		{
			if (element->specific.player.last_run >= 0)
				element->specific.player.last_run = 0;
			element->specific.player.last_run-=1;
			if (element->specific.player.last_run > -(MAX_MOVEMENT_FORCE / MOVEMENT_ACCEL))
				element->speed.x = element->specific.player.last_run*MOVEMENT_ACCEL;
			else
				element->speed.x = -MAX_MOVEMENT_FORCE;
		}
		else
		if (spGetInput()->axis[0] > 0)
		{
			if (element->specific.player.last_run <= 0)
				element->specific.player.last_run = 0;
			element->specific.player.last_run+=1;
			if (element->specific.player.last_run < (MAX_MOVEMENT_FORCE / MOVEMENT_ACCEL))
				element->speed.x = element->specific.player.last_run*MOVEMENT_ACCEL;
			else
				element->speed.x = MAX_MOVEMENT_FORCE;
		}
		else
		{
			if (element->specific.player.last_run < 0)
				element->specific.player.last_run = 0;
			else
			if (element->specific.player.last_run > 0)
				element->specific.player.last_run = 0;
		}
	}
}

void xHit( pPhysicsElement element,int pos)
{

}

void yHit( pPhysicsElement element,int pos)
{	
	if (element->levelObject == level->choosenPlayer)
	{
		//Other half of the Y-Movement stuff
		if (pos == -1)
		{
			if (spGetInput()->button[SP_BUTTON_LEFT])
			{
				if ( element->specific.player.in_jump == 0 && element->specific.player.can_jump) // start the jump
					element->specific.player.in_jump = 1;
			}
			else
				element->specific.player.in_jump = 0;
		}

		if (pos == 1 && element->specific.player.can_jump) //collision on top
		{
			// start turn-around
			element->specific.player.in_jump = JUMP_UPWARDS_TIME;
			element->specific.player.can_jump = 0;
		}		
	}
}
