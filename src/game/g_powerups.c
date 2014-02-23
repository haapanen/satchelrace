#include "g_local.h"

typedef struct Powerup_s
{
    const char *name;
    gentity_t *(*spawn)( gentity_t *spawner, void (*think)(gentity_t *self) );
    void (*think)(gentity_t *self);
} Powerup_t;

gentity_t *spawn_noSlow( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawn_lowGravity( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawn_satchelBoost( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawn_slow( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawn_gravity( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawn_satchelUnboost( gentity_t *spawner, void (*think)(gentity_t *self) );
gentity_t *spawn_root( gentity_t *spawner, void (*think)(gentity_t *self) );

void think_noSlow( gentity_t *self );

static const Powerup_t powerups[] = {
    {"noslow", spawn_noSlow, think_noSlow},
    {"lowgravity", spawn_lowGravity, NULL},
    {"satchelboost", spawn_satchelBoost, NULL},
    {"slow", spawn_slow, NULL},
    {"gravity", spawn_gravity, NULL},
    {"satchelunboost", spawn_satchelUnboost, NULL},
    {"root", spawn_root, NULL}
};
static int numPowerups = sizeof(powerups)/sizeof(Powerup_t);

gentity_t * spawn_noSlow( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_noslow_spawner";
    powerup->think = think;
    G_SetOrigin(powerup, spawner->r.currentOrigin);
    G_Printf("Spawned a noSlow spawner.\n");
    return powerup;
}

gentity_t * spawn_lowGravity( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_lowGravity";
    powerup->think = think;
    powerup->nextthink = level.time + FRAMETIME;
}

gentity_t * spawn_satchelBoost( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_satchelBoost";
    powerup->think = think;
    powerup->nextthink = level.time + FRAMETIME;
}

gentity_t * spawn_slow( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_slow";
    powerup->think = think;
    powerup->nextthink = level.time + FRAMETIME;
}

gentity_t * spawn_gravity( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_gravity";
    powerup->think = think;
    powerup->nextthink = level.time + FRAMETIME;
}

gentity_t * spawn_satchelUnboost( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_satchelUnboost";
    powerup->think = think;
    powerup->nextthink = level.time + FRAMETIME;
}

gentity_t * spawn_root( gentity_t *spawner, void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_root";
    powerup->think = think;
    powerup->nextthink = level.time + FRAMETIME;
}

void testtouch(gentity_t *self, gentity_t *other, trace_t *trace)
{
    G_Printf("Trying to pickup think_noSlow.\n");
}

void dummythink(gentity_t *self)
{
    return;
}

void think_noSlow( gentity_t *spawner, gentity_t *self )
{
    gitem_t *item = NULL;
    gentity_t *dropped = NULL;
    
    item = BG_FindItemForPowerup( PW_NOSLOW );
    if(!item)
    {
        G_Printf("Error: couldn't find powerup.\n");
        return;
    }

    dropped = G_Spawn();
    dropped->s.eType = ET_ITEM;
    dropped->s.modelindex = item - bg_itemlist;
    dropped->s.otherEntityNum2 = 1;
    dropped->classname = item->classname;
    dropped->item = item;

    VectorSet( dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );			//----(SA)	so items sit on the ground
    VectorSet( dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, 2*ITEM_RADIUS );	//----(SA)	so items sit on the ground
    dropped->r.contents = CONTENTS_TRIGGER|CONTENTS_ITEM;

    dropped->clipmask = CONTENTS_SOLID | CONTENTS_MISSILECLIP;	

    dropped->touch = testtouch;
    dropped->think = dummythink;
    dropped->nextthink = 100;

    G_SetOrigin(dropped, spawner->r.currentOrigin);
    G_SetAngle(dropped, spawner->r.currentAngles);
    trap_LinkEntity(dropped);
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
                CP(va("cp \"^5Spawned %s\n\"", powerups[i].name));
            } else
            {
                G_LogPrintf("Undefined powerup: %s\n", arg);
            }
            break;
        }
    }
}