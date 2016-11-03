#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
    extern "C" {
#endif



/* --------- */		
/* Variables */
/* --------- */

#define TAILLE_MAX			1000
#define FLOATING_PRECISION	7
#define WAYPOINT_MAX 		10
float coord[WAYPOINT_MAX][2];


/* ---------- */
/* Prototypes */
/* ---------- */

int chercher(char * chaine);
int creer_wpl(int nb_waypoint);
int displayOnTable();

#ifdef __cplusplus
    }
#endif
