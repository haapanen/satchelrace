#include "g_local.h"

typedef struct Powerup_s
{
    const char *name;
    const char *text;
    gentity_t *(*spawn)( gentity_t *spawner, void (*think)(gentity_t *self) );
    void (*think)(gentity_t *self);
} Powerup_t;

gentity_t *spawner_noSlow( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_lowGravity( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_satchelBoost( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_slow( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_gravity( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_satchelUnboost( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawner_root( gentity_t *spawner, void (*think)(gentity_t *self) );

void think_noSlow( gentity_t *self );

static const Powerup_t powerups[] = {
    {"noslow", "No Slow", spawner_noSlow, think_noSlow},
    {"lowgravity", "Low Gravity", spawner_lowGravity, NULL},
    {"satchelboost", "Satchel Boost", spawner_satchelBoost, NULL},
    {"slow", "Slow Others", spawner_slow, NULL},
    {"gravity", "Gravity", spawner_gravity, NULL},
    {"satchelunboost", "Satchel Unboost", spawner_satchelUnboost, NULL},
    {"root", "Root", spawner_root, NULL}
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

    VectorSet( powerup->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );			//----(SA)	so items sit on the ground
    VectorSet( powerup->r.maxs, ITEM_RADIUS, ITEM_RADIUS, 2*ITEM_RADIUS );	//----(SA)	so items sit on the ground
    powerup->r.contents = CONTENTS_TRIGGER|CONTENTS_ITEM;

    powerup->clipmask = CONTENTS_SOLID | CONTENTS_MISSILECLIP;	

    powerup->touch = touch;
    powerup->think = dummythink;
    powerup->nextthink = 100;

    G_SetOrigin(powerup, ent->r.currentOrigin);
    G_SetAngle(powerup, ent->r.currentAngles);

    powerup->parent = ent;
    trap_LinkEntity(powerup);
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
    
}

gentity_t * spawner_satchelBoost( gentity_t *spawner, void (*think)(gentity_t *self) )
{

}

gentity_t * spawner_slow( gentity_t *spawner, void (*think)(gentity_t *self) )
{
 
}

gentity_t * spawner_gravity( gentity_t *spawner, void (*think)(gentity_t *self) )
{

}

gentity_t * spawner_satchelUnboost( gentity_t *spawner, void (*think)(gentity_t *self) )
{

}

gentity_t * spawner_root( gentity_t *spawner, void (*think)(gentity_t *self) )
{

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

    
}

void think_noSlow( gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;
    
    item = BG_FindItemForPowerup( PW_NOSLOW );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    self->child = DropPowerup(self, item, TouchPowerupNoSlow, qfalse);
}

void Cmd_Powerup_f( gentity_t * ent ) 
{
    int i = 0;
    int argc = trap_Argc();
    char arg[MAX_TOKEN_CHARS] = "\0";

    if(argc < 2)
    {
        CP("print \"usage: ^7powerup [type]\n\"");
        return;
    }

    trap_Argv(1, arg, sizeof(arg));
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
                level.numPowerups++;
                CP(va("cp \"^5Spawned %s powerup\n\"", powerups[i].text));
            } else
            {
                G_LogPrintf("Undefined powerup: %s\n", arg);
            }
            break;
        }
    }
}