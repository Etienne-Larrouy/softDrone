//==============================================================================
//
// Title:       serial.h
// Purpose:     Fonctions de base pour parametrer l'envoi et la réception de données sur un port
//				série ainsi que l'encodage de trames.
//
// Created by DY David.
//
//==============================================================================

#ifndef __serial_H__
#define __serial_H__
#endif

#ifdef __cplusplus
    extern "C" {
#endif

#include <windows.h> 	// Windows API, port série
#include <stdio.h>		// Standard I/O  
#include <rs232.h>		// Liaison RS232
#include "cvidef.h"
#include "globals.h"

 
		

/* --------- */		
/* Variables */
/* --------- */

HANDLE hCom;			// Le port COM est vu comme un fichier special
						// Handler du port 

char tx_buffer[150];	// Buffer de transmission (trame)
char rx_buffer[150];	// Buffer de reception
int num_com;			// numero du port COM



/* ---------- */
/* Prototypes */
/* ---------- */

// Initialisation du port COM
int init_port_com(int comNumber);

// Ajout du CRC
void AddCRC(unsigned int frame_length);

// Constitution et envoi de la trame
void SendData(unsigned char cmd,unsigned char addr, unsigned char *snd, unsigned char len);

// Fonction de decodage de trame, pour la reception
void Decode64(unsigned char *ptrOut, unsigned char len, unsigned char ptrIn,unsigned char max); 

// Envoi de la trame
int sendStringToCom(char* toSend, int length);

					

