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
 Janek Sch�fer (foxblock) , foxblock_gmail_com
 Alexander Matthes (Ziz) , zizsdl_at_googlemail.com
*/

#ifndef _PHYSIC_H
#define _PHYSIC_H

#include <sparrow3d.h>
#include "global_defines.h"

typedef struct sPhysicsElement *pPhysicsElement;
typedef struct sPhysicsCollisionChain *pPhysicsCollisionChain;
typedef struct sPhysicsCollision *pPhysicsCollision;

#include "level.h"

#define COLLISION_CHAIN_COUNT 6

typedef struct sPhysicsElement {
	struct {Sint32 x,y;} position,backupPosition,speed,newSpeed;
	Sint32 w,h; //32 pixel == SP_ONE
	int permeability; //bit,direction: 1,left 2,top 4,right 8,down (15 means solid, 0 transparent
	int floating;
	int moveable;
	int platform;
	int killed;
	int is_static;
	int freeFallCounter;
	int lastDirection;
	int background;
	LevelObjectType type;
	pLevelObject levelObject;
	pPhysicsElement prev,next;
} tPhysicsElement;

pPhysicsElement createPhysicsElement(Sint32 x,Sint32 y,Sint32 w,Sint32 h,
			int permeability,int floating,int moveable,int platform,int background, pLevelObject levelObject, int static_);
void createPhysicsFromLevel(pLevel level);
void clearPhysics(); //Deletes the whole scene
void doPhysics(void ( *setSpeed )( pPhysicsElement element ),
               int ( *gravFeedback )( pPhysicsCollision collision ),
               int ( *yFeedback )( pPhysicsCollision collision ),
               int ( *xFeedback )( pPhysicsCollision collision ), pLevel level);
void updateLevelObjects();

int getCollisionCount(); //DEBUG

#endif
