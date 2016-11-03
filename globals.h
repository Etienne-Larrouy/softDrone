//==============================================================================
//
// Title:       globals.h
// Purpose:     Definitions de tout ce qui est utile au programme.
//
// Created by DY David.
//
//==============================================================================



/* Prototype(s) */
void getDataFromFileName(char file[]) ;
void initGpxDocument();
void addPointToGpx(char cLat[],char cLon[],char cAltitude[]);
void traceGpx(int panel);
int distancePossibe(float fLat,float fLon);
int trajectoirePossible(int mouseX,int mouseY);


/* Definition de types */
typedef unsigned char u8;
typedef signed long s32;
typedef signed short s16;

/*
// Structure d'un point GPS 
struct GPS_Pos_t
{
	s32 Longitude;
	s32 Latitude;
	s32 Altitude;
	u8 Status;
};

// Structure d'un waypoint 
struct Point_t 
{
	struct GPS_Pos_t Position;
	s16 Heading;
	u8 ToleranceRadius;
	u8 HoldTime;
	u8 EventFlag;
	u8 Index;
	u8 reserve[8];
};

// Parametres pour un point 
#define HEADING_OFF			0
#define HEADING_POI			-1
#define RADIUS				5
#define HOLD_TIME 			60
#define POINT_TYPE_INVALID	255
#define POINT_TYPE_WP		0
#define POINT_TYPE_POI		1


// GPSData flags 
#define INVALID 	0x00
#define	NEWDATA		0x01
#define	PROCESSED	0x02


// Adresses 
#define FC_ADDRESS 		1 
#define NC_ADDRESS 		2 
#define MK3MAG_ADDRESS  3 
#define BL_CTRL_ADDRESS 5 

 

// structure ExternalControl, envoyee au Mikrokopter 
struct str_ExternControl
{
unsigned char Digital[2];        
unsigned char RemoteButtons; 
signed char   Nick;                     
signed char   Roll;
signed char   Gier;
unsigned char Gas;              // valeur de gaz
signed char   Hight;    
unsigned char free;             
unsigned char Frame;    		// valeur retournee par la FC pour confirmation
unsigned char Config;  			// 1 -> activation du contrôle externe
};
        
struct str_ExternControl ExternControl; // Creation d'une instance de cette structure

*/
