#include "g_local.h"

void CheckWinner(gentity_t *self)
{
    int i = 0;
    int count = 0;
    vec3_t range = {self->horizontalRange, self->horizontalRange, self->verticalRange};
    vec3_t mins = {0, 0, 0};
    vec3_t maxs = {0, 0, 0};
    int entityList[MAX_GENTITIES];

    VectorSubtract( self->r.currentOrigin, range, mins );
    VectorAdd( self->r.currentOrigin, range, maxs ); 

    count = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

    for ( i = 0; i < count; i++ ) {
        gentity_t *ent = NULL;
        ent = &g_entities[entityList[i]];

        if ( !ent->client ) {
            continue;
        }

        if( ent->client->sess.racing )
        {
            if(ent->client->sess.checkpointVisited[level.numCheckpoints - 1] == qtrue)
            {
                int i = 0;
                int msec = 0;
                int sec = 0;
                int min = 0;
                msec = level.time - ent->client->sess.raceStartTime;
                min = msec / 60000;
                msec = msec - min * 60000;
                sec = msec / 1000;
                msec = msec - sec * 1000;
                trap_SendServerCommand(-1, va("cpm \"%s ^7reached the ^1end^7 in %02d:%02d:%03d.\n\"", 
                    ent->client->pers.netname, min, sec, msec));
                ent->client->sess.racing = qfalse;
                // Reset powerups
                for(; i < NUM_SR_POWERUP_TYPES; i++)
                {
                    ent->client->powerups[i] = 0;
                }
            } else
            {
                // Kind of messy but checks if client has visited all checkpoints.
                int i = 0;
                int checkpointsVisited[MAX_CHECKPOINTS];
                int checkpointsNotVisited = 0;
                char buf[MAX_TOKEN_CHARS] = "^5You skipped the following checkpoints: ";
                for(; i < level.numCheckpoints; i++)
                {
                    if(ent->client->sess.checkpointVisited[i] == qfalse)
                    {
                        checkpointsVisited[checkpointsNotVisited++] =
                            i;
                    }
                }

                for(i = 0; i < checkpointsNotVisited; i++)
                {
                    if(i == checkpointsNotVisited - 1)
                    {
                        Q_strcat(buf, sizeof(buf), va("%d", checkpointsVisited[i] + 1));
                    } else
                    {
                        Q_strcat(buf, sizeof(buf), va("%d, ", checkpointsVisited[i] + 1));
                    }
                }

                G_PrintClientSpammyCenterPrint(ent->client->ps.clientNum,
                    va("%s", buf));
            }

        } 
    }

    self->nextthink = FRAMETIME;
}

void CheckRacersNearCP(gentity_t *self)
{
    int currentCP = 1;
    int k = 0;
    int i = 0;
    int count = 0;
    vec3_t range = {self->horizontalRange, self->horizontalRange, self->verticalRange};
    vec3_t mins = {0, 0, 0};
    vec3_t maxs = {0, 0, 0};
    int entityList[MAX_GENTITIES];

    VectorSubtract( self->r.currentOrigin, range, mins );
    VectorAdd( self->r.currentOrigin, range, maxs ); 

    count = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);

    for ( i = 0; i < count; i++ ) {
        int msec = 0;
        int sec = 0;
        int min = 0;
        gentity_t *ent = NULL;
        ent = &g_entities[entityList[i]];

        if ( !ent->client ) {
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

            if(currentCP != 0)
            {
                if(ent->client->sess.checkpointVisited[currentCP - 1])
                {
                    trap_SendServerCommand(-1, va("cpm \"%s ^7reached ^3checkpoint ^7%d in %02d:%02d:%03d.\n\"", ent->client->pers.netname, self->position + 1,
                        min, sec, msec));
                    ent->client->sess.checkpointVisited[self->position] = qtrue;
                }
                else
                {
                    CP("cp \"^7You missed a ^3checkpoint^7, go back and visit it before visiting this one.\n\"");
                }
            }
            else if(currentCP == 0)
            {
                trap_SendServerCommand(-1, va("cpm \"%s ^7reached ^3checkpoint ^7%d in %02d:%02d:%03d.\n\"", ent->client->pers.netname, self->position + 1,
                    min, sec, msec));
                ent->client->sess.checkpointVisited[self->position] = qtrue;
            }


        } 
    }

    self->nextthink = FRAMETIME;
}

#define RANGE_VERYSMALL 30
#define RANGE_SMALL 100
#define RANGE_MEDIUM 300
#define RANGE_LARGE 500

void RouteMakerCheckpoints( gentity_t * ent ) 
{
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS] = "\0";
    gentity_t *checkpoint = NULL;

    if(level.numCheckpoints == MAX_CHECKPOINTS)
    {
        CP("print \"^1error: ^7maximum number of checkpoints already on map.\n\"");
        return;
    }

    checkpoint = G_Spawn();
    checkpoint->classname = "route_cp";
    checkpoint->position = level.numCheckpoints;
    checkpoint->think = CheckRacersNearCP;
    checkpoint->nextthink = level.time + FRAMETIME;
    // Experimentary
    checkpoint->s.eType = ET_ITEM;
    checkpoint->item = BG_FindItemForPowerup( ROUTE_CHECKPOINT );
    checkpoint->s.modelindex = checkpoint->item - bg_itemlist;
    checkpoint->s.otherEntityNum2 = 1;
    if(trap_Argc() == 2)
    {
        checkpoint->horizontalRange = sr_defaultAreaRange.integer;
        checkpoint->verticalRange = sr_defaultAreaRange.integer;
    } else if(trap_Argc() == 3)
    {
        char rangeStr[MAX_TOKEN_CHARS] = "\0";
        int range = 0;
        trap_Argv(2, rangeStr, sizeof(rangeStr));
        range = atoi(rangeStr);

        if(range)
        {
            checkpoint->horizontalRange = range;
            checkpoint->verticalRange = range;
        } else if(!Q_stricmp(rangeStr, "verysmall"))
        {
            checkpoint->horizontalRange = RANGE_VERYSMALL;
            checkpoint->verticalRange = RANGE_VERYSMALL;
        } else if(!Q_stricmp(rangeStr, "small"))
        {
            checkpoint->horizontalRange = RANGE_SMALL;
            checkpoint->verticalRange = RANGE_SMALL;
        } else if(!Q_stricmp(rangeStr, "medium"))
        {
            checkpoint->horizontalRange = RANGE_MEDIUM;
            checkpoint->verticalRange = RANGE_MEDIUM;
        } else if(!Q_stricmp(rangeStr, "large"))
        {
            checkpoint->horizontalRange = RANGE_LARGE;
            checkpoint->verticalRange = RANGE_LARGE;
        } else
        {
            checkpoint->horizontalRange = sr_defaultAreaRange.integer;
            checkpoint->verticalRange = sr_defaultAreaRange.integer;
        }
    } else if(trap_Argc() == 4)
    {
        char horizontalRangeStr[MAX_TOKEN_CHARS] = "\0";
        char verticalRangeStr[MAX_TOKEN_CHARS] = "\0";

        int horizontalRange = atoi(horizontalRangeStr);
        int verticalRange = atoi(verticalRangeStr);

        trap_Argv(2, horizontalRangeStr, sizeof(horizontalRangeStr));
        trap_Argv(3, verticalRangeStr, sizeof(verticalRangeStr));            

        horizontalRange = atoi(horizontalRangeStr);
        verticalRange = atoi(verticalRangeStr);

        if(horizontalRange)
        {
            checkpoint->horizontalRange = horizontalRange;
        } else
        {
            checkpoint->horizontalRange = sr_defaultAreaRange.integer;
        }

        if(verticalRange)
        {
            checkpoint->verticalRange = verticalRange;
        } else
        {
            checkpoint->verticalRange = sr_defaultAreaRange.integer;
        }
        CP(va("cp \"^7Added a ^3checkpoint ^7(%d, %d)\n\"", checkpoint->horizontalRange, checkpoint->verticalRange));
    }

    G_SetOrigin(checkpoint, ent->r.currentOrigin);
    G_SetAngle(checkpoint, ent->r.currentAngles);

    trap_LinkEntity(checkpoint);
    //     VectorCopy( ent->r.currentOrigin, checkpoint->r.currentOrigin );
    //     VectorCopy( ent->client->ps.viewangles, checkpoint->r.currentAngles );
    level.checkpoints[level.numCheckpoints] = checkpoint;
    level.numCheckpoints++;
    CP(va("cp \"^7Added a ^3checkpoint ^7(%d, %d).\n\"", checkpoint->horizontalRange, checkpoint->verticalRange));
}

void RouteMakerBegin( gentity_t * ent ) 
{
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS];
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
        CP("cp \"^7Added a ^2start ^7spot.\n\"");
    } else
    {
        ClearRoute();
        CP("cp \"^7Added a ^2start ^7spot and cleared the route.\n\"");
        CP("print \"^7If you do not wish to clear the route when you define a start spot, do /route begin -keep\n\"");
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
    end->think = CheckWinner;
    end->nextthink = level.time + FRAMETIME;

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

    } else if(argc == 3)
    {
        char rangeStr[MAX_TOKEN_CHARS] = "\0";
        int range = 0;
        trap_Argv(2, rangeStr, sizeof(rangeStr));
        range = atoi(rangeStr);

        if(range)
        {
            end->horizontalRange = range;
            end->verticalRange = range;
        } else if(!Q_stricmp(rangeStr, "verysmall"))
        {
            end->horizontalRange = RANGE_VERYSMALL;
            end->verticalRange = RANGE_VERYSMALL;
        } else if(!Q_stricmp(rangeStr, "small"))
        {
            end->horizontalRange = RANGE_SMALL;
            end->verticalRange = RANGE_SMALL;
        } else if(!Q_stricmp(rangeStr, "medium"))
        {
            end->horizontalRange = RANGE_MEDIUM;
            end->verticalRange = RANGE_MEDIUM;
        } else if(!Q_stricmp(rangeStr, "large"))
        {
            end->horizontalRange = RANGE_LARGE;
            end->verticalRange = RANGE_LARGE;
        } else
        {
            end->horizontalRange = sr_defaultAreaRange.integer;
            end->verticalRange = sr_defaultAreaRange.integer;
        }
    } else if(argc == 4)
    {
        char horizontalRangeStr[MAX_TOKEN_CHARS] = "\0";
        char verticalRangeStr[MAX_TOKEN_CHARS] = "\0";

        int horizontalRange = atoi(horizontalRangeStr);
        int verticalRange = atoi(verticalRangeStr);

        trap_Argv(2, horizontalRangeStr, sizeof(horizontalRangeStr));
        trap_Argv(3, verticalRangeStr, sizeof(verticalRangeStr));            

        horizontalRange = atoi(horizontalRangeStr);
        verticalRange = atoi(verticalRangeStr);

        if(horizontalRange)
        {
            end->horizontalRange = horizontalRange;
        } else
        {
            end->horizontalRange = sr_defaultAreaRange.integer;
        }

        if(verticalRange)
        {
            end->verticalRange = verticalRange;
        } else
        {
            end->verticalRange = sr_defaultAreaRange.integer;
        }
    }
    CP(va("cp \"^7Added an ^1end ^7spot (%d, %d)\n\"", end->horizontalRange, end->verticalRange));
}

void RouteMakerClear( gentity_t * ent ) 
{
    int i = 0;
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
    AP("cpm \"^8SR^7: route has been cleared. Racing has stopped.\n\"");
}

void RouteMakerClearCP( gentity_t * ent ) 
{
    int i = 0;
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
    ClearPowerups();
    trap_SendServerCommand(ent->client->ps.clientNum,
        "print \"^8SR^7: deleted all powerups\n\"");
}

void Cmd_Route_f( gentity_t * ent ) 
{
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS] = "\0";
    if( !ent->client->sess.routeMaker )
    {
        CP("cp \"^7You need to be a route maker to create routes.\n\"");
        return;
    }

    // /route begin
    // /route end
    // /route clear
    // 

    if(argc < 2)
    {
        CP("print \"usage: /route [begin|end|cp|clear|clearcp|clearpw]\n\"");
        return; 
    }

    trap_Argv(1, arg, sizeof(arg));

    if(!Q_stricmp(arg, "begin"))
    {
        RouteMakerBegin(ent);
    } else if(!Q_stricmp(arg, "end"))
    {
        RouteMakerEnd(ent);
    } else if(!Q_stricmp(arg, "clear"))
    {
        RouteMakerClear(ent);
    } else if(!Q_stricmp(arg, "clearcp")) 
    {
        RouteMakerClearCP(ent);
    } else if (!Q_stricmp(arg, "clearpw"))
    {
        RouteMakerClearPW(ent);
    } else if(!Q_stricmp(arg, "cp"))
    {
        RouteMakerCheckpoints( ent );
    } else
    {
        CP("print \"usage: /route [begin|end|cp|clear|clearcp|clearpw]\n\"");
        return;
    }
}

void Cmd_ShowRoute_f( gentity_t * ent ) 
{
    int travelTime;
    char arg[MAX_TOKEN_CHARS] = "\0";

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
    CP(va("cp \"^7Estimated time to show route: ^2%d ^7seconds\n\"", travelTime));
    ent->client->sess.nextCp = -1;
    ent->client->sess.showingRoute = qtrue;
}

void Cmd_RestartRun_f( gentity_t * ent )
{
    int i;
    if(level.numCheckpoints > 0)
    {
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

        //Teleporting to beginning
        ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
        VectorCopy(level.routeBegin->r.currentOrigin, ent->client->ps.origin);
        SetClientViewAngle(ent, level.routeBegin->r.currentAngles);

        //If he has already ended the run, put him to racing status again.
        if(!ent->client->sess.racing)
        {
            ent->client->sess.racing = qtrue;
        }

        //Give message to server that he reset his run
        trap_SendServerCommand(-1, va("cpm \"%s^7has restarted his run.", ent->client->pers.netname));
        CP("cp \"^7Your run has been successfully restarted.\n\"");
    }

}

void Cmd_StopShowRoute_f( gentity_t * ent ) 
{
    ent->client->sess.showingRoute = qfalse;
    ent->client->sess.nextCp = MAX_CHECKPOINTS + 1;
}
