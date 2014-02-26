#include "g_local.h"

typedef struct Powerup_s
{
    const char *name;
    const char *text;
    // for model
    satchelRacePowerup_t pw;
    satchelRacePowerup_t modelIndex;
    gentity_t *(*spawn)( gentity_t *spawner, void (*think)(gentity_t *self) );
    void (*think)(gentity_t *self);
    void (*touch)(gentity_t *self, gentity_t *player, trace_t *trace);
} Powerup_t;

gentity_t *spawner_noSlow( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_lowGravity( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_satchelBoost( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_slow( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_gravity( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_satchelUnboost( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_root( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_slick( gentity_t *spawner, void (*think)(gentity_t *self) );


void think_noSlow( gentity_t *self );
void think_lowGravity( gentity_t *self );
void think_root( gentity_t *self );
void think_satchelBoost( gentity_t *self );
void think_satchelUnboost( gentity_t *self );
void think_slow( gentity_t *self );
void think_gravity( gentity_t *self );
void think_slick( gentity_t *self );

void TouchPowerupGravity(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupLowGravity(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupNoSlow(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupRoot(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupSatchelBoost(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupSatchelUnboost(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupSlow(gentity_t *self, gentity_t *player, trace_t *trace);
void TouchPowerupSlick(gentity_t *self, gentity_t *player, trace_t *trace);

static const Powerup_t powerups[] = {
    {"noslow", "No Slow", PW_NOSLOW, PW_NOSLOW, spawner_noSlow, think_noSlow, TouchPowerupNoSlow},
    {"lowgravity", "Low Gravity", PW_NOSLOW, PW_LOWGRAVITY, spawner_lowGravity, think_lowGravity, TouchPowerupLowGravity},
    {"satchelboost", "Satchel Boost", PW_SATCHELBOOST, PW_SATCHELBOOST, spawner_satchelBoost, think_satchelBoost, TouchPowerupSatchelBoost},
    {"slow", "Slow Others", PW_SLOW, PW_SLOW, spawner_slow, think_slow, TouchPowerupSlow},
    {"gravity", "Gravity", PW_GRAVITY, PW_GRAVITY, spawner_gravity, think_gravity, TouchPowerupGravity},
    {"satchelunboost", "Satchel Unboost", PW_SATCHELBOOST, PW_SATCHELUNBOOST, spawner_satchelUnboost, think_satchelUnboost, TouchPowerupSatchelUnboost},
    {"root", "Root", PW_ROOT_PROTECTION, PW_ROOT_PROTECTION, spawner_root, think_root, TouchPowerupRoot},
    {"slick", "Slick Others", PW_SLICK, NUM_SR_POWERUP_TYPES, spawner_slick, think_slick, TouchPowerupSlick}
};
static int numPowerups = sizeof(powerups)/sizeof(Powerup_t);

void ClearPowerups()
{
    int i = 0;
    for(; i < MAX_POWERUPS; i++)
    {
        if(level.powerups[i])
        {
            // Make sure there's no existing powerup-items on the map
            if(level.powerups[i]->child)
            {
                G_FreeEntity(level.powerups[i]->child);
                level.powerups[i]->child = NULL;
            }
            G_FreeEntity(level.powerups[i]);
            level.powerups[i] = NULL;
        }
    }
    level.numPowerups = 0;
}

void ClearPowerupItems()
{
    int i = 0;
    for(; i < MAX_POWERUPS; i++)
    {
        if(level.powerups[i])
        {
            // Make sure there's no existing powerup-items on the map
            if(level.powerups[i]->child)
            {
                G_FreeEntity(level.powerups[i]->child);
                level.powerups[i]->child = NULL;
            }
        }
    }
}

void dummythink(gentity_t *self)
{
    return;
}

// Zero: drops a powerup
// ent is the entity that is dropping the powerup
// touch is called when pwup is touched.
gentity_t *DropPowerup( gentity_t *ent, gitem_t *item, 
                       void (*touch)(gentity_t *self, gentity_t *other, trace_t *trace), 
                       qboolean gravity )
{
    gentity_t *powerup = NULL;

    if(!item)
    {
        return NULL;
    }

    powerup = G_Spawn();
    powerup->s.eType = ET_ITEM;
    powerup->s.modelindex = item - bg_itemlist;
    powerup->s.otherEntityNum2 = 1;
    powerup->classname = item->classname;
    powerup->item = item;
    if(gravity)
    {
        powerup->s.pos.trType = TR_GRAVITY;
    }

    VectorSet( powerup->r.mins, -POWERUP_ITEM_RADIUS, -POWERUP_ITEM_RADIUS, 0 );			//----(SA)	so items sit on the ground
    VectorSet( powerup->r.maxs, POWERUP_ITEM_RADIUS, POWERUP_ITEM_RADIUS, 2*POWERUP_ITEM_RADIUS );	//----(SA)	so items sit on the ground
    powerup->r.contents = CONTENTS_TRIGGER | CONTENTS_ITEM;

    powerup->clipmask = CONTENTS_SOLID | CONTENTS_MISSILECLIP;	

    powerup->touch = touch;
    powerup->think = dummythink;
    powerup->nextthink = 100;

    G_SetOrigin(powerup, ent->r.currentOrigin);
    G_SetAngle(powerup, ent->r.currentAngles);

    powerup->parent = ent;
    trap_LinkEntity(powerup);
    return powerup;
}

gentity_t * spawner_noSlow( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_noslow_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t * spawner_lowGravity( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_lowgravity_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t * spawner_satchelBoost( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_satchelboost_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t * spawner_slow( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_slow_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t * spawner_gravity( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_gravity_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t * spawner_satchelUnboost( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_satchelunboost_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t * spawner_root( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_lowgravity_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

gentity_t *spawner_slick( gentity_t *spawner, void(*think)(gentity_t *self))
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_slick_spawner";
    powerupSpawner->think = think;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

void testtouch(gentity_t *self, gentity_t *other, trace_t *trace)
{
    G_PrintClientSpammyCenterPrint(other->client->ps.clientNum, 
        "Touching testtouch");
}

void TouchPowerupNoSlow(gentity_t *self, gentity_t *player, trace_t *trace)
{
    if(!player->client)
    {
        return;
    }

    player->client->powerups[PW_NOSLOW] = level.time + sr_pw_noSlowDuration.integer;
     
    AP(va("chat \"%s ^7picked up a ^5No Slow ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_noSlow( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;
    
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupNoSlow, qfalse);
}

void TouchPowerupLowGravity(gentity_t *self, gentity_t *player, trace_t *trace)
{
    if(!player->client)
    {
        return;
    }

    player->client->powerups[PW_LOWGRAVITY] = level.time + sr_pw_lowGravityDuration.integer;

    AP(va("chat \"%s ^7picked up a ^5Low Gravity ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_lowGravity( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupLowGravity, qfalse);
}

void TouchPowerupRoot(gentity_t *self, gentity_t *player, trace_t *trace)
{
    if(!player->client)
    {
        return;
    }

    player->client->powerups[PW_ROOT_PROTECTION] = level.time + sr_pw_rootDuration.integer + FRAMETIME;

    level.rootPlayers = level.time + sr_pw_rootDuration.integer;

    AP(va("chat \"%s ^7picked up a ^5Root ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_root( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupRoot, qfalse);
}

void TouchPowerupSatchelBoost(gentity_t *self, gentity_t *player, trace_t *trace)
{
    if(!player->client)
    {
        return;
    }

    player->client->powerups[PW_SATCHELBOOST] = sr_pw_satchelBoost.integer;

    AP(va("chat \"%s ^7picked up a ^5Satchel Boost ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_satchelBoost( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupSatchelBoost, qfalse);
}

void TouchPowerupSatchelUnboost(gentity_t *self, gentity_t *player, trace_t *trace)
{
    int i = 0;
    if(!player->client)
    {
        return;
    }

    for(; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        gentity_t *target = g_entities + clientNum;

        if(target == player)
        {
            continue;
        } else
        {
            target->client->powerups[PW_SATCHELUNBOOST] = sr_pw_satchelUnboost.integer;
        }
    }

    AP(va("chat \"%s ^7picked up a ^5Satchel Unboost ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_satchelUnboost( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupSatchelUnboost, qfalse);
}

void TouchPowerupSlow(gentity_t *self, gentity_t *player, trace_t *trace)
{
    int i = 0;
    if(!player->client)
    {
        return;
    }

    for(; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        gentity_t *target = g_entities + clientNum;

        if(target == player)
        {
            player->client->powerups[PW_SLOW] = 0;
            continue;
        } else
        {
            target->client->powerups[PW_SLOW] = level.time + sr_pw_slowDuration.integer;
        }
    }

    AP(va("chat \"%s ^7picked up a ^5Slow ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_slow( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupSlow, qfalse);
}

void TouchPowerupGravity(gentity_t *self, gentity_t *player, trace_t *trace)
{
    int i = 0;
    if(!player->client)
    {
        return;
    }

    for(; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        gentity_t *target = g_entities + clientNum;

        if(target == player)
        {
            continue;
        } else
        {
            target->client->powerups[PW_GRAVITY] = level.time + sr_pw_gravityDuration.integer;
        }
    }

    AP(va("chat \"%s ^7picked up a ^5Slow ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_gravity( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupGravity, qfalse);
}

void TouchPowerupSlick(gentity_t *self, gentity_t *player, trace_t *trace)
{
    int i = 0;
    if(!player->client)
    {
        return;
    }

    for(; i < level.numConnectedClients; i++)
    {
        int clientNum = level.sortedClients[i];
        gentity_t *target = g_entities + clientNum;

        if(target == player)
        {
            continue;
        } else
        {
            target->client->powerups[PW_SLICK] = level.time + sr_pw_slickDuration.integer;
        }
    }

    AP(va("chat \"%s ^7picked up a ^5Slick ^7powerup\"", player->client->pers.netname));
    self->parent->child = NULL;
    G_FreeEntity(self);
}

void think_slick( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( self->powerupModelType );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupSlick, qfalse);
}


void think_random( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;
    int i = rand() % numPowerups;

    // FIXME: wrong PW_
    item = BG_FindItemForPowerup( PW_RANDOM );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup model.\n");
        return;
    }

    self->child = DropPowerup(self, item, powerups[i].touch, qfalse);
}

gentity_t *SpawnRandomPowerupSpawner( gentity_t *spawner ) 
{
    gentity_t *powerupSpawner = G_Spawn();
    powerupSpawner->classname = "powerup_random_spawner";
    powerupSpawner->think = think_random;
    powerupSpawner->child = NULL;
    G_SetOrigin(powerupSpawner, spawner->r.currentOrigin);
    return powerupSpawner;
}

void PrintPowerupHelp( gentity_t *ent )
{
    int i = 0;
    BeginBufferPrint();
    BufferPrint(ent, "Powerup:\n");
    BufferPrint(ent, "Usage: /powerup [name|any|random]\n");
    BufferPrint(ent, "name: spawns a specified powerup.\n");
    BufferPrint(ent, "list of powerups: ");
    for(; i < numPowerups; i++)
    {
        BufferPrint(ent, va("%s ", powerups[i].name));
    }
    BufferPrint(ent, "\nany: spawns a powerup that can be any powerup.\n");
    BufferPrint(ent, "random: spawns a random powerup that is always the same.");
    FinishBufferPrint(ent);
}

void PrintPowerupInfo( gentity_t *ent, char *targetPW )
{
	int i = 0;
	int allowArg = 0;
	BeginBufferPrint();
	
	for(; i < numPowerups; i++)
    {
       if(Q_stricmp(powerups[i].name, targetPW) == 0)
	   {
		   allowArg = 1;
	   }
    }
	
	if(allowArg == 1)
	{
		allowArg = 0;
		BufferPrint(ent, va("^5Currently showing information for: %s\n ", targetPW));
		if(Q_stricmp("noslow", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Removes your slowing effects for %.2f seconds.\n",  (float)sr_pw_noSlowDuration.integer / 1000));
		}
		else if(Q_stricmp("lowgravity", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Decreases your gravity to %d for %.2f seconds.\n", sr_pw_lowGravity.integer, (float)sr_pw_lowGravityDuration.integer / 1000));
		}
		else if(Q_stricmp("satchelboost", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Increases your blast force by %dx for next %d satchel/s.\n",  sr_pw_satchelBoostKnockback.integer, sr_pw_satchelBoost.integer));
		}
		else if(Q_stricmp("satchelunboost", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Decreases the opponents satchel blast force by %dx for next %d satchel/s.\n",  sr_pw_satchelUnboostKnockback.integer, sr_pw_satchelUnboost.integer));
		}
		else if(Q_stricmp("slow", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Decreases the opponents speed by %d percent for %.2f seconds.\n",  sr_pw_slowPercent.integer, (float)sr_pw_slowDuration.integer / 1000));
		}
		else if(Q_stricmp("gravity", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Increases the opponents gravity to %d for %.2f seconds.\n",  sr_pw_gravity.integer, (float)sr_pw_gravityDuration.integer / 1000));
		}
		else if(Q_stricmp("root", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Stops the enemy from moving for %.2f seconds.\n",  (float)sr_pw_rootDuration.integer / 1000));
		}
		else if(Q_stricmp("slick", targetPW) == 0)
		{
			BufferPrint(ent, va("^7Makes the ground slippery for opposing players for %.2f seconds\n",  (float)sr_pw_slickDuration.integer / 1000));
		}
	}
	else
	{
		BufferPrint(ent, "Invalid powerup.\n");
    }
    
	
    FinishBufferPrint(ent);
}

void CreatePowerupSpawner( gentity_t * powerup ) 
{
    int i = 0;

    if(level.numPowerups == MAX_POWERUPS)
    {
        return;
    }

    if(powerup->powerupType == PW_RANDOM)
    {
        powerup->think = think_random;
        powerup->child = NULL;
        level.powerups[level.numPowerups++] = powerup;
        return;
    }

    // lets find the correct pwup by id
    for(i = 0; i < numPowerups; i++)
    {
        if(powerups[i].pw == powerup->powerupType)
        {
            powerup->think = powerups[i].think;
            powerup->child = NULL;
            level.powerups[level.numPowerups++] = powerup;
        }
    }
}

void Cmd_Powerup_f( gentity_t * ent ) 
{
    int i = 0;
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS] = "\0";
	char arg2[MAX_TOKEN_CHARS] = "\0";

    if( !ent->client->sess.routeMaker )
    {
        CP("cp \"^7You need to be a route maker to create powerups.\n\"");
        return;
    }

    if(argc < 2)
    {
        CP("print \"usage: ^7powerup [type]\n\"");
        return;
    }
	
    trap_Argv(1, arg, sizeof(arg));

    if(!Q_stricmp(arg, "help"))
    {
        PrintPowerupHelp(ent);
        return;
    }
	
	
	if(!Q_stricmp(arg, "info"))
	{
		trap_Argv(2, arg, sizeof(arg));
		PrintPowerupInfo(ent, arg);
		return;
	}

    // Special case, spawn random pwup
    if(!Q_stricmp(arg, "random"))
    {
        int index = rand() % numPowerups;

        if(powerups[index].spawn && powerups[index].think)
        {
            if(level.numPowerups == MAX_POWERUPS)
            {
                CP("print \"^1error: ^7too many powerups spawned.\n\"");
                return;
            }
            level.powerups[level.numPowerups] = powerups[index].spawn(ent, powerups[index].think);
            level.powerups[level.numPowerups]->powerupType = powerups[index].pw;
            level.powerups[level.numPowerups]->powerupModelType = powerups[index].modelIndex;
            level.numPowerups++;
            CP("cp \"^5Spawned a random powerup\n\"");
        }
        return;
    }

    if(!Q_stricmp(arg, "any"))
    {
        if(level.numPowerups == MAX_POWERUPS)
        {
            CP("print \"^1error: ^7too many powerups spawned.\n\"");
            return;
        }
        level.powerups[level.numPowerups] = SpawnRandomPowerupSpawner( ent );
        level.powerups[level.numPowerups]->powerupType = PW_RANDOM;
        level.powerups[level.numPowerups]->powerupModelType = PW_RANDOM;
        level.numPowerups++;
        CP("cp \"^5Spawned a random powerup\n\"");
        return;
    }

    for(; i < numPowerups; i++)
    {
        if(!Q_stricmp(arg, powerups[i].name))
        {
            if(powerups[i].spawn && powerups[i].think)
            {
                if(level.numPowerups == MAX_POWERUPS)
                {
                    CP("print \"^1error: ^7too many powerups spawned.\n\"");
                    return;
                }
                level.powerups[level.numPowerups] = powerups[i].spawn(ent, powerups[i].think);
                level.powerups[level.numPowerups]->powerupType = powerups[i].pw;
                level.powerups[level.numPowerups]->powerupModelType = powerups[i].modelIndex;
                level.numPowerups++;
                CP(va("cp \"^5Spawned a %s powerup\n\"", powerups[i].text));
            } else
            {
                G_LogPrintf("Undefined powerup: %s\n", arg);
            }
            break;
        }
    }
}