#include "g_local.h"

void ThinkEndpoint(gentity_t *self)
{
    int i = 0;
    int count = 0;
	qboolean forceStopStatus = self->forceStop;
	qboolean playerStopped = qfalse;
	qboolean raceFinished = qfalse;
    vec3_t range = {self->horizontalRange, self->horizontalRange, self->verticalRange};
    vec3_t mins = {0, 0, 0};
    vec3_t maxs = {0, 0, 0};
    int entityList[MAX_GENTITIES];

    VectorSubtract( self->r.currentOrigin, range, mins );
    VectorAdd( self->r.currentOrigin, range, maxs );

    count = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

    for ( i = 0; i < count; i++ )
    {
        gentity_t *ent = NULL;
        ent = &g_entities[entityList[i]];

        if ( !ent->client )
        {
            continue;
        }

        if( ent->client->sess.racing )
        {
            int i = 0;
            int msec = 0;
            int sec = 0;
            int min = 0;
            int missedCP = 0;
            msec = level.time - ent->client->sess.raceStartTime;
            min = msec / 60000;
            msec = msec - min * 60000;
            sec = msec / 1000;
            msec = msec - sec * 1000;
            if(level.routeSettings.cpOrder == qtrue)
            {
                if(ent->client->sess.checkpointVisited[level.numCheckpoints - 1] == qtrue)
                {
					if(forceStopStatus)
					{
						if(!VectorLength(ent->client->ps.velocity) > 0)
							playerStopped = qtrue;

						if(playerStopped)
						{
							raceFinished = qtrue;
						}					
					}
					else
					{
						raceFinished = qtrue;
					}
                
                }
                else
                {
                    // Kind of messy but checks if client has visited all checkpoints. 
                    int i = 0;
                    int checkpointsVisited[MAX_CHECKPOINTS];
                    int checkpointsNotVisited = 0;
                    char buf[MAX_TOKEN_CHARS ] = "^5You skipped the following checkpoints: ";
                    for(; i < level.numCheckpoints; i++)
                    {
                        if(ent->client->sess.checkpointVisited[i] == qfalse)
                        {
                            checkpointsVisited[checkpointsNotVisited++] = i;
                        }
                    }

                    for(i = 0; i < checkpointsNotVisited; i++)
                    {
                        if(i == checkpointsNotVisited - 1)
                        {
                            Q_strcat(buf, sizeof(buf), va("%d", checkpointsVisited[i] + 1));
                        }
                        else
                        {
                            Q_strcat(buf, sizeof(buf), va("%d, ", checkpointsVisited[i] + 1));
                        }
                    }

                    G_PrintClientSpammyCenterPrint(ent->client->ps.clientNum,
                                                   va("%s", buf));
                }
            }
            else if(level.routeSettings.cpOrder == qfalse)
            {
                if(ent->client->sess.visitedCheckpoints == level.numCheckpoints)
                {
					if(forceStopStatus)
					{
						if(!VectorLength(ent->client->ps.velocity) > 0)
							playerStopped = qtrue;

						if(playerStopped)
							raceFinished = qtrue;					
					}
					else
					{
						raceFinished = qtrue;
					}
                }
                else
                {
                    missedCP = level.numCheckpoints - ent->client->sess.visitedCheckpoints;
                    CP(va("cp \"^7You missed %d ^5checkpoint(s).\n\"", missedCP)) ;
                }
            }
            else
            {
                return;
            }

			if(raceFinished)
			{
				trap_SendServerCommand(-1, va("cpm \"%s ^7reached the ^1end^7 in %02d:%02d:%03d.\n\"",
                                                  ent->client->pers.netname, min, sec, msec));
				ent->client->sess.racing = qfalse;
				// Reset powerups
				for(; i < NUM_SR_POWERUP_TYPES; i++)
				{
					ent->client->powerups[i] = 0;
				}
			}
        }
    }

    self->nextthink = FRAMETIME ;
}


void ThinkCheckpoint(gentity_t *self)
{
    int currentCP = 1;
    int i = 0;
    int count = 0;
    int visitedCP = 0;
	qboolean forceStopStatus = self->forceStopCp;
	qboolean playerStopped = qfalse;
	qboolean cpFinished = qfalse;
	qboolean validateCp = qfalse;
    vec3_t range = {self->horizontalRange, self->horizontalRange, self->verticalRange};
    vec3_t mins = {0, 0, 0};
    vec3_t maxs = {0, 0, 0};
    int entityList[MAX_GENTITIES];

    VectorSubtract( self->r.currentOrigin, range, mins );
    VectorAdd( self->r.currentOrigin, range, maxs );

    count = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);


    for ( i = 0; i < count; i++ )
    {
        int msec = 0;
        int sec = 0;
        int min = 0;
        gentity_t *ent = NULL;
        ent = &g_entities[entityList[i]];

        if ( !ent->client )
        {
            continue;
        }


        if( ent->client->sess.racing && !ent->client->sess.checkpointVisited[self->position] )
        {
            msec = level.time - ent->client->sess.raceStartTime;
            min = msec / 60000;
            msec = msec - min * 60000;
            sec = msec / 1000;
            msec = msec - sec * 1000;
            currentCP = self->position;
			
            if(level.routeSettings.cpOrder)
            {
                if(currentCP != 0)
                {
                    if(ent->client->sess.checkpointVisited[currentCP - 1])
                    {
						cpFinished = qtrue;
                    }
                    else
                    {
                        CP("cp \"^7You missed a ^3checkpoint^7, go back and visit it before visiting this one.\n\"") ;
                    }
                }

                else if(currentCP == 0)
                {
                    cpFinished = qtrue;
                }
            }
            else
            {
                cpFinished = qtrue;
			}

			if(cpFinished)
			{
				if(forceStopStatus)
				{
					if(!VectorLength(ent->client->ps.velocity) > 0)
					{
						playerStopped = qtrue;
					}
					else
					{
						return;
					}
					if(playerStopped)
					{
						validateCp = qtrue;
					}
					else
					{
						return;
					}
				}
				else
				{
					validateCp = qtrue;
				}
			}

			if(validateCp)
			{
				trap_SendServerCommand(-1, va("cpm \"%s ^7reached ^3checkpoint ^7%d in %02d:%02d:%03d.\n\"", ent->client->pers.netname, self->position + 1,
											min, sec, msec));
				ent->client->sess.checkpointVisited[self->position] = qtrue;
			}
        }			
    }
    self->nextthink = FRAMETIME ;
}

#define RANGE_VERYSMALL 30
#define RANGE_SMALL 100
#define RANGE_MEDIUM 300
#define RANGE_LARGE 500

void RouteMakerCheckpointsOrder( gentity_t *ent, char *getValue )
{
    int state = atoi(getValue);
    if(state == 1)
    {
        level.routeSettings.cpOrder = qtrue;
        CP("cp \"^7Checkpoint order: ^2On\n\"") ;
    }
    else if (state == 0)
    {
        level.routeSettings.cpOrder = qfalse;
        CP("cp \"^7Checkpoint order: ^1Off\n\"") ;
    }
    else
    {
        CP("print \"^7Invalid usage. CPOrder : 1(true) or 0(false)") ;
        level.routeSettings.cpOrder = qtrue;
    }
}

void RouteMakerCheckpoints( gentity_t * ent )
{
    gentity_t *checkpoint = NULL;

    if(level.numCheckpoints == MAX_CHECKPOINTS)
    {
        CP("print \"^1error: ^7maximum number of checkpoints already on map.\n\"") ;
        return;
    }

    checkpoint = G_Spawn();
    checkpoint->classname = "route_cp";
    checkpoint->position = level.numCheckpoints;
    checkpoint->think = ThinkCheckpoint;
    checkpoint->nextthink = level.time + FRAMETIME ;
    // Experimentary
    checkpoint->s.eType = ET_ITEM;
    checkpoint->item = BG_FindItemForPowerup( ROUTE_CHECKPOINT );
    checkpoint->s.modelindex = checkpoint->item - bg_itemlist;
    checkpoint->s.otherEntityNum2 = 1;
    if(trap_Argc() == 2)
    {
        checkpoint->horizontalRange = sr_defaultAreaRange.integer;
        checkpoint->verticalRange = sr_defaultAreaRange.integer;
    }
    else if(trap_Argc() >= 3)
    {
        char rangeStr[MAX_TOKEN_CHARS ] = "\0";
        int range = 0;
        trap_Argv(2, rangeStr, sizeof(rangeStr));
        range = atoi(rangeStr);

        if(range)
        {
            checkpoint->horizontalRange = range;
            checkpoint->verticalRange = range;
        }
        else if(!Q_stricmp(rangeStr, "verysmall"))
        {
            checkpoint->horizontalRange = RANGE_VERYSMALL;
            checkpoint->verticalRange = RANGE_VERYSMALL;
        }
        else if(!Q_stricmp(rangeStr, "small"))
        {
            checkpoint->horizontalRange = RANGE_SMALL;
            checkpoint->verticalRange = RANGE_SMALL;
        }
        else if(!Q_stricmp(rangeStr, "medium"))
        {
            checkpoint->horizontalRange = RANGE_MEDIUM;
            checkpoint->verticalRange = RANGE_MEDIUM;
        }
        else if(!Q_stricmp(rangeStr, "large"))
        {
            checkpoint->horizontalRange = RANGE_LARGE;
            checkpoint->verticalRange = RANGE_LARGE;
        }
        else
        {
            checkpoint->horizontalRange = sr_defaultAreaRange.integer;
            checkpoint->verticalRange = sr_defaultAreaRange.integer;
        }
    }
	/*
    else if(trap_Argc() == 4)
    {
        char horizontalRangeStr[MAX_TOKEN_CHARS ] = "\0";
        char verticalRangeStr[MAX_TOKEN_CHARS ] = "\0";

        int horizontalRange = atoi(horizontalRangeStr);
        int verticalRange = atoi(verticalRangeStr);

        trap_Argv(2, horizontalRangeStr, sizeof(horizontalRangeStr));
        trap_Argv(3, verticalRangeStr, sizeof(verticalRangeStr));

        horizontalRange = atoi(horizontalRangeStr);
        verticalRange = atoi(verticalRangeStr);

        if(horizontalRange)
        {
            checkpoint->horizontalRange = horizontalRange;
        }
        else
        {
            checkpoint->horizontalRange = sr_defaultAreaRange.integer;
        }

        if(verticalRange)
        {
            checkpoint->verticalRange = verticalRange;
        }
        else
        {
            checkpoint->verticalRange = sr_defaultAreaRange.integer;
        }
        CP(va("cp \"^7Added a ^3checkpoint ^7(%d, %d)\n\"", checkpoint->horizontalRange, checkpoint->verticalRange)) ;
    }
	*/

	if(trap_Argc() == 4)
    {
        //ForceStop
		char forceStopInputStr[MAX_TOKEN_CHARS ] = "\0";
		trap_Argv(3, forceStopInputStr, sizeof(forceStopInputStr));
		if(!Q_stricmp(forceStopInputStr, "1"))
		{
			checkpoint->forceStopCp = qtrue;
		}
		else if(!Q_stricmp(forceStopInputStr, "0"))
		{
			checkpoint->forceStopCp = qfalse;
		}
		else
		{

		}
    }

    G_SetOrigin(checkpoint, ent->r.currentOrigin);
    G_SetAngle(checkpoint, ent->r.currentAngles);

    trap_LinkEntity(checkpoint);
    //     VectorCopy( ent->r.currentOrigin, checkpoint->r.currentOrigin );
    //     VectorCopy( ent->client->ps.viewangles, checkpoint->r.currentAngles );
    level.checkpoints[level.numCheckpoints] = checkpoint;
    level.numCheckpoints++;
    CP(va("cp \"^7Added a ^3checkpoint ^7(%d, %d).\n\"", checkpoint->horizontalRange, checkpoint->verticalRange)) ;
}

void RouteMakerBegin( gentity_t * ent )
{
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS ];
    qboolean keepRoute = qfalse;
    gentity_t *begin = NULL;
    // gitem_t *item = BG_FindItemForClassName("route_begin");

    if(level.routeBegin != NULL)
    {
        G_FreeEntity(level.routeBegin);
        level.routeBegin = NULL;
    }

    if(argc == 3)
    {
        trap_Argv(2, arg, sizeof(arg));
        if(!Q_stricmp(arg, "-keep"))
        {
            keepRoute = qtrue;
        }
    }

    if(keepRoute)
    {
        CP("cp \"^7Added a ^2start ^7spot.\n\"") ;
    }
    else
    {
        ClearRoute();
        CP("cp \"^7Added a ^2start ^7spot and cleared the route.\n\"") ;
        CP("print \"^7If you do not wish to clear the route when you define a start spot, do /route begin -keep\n\"") ;
    }

    begin = G_Spawn();
    begin->classname = "route_begin";
    // Experimentary
    begin->s.eType = ET_ITEM;
    begin->item = BG_FindItemForPowerup( ROUTE_STARTPOINT );
    begin->s.modelindex = begin->item - bg_itemlist;
    begin->s.otherEntityNum2 = 1;
    G_SetOrigin(begin, ent->r.currentOrigin);
    G_SetAngle(begin, ent->client->ps.viewangles);
    trap_LinkEntity(begin);

    level.routeBegin = begin;
}

void RouteMakerEnd( gentity_t *ent )
{
    gentity_t *end = NULL;
    int argc = trap_Argc();

    if(level.routeEnd != NULL)
    {
        G_FreeEntity(level.routeEnd);
        level.routeEnd = NULL;
    }

    end = G_Spawn();
    end->classname = "route_end";
    end->think = ThinkEndpoint;
    end->nextthink = level.time + FRAMETIME ;

    end->s.eType = ET_ITEM;
    end->item = BG_FindItemForPowerup( ROUTE_ENDPOINT );
    end->s.modelindex = end->item - bg_itemlist;
    end->s.otherEntityNum2 = 1;
    G_SetOrigin(end, ent->r.currentOrigin);
    G_SetAngle(end, ent->client->ps.viewangles);
    trap_LinkEntity(end);
    level.routeEnd = end;


    // route end [x-y] [z]
    if(argc == 2)
    {
        // use default range == 300
        end->horizontalRange = sr_defaultAreaRange.integer;
        end->verticalRange = sr_defaultAreaRange.integer;
    }
    else if(argc >= 3)
    {
        char rangeStr[MAX_TOKEN_CHARS ] = "\0";
        int range = 0;
        trap_Argv(2, rangeStr, sizeof(rangeStr));
        range = atoi(rangeStr);

        if(range)
        {
            end->horizontalRange = range;
            end->verticalRange = range;
        }
        else if(!Q_stricmp(rangeStr, "verysmall"))
        {
            end->horizontalRange = RANGE_VERYSMALL;
            end->verticalRange = RANGE_VERYSMALL;
        }
        else if(!Q_stricmp(rangeStr, "small"))
        {
            end->horizontalRange = RANGE_SMALL;
            end->verticalRange = RANGE_SMALL;
        }
        else if(!Q_stricmp(rangeStr, "medium"))
        {
            end->horizontalRange = RANGE_MEDIUM;
            end->verticalRange = RANGE_MEDIUM;
        }
        else if(!Q_stricmp(rangeStr, "large"))
        {
            end->horizontalRange = RANGE_LARGE;
            end->verticalRange = RANGE_LARGE;
        }
        else
        {
			/* Vallz: A little design flaw, I'll fix later.
            char horizontalRangeStr[MAX_TOKEN_CHARS ] = "\0";
			char verticalRangeStr[MAX_TOKEN_CHARS ] = "\0";

			int horizontalRange = atoi(horizontalRangeStr);
			int verticalRange = atoi(verticalRangeStr);

			trap_Argv(2, horizontalRangeStr, sizeof(horizontalRangeStr));
			trap_Argv(3, verticalRangeStr, sizeof(verticalRangeStr));

			horizontalRange = atoi(horizontalRangeStr);
			verticalRange = atoi(verticalRangeStr);

			if(horizontalRange)
			{
				end->horizontalRange = horizontalRange;
			}
			else
			{
				end->horizontalRange = sr_defaultAreaRange.integer;
			}

			if(verticalRange)
			{
				end->verticalRange = verticalRange;
			}
			else
			{
				end->verticalRange = sr_defaultAreaRange.integer;
			}
			}
			*/
		}
	}
	if(argc == 4)
    {
        //ForceStop
		char forceStopInputStr[MAX_TOKEN_CHARS ] = "\0";
		trap_Argv(3, forceStopInputStr, sizeof(forceStopInputStr));
		if(!Q_stricmp(forceStopInputStr, "1"))
		{
			end->forceStop = qtrue;
		}
		else if(!Q_stricmp(forceStopInputStr, "0"))
		{
			end->forceStop = qfalse;
		}
		else
		{

		}
    }
	
    CP(va("cp \"^7Added an ^1end ^7spot (%d, %d)\n\"", end->horizontalRange, end->verticalRange)) ;
}

void RouteMakerClear( gentity_t * ent )
{
    int i = 0;

	if(level.raceIsStarting)
	{
		AP("cpm \"^8SR^7: can't clear route while race is starting.\n\"") ;
		return;
	}

    if(level.routeBegin != NULL)
    {
        G_FreeEntity(level.routeBegin);
        level.routeBegin = NULL;
    }
    if(level.routeEnd != NULL)
    {
        G_FreeEntity(level.routeEnd);
        level.routeEnd = NULL;
    }
    for(; i < MAX_CHECKPOINTS; i++)
    {
        if(level.checkpoints[i])
        {
            G_FreeEntity(level.checkpoints[i]);
            level.checkpoints[i] = NULL;
        }
    }
    level.numCheckpoints = 0;
    ResetRacingState();
    ClearPowerups();
    AP("cpm \"^8SR^7: route has been cleared.\n\"") ;
}

void RouteMakerClearCP( gentity_t * ent )
{
    int i = 0;
	if(level.raceIsStarting)
	{
		AP("cpm \"^8SR^7: can't clear checkpoints while race is starting.\n\"") ;
		return;
	}
    for(; i < MAX_CHECKPOINTS; i++)
    {
        if(level.checkpoints[i])
        {
            G_FreeEntity(level.checkpoints[i]);
            level.checkpoints[i] = NULL;
            level.numCheckpoints = 0;
        }
    }
}

void RouteMakerClearPW( gentity_t * ent )
{
	if(level.raceIsStarting)
	{
		AP("cpm \"^8SR^7: can't clear powerups while race is starting.\n\"") ;
		return;
	}
    ClearPowerups();
    trap_SendServerCommand(ent->client->ps.clientNum,
                           "print \"^8SR^7: deleted all powerups\n\"");
}

void Cmd_Route_f( gentity_t * ent )
{
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS ] = "\0";
    if( !ent->client->sess.routeMaker )
    {
        CP("cp \"^7You need to be a route maker to create routes.\n\"") ;
        return;
    }

    // /route begin
    // /route end
    // /route clear
    // 

    if(argc < 2)
    {
        CP("print \"usage: /route [begin|end|cp|clear|clearcp|clearpw]\n\"") ;
        return;
    }

    trap_Argv(1, arg, sizeof(arg));

    if(!Q_stricmp(arg, "begin"))
    {
        RouteMakerBegin(ent);
    }
    else if(!Q_stricmp(arg, "end"))
    {
        RouteMakerEnd(ent);
    }
    else if(!Q_stricmp(arg, "clear"))
    {
        RouteMakerClear(ent);
    }
    else if(!Q_stricmp(arg, "clearcp"))
    {
        RouteMakerClearCP(ent);
    }
    else if (!Q_stricmp(arg, "clearpw"))
    {
        RouteMakerClearPW(ent);
    }
    else if(!Q_stricmp(arg, "cp"))
    {
        RouteMakerCheckpoints( ent );
    }
    else if(!Q_stricmp(arg, "cporder"))
    {
        trap_Argv(2, arg, sizeof(arg));
        RouteMakerCheckpointsOrder(ent, arg);
    }
    else
    {
        CP("print \"usage: /route [begin|end|cp|clear|clearcp|clearpw]\n\"") ;
        return;
    }
}

void Cmd_ShowRoute_f( gentity_t * ent )
{
    int travelTime;
    char arg[MAX_TOKEN_CHARS ] = "\0";

    if(ent->client->sess.racing)
    {
        return;
    }

    ent->client->sess.timeBetweenRouteSpotsMS = 1000;

    if(trap_Argc() == 1)
    {
        ent->client->sess.timeBetweenRouteSpotsSec = 1;
    }
    if(trap_Argc() == 2)
    {
        trap_Argv(1, arg, sizeof(arg));
        ent->client->sess.timeBetweenRouteSpotsSec = atoi(arg);
        ent->client->sess.timeBetweenRouteSpotsMS = ent->client->sess.timeBetweenRouteSpotsSec * 1000;
        if(ent->client->sess.timeBetweenRouteSpotsMS < 1000)
        {
            ent->client->sess.timeBetweenRouteSpotsMS = 1000;
        }
        ent->client->sess.lastRouteSpotTime = 0;
    }
    travelTime = ent->client->sess.timeBetweenRouteSpotsSec * (level.numCheckpoints + 2);
    CP(va("cp \"^8SR^7:Estimated time to show route: ^2%d ^7seconds\n\"", travelTime)) ;
    ent->client->sess.nextCp = -1;
    ent->client->sess.showingRoute = qtrue;
}

void Cmd_RestartRun_f( gentity_t * ent )
{
    int i;
    if(level.numCheckpoints > 0)
    {
        if(level.routeSettings.cpOrder == qfalse)
        {
            ent->client->sess.visitedCheckpoints = 0;
        }

        //Resetting CPS
        for(i = 0; i < level.numCheckpoints; i++)
        {
            ent->client->sess.checkpointVisited[i] = qfalse;
        }
        //Timers
        if(ent->client->sess.raceStartTime > 0)
        {
            ent->client->sess.raceStartTime = level.time + 10;
        }

		//If he has already ended the run, put him to racing status again, if there's an endpoint.
		if(!ent->client->sess.racing)
        {
            ent->client->sess.racing = qtrue;
        }

        //Teleporting to beginning
        ent->client->ps.eFlags ^= EF_TELEPORT_BIT ;
        VectorCopy(level.routeBegin->r.currentOrigin, ent->client->ps.origin);
        SetClientViewAngle(ent, level.routeBegin->r.currentAngles);

        //Give message to server that he reset his run
        trap_SendServerCommand(-1, va("cpm \"^8SR: %s^8 ^7 has restarted his run.", ent->client->pers.netname));
        CP("cp \"^8SR^7: Your run has been successfully restarted.\n\"") ;
    }
}

void Cmd_StopShowRoute_f( gentity_t * ent )
{
    ent->client->sess.showingRoute = qfalse;
    ent->client->sess.nextCp = MAX_CHECKPOINTS + 1;
}


// From etpub
void WriteString(const char *s, fileHandle_t f)
{
    char buf[MAX_STRING_CHARS ];

    buf[0] = '\0';
    if(s[0])
    {
        //Q_strcat(buf, sizeof(buf), s);
        Q_strncpyz(buf, s, sizeof(buf));
        trap_FS_Write(buf, strlen(buf), f);
    }
    trap_FS_Write("\n", 1, f);
}

void WriteInt(int v, fileHandle_t f)
{
    char buf[32];

    Com_sprintf(buf, 32, "%d", v);
    //sprintf(buf, "%d", v);
    if(buf[0]) trap_FS_Write(buf, strlen(buf), f);
    trap_FS_Write("\n", 1, f);
}

void WriteFloat(float v, fileHandle_t f)
{
    char buf[32];

    Com_sprintf(buf, 32, "%f", v);
    //sprintf(buf, "%f", v);
    if(buf[0]) trap_FS_Write(buf, strlen(buf), f);
    trap_FS_Write("\n", 1, f);
}

qboolean SaveRoute( const char *routeName )
{
    fileHandle_t f = 0;
    int i = 0;
    // TODO: .route, else people could write over qagame..
    int len = trap_FS_FOpenFile(routeName, &f, FS_WRITE);

    if(len < 0)
    {
        G_Printf("Couldn't open file \"%s\" to save a race route.\n",
                 routeName);
        return qfalse;
    }
    else if(len > 0)
    {
        G_Printf("Route with name \"%s\" exists.", routeName);
        return qfalse;
    }

    // Write startpoint
    if(!level.routeBegin || !level.routeEnd ||
        !level.numCheckpoints)
    {
        G_Printf("No begin, end or checkpoints defined\n");
        return qfalse;
    }
    // Location and angles is all we need
    trap_FS_Write("// ", 3, f);
    WriteString(routeName, f);

    trap_FS_Write("[route]\n", 8, f);
    trap_FS_Write("map = ", 6, f);
    trap_FS_Write("\n", 1, f);
    trap_FS_Write("cpOrder = ", 10, f);
    WriteInt(level.routeSettings.cpOrder, f);


    trap_FS_Write("[begin]\n", 8, f);

    // Position
    trap_FS_Write("posx = ", 7, f);
    WriteFloat(level.routeBegin->r.currentOrigin[0], f);

    trap_FS_Write("posy = ", 7, f);
    WriteFloat(level.routeBegin->r.currentOrigin[1], f);

    trap_FS_Write("posz = ", 7, f);
    WriteFloat(level.routeBegin->r.currentOrigin[2], f);

    // Angles
    trap_FS_Write("angx = ", 7, f);
    WriteFloat(level.routeBegin->r.currentAngles[0], f);

    trap_FS_Write("angy = ", 7, f);
    WriteFloat(level.routeBegin->r.currentAngles[1], f);

    trap_FS_Write("angz = ", 7, f);
    WriteFloat(level.routeBegin->r.currentAngles[2], f);

    trap_FS_Write("\n", 1, f);

    // Endpoint, we need pos, angles and size
    trap_FS_Write("[end]\n", 6, f);

    // Position
    trap_FS_Write("posx = ", 7, f);
    WriteFloat(level.routeEnd->r.currentOrigin[0], f);

    trap_FS_Write("posy = ", 7, f);
    WriteFloat(level.routeEnd->r.currentOrigin[1], f);

    trap_FS_Write("posz = ", 7, f);
    WriteFloat(level.routeEnd->r.currentOrigin[2], f);

    // Angles
    trap_FS_Write("angx = ", 7, f);
    WriteFloat(level.routeEnd->r.currentAngles[0], f);

    trap_FS_Write("angy = ", 7, f);
    WriteFloat(level.routeEnd->r.currentAngles[1], f);

    trap_FS_Write("angz = ", 7, f);
    WriteFloat(level.routeEnd->r.currentAngles[2], f);

    // Size
    trap_FS_Write("horizontal = ", 13, f);
    WriteInt(level.routeEnd->horizontalRange, f);

    trap_FS_Write("vertical = ", 11, f);
    WriteInt(level.routeEnd->verticalRange, f);

    trap_FS_Write("\n", 1, f);

    for(i = 0; i < level.numCheckpoints; i++)
    {
        // We need position and size
        trap_FS_Write("[cp]\n", 5, f);

        // Position
        trap_FS_Write("posx = ", 7, f);
        WriteFloat(level.checkpoints[i]->r.currentOrigin[0], f);

        trap_FS_Write("posy = ", 7, f);
        WriteFloat(level.checkpoints[i]->r.currentOrigin[1], f);

        trap_FS_Write("posz = ", 7, f);
        WriteFloat(level.checkpoints[i]->r.currentOrigin[2], f);

        // Angles
        trap_FS_Write("angx = ", 7, f);
        WriteFloat(level.checkpoints[i]->r.currentAngles[0], f);

        trap_FS_Write("angy = ", 7, f);
        WriteFloat(level.checkpoints[i]->r.currentAngles[1], f);

        trap_FS_Write("angz = ", 7, f);
        WriteFloat(level.checkpoints[i]->r.currentAngles[2], f);

        // Size
        trap_FS_Write("horizontal = ", 13, f);
        WriteInt(level.checkpoints[i]->horizontalRange, f);

        trap_FS_Write("vertical = ", 11, f);
        WriteInt(level.checkpoints[i]->verticalRange, f);

        trap_FS_Write("\n", 1, f);
    }

    for(i = 0; i < level.numPowerups; i++)
    {
        // We need position and type
        trap_FS_Write("[pw]\n", 5, f);

        // Position
        trap_FS_Write("posx = ", 7, f);
        WriteFloat(level.powerups[i]->r.currentOrigin[0], f);

        trap_FS_Write("posy = ", 7, f);
        WriteFloat(level.powerups[i]->r.currentOrigin[1], f);

        trap_FS_Write("posz = ", 7, f);
        WriteFloat(level.powerups[i]->r.currentOrigin[2], f);

        trap_FS_Write("type = ", 7, f);
        WriteInt(level.powerups[i]->powerupType, f);

        trap_FS_Write("\n", 1, f);
    }

    trap_FS_FCloseFile(f);

    G_Printf("Saved route %s\n", routeName);
    AP(va("cpm \"^8SR^7: Saved route %s\n\"", routeName)) ;
    return qtrue;
}


// From etpub
void ReadString(char **cnf, char *s, int size)
{
    char *t;

    //COM_MatchToken(cnf, "=");
    t = COM_ParseExt(cnf, qfalse);
    if(!strcmp(t, "="))
    {
        t = COM_ParseExt(cnf, qfalse);
    }
    else
    {
        G_Printf("readconfig: warning missing = before "
                 "\"%s\" on line %d\n",
                 t,
                 COM_GetCurrentParseLine());
    }
    s[0] = '\0';
    while(t[0])
    {
        if((s[0] == '\0' && strlen(t) <= size) ||
            (strlen(t)+strlen(s) < size))
        {
            Q_strcat(s, size, t);
            Q_strcat(s, size, " ");
        }
        t = COM_ParseExt(cnf, qfalse);
    }
    // trim the trailing space
    if(strlen(s) > 0 && s[strlen(s)-1] == ' ')
        s[strlen(s)-1] = '\0';
}

void ReadInt(char **cnf, int *v)
{
    char *t;

    //COM_MatchToken(cnf, "=");
    t = COM_ParseExt(cnf, qfalse);
    if(!strcmp(t, "="))
    {
        t = COM_ParseExt(cnf, qfalse);
    }
    else
    {
        G_Printf("readconfig: warning missing = before "
                 "\"%s\" on line %d\n",
                 t,
                 COM_GetCurrentParseLine());
    }
    *v = atoi(t);
}

void ReadFloat(char **cnf, float *v)
{
    char *t;

    //COM_MatchToken(cnf, "=");
    t = COM_ParseExt(cnf, qfalse);
    if(!strcmp(t, "="))
    {
        t = COM_ParseExt(cnf, qfalse);
    }
    else
    {
        G_Printf("readconfig: warning missing = before "
                 "\"%s\" on line %d\n",
                 t,
                 COM_GetCurrentParseLine());
    }
    *v = atof(t);
}

void ThinkEndpoint(gentity_t *self);
void ThinkCheckpoint(gentity_t *self);

qboolean LoadRoute( const char *routeName )
{
    fileHandle_t f = 0;

    char *cnf = NULL, *cnf2 = NULL;
    char *t = NULL;
    qboolean routeOpen = qfalse;
    qboolean beginOpen = qfalse;
    qboolean endOpen = qfalse;
    qboolean cpOpen = qfalse;
    qboolean pwOpen = qfalse;
    gentity_t *begin = NULL;
    gentity_t *end = NULL;
    gentity_t *checkpoint = NULL;
    gentity_t *powerup = NULL;

    int len = trap_FS_FOpenFile(routeName, &f, FS_READ);
    if(len < 0)
    {
        G_Printf("Couldn't find route \"%s\".\n", routeName);
        return qfalse;
    }

    cnf = malloc(len + 1);
    cnf2 = cnf;

    trap_FS_Read(cnf, len, f);
    cnf[len] = 0;
    trap_FS_FCloseFile(f);

    ClearRoute();

    COM_BeginParseSession( "route" );

    t = COM_Parse(&cnf);
    while(*t)
    {
        if( !Q_stricmp(t, "[route]") ||
            !Q_stricmp(t, "[begin]") ||
            !Q_stricmp(t, "[end]") ||
            !Q_stricmp(t, "[cp]") ||
            !Q_stricmp(t, "[pw]" ) )
        {
            if(routeOpen)
            {
                
            } else if(beginOpen)
            {
                level.routeBegin = begin;
                G_SetOrigin(begin, begin->r.currentOrigin);
                G_SetAngle(begin, begin->r.currentAngles);
                trap_LinkEntity(begin);
            }
            else if(endOpen)
            {
                level.routeEnd = end;
                G_SetOrigin(end, end->r.currentOrigin);
                G_SetAngle(end, end->r.currentAngles);
                trap_LinkEntity(end);
            }
            else if(cpOpen)
            {
                level.checkpoints[level.numCheckpoints++] = checkpoint;
                G_SetOrigin(checkpoint, checkpoint->r.currentOrigin);
                G_SetAngle(checkpoint, checkpoint->r.currentAngles);

                trap_LinkEntity(checkpoint);
            }
            else if(pwOpen)
            {
                G_SetOrigin(powerup, powerup->r.currentOrigin);
                powerup->powerupModelType = NUM_SR_POWERUP_TYPES;
                CreatePowerupSpawner( powerup );
            }
            routeOpen = beginOpen = endOpen = cpOpen = pwOpen = qfalse;
        }

        if(routeOpen)
        {
            if(!Q_stricmp(t, "map"))
            {

            }
            else if(!Q_stricmp(t, "knockback"))
            {
            }
            else if(!Q_stricmp(t, "speed"))
            {
            }
            else if(!Q_stricmp(t, "gravity"))
            {
            }
            else if(!Q_stricmp(t, "cpOrder"))
            {
                ReadInt(&cnf, &level.routeSettings.cpOrder);
            }
            else
            {
                G_Printf("route: parse error near %s on line %d\n",
                         t, COM_GetCurrentParseLine());
            }
        }
        else if(beginOpen)
        {
            if(!Q_stricmp(t, "posx"))
            {
                ReadFloat(&cnf, &begin->r.currentOrigin[0]);
            }
            else if(!Q_stricmp(t, "posy"))
            {
                ReadFloat(&cnf, &begin->r.currentOrigin[1]);
            }
            else if(!Q_stricmp(t, "posz"))
            {
                ReadFloat(&cnf, &begin->r.currentOrigin[2]);
            }
            else if(!Q_stricmp(t, "angx"))
            {
                ReadFloat(&cnf, &begin->r.currentAngles[0]);
            }
            else if(!Q_stricmp(t, "angy"))
            {
                ReadFloat(&cnf, &begin->r.currentAngles[1]);
            }
            else if(!Q_stricmp(t, "angz"))
            {
                ReadFloat(&cnf, &begin->r.currentAngles[2]);
            }
            else
            {
                G_Printf("route: parse error near %s on line %d\n",
                         t, COM_GetCurrentParseLine());
            }
        }
        else if(endOpen)
        {
            if(!Q_stricmp(t, "posx"))
            {
                ReadFloat(&cnf, &end->r.currentOrigin[0]);
            }
            else if(!Q_stricmp(t, "posy"))
            {
                ReadFloat(&cnf, &end->r.currentOrigin[1]);
            }
            else if(!Q_stricmp(t, "posz"))
            {
                ReadFloat(&cnf, &end->r.currentOrigin[2]);
            }
            else if(!Q_stricmp(t, "angx"))
            {
                ReadFloat(&cnf, &end->r.currentAngles[0]);
            }
            else if(!Q_stricmp(t, "angy"))
            {
                ReadFloat(&cnf, &end->r.currentAngles[1]);
            }
            else if(!Q_stricmp(t, "angz"))
            {
                ReadFloat(&cnf, &end->r.currentAngles[2]);
            }
            else if(!Q_stricmp(t, "horizontal"))
            {
                ReadInt(&cnf, &end->horizontalRange);
            }
            else if(!Q_stricmp(t, "vertical"))
            {
                ReadInt(&cnf, &end->verticalRange);
            }
            else
            {
                G_Printf("route: parse error near %s on line %d\n",
                         t, COM_GetCurrentParseLine());
            }
        }
        else if(cpOpen)
        {
            if(!Q_stricmp(t, "posx"))
            {
                ReadFloat(&cnf, &checkpoint->r.currentOrigin[0]);
            }
            else if(!Q_stricmp(t, "posy"))
            {
                ReadFloat(&cnf, &checkpoint->r.currentOrigin[1]);
            }
            else if(!Q_stricmp(t, "posz"))
            {
                ReadFloat(&cnf, &checkpoint->r.currentOrigin[2]);
            }
            else if(!Q_stricmp(t, "angx"))
            {
                ReadFloat(&cnf, &checkpoint->r.currentAngles[0]);
            }
            else if(!Q_stricmp(t, "angy"))
            {
                ReadFloat(&cnf, &checkpoint->r.currentAngles[1]);
            }
            else if(!Q_stricmp(t, "angz"))
            {
                ReadFloat(&cnf, &checkpoint->r.currentAngles[2]);
            }
            else if(!Q_stricmp(t, "horizontal"))
            {
                ReadInt(&cnf, &checkpoint->horizontalRange);
            }
            else if(!Q_stricmp(t, "vertical"))
            {
                ReadInt(&cnf, &checkpoint->verticalRange);
            }
            else
            {
                G_Printf("route: parse error near %s on line %d\n",
                         t, COM_GetCurrentParseLine());
            }
        }
        else if(pwOpen)
        {
            if(!Q_stricmp(t, "posx"))
            {
                ReadFloat(&cnf, &powerup->r.currentOrigin[0]);
            }
            else if(!Q_stricmp(t, "posy"))
            {
                ReadFloat(&cnf, &powerup->r.currentOrigin[1]);
            }
            else if(!Q_stricmp(t, "posz"))
            {
                ReadFloat(&cnf, &powerup->r.currentOrigin[2]);
            }
            else if(!Q_stricmp(t, "type"))
            {
                ReadInt(&cnf, (int*)(&powerup->powerupType));
            }
        }

        if(!Q_stricmp(t, "[route]"))
        {
            routeOpen = qtrue;
        }
        else if(!Q_stricmp(t, "[begin]"))
        {
            begin = G_Spawn();
            begin->classname = "route_begin";
            begin->s.eType = ET_ITEM;
            begin->item = BG_FindItemForPowerup( ROUTE_STARTPOINT );
            begin->s.modelindex = begin->item - bg_itemlist;
            begin->s.otherEntityNum2 = 1;
            beginOpen = qtrue;
        }
        else if(!Q_stricmp(t, "[end]"))
        {
            end = G_Spawn();
            end->classname = "route_end";
            end->think = ThinkEndpoint;
            end->nextthink = level.time + FRAMETIME ;
            end->s.eType = ET_ITEM;
            end->item = BG_FindItemForPowerup( ROUTE_ENDPOINT );
            end->s.modelindex = end->item - bg_itemlist;
            end->s.otherEntityNum2 = 1;
            endOpen = qtrue;
        }
        else if(!Q_stricmp(t, "[cp]"))
        {
            checkpoint = G_Spawn();
            checkpoint->classname = "route_cp";
            checkpoint->position = level.numCheckpoints;
            checkpoint->think = ThinkCheckpoint;
            checkpoint->nextthink = level.time + FRAMETIME ;
            // Experimentary
            checkpoint->s.eType = ET_ITEM;
            checkpoint->item = BG_FindItemForPowerup( ROUTE_CHECKPOINT );
            checkpoint->s.modelindex = checkpoint->item - bg_itemlist;
            checkpoint->s.otherEntityNum2 = 1;
            cpOpen = qtrue;
        }
        else if(!Q_stricmp(t, "[pw]"))
        {
            powerup = G_Spawn();
            pwOpen = qtrue;
        }

        t = COM_Parse(&cnf);
    }

    if(routeOpen)
    {
    }

    if(beginOpen)
    {
        level.routeBegin = begin;
        G_SetOrigin(begin, begin->r.currentOrigin);
        G_SetAngle(begin, begin->r.currentAngles);
        trap_LinkEntity(begin);
    }

    if(endOpen)
    {
        level.routeEnd = end;
        G_SetOrigin(end, end->r.currentOrigin);
        G_SetAngle(end, end->r.currentAngles);
        trap_LinkEntity(end);
    }

    if(cpOpen)
    {
        level.checkpoints[level.numCheckpoints++] = checkpoint;
        G_SetOrigin(checkpoint, checkpoint->r.currentOrigin);
        G_SetAngle(checkpoint, checkpoint->r.currentAngles);

        trap_LinkEntity(checkpoint);
    }

    if(pwOpen)
    {
        G_SetOrigin(powerup, powerup->r.currentOrigin);
        powerup->powerupModelType = NUM_SR_POWERUP_TYPES;
        CreatePowerupSpawner( powerup );
    }
    free(cnf2);
    AP(va("cpm \"^8SR^7: Loaded route %s\n\"", routeName)) ;
    G_Printf("Loaded route %s\n", routeName);
    return qtrue;
}

void DeleteRoute( const char *routeName )
{
}