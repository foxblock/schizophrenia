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
 Janek Sch�fer (foxblock) , TODO_at_ADD.MAIL
 Alexander Matthes (Ziz) , zizsdl_at_googlemail.com
*/

/* This file contains some defines needed by multiply files of schizophrenia */

/* Determines, whether Z sorting is activated or not. It should be deactivated
   and the primitives should be sorted internal */
#define Z_SORTING 0

/* If Z_SORTING is 0, no resetting of the zbuffer is necessary */
//#define RESET_ZBUFFER spResetZBuffer();
#define RESET_ZBUFFER

/* At some point, if everywhere tiles are drawn, this define should be erased,
   so, that no (unnecessary) cleaning is done anymore */
#define CLEAN_TARGET spClearTarget(0);