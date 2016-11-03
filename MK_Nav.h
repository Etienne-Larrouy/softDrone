/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2013. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: mainPanelCB */
#define  PANEL_TEXTMSG_4                  2       /* control type: textMsg, callback function: (none) */
#define  PANEL_CANVAS                     3       /* control type: canvas, callback function: canvas */
#define  PANEL_DECORATION_2               4       /* control type: deco, callback function: (none) */
#define  PANEL_TEXT_LON                   5       /* control type: string, callback function: (none) */
#define  PANEL_TEXT_LAT                   6       /* control type: string, callback function: (none) */
#define  PANEL_TABLE                      7       /* control type: table, callback function: TableCB */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

#define  MENU                             1
#define  MENU_FICHIER                     2
#define  MENU_FICHIER_ITEM6               3       /* callback function: openProjectCB */
#define  MENU_FICHIER_IMPORTERCARTE       4       /* callback function: importerCarteCB */
#define  MENU_FICHIER_IMPORTERGPX         5       /* callback function: importerTrajetCB */
#define  MENU_FICHIER_Sauvegarder_Projet  6       /* callback function: SAVE_PROJECT_CB */
#define  MENU_FICHIER_SAUVEGARDERGPX      7       /* callback function: saveRun */
#define  MENU_FICHIER_ITEM8               8
#define  MENU_EDITION                     9
#define  MENU_EDITION_ITEM3               10      /* callback function: EffacerTrajetCB */
#define  MENU_EDITION_EFFACERCARTE        11      /* callback function: effacerCarteCB */
#define  MENU_CONFIG                      12
#define  MENU_CONFIG_ITEM5                13
#define  MENU_CONFIG_ITEM5_SUBMENU        14
#define  MENU_CONFIG_ITEM5_ITEM1          15      /* callback function: changeAngleMaxCB */
#define  MENU_CONFIG_ITEM5_ITEM2          16      /* callback function: changeDistanceMinCB */
#define  MENU_CONFIG_ITEM5_ITEM4_2        17      /* callback function: changeAltMaxCB */
#define  MENU_CONFIG_ITEM5_ITEM5_2        18      /* callback function: changePenteMaxCB */
#define  MENU_CONFIG_COMMUNICATION        19
#define  MENU_CONFIG_COMMUNICATION_SUBMENU 20
#define  MENU_CONFIG_COMMUNICATION_PORT   21      /* callback function: changePortNumberCB */
#define  MENU_CONFIG_COMMUNICATION_DEBIT  22      /* callback function: changeBaudRateCB */
#define  MENU_CONFIG_COMMUNICATION_PARITY_BIT 23  /* callback function: tickParityBitCB */
#define  MENU_CONFIG_COMMUNICATION_STOP_BIT 24    /* callback function: tickStopBitCB */
#define  MENU_CONFIG_ITEM6_2              25
#define  MENU_CONFIG_ITEM7_2              26
#define  MENU_MENU1                       27


     /* Callback Prototypes: */

int  CVICALLBACK canvas(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK changeAltMaxCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK changeAngleMaxCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK changeBaudRateCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK changeDistanceMinCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK changePenteMaxCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK changePortNumberCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK effacerCarteCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK EffacerTrajetCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK importerCarteCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK importerTrajetCB(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK mainPanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK openProjectCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK SAVE_PROJECT_CB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK saveRun(int menubar, int menuItem, void *callbackData, int panel);
int  CVICALLBACK TableCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK tickParityBitCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK tickStopBitCB(int menubar, int menuItem, void *callbackData, int panel);
void CVICALLBACK DelWptCallback (int menuBarHandle, int menuItemID, void *callbackData, int panel);

#ifdef __cplusplus
    }
#endif
