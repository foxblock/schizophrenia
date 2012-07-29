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

#include "level.h"

pLevelObjectGroup createGroup(pLevelObjectGroup *firstGroup, LevelObjectType type)
{
	pLevelObjectGroup group = (pLevelObjectGroup)malloc(sizeof(tLevelObjectGroup));
	group->type = type;
	group->firstObject = NULL;
	if (*firstGroup)
	{
		(*firstGroup)->prev->next = group;
		group->prev = (*firstGroup)->prev;
		group->next = (*firstGroup);
		(*firstGroup)->prev = group;
	}
	else
	{
		group->next = group;
		group->prev = group;
	}
	(*firstGroup) = group;
	return group;
}

pLevelObject createObject(pLevelObjectGroup group,LevelObjectType type)
{
	pLevelObject obj = (pLevelObject)malloc(sizeof(tLevelObject));
	//Some default values. Keep in mind: Not every value is used for every object
	obj->type = type;
	obj->animation = NULL;
	obj->x = 0;
	obj->y = 0;
	obj->w = 24;
	obj->h = 24;
	obj->speed = 0;
	obj->direction = RIGHT;
	obj->some_char = NULL;
	obj->kind = 0;
	if (group->firstObject)
	{
		group->firstObject->prev->next = obj;
		obj->prev = group->firstObject->prev;
		obj->next = group->firstObject;
		group->firstObject->prev = obj;
	}
	else
	{
		obj->next = obj;
		obj->prev = obj;
	}
	group->firstObject = obj;
	return obj;
}

void allocLayer(pLayer layer,int width, int height)
{
	layer->tile = (pTile)malloc(sizeof(tTile)*width*height);
	memset(layer->tile,0,width*height*sizeof(tTile));
	layer->width = width;
	layer->height = height;
}

#define READ_TIL_TAG_BEGIN end = spReadUntil(file,buffer,65536,'<'); if (end) break;
#define READ_TIL_TAG_END end = spReadUntil(file,buffer,65536,'>'); if (end) break;

int getHexSign(char sign)
{
	if (sign >= '0' && sign <= '9')
		return sign-'0';
	if (sign >= 'A' && sign <= 'F')
		return sign-'A'+10;
	if (sign >= 'a' && sign <= 'f')
		return sign-'a'+10;
	return 0;
}

Uint16 hex2color(char* value)
{
	if (strlen(value) < 6)
		return 0;
	int r = getHexSign(value[0])*16+getHexSign(value[1]);
	int g = getHexSign(value[2])*16+getHexSign(value[3]);
	int b = getHexSign(value[4])*16+getHexSign(value[5]);
	return spGetRGB(r,g,b);
}

typedef struct sTile_set *pTile_set;
typedef struct sTile_set {
	int firstgid;
	int lastgid;
	int width,height,tilewidth,tileheight;
	char* filename;
	pTile_set next;
} tTile_set;

spSpritePointer get_sprite(int tilenr,pTile_set tile_set,spSpritePointer *table)
{
	if (tilenr <= 0)
		return NULL;
	if (table[tilenr])
		return table[tilenr];
	while (tile_set)
	{
		if (tile_set->firstgid<=tilenr && tile_set->lastgid>=tilenr)
		{
			//Calculating the position of the tile.
			int lineMax = tile_set->width/tile_set->tilewidth;
			int lineNumber = (tilenr-tile_set->firstgid)/lineMax;
			int linePosition = (tilenr-tile_set->firstgid)%lineMax;
			table[tilenr] = spNewSprite(NULL);
			char filename[1024];
			sprintf(filename,"./level/%s",tile_set->filename);
			spNewSubSpriteWithTiling(table[tilenr],spLoadSurface(filename),linePosition*tile_set->tilewidth,lineNumber*tile_set->tileheight,tile_set->tilewidth,tile_set->tileheight,1000);
			return table[tilenr];
		}
		tile_set = tile_set->next;
	}
	return NULL;
}

pLevel loadLevel(char* filename)
{
	SDL_RWops *file = SDL_RWFromFile(filename, "rt");
	if (!file)
	{
		printf("Level \"%s\" not found. We're all gone die!\n",filename);
		return NULL;
	}
	//Creating a new, empty level
	pLevel level = (pLevel)malloc(sizeof(tLevel));
	level->layer.physic.tile = NULL;
	level->layer.background.tile = NULL;
	level->layer.player.tile = NULL;
	level->layer.foreground.tile = NULL;
	level->actualCamera.x = 0;
	level->actualCamera.y = 0;
	level->targetCamera.x = 0;
	level->targetCamera.y = 0;
	level->backgroundColor = 65535; //white
	level->spriteTable = NULL;
	level->firstObjectGroup = NULL;
	level->choosenPlayer = NULL;
	
	//Some values, which will be read and used later
	int width = 0;
	int height = 0;
	pTile_set tile_set = NULL;
	
	char buffer[65536];
	//Reading to the begin of the first tag
	spReadUntil(file,buffer,65536,'<');
	//Reading the first tag
	spReadUntil(file,buffer,65536,'>');
	if (buffer[0] == '?')
		printf("Reading first line %s - Everything as expected\n",buffer);
	else
	{
		printf("Reading first line %s - What the hell is this?\n",buffer);
		SDL_RWclose(file);
		return NULL;
	}
	//Reading the first level specific tag
	spReadUntil(file,buffer,65536,'<');
	spReadUntil(file,buffer,65536,'>');
	if (strstr(buffer,"map"))
	{
		printf("Reading level map\n");
		//Parsing parameters of "map"
		char *begin = strchr(buffer,' ');
		while (1)
		{
			if (!begin)
				break;
			begin++; // sign after ' '
			char* end = strchr(begin,' ');
			if (end)
				end[0] = 0;
			else
				break;
			//the parameter is now in "begin"
			//we search only width and height
			if (strstr(begin,"width")==begin) //width at the BEGIN of the string!
			{
				char* number_begin = strchr(begin,'\"');
				number_begin++; //now the begin of the number
				width = atoi(number_begin);
				printf("Level width is %i\n",width);
			}
			else
			if (strstr(begin,"height")==begin) //height at the BEGIN of the string!
			{
				char* number_begin = strchr(begin,'\"');
				number_begin++; //now the begin of the number
				height = atoi(number_begin);
				printf("Level height is %i\n",height);
			}			
			begin = end;
		}
		//Now reading everything, which is between <map …> and </map>
		//Creating the layers and setting them to NULL.
		printf("Creating layers\n");
		allocLayer(&(level->layer.physic),width,height);
		allocLayer(&(level->layer.background),width,height);
		allocLayer(&(level->layer.player),width,height);
		allocLayer(&(level->layer.foreground),width,height);
		int end = 0;
		while (!end)
		{
			READ_TIL_TAG_BEGIN
			READ_TIL_TAG_END
			//possibilites are: properties, tileset, layer, objectgroup or /map
			if (strstr(buffer,"properties") == buffer)
			{
				//Reading the properties. Possibilites are: "background color"
				while (!end)
				{
					READ_TIL_TAG_BEGIN
					READ_TIL_TAG_END
					if (strstr(buffer,"/properties") == buffer)
						break;
					else
					if (strstr(buffer,"property") == buffer)
					{
						char* property = strstr(buffer,"name");
						property = strchr(property,'\"');
						property++;
						char* end_s = strchr(property,'\"');
						end_s[0] = 0;
						printf("Found the property \"%s\"\n",property);
						if (strcmp(property,"background color") == 0)
						{
							end_s++;
							char* value = strstr(end_s,"value");
							value = strstr(value,"#");
							end_s = strchr(value,'\"');
							end_s[0] = 0;
							level->backgroundColor = hex2color(value);
							printf("Set the background color to %s (%i)\n",value,level->backgroundColor);
						}
					}
				}
			}
			else
			if (strstr(buffer,"tileset") == buffer)
			{
				//Reading the parameters of the tag
				pTile_set newset = (pTile_set)malloc(sizeof(tTile_set));
				newset->next = tile_set;
				tile_set = newset;				
				//firstgid
				char* attribute = strstr(buffer,"firstgid");
				attribute = strchr(attribute,'\"');
				attribute++;
				char* end_s = strchr(attribute,'\"');
				end_s[0] = 0;				
				newset->firstgid = atoi(attribute);
				end_s[0] = '\"';
				//tilewidth
				attribute = strstr(buffer,"tilewidth");
				attribute = strchr(attribute,'\"');
				attribute++;
				end_s = strchr(attribute,'\"');
				end_s[0] = 0;				
				newset->tilewidth = atoi(attribute);
				end_s[0] = '\"';
				//tileheight
				attribute = strstr(buffer,"tileheight");
				attribute = strchr(attribute,'\"');
				attribute++;
				end_s = strchr(attribute,'\"');
				end_s[0] = 0;				
				newset->tileheight = atoi(attribute);
				
				newset->filename = NULL;
				//Reading the tilesets.
				while (!end)
				{
					READ_TIL_TAG_BEGIN
					READ_TIL_TAG_END
					if (strstr(buffer,"image") == buffer)
					{
						//filename (source)
						attribute = strstr(buffer,"source");
						attribute = strchr(attribute,'\"');
						attribute++;
						char* end_s = strchr(attribute,'\"');
						end_s[0] = 0;
						newset->filename = (char*)malloc(strlen(attribute)+1);
						sprintf(newset->filename,"%s",attribute);
						end_s[0] = '\"';
						//width
						attribute = strstr(buffer,"width");
						attribute = strchr(attribute,'\"');
						attribute++;
						end_s = strchr(attribute,'\"');
						end_s[0] = 0;
						newset->width = atoi(attribute);
						end_s[0] = '\"';
						//height
						attribute = strstr(buffer,"height");
						attribute = strchr(attribute,'\"');
						attribute++;
						end_s = strchr(attribute,'\"');
						end_s[0] = 0;
						newset->height = atoi(attribute);
						newset->lastgid = newset->firstgid+(newset->width/newset->tilewidth)*(newset->height/newset->tileheight)-1;
						printf("Added tileset from %i to %i with \"%s\"\n",newset->firstgid,newset->lastgid,newset->filename);
					}
					else
					if (strstr(buffer,"/tileset") == buffer)
						break;
				}				
			}
			else
			if (strstr(buffer,"layer") == buffer)
			{
				//Creating spriteLookUptable if not existing
				if (level->spriteTable == NULL)
				{
					level->spriteTable = (spSpritePointer*)malloc((tile_set->lastgid+1)*sizeof(spSpritePointer));
					memset(level->spriteTable,0,(tile_set->lastgid+1)*sizeof(spSpritePointer));
					level->spriteTableCount = tile_set->lastgid+1;
				}
				//Loading layer
				pLayer layer = NULL;
				char* layername = strstr(buffer,"name");
				layername = strchr(layername,'\"');
				layername++;
				char* end_s = strchr(layername,'\"');
				end_s[0] = 0;
				if (strstr(layername,"background"))
				{
					layer = &(level->layer.background);
					printf("Loading Background layer\n");
				}
				else
				if (strstr(layername,"player"))
				{
					layer = &(level->layer.player);
					printf("Loading Player layer\n");
				}
				else
				if (strstr(layername,"foreground"))
				{
					layer = &(level->layer.foreground);
					printf("Loading Foreground layer\n");
				}
				else
				if (strstr(layername,"collision") || strstr(layername,"physic"))
				{
					layer = &(level->layer.physic);
					printf("Loading Physic layer\n");
				}
				else
					printf("Unknown layer \"%s\". Will crash with segfault. ;-)\n",layername);
				//So, now we know the kind of the layer, time to load the "data"
				READ_TIL_TAG_BEGIN
				READ_TIL_TAG_END
				if (strstr(buffer,"data") == buffer)
				{
					char* encoding = strstr(buffer,"encoding");
					encoding = strchr(encoding,'\"');
					encoding++;
					char* end_s = strchr(encoding,'\"');
					end_s[0] = 0;
					if (strcmp(encoding,"csv"))
						printf("Expected csv-encoding. This will NOT work!\n");
				}
				else
					printf("Expected data-tag in layer, you know? Will Crash.\n");
				READ_TIL_TAG_BEGIN
				//Reading layer
				int i;
				char* next_number = buffer;
				for (i = 0; i < layer->width*layer->height && next_number; i++)
				{
					while (next_number[0] < '0' || next_number[0] > '9')
						next_number++;
					layer->tile[i].nr = atoi(next_number);
					if (layer == &(level->layer.physic))
						layer->tile[i].sprite = NULL;
					else
						layer->tile[i].sprite = get_sprite(layer->tile[i].nr,tile_set,level->spriteTable);
					next_number = strchr(next_number,',');
					
				}
				
				READ_TIL_TAG_END
				//endtag data
				if (strcmp(buffer,"/data"))
					printf("Expected </data> instead of <%s>\n",buffer);
				READ_TIL_TAG_BEGIN
				READ_TIL_TAG_END
				//endtag layer
				if (strcmp(buffer,"/layer"))
					printf("Expected </layer> instead of <%s>\n",buffer);
			}
			else
			if (strstr(buffer,"objectgroup") == buffer)
			{
				LevelObjectType defaultType = GROUP;
				char* name = strstr(buffer,"name");
				name = strchr(name,'\"');
				name++;
				char* end_s = strchr(name,'\"');
				end_s[0] = 0;				
				//Which type?
				if (strcmp(name,"player") == 0)
					defaultType = PLAYER;
				else
				if (strcmp(name,"bug") == 0)
					defaultType = BUG;
				else
				if (strcmp(name,"nega") == 0)
					defaultType = NEGA;
				else
				if (strcmp(name,"box") == 0)
					defaultType = BOX;
				else
				if (strcmp(name,"door") == 0)
					defaultType = DOOR;
				else
				if (strcmp(name,"trophies") == 0)
					defaultType = TROPHIES;
				else
				if (strcmp(name,"generic") == 0)
					defaultType = GENERIC;
				else
				if (strstr(name,"collectible") == name)
					defaultType = COLLECTIBLE;
				end_s[0] = '\"';
				printf("Loading objectgroup of kind %i\n",defaultType);
				//Creating the group.
				pLevelObjectGroup group = createGroup(&(level->firstObjectGroup),defaultType);
				//Reading the objects of the group
				while (!end)
				{
					READ_TIL_TAG_BEGIN
					READ_TIL_TAG_END
					if (strstr(buffer,"/objectgroup") == buffer)
						break;
					else
					if (strstr(buffer,"object") == buffer)
					{
						LevelObjectType objectType = defaultType;
						pLevelObject obj = createObject(group,objectType);
						
						//name (if exist)
						char* attribute = strstr(buffer,"name");
						if (attribute)
						{
							attribute = strchr(attribute,'\"');
							attribute++;
							char* end_s = strchr(attribute,'\"');
							end_s[0] = 0;
							if (defaultType == BUG) //BUG
							{
								if (strcmp(attribute,"right") == 0)
									obj->direction = RIGHT;
								else
								if (strcmp(attribute,"left") == 0)
									obj->direction = LEFT;
							}
							else
							if (defaultType == DOOR || defaultType == GENERIC) //DOOR, GENERIC
							{
								obj->some_char = (char*)malloc(strlen(attribute)+1);
								sprintf(obj->some_char,"%s",attribute);
							}
							if (defaultType == TROPHIES || defaultType == COLLECTIBLE) //TROPHIES, COLLECTIBLE
								obj->kind = attribute[0]-'0'; //works fine from 0 to 9. :)
							else
							if (defaultType == GROUP) //GROUP
							{
								if (strcmp(attribute,"switch") == 0)
									obj->type = SWITCH;
								else
								if (strcmp(attribute,"button") == 0)
									obj->type = BUTTON;
								else
								if (strcmp(attribute,"platform") == 0)
									obj->type = PLATFORM;								
							}
							end_s[0] = '\"';
						}
						printf("  Create object of type %i\n",obj->type);
						//x
						attribute = strstr(buffer,"x");
						attribute = strchr(attribute,'\"');
						attribute++;
						end_s = strchr(attribute,'\"');
						end_s[0] = 0;
						obj->x = atoi(attribute);
						end_s[0] = '\"';
						//y
						attribute = strstr(buffer,"y");
						attribute = strchr(attribute,'\"');
						attribute++;
						end_s = strchr(attribute,'\"');
						end_s[0] = 0;
						obj->y = atoi(attribute);
						//w (if exist)
						attribute = strstr(buffer,"width");
						if (attribute)
						{
							attribute = strchr(attribute,'\"');
							attribute++;
							obj->w = atoi(attribute);
						}
						//w (if exist)
						attribute = strstr(buffer,"height");
						if (attribute)
						{
							attribute = strchr(attribute,'\"');
							attribute++;
							obj->w = atoi(attribute);
						}
						
						char* temp;
						char buffer[256];
						SDL_Surface* surface;
						switch (obj->type) //Creating the animation
						{
							case PLAYER:
								obj->animation = spLoadSpriteCollection("./sprites/player.ssc",NULL);
								break; //finish me!
							case BUG:
								obj->animation = spLoadSpriteCollection("./sprites/enemy01.ssc",NULL);
								break; //finish me!
							case NEGA:
								obj->animation = spLoadSpriteCollection("./sprites/enemy02.ssc",NULL);
								break;
							case BOX:
								obj->animation = spLoadSpriteCollection("./sprites/box.ssc",NULL);
								break;
							case SWITCH:
								obj->animation = spLoadSpriteCollection("./sprites/switch.ssc",NULL);
								break; //finish me!
							case BUTTON:
								obj->animation = spLoadSpriteCollection("./sprites/button.ssc",NULL);
								break;
							case DOOR:
								obj->animation = spLoadSpriteCollection("./sprites/door.ssc",NULL);
								break; //finish me!
							case PLATFORM:
								printf("    Creating custom platform\n");
								obj->animation = spNewSpriteCollection();
								spAddSpriteToCollection(obj->animation,spNewSprite(NULL));
								surface = spCreateSurface(obj->w,obj->h);
								SDL_FillRect(surface,NULL,12345);
								spNewSubSpriteNoTiling(spActiveSprite(obj->animation),surface,1000);
								spDeleteSurface(surface); //For the ref counter
								break;
							case TROPHIES:
								obj->animation = spLoadSpriteCollection(name,NULL); 
								sprintf(buffer,"collectible0%i",obj->kind);
								spSelectSprite(obj->animation,"full");
								break;
							case GENERIC:
								temp = strchr(name,'@');
								if (temp)
								{
									temp[0] = 0;
									obj->animation = spLoadSpriteCollection(name,NULL);
									printf("    Creating animation %s",name);
									temp[0] = '@';
									spSelectSprite(obj->animation,&(temp[1]));
									printf(" with default=\"%s\"\n",&(temp[1]));
								}
								else
									obj->animation = spLoadSpriteCollection(name,NULL);
								break;
							case COLLECTIBLE:
								obj->animation = spLoadSpriteCollection(name,NULL); 
								sprintf(buffer,"%i",obj->kind);
								spSelectSprite(obj->animation,buffer);
								break;
						}
						
					}
				}					
			}
		}
		printf("Deleting temporary tile_set list\n");
		while (tile_set)
		{
			pTile_set next = tile_set->next;
			free(tile_set->filename);
			free(tile_set);
			tile_set = next;
		}
	}
	else
	{
		printf("Expected \"map …\", not \"%s\".\n",buffer);
		SDL_RWclose(file);
		return NULL;
	}
	level->actualCamera.x = width<<SP_ACCURACY-1;
	level->actualCamera.y = height<<SP_ACCURACY-1;
	level->targetCamera.x = width<<SP_ACCURACY-1;
	level->targetCamera.y = height<<SP_ACCURACY-1;
	SDL_RWclose(file);
	return level;
}

void drawLevel(pLevel level)
{
	spSetVerticalOrigin(SP_TOP);
	spSetHorizontalOrigin(SP_LEFT);
	int screenWidth = spGetWindowSurface()->w;
	int screenHeight = spGetWindowSurface()->h;
	int screenTileWidth  = screenWidth/32+2;
	int screenTileHeight = screenWidth/32+2;
	int screenTileBeginX = level->actualCamera.x >> SP_ACCURACY;
	int screenTileBeginY = level->actualCamera.y >> SP_ACCURACY;
	int startX = screenTileBeginX-screenTileWidth/2;
	int startY = screenTileBeginY-screenTileHeight/2;
	int endX = screenTileBeginX+screenTileWidth/2;
	int endY = screenTileBeginY+screenTileHeight/2;
	//layer
	int l;
	for (l = -3; l < 0; l++)
	{
		pLayer layer;
		switch (l)
		{
			case -3: layer = &(level->layer.background); break;
			case -2: layer = &(level->layer.player); break;
			case -1: layer = &(level->layer.foreground); break;
		}
		int x,y;
		for (x = startX; x < endX; x++)
			for (y = startY; y < endY; y++)
			{
				if (x < 0 || x >= layer->width)
					continue;
				if (y < 0 || y >= layer->height)
					continue;
				spSpritePointer sprite = layer->tile[x+y*layer->width].sprite;
				if (sprite == NULL)
					continue;
				int positionX = (x-screenTileBeginX)*32+screenWidth/2-((level->actualCamera.x >> SP_ACCURACY -5) & 31);
				int positionY = (y-screenTileBeginY)*32+screenHeight/2-((level->actualCamera.y >> SP_ACCURACY -5) & 31);
				spDrawSprite(positionX,positionY,l,sprite);
			}
	}
	
	spLine(screenWidth/2-5,screenHeight/2,-1,screenWidth/2+5,screenHeight/2,-1,0);
	spLine(screenWidth/2,screenHeight/2-5,-1,screenWidth/2,screenHeight/2+5,-1,0);
	int dx = level->targetCamera.x-level->actualCamera.x >> SP_ACCURACY - 5;
	int dy = level->targetCamera.y-level->actualCamera.y >> SP_ACCURACY - 5;
	spLine(screenWidth/2-5+dx,screenHeight/2+dy,-1,screenWidth/2+5+dx,screenHeight/2+dy,-1,0);
	spLine(screenWidth/2+dx,screenHeight/2-5+dy,-1,screenWidth/2+dx,screenHeight/2+5+dy,-1,12345);


	spSetVerticalOrigin(SP_CENTER);
	spSetHorizontalOrigin(SP_CENTER);	
}

void deleteLevel(pLevel level)
{
	if (level->layer.physic.tile)
		free(level->layer.physic.tile);
	if (level->layer.background.tile)
		free(level->layer.background.tile);
	if (level->layer.player.tile)
		free(level->layer.player.tile);
	if (level->layer.foreground.tile)
		free(level->layer.foreground.tile);
	int i;
	for (i = 0; i < level->spriteTableCount; i++)
		if (level->spriteTable[i])
			spDeleteSprite(level->spriteTable[i]);
	free(level->spriteTable);
	//Deleting all objects
	pLevelObjectGroup group = level->firstObjectGroup;
	if (group)
	do
	{
		pLevelObject obj = group->firstObject;
		if (obj)
		do
		{
			if (obj->some_char)
				free(obj->some_char);
			if (obj->animation)
				spDeleteSpriteCollection(obj->animation,0);
			pLevelObject next = obj->next;
			free(obj);
			obj = next;
		}
		while (obj != group->firstObject);
		pLevelObjectGroup next = group->next;
		free(group);
		group = next;
	}
	while (group != level->firstObjectGroup);
	free(level);
}

void calcCamera(pLevel level,Sint32 steps)
{
	int i;
	for (i = 0; i < steps; i++)
	{
		level->actualCamera.x += (level->targetCamera.x-level->actualCamera.x)>>7;
		level->actualCamera.y += (level->targetCamera.y-level->actualCamera.y)>>7;
	}
}
