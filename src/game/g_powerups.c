#include "g_local.h"

typedef struct Powerup_s
{
    const char *name;
    void (*spawn)( void (*think)(gentity_t *self) );
    void (*think)(gentity_t *self);
} Powerup_t;

static const Powerup_t powerups[] = {
    {"noslow", NULL, NULL},
    {"lowgravity", NULL, NULL},
    {"satchelboost", NULL, NULL},
    {"slow", NULL, NULL},
    {"gravity", NULL, NULL},
    {"satchelunboost", NULL, NULL},
    {"root", NULL, NULL}
};
static int numPowerups = sizeof(powerups)/sizeof(Powerup_t);

void spawn_noSlow( void (*think)(gentity_t *self) )
{
    gentity_t *powerup = G_Spawn();
    powerup->classname = "powerup_noslow";
    powerup->think = think;
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
                powerups[i].spawn(powerups[i].think);
            } else
            {
                G_LogPrintf("Undefined powerup: %s\n", arg);
            }
            break;
        }
    }
}