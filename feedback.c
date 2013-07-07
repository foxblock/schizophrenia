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

void selectCorrectSprite(pPhysicsElement player)
{
	if (player->player.in_jump || player->freeFallCounter)
	{
		if (player->player.last_run < 0)
			spSelectSprite(player->levelObject->animation,"jump left");
		else
			spSelectSprite(player->levelObject->animation,"jump right");
	}
	else
	{
		if (player->speed.x)
		{
			if (player->player.pushes)
			{
				if (player->player.last_run < 0)
					spSelectSprite(player->levelObject->animation,"push left");
				else
					spSelectSprite(player->levelObject->animation,"push right");
				player->player.pushes = 0;
			}
			else
			{
				if (player->player.last_run < 0)
					spSelectSprite(player->levelObject->animation,"run left");
				else
					spSelectSprite(player->levelObject->animation,"run right");
			}
		}
		else
		{
			if (player->player.last_run < 0)
				spSelectSprite(player->levelObject->animation,"stand left");
			else
				spSelectSprite(player->levelObject->animation,"stand right");
		}
	}	
}

void setSpeed( pPhysicsElement element )
{
	if (element->levelObject == NULL)
		return;
	if (element->type == PLATFORM)
	{
		if (element->had_collision)
			element->levelObject->direction = 1 - element->levelObject->direction;
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
	if (element->type == PLAYER)
	{
		if (element->levelObject == level->choosenPlayer)
		{
			//Half of the Y-Movement stuff
			if ( !spGetInput()->button[SP_BUTTON_LEFT] )
			{
				if ( element->player.in_jump >= JUMP_MIN_TIME && element->player.can_jump ) // start turn-around
				{
					element->player.in_jump = JUMP_UPWARDS_TIME;
					element->player.can_jump = 0;
				}
				if ( element->player.in_jump == 0 ) // allow another jump only after another press of the button
					element->player.can_jump = 1;
			}

			//Moving the player X
			if (spGetInput()->axis[0] < 0)
			{
				if (element->player.last_run >= 0)
					element->player.last_run = 0;
				element->player.last_run-=1;
				if (element->player.last_run > -(MAX_MOVEMENT_FORCE / MOVEMENT_ACCEL))
					element->speed.x = element->player.last_run*MOVEMENT_ACCEL;
				else
					element->speed.x = -MAX_MOVEMENT_FORCE;
			}
			else
			if (spGetInput()->axis[0] > 0)
			{
				if (element->player.last_run <= 0)
					element->player.last_run = 0;
				element->player.last_run+=1;
				if (element->player.last_run < (MAX_MOVEMENT_FORCE / MOVEMENT_ACCEL))
					element->speed.x = element->player.last_run*MOVEMENT_ACCEL;
				else
					element->speed.x = MAX_MOVEMENT_FORCE;
			}
		}
		//For every player:
		if (element->player.in_jump)
		{
			if (element->player.in_jump < JUMP_UPWARDS_TIME) // moving upwards
			{
				element->player.in_jump++;
				element->speed.y-=JUMP_FORCE;
				element->freeFallCounter = 0;
			}
			else
			if (element->player.in_jump < JUMP_END_TIME) // smooth turn-around (peak of jump)
			{
				element->player.in_jump++;
				element->speed.y-=GRAVITY_MAX*(JUMP_END_TIME-element->player.in_jump)/(JUMP_END_TIME-JUMP_UPWARDS_TIME);
				element->freeFallCounter = 0;
				element->player.can_jump = 0;
			}
			else // end jump
			{
				element->player.in_jump = 0;
				element->player.can_jump = 0;
			}
		}
		selectCorrectSprite(element);
	}
	if (element->type == BUTTON)
	{
		if (element->had_collision)
			spSelectSprite(element->levelObject->animation,"on");
		else
			spSelectSprite(element->levelObject->animation,"off");
	}
}

void xHit( pPhysicsElement element,int pos)
{
	if (element->levelObject == level->choosenPlayer)
		element->player.pushes = 1;
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
				if ( element->player.in_jump == 0 && element->player.can_jump) // start the jump
					element->player.in_jump = 1;
			}
			else
				element->player.in_jump = 0;
		}

		if (pos == 1 && element->player.can_jump) //collision on top
		{
			// start turn-around
			element->player.in_jump = JUMP_UPWARDS_TIME;
			element->player.can_jump = 0;
		}		
	}
}
