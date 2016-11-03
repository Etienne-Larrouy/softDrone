/**************************************************************************/
/*																		  */
/*             			   -	MK Nav	-								  */
/* 																	      */
/*                    David Dy & Stéphane Juban (2011)                    */
/* 																		  */
/*          					                                          */
/**************************************************************************/


#include "MK_Nav.h"
#include "traitements_fichier.h" 
#include "serial.h"
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cvirte.h>
#include <userint.h>
#include <time.h>
#include <math.h>
#include "cvixml.h"
#include "globals.h"
						  

#define MAX_WAYPOINTS 500

typedef struct{
	float fLat;
	float fLon;//lat et lon réelles
	int iPosX;
	int iPosY;// position x, y dans sur le logiciel
	int ele;//altitude
}Coord;

Coord trajet[MAX_WAYPOINTS];	 // tableau de waypoints


char curDir[312];	   //répertoire de travail
static int panelHandle;							// Fenetre principale 
static int image_GPS;
static char file_path[512];
FILE* fProj;

/*Variables d'étalonnage*/
int zoomLevel = 0;									// niveau de zoom de l'image
float fLatCenter =0.0,fLonCenter=0.0;				// lat et lon au centre de la carte
float fLat, fLon;									// lat et lon de manipulation (change tout le temps conrairement aux autres
float fLatPix = 0.0, fLonPix=0.0;					// lat et lon par pixel
float fLatMax, fLonMin;								//

/*GPX vairables*/
char gpxFileName[128];
CVIXMLDocument gpxDoc;
CVIXMLElement rootNode;
CVIXMLElement trkSeg;

/*variables config*/
int parityBitTicked = 0; //bool
int stopBitTicked = 0;   //bool
int baudRate = 9600;
int portNumber = 0;
/*variables de trajet*/
int nb_waypoint = 0, image_chargee = 0;
int angleMax = 90; // valeur maximale de virage par defaut
int distanceMin = 50;
int altMin = 1; 
int altMax = 40;
int penteMax = 5;

int MenuBar, MenuID;
int indexToDelete = -1;

int main (int argc, char *argv[])
{
	GetDir(curDir);
	int prevCursor;
	
	if (InitCVIRTE (0, argv, 0) == 0)return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "MK_Nav.uir", PANEL)) < 0) return -1;
	
	SetPanelAttribute(panelHandle, ATTR_SCALE_CONTENTS_ON_RESIZE, 1);
	SetPanelAttribute(panelHandle, ATTR_HEIGHT, 720);
	SetPanelAttribute(panelHandle, ATTR_WIDTH, 1220);
	SetPanelAttribute(panelHandle, ATTR_LEFT, VAL_AUTO_CENTER);	
	SetPanelAttribute(panelHandle, ATTR_TITLE, "MK Nav v2.0");
	SetPanelPos(panelHandle, VAL_AUTO_CENTER, VAL_AUTO_CENTER);
	
	MenuBar = NewMenuBar (0);
	MenuID = NewMenu (MenuBar, "WptMenu", -1);
	NewMenuItem (MenuBar, MenuID, "Supprimer Waypoint", -1, 0, DelWptCallback, 0);
	
	/* Set default values for the UI */
    GetMouseCursor (&prevCursor);
    SetMouseCursor (VAL_POINTING_FINGER_CURSOR);
	
	/* Display the main panel and run the UI */
    DisplayPanel (panelHandle);
    RunUserInterface ();
	
	/* Reset the cursor, free resources, and return */
    SetMouseCursor (prevCursor);
    DiscardPanel (panelHandle);
    CloseCVIRTE ();

	return 0;
}


void CVICALLBACK changeAltMaxCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cAltitude[17];
	int status = GenericMessagePopup("MK_Nav Message",
									 "Veuillez choisir une altitude maximum",
									 "Valider",
						 			 "Annuler",
						 			 0,//pas de troisième bouton
						 			 cAltitude,//pas de reponse utilisateur
						 		  	 16,//taille de la reponse
						 			 0,//boutons en bas de la fenetre
						 			 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
					 				 VAL_GENERIC_POPUP_BTN1,
						 			 VAL_GENERIC_POPUP_BTN2);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		altMax = atoi(cAltitude);	
	}		
}

void CVICALLBACK changePenteMaxCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cPente[17];
	int status = GenericMessagePopup("MK_Nav Message",
									 "Veuillez choisir une pente maximum",
									 "Valider",
						 			 "Annuler",
						 			 0,//pas de troisième bouton
						 			 cPente,//pas de reponse utilisateur
						 		  	 16,//taille de la reponse
						 			 0,//boutons en bas de la fenetre
						 			 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
					 				 VAL_GENERIC_POPUP_BTN1,
						 			 VAL_GENERIC_POPUP_BTN2);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		penteMax = atoi(cPente);	
	}	
}

void CVICALLBACK changeAngleMaxCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cAngle[17];
	int status = GenericMessagePopup("MK_Nav Message",
									 "Veuillez choisir un angle maximum",
									 "Valider",
						 			 "Annuler",
						 			 0,//pas de troisième bouton
						 			 cAngle,//pas de reponse utilisateur
						 		  	 16,//taille de la reponse
						 			 0,//boutons en bas de la fenetre
						 			 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
					 				 VAL_GENERIC_POPUP_BTN1,
						 			 VAL_GENERIC_POPUP_BTN2);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		angleMax = atoi(cAngle);	
	}
}
void CVICALLBACK tickParityBitCB(int menubar, int menuItem, void *callbackData, int panel)
{
	if(parityBitTicked == 1)
	{
		parityBitTicked = 0;
		SetMenuBarAttribute(menubar,MENU_CONFIG_COMMUNICATION_PARITY_BIT,ATTR_CHECKED,0);
	}
	else
	{
		parityBitTicked = 1;
		SetMenuBarAttribute(menubar,MENU_CONFIG_COMMUNICATION_PARITY_BIT,ATTR_CHECKED,1);
	}
	
}
void CVICALLBACK tickStopBitCB(int menubar, int menuItem, void *callbackData, int panel)
{
	if(stopBitTicked == 1)
	{
		stopBitTicked = 0;		
		SetMenuBarAttribute(menubar,MENU_CONFIG_COMMUNICATION_STOP_BIT,ATTR_CHECKED,0);
	}
	else
	{
		stopBitTicked = 1;
		SetMenuBarAttribute(menubar,MENU_CONFIG_COMMUNICATION_STOP_BIT,ATTR_CHECKED,1);	
	}
		
}
void CVICALLBACK changePortNumberCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cPort[8];
	int status = GenericMessagePopup("MK_Nav Message",
									"Choisissez un port",
									"Valider",
									"Annuler",//pas de deuxieme
									 0,//pas de troisième bouton
									 cPort,//pas de reponse utilisateur
									 7,//taille de la reponse
									 0,//boutons en bas de la fenetre
									 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
									 VAL_GENERIC_POPUP_BTN1,
									 VAL_GENERIC_POPUP_NO_CTRL);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		portNumber = atoi(cPort);	
	}
}
void CVICALLBACK changeBaudRateCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cBaudRate[8];
	int status = GenericMessagePopup("MK_Nav Message",
									"Choisissez le débit binaire (Bauds)",
									"Valider",
									"Annuler",//pas de deuxieme
									 0,//pas de troisième bouton
									 cBaudRate,//pas de reponse utilisateur
									 7,//taille de la reponse
									 0,//boutons en bas de la fenetre
									 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
									 VAL_GENERIC_POPUP_BTN1,
									 VAL_GENERIC_POPUP_NO_CTRL);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		baudRate = atoi(cBaudRate);	
	}	
}

/* Réinitialiser : effacer tous les points */

void CVICALLBACK effacerCarteCB (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	  		int nb_columns = 3;
			int confirmation_erase = 0;  
			
			//confirmation_erase = ConfirmPopup("Confirmation", "Etes-vous sûr de vouloir tout supprimer ?");
			confirmation_erase = GenericMessagePopup("MK_Nav Message",
													"Etes vous sûr de vouloir effacer la carte ?",
													"Oui",
													"Non",
													0,//pas de troisième bouton
													0,//pas de reponse utilisateur
													0,//taille de la reponse
													0,//boutons en bas de la fenetre
													VAL_GENERIC_POPUP_NO_CTRL, //keystrokes
													VAL_GENERIC_POPUP_BTN1,
													VAL_GENERIC_POPUP_BTN2);
													
													
			//Si l'utilisateur confirme on supprime tout
			if(confirmation_erase == VAL_GENERIC_POPUP_BTN1){
				
				//Supprime l'image et le trajet
				CanvasClear (panel, PANEL_CANVAS, VAL_ENTIRE_OBJECT);
		
				
				
				image_chargee = 0;		//Image non chargée
				
				SetMenuBarAttribute(menuBar,MENU_FICHIER_Sauvegarder_Projet,ATTR_DIMMED,1);
				CVIXMLDiscardDocument(gpxDoc);
				SetCtrlVal(panel,PANEL_TEXT_LAT,"");
				SetCtrlVal(panel,PANEL_TEXT_LON,"");
				nb_waypoint = 0;
				DeleteTableRows(panel,PANEL_TABLE,1,-1);
			}
}
void CVICALLBACK saveRun(int menubar, int menuItem, void *callbackData, int panel)
{
	char gpxPath[256];
	int i;
	char cLat[16], cLon[16], cAlt[8];
	CVIXMLElement trkpt;
	CVIXMLElement ele;
	for(i=0;i<nb_waypoint;i++)
	{
		sprintf(cLat,"%f",trajet[i].fLat);
		sprintf(cLon,"%f",trajet[i].fLon);
		sprintf(cAlt,"%d",trajet[i].ele);
		CVIXMLNewElement(trkSeg,-1,"trkpt",&trkpt);
		CVIXMLAddAttribute(trkpt,"lat",cLat);
		CVIXMLAddAttribute(trkpt,"lon",cLon);
	
		CVIXMLNewElement(trkpt,-1,"ele",&ele);
		CVIXMLSetElementValue(ele,cAlt);
	}
	sprintf(gpxPath,"Gpx\\%s",gpxFileName);
	CVIXMLSaveDocument(gpxDoc,0,gpxPath);
}


void CVICALLBACK EffacerTrajetCB(int menubar, int menuItem, void *callbackData, int panel)
{
	int nb_columns = 3;
	int confirmation_erase = 0;  
	
	//confirmation_erase = ConfirmPopup("Confirmation", "Etes-vous sûr de vouloir tout supprimer ?");
	confirmation_erase = GenericMessagePopup("MK_Nav Message",
											"Etes vous sûr de vouloir effacer le trajet ?",
											"Oui",
											"Non",
											0,//pas de troisième bouton
											0,//pas de reponse utilisateur
											0,//taille de la reponse
											0,//boutons en bas de la fenetre
											VAL_GENERIC_POPUP_NO_CTRL, //keystrokes
											VAL_GENERIC_POPUP_BTN1,
											VAL_GENERIC_POPUP_BTN2);
											
											
	//Si l'utilisateur confirme on supprime tout
	if(confirmation_erase == VAL_GENERIC_POPUP_BTN1){
		
		//Supprime l'image et le trajet
		CanvasClear (panel, PANEL_CANVAS, VAL_ENTIRE_OBJECT);
		//réafficher la carte
		CanvasDrawBitmap (panel, PANEL_CANVAS, image_GPS, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
		
		CVIXMLDiscardDocument(gpxDoc);
		SetCtrlVal(panel,PANEL_TEXT_LAT,"");
		SetCtrlVal(panel,PANEL_TEXT_LON,"");
		nb_waypoint = 0;
		DeleteTableRows(panel,PANEL_TABLE,1,-1);
		initGpxDocument();//réinitialisation pour cette fois...
	}
}

void CVICALLBACK importerTrajetCB(int menubar, int menuItem, void *callbackData, int panel)
{
	CVIXMLElement trkNode;
	CVIXMLElement trkpt;
	CVIXMLElement ele;
	CVIXMLAttribute lat;
	CVIXMLAttribute lon;
	int mouseX,mouseY;
	int nbTrkpt,i;
	char gpxPath[512];
	char cAltitude[5];
	char cLat[16];
	char cLon[16];
	char wpt[12];
	if(image_chargee != 0) //si une map est chargée
	{
		char dir[330];
		sprintf(dir,"%s\\Gpx",curDir);
		if(FileSelectPopup (dir, "*.gpx", "*.*", "Charger un trajet", VAL_LOAD_BUTTON, 0, 0, 1, 0, gpxPath) <= 0)
		{		/* Si le trajet n'est pas chargée */
		}
		else
		{
			if(nb_waypoint > 0)
			{					
				
				//Supprime l'image et le trajet
				CanvasClear (panel, PANEL_CANVAS, VAL_ENTIRE_OBJECT);
				CVIXMLDiscardDocument(gpxDoc);
				SetCtrlVal(panel,PANEL_TEXT_LAT,"");
				SetCtrlVal(panel,PANEL_TEXT_LON,"");
				nb_waypoint = 0;
				DeleteTableRows(panel,PANEL_TABLE,1,-1);
				CanvasDrawBitmap (panel, PANEL_CANVAS, image_GPS, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);	
			}
			CVIXMLStatus status;
			char error[128];
			if((status = CVIXMLLoadDocument(gpxPath,&gpxDoc)) == 0)
			{
				CVIXMLGetRootElement(gpxDoc,&rootNode);
				CVIXMLGetChildElementByTag(rootNode,"trk",&trkNode);
				CVIXMLGetChildElementByTag(trkNode,"trkseg",&trkSeg);
				CVIXMLGetNumChildElements(trkSeg,&nbTrkpt);
				for(i = 0; i<nbTrkpt;i++)
				{
					nb_waypoint++;
					CVIXMLGetChildElementByIndex(trkSeg,i,&trkpt);
					
					CVIXMLGetAttributeByName(trkpt,"lat",&lat);
					CVIXMLGetAttributeByName(trkpt,"lon",&lon);
					CVIXMLGetAttributeValue(lat,cLat);
					CVIXMLGetAttributeValue(lon,cLon);
					fLat = atof(cLat);
					fLon = atof(cLon);
					trajet[i].fLat = fLat;
					trajet[i].fLon = fLon;
					
					mouseX =  (fLon-fLonMin)/fLonPix;
					mouseY =  (fLatMax-fLat)/fLatPix;
					trajet[i].iPosX = mouseX;
					trajet[i].iPosY = mouseY;
				
			
					CVIXMLGetChildElementByTag(trkpt,"ele",&ele);
					CVIXMLGetElementValue(ele,cAltitude);
					trajet[i].ele = atoi(cAltitude); 	
				}
				traceGpx(panel);
			}
			else
			{
				CVIXMLGetErrorString(status,error,127);
				printf("\nerror : %s",error);
			}
		}
		
	}
	else
	{
		  GenericMessagePopup("MK_Nav Message",
													"Vous devez importer une carte avant d'importer un trajet",
													"Ok",
													0,
													0,//pas de troisième bouton
													0,//pas de reponse utilisateur
													0,//taille de la reponse
													0,//boutons en bas de la fenetre
													VAL_GENERIC_POPUP_NO_CTRL, //keystrokes
													VAL_GENERIC_POPUP_BTN1,
													VAL_GENERIC_POPUP_BTN1);
	}
}

void traceGpx(int panel)
{
	
	int i,nbRows;
	char wpt[12];
	GetNumTableRows(panel,PANEL_TABLE,&nbRows);
	for(i = nbRows; i < nb_waypoint;i++)
	{
		InsertTableRows(panel,PANEL_TABLE,-1,1,-1);
		SetTableCellVal(panel,PANEL_TABLE,MakePoint(1,i+1),trajet[i].fLat);
		SetTableCellVal(panel,PANEL_TABLE,MakePoint(2,i+1),trajet[i].fLon);
		SetTableCellVal(panel,PANEL_TABLE,MakePoint(3,i+1),trajet[i].ele);
		
		SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_WIDTH,1);
		SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_COLOR,VAL_RED);
		SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_FILL_COLOR,VAL_RED);
		//Trace un losange à partir de la zone de clic
		Point points[4] = {{trajet[i].iPosX-3,trajet[i].iPosY},{trajet[i].iPosX,trajet[i].iPosY-3},{trajet[i].iPosX+3,trajet[i].iPosY},{trajet[i].iPosX,trajet[i].iPosY+3}};
		CanvasDrawPoly(panel,PANEL_CANVAS,4,points,1,VAL_DRAW_FRAME_AND_INTERIOR);
	
		if(i > 0)
		{
			SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_WIDTH,2);
			CanvasDrawLine(panel,PANEL_CANVAS,MakePoint(trajet[i].iPosX,trajet[i].iPosY),MakePoint(trajet[i-1].iPosX,trajet[i-1].iPosY));
		}
	
	
	
		sprintf(wpt,"%d-%dm",i+1,trajet[i].ele);	
		SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_FILL_COLOR,VAL_OFFWHITE);
		SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_COLOR,VAL_BLACK);
		if(trajet[i].iPosX <= 320)
		{
			if(trajet[i].iPosY <= 320)
			{
				CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[i].iPosX,trajet[i].iPosY),VAL_UPPER_LEFT);		
			}
			else
			{
				CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[i].iPosX,trajet[i].iPosY),VAL_LOWER_LEFT);							
			}
		}
		else
		{
			if(trajet[i].iPosY <= 320)
			{
				CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[i].iPosX,trajet[i].iPosY),VAL_UPPER_RIGHT);
			}
			else
			{
				CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[i].iPosX,trajet[i].iPosY),VAL_LOWER_RIGHT);
			}
		}	
	}
}

int CVICALLBACK canvas (int panel, int control, int event, void *callbackData, int eventData1, int eventData2){
	int popup;
	
	int mouseX = 0, mouseY = 0;
	float lat = 0.0, longi = 0.0;
	char responseBuffer[65];
	char wpt[12];
	char cLat[16];
	char cLon[16];
	char cAltitude[5];
	
	switch (event)    
	{
		case EVENT_LEFT_CLICK:
			//On intéragit uniquement si l'image a préalablement été charger
			if(image_chargee == 1)
			{
				if(nb_waypoint < MAX_WAYPOINTS)
				{
					//Récupération des coordonnées de la souris au moment du clic
					GetRelativeMouseState (panel, PANEL_CANVAS, &mouseX, &mouseY, 0, 0, 0); 
					//printf("coordonnees -- x = %d\n              y = %d\n",mouseX,mouseY);
					if(nb_waypoint == 0)
					{
						int ok = 0;
						while(!ok)
						{
							GenericMessagePopup("MK_Nav Message",
												"Quelle altitude(m) pour ce point ?",
												"Valider",
												 0,//pas de deuxieme
												 0,//pas de troisième bouton
												 cAltitude,//pas de reponse utilisateur
												 4,//taille de la reponse
												 0,//boutons en bas de la fenetre
												 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
												 VAL_GENERIC_POPUP_BTN1,
												 VAL_GENERIC_POPUP_NO_CTRL);
							if(atoi(cAltitude) > altMax || atoi(cAltitude) < altMin)
							{
								char sentence[128];
								sprintf(sentence,"L'altitude du drone doit être comprise entre %dm  et %dm",altMin,altMax);
								GenericMessagePopup("MK_Nav Message",
													sentence,
													"Ok",
													0,
													0,//pas de troisième bouton
													0,//pas de reponse utilisateur
													0,//taille de la reponse
													0,//boutons en bas de la fenetre
													VAL_GENERIC_POPUP_NO_CTRL, //keystrokes
													VAL_GENERIC_POPUP_BTN1,
													VAL_GENERIC_POPUP_BTN1);		
							}
							else
							{
								ok = 1;	
							}
						}
					}
					else
					{
						if(trajet[nb_waypoint-1].ele > altMax)
							sprintf(cAltitude,"%d",altMax);
						else if(trajet[nb_waypoint-1].ele < altMin)
							sprintf(cAltitude,"%d",altMin);
						else
							sprintf(cAltitude,"%d",trajet[nb_waypoint-1].ele); //on prend l'altitude du point précédent
					}
					fLat = fLatMax - mouseY*fLatPix;
					fLon = fLonMin + mouseX*fLonPix;
				
					//printf("coordonnees -- lat = %f\n              lon = %f\n",fLat,fLon);
				
				
				 
					if(trajectoirePossible(mouseX,mouseY) == 1  && distancePossibe(fLat,fLon) == 1)
					{
						trajet[nb_waypoint].fLat = fLat;
						trajet[nb_waypoint].fLon = fLon;
						trajet[nb_waypoint].ele = atoi(cAltitude);
						trajet[nb_waypoint].iPosX = mouseX;
						trajet[nb_waypoint].iPosY = mouseY;
						
						sprintf(cLat,"%f",fLat);
						SetCtrlVal(panel,PANEL_TEXT_LAT,cLat);
						sprintf(cLon,"%f",fLon);
						SetCtrlVal(panel,PANEL_TEXT_LON,cLon);
					
						nb_waypoint++;
						
						traceGpx(panel);
				
					}//if(trajectoirePossible(mouseX,mouseY) == 1)
				}//if(nb_waypoint < MAX_WAYPTS) 
			}//if(image_chargee == 1)
			break;
		case EVENT_RIGHT_CLICK:
			if(image_chargee == 1 && nb_waypoint > 0)
			{
				int i;
				GetRelativeMouseState (panel, PANEL_CANVAS, &mouseX, &mouseY, 0, 0, 0);
				for(i=0;i<nb_waypoint;i++)
				{
					 if(trajet[i].iPosX <= (mouseX+3) &&  trajet[i].iPosX >= (mouseX-3))
						 if(trajet[i].iPosY <= (mouseY+3) &&   trajet[i].iPosY >= (mouseY-3))
						 {
							indexToDelete = i;
							RunPopupMenu(MenuBar,MenuID, panel, eventData1, eventData2, 300, 400, 10, 10) ;
						 }
				}
			}
			break;
		
			
	}//switch
	return 0;
}

void deletePointFromCourse(int i)
{
	if(i>=0 && i<=nb_waypoint)
	{
		int j;
		for(j=i;j<nb_waypoint-1;j++)
		{
			trajet[j] = trajet[j+1];	
		}
	}
}

void CVICALLBACK DelWptCallback (int menuBarHandle, int menuItemID, void *callbackData, int panel) 
{

	deletePointFromCourse(indexToDelete);
	nb_waypoint--;

	//Supprime l'image et le trajet
	CanvasClear (panelHandle, PANEL_CANVAS, VAL_ENTIRE_OBJECT);
	//réafficher la carte
	CanvasDrawBitmap (panelHandle, PANEL_CANVAS, image_GPS, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
	//Trace le nouveau trajet
	DeleteTableRows(panel,PANEL_TABLE,1,-1);  
	traceGpx(panel);
}



void addPointToGpx(char cLat[],char cLon[],char alt[])
{
	CVIXMLElement trkpt;
	CVIXMLElement ele;
	
	CVIXMLNewElement(trkSeg,-1,"trkpt",&trkpt);
	CVIXMLAddAttribute(trkpt,"lat",cLat);
	CVIXMLAddAttribute(trkpt,"lon",cLon);
	
	CVIXMLNewElement(trkpt,-1,"ele",&ele);
	CVIXMLSetElementValue(ele,alt);
}

int trajectoirePossible(int mouseX,int mouseY)
{
	int ok = 0;
	
	int currentVect[2];
	int newVect[2];
	float phi;
	float newVectMod,currentVectMod;
	if(nb_waypoint <= 1)
	{
		ok=1;
	}
	else if(nb_waypoint >=2) //a partir du 3eme waypoint
	{
		currentVect[0] = trajet[nb_waypoint-2].iPosX - trajet[nb_waypoint-1].iPosX;//vecteur partant du nouveau point vers le précedent
		currentVect[1] = trajet[nb_waypoint-2].iPosY - trajet[nb_waypoint-1].iPosY;
		
		newVect[0] = trajet[nb_waypoint-1].iPosX - mouseX;
		newVect[1] = trajet[nb_waypoint-1].iPosY - mouseY;
		
		int prod = newVect[0]*currentVect[0] + newVect[1]*currentVect[1];
		newVectMod = sqrt(newVect[0]*newVect[0]+newVect[1]*newVect[1]);
		currentVectMod = sqrt(currentVect[0]*currentVect[0]+currentVect[1]*currentVect[1]);
		phi = acos(prod/(newVectMod*currentVectMod));
		// u.v = |u|.|v|.cos(phi)
		phi = phi*180/(2*acos(0.0)); //2cos(0) = pi
		
		if(!(ok = (phi <= angleMax)))
		{
		char sentence[128];
		sprintf(sentence,"Les changements de directions supérieurs à %d° ne sont pas permis",angleMax);
		GenericMessagePopup("MK_Nav Message",
							sentence,
							"Ok",
							0,
							0,//pas de troisième bouton
							0,//pas de reponse utilisateur
							0,//taille de la reponse
							0,//boutons en bas de la fenetre
							VAL_GENERIC_POPUP_NO_CTRL, //keystrokes
							VAL_GENERIC_POPUP_BTN1,
							VAL_GENERIC_POPUP_BTN1);	
		}
	}
	return ok;
}

int distancePossibe(float fLat, float fLon)
{
	int ok = 0;
	if(nb_waypoint >= 1)
	{
		float distance;
		distance = 6371000 * acos(sin(fLat*PI/180)*sin(trajet[nb_waypoint-1].fLat*PI/180)+cos(fLat*PI/180)*cos(trajet[nb_waypoint-1].fLat*PI/180)*cos(fLon*PI/180-trajet[nb_waypoint-1].fLon*PI/180));
		if(!(ok = (distance >= distanceMin)))
		{
			char sentence[128];
			sprintf(sentence,"Les déplacements de moins de  %d mètres ne sont pas permis",distanceMin);
			GenericMessagePopup("MK_Nav Message",
								sentence,
								"Ok",
								0,
								0,//pas de troisième bouton
								0,//pas de reponse utilisateur
								0,//taille de la reponse
								0,//boutons en bas de la fenetre
								VAL_GENERIC_POPUP_NO_CTRL, //keystrokes
								VAL_GENERIC_POPUP_BTN1,
								VAL_GENERIC_POPUP_BTN1);	
		}
	}
	else
	{
		ok = 1;	
	}
	return ok;
}

void CVICALLBACK changeDistanceMinCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cDist[17];
	int status = GenericMessagePopup("MK_Nav Message",
									 "Veuillez choisir une distance minimum",
									 "Valider",
						 			 "Annuler",
						 			 0,//pas de troisième bouton
						 			 cDist,//pas de reponse utilisateur
						 		  	 16,//taille de la reponse
						 			 0,//boutons en bas de la fenetre
						 			 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
					 				 VAL_GENERIC_POPUP_BTN1,
						 			 VAL_GENERIC_POPUP_BTN2);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		distanceMin = atoi(cDist);	
	}	
}

/*  --------------------- PROBLEMES A RESOUDRE : ---------------------
- reinit de .gpx bug
- reinit de .gpx plus gd bug                      => WAYPOINTS_MAX
*/


/*  --------------------- IMPROVEMENTS : --------------------- 
- structure de données
- introduire WAYPOINT_MAX dans un popup d'erreur
- Charger le lieu en fonction du fichier gpx et faire le parcours automatiquement
*/


/*  --------------------- TO DO : --------------------- 
- gestion d'erreur entre les waypoints
*/

int CVICALLBACK mainPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

void CVICALLBACK importerCarteCB(int menubar, int menuItem, void *callbackData, int panel)
{
	int popup;
	char dir[330];
	sprintf(dir,"%s\\maps",curDir);
	if(FileSelectPopup (dir, "*.png", "*.*", "Charger une carte", VAL_LOAD_BUTTON, 0, 0, 1, 0, file_path) <= 0)
	{		/* Si la carte n'est pas chargée */
	}
	else
	{
		nb_waypoint=0;
		getDataFromFileName(file_path);
		//Récupération de l'image
		GetBitmapFromFile(file_path, &image_GPS);
		//Affiche l'image
		CanvasDrawBitmap (panel, PANEL_CANVAS, image_GPS, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
		
		image_chargee = 1;		//Image chargée
		SetMenuBarAttribute(menubar,MENU_FICHIER_Sauvegarder_Projet,ATTR_DIMMED,0);

	}
}

void initGpxDocument()
{
	time_t timestamp;
	struct tm *t;
	char ctime[128];
	CVIXMLElement metadata;
	CVIXMLElement desc;
	CVIXMLElement author;
	CVIXMLElement name,trkName;
	CVIXMLElement email;
	CVIXMLElement trkNode;
	CVIXMLElement trkTime;
	
	CVIXMLNewDocument("gpx",&gpxDoc);
	CVIXMLGetRootElement(gpxDoc,&rootNode);
	CVIXMLAddAttribute(rootNode,"creator","Ensim Drone Proj");
	CVIXMLAddAttribute(rootNode,"xmlns","http://www.topografix.com/GPX/1/1");
	CVIXMLAddAttribute(rootNode,"xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
	CVIXMLAddAttribute(rootNode,"xsi:schemaLocation","http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd");
	
	CVIXMLNewElement(rootNode,-1,"metadata", &metadata);
	
	CVIXMLNewElement(metadata,-1,"desc",&desc);
	CVIXMLSetElementValue(desc,"Drone Run");
	
	CVIXMLNewElement(metadata,-1,"author",&author);
	CVIXMLNewElement(author,-1,"name",&name);
	CVIXMLSetElementValue(name,"Drone Project Team2012");
	CVIXMLNewElement(author,-1,"email",&email);
	CVIXMLAddAttribute(email,"id","thomas.prampart.etu");
	CVIXMLAddAttribute(email,"domain","univ-lemans.fr");
	
	CVIXMLNewElement(rootNode,-1,"trk",&trkNode);
	CVIXMLNewElement(trkNode,-1,"name",&trkName);
	CVIXMLSetElementValue(trkName,"Drone Rune");
	CVIXMLNewElement(trkNode,-1,"time",&trkTime);
	
	timestamp = time(NULL);
	t = localtime(&timestamp);
	int year = 1900 + t->tm_year;
	sprintf(ctime,"%d-%d-%dT%d:%d:%dZ",year,t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	
	CVIXMLSetElementValue(trkTime,ctime);
	
	CVIXMLNewElement(trkNode,-1,"trkseg",&trkSeg);
}

void getDataFromFileName(char file[])
{
	int TEMPSIZE = 32;
	char *name;
	char *extension;
	char temp[TEMPSIZE];
	int i=0,j=0; 
	memset(temp,0x0,TEMPSIZE);//mise a zero de temp
	name = strrchr(file,'\\');//name pointe le dernier antislash de file 
	name = name + 1;//permet d'enlever l'antislash en début de chaine
	
	strcpy(gpxFileName,name);
	extension = strrchr(gpxFileName,'.');
	strcpy(extension,".gpx");
	
	//printf("filename = %s\n",name);
	for(i=0,j=0;name[i] !='_';i++,j++)
	{
		temp[j] = name[i];	
	}
	fLatCenter = atof(temp); // stockage Latitude du centre de la carte
	i++;//pour sauter la ',' dans name[]
	memset(temp,0x0,TEMPSIZE);
	for(j=0;name[i] != '_';i++,j++)
	{
		temp[j]=name[i];	
	}
	fLonCenter = atof(temp);
	memset(temp,0x0,TEMPSIZE);
	i++;
	for(j=0;name[i] != '_';i++,j++)
	{
		temp[j] = name[i];	
	}
	zoomLevel = atoi(temp);
	switch(zoomLevel)
	{
		case 16:
			float latWidth = 0.009187, lonWidth = 0.013733;
			fLatPix = latWidth/640;
			fLonPix = lonWidth/640;
			fLatMax = fLatCenter + latWidth/2;
			fLonMin = fLonCenter - lonWidth/2;
			break;
		case 18:
			break;
	}
	
	initGpxDocument();
	
	/*
	printf("Latitude = %f\n",centerLatitude);
	printf("Longitude = %f\n",centerLongitude);
	printf("zoom level = %d\n",zoomLevel);
	*/
}


int  CVICALLBACK TableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch(event)
	{
		
		case EVENT_VAL_CHANGED:
			
			char wpt[12];
			int val;
			GetTableCellVal(panel,PANEL_TABLE,MakePoint(eventData2,eventData1),&val);
			if(val<= altMax)
				trajet[eventData1-1].ele = val;
			else
			{
				trajet[eventData1-1].ele = altMax;
				SetTableCellVal(panel,PANEL_TABLE,MakePoint(eventData2,eventData1),altMax);
			}
			sprintf(wpt,"%d-%dm",eventData1,trajet[eventData1-1].ele);	
			SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_FILL_COLOR,VAL_OFFWHITE);
			SetCtrlAttribute(panel,PANEL_CANVAS,ATTR_PEN_COLOR,VAL_BLACK);
			if(trajet[eventData1-1].iPosX <= 320)
			{
				if(trajet[eventData1-1].iPosY <= 320)
				{
					CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[eventData1-1].iPosX,trajet[eventData1-1].iPosY),VAL_UPPER_LEFT);		
				}
				else
				{
					CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[eventData1-1].iPosX,trajet[eventData1-1].iPosY),VAL_LOWER_LEFT);							
				}
			}
			else
			{
				if(trajet[eventData1-1].iPosY <= 320)
				{
					CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[eventData1-1].iPosX,trajet[eventData1-1].iPosY),VAL_UPPER_RIGHT);
				}
				else
				{
					CanvasDrawTextAtPoint(panel,PANEL_CANVAS,wpt,VAL_MESSAGE_BOX_META_FONT,MakePoint(trajet[eventData1-1].iPosX,trajet[eventData1-1].iPosY),VAL_LOWER_RIGHT);
				}
			}
			break;
	}						
	return 0;
}



void CVICALLBACK SAVE_PROJECT_CB(int menubar, int menuItem, void *callbackData, int panel)
{
	char cProject[512];
	char cProjectName[128];
	int status = GenericMessagePopup("MK_Nav Message",
									 "Sous quel nom souhaitez vous sauvegarder ce projet?",
									 "Valider",
						 			 "Annuler",
						 			 0,//pas de troisième bouton
						 			 cProjectName,
						 		  	 127,//taille de la reponse
						 			 0,//boutons en bas de la fenetre
						 			 VAL_GENERIC_POPUP_INPUT_STRING, //keystrokes
					 				 VAL_GENERIC_POPUP_BTN1,
						 			 VAL_GENERIC_POPUP_BTN2);
	if(status == VAL_GENERIC_POPUP_BTN1)
	{
		char cElem[256];
		char *map = strrchr(file_path,'\\'); 
		map++;	
		sprintf(cProject,"%s\\projects\\%s.nav",curDir,cProjectName);
	   	fProj = fopen(cProject,"w+");
		
		
			
		sprintf(cElem,"map=%s\n",map);
		fwrite(cElem,strlen(cElem),1,fProj);
		
		sprintf(cElem, "trajet=%s\n",gpxFileName);
		fwrite(cElem,strlen(cElem),1,fProj);		
	
		fclose(fProj);
	}
}


void CVICALLBACK openProjectCB(int menubar, int menuItem, void *callbackData, int panel)
{
	char dir[256];
	char projectFile[512];
	sprintf(dir,"%s\\projects",curDir);
	if(FileSelectPopup (dir, "*.nav", "*.*", "Ouvrir un projet", VAL_LOAD_BUTTON, 0, 0, 1, 0, projectFile) <= 0)
	{		/* Si le trajet n'est pas chargée */
	}
	else
	{
		char target[512];
		fProj = fopen(projectFile,"r+");
		fgets(target,512,fProj);
		
		char* map = strrchr(target,'=');
		map++;
		static char path[512];
		sprintf(path,"H:\\ECHANGES_ETUDIANTS\\PROMO-1013\\drone\\MKNav_v2\\maps\\%s\0",map);
		printf("%d",strlen(path));
		//Récupération de l'image
		GetBitmapFromFile(path, &image_GPS);
		//Affiche l'image
		CanvasDrawBitmap (panel, PANEL_CANVAS, image_GPS, VAL_ENTIRE_OBJECT, VAL_ENTIRE_OBJECT);
		
		image_chargee = 1;		//Image chargée
		SetMenuBarAttribute(menubar,MENU_FICHIER_Sauvegarder_Projet,ATTR_DIMMED,0);
	}
}
