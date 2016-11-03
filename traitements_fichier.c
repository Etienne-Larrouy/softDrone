/**************************************************************************/
/*																		  */
/*             		  -	  Traitement du fichier 	-					  */
/* 																	      */
/*                    David Dy & Stéphane Juban (2011)                    */
/* 																		  */
/*          					                                          */
/**************************************************************************/

#include <string.h>
#include <ansi_c.h>  
#include <traitements_fichier.h>
#include "MK_Nav.h"

int nbwaypoint = 0;

//Fonction permettant de récupérer latitude et longitude
int chercher(char * chaine){
	FILE * temp = NULL;
	float retour[2];
	
	retour[0] = 0.0;
	retour[1] = 0.0;
	
	temp = fopen("temp.txt", "w+");					//Créer un fichier contenant la chaîne à traiter
    fprintf(temp, chaine);
    fclose(temp);
	
	//Traitement du fichier
	temp = fopen("temp.txt", "r");
	if(temp != NULL){
		fscanf(temp, "    <rtept lat=\"%f\" lon=\"%f\">", &retour[0], &retour[1]);
		if(retour[0] != 0.0 && retour[1] != 0.0){
			coord[nbwaypoint][0] = retour[0];
			coord[nbwaypoint][1] = retour[1];
			nbwaypoint++;
		}
		
		return 1;
	}else{
		return 0;
	}
}


//Fonction permettant de créer le fichier .wpl
int creer_wpl(int nb_waypoint){
	int i = 0, altitude = 0, attente = 5;
	float latitude = 0.0, longitude = 0.0;
	FILE* fichier = NULL;
 
    fichier = fopen("chemin.wpl", "w");
 
    if (fichier != NULL)
    {
        fprintf(fichier, "[General]\nFileVersion=2\nNumberOfWaypoints=%d\nUsePOI=1\nPOI_CAM_NICK_CTRL=0\n[POI]\nAltitude=1\nLatitude=0\nLongitude=0\n", nb_waypoint);

		for(i=1; i<=nb_waypoint; i++){
			fprintf(fichier, "[Waypoint%d]\nLatitude=%f\nLongitude=%f\nRadius=10\nAltitude=%d\nClimbRate=0\nDelayTime=%d\nWP_Event_Channel_Value=0\nHeading=-1\n", i, latitude, longitude, altitude, attente);			
		}
		
		fclose(fichier);
    }
 
    return 0;
}


