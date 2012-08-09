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

typedef struct sPhysicsElement *pPhysicsElement;
typedef struct sPhysicsCollision *pPhysicsCollision;

#include "level.h"

typedef struct sPhysicsElement {
	struct {Sint32 x,y;} position,backupPosition,speed;
	Sint32 w,h; //32 pixel == SP_ONE
	Sint32 sqSize; //The square of the bounding circle of the rectangle; for distance
	int gravitation;
	int permeability; //bit,direction: 0,left 1,top 2,right 3,down (0 means solid)
	int superPower;
	int freeFallCounter;
	LevelObjectType type;
	pLevelObject levelObject;
	pPhysicsElement prev,next;
} tPhysicsElement;

typedef struct sPhysicsCollision {
	pPhysicsElement element[2];
	int hitPosition[2]; //like permeability; always pairs like left/right or top/down
	pPhysicsCollision prev,next;
} tPhysicsCollision;

pPhysicsElement createPhysicsElement(Sint32 px,Sint32 py,Sint32 w,Sint32 h,
			int moveable,int gravitation,int superPower,pLevelObject levelObject);
void createPhysicsFromLevel(pLevel level);
void clearPhysics(); //Deletes the whole scene
void doPhysics(int TimeForOneStep,void ( *setSpeed )( pPhysicsElement element ),
               void ( *gravFeedback )( pPhysicsCollision collision ),
               void ( *yFeedback )( pPhysicsCollision collision ),
               void ( *xFeedback )( pPhysicsCollision collision ));
void updateLevelObjects();

#endif
