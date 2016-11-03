
//==============================================================================
//
// Title:       serial.c
// Purpose:     Fonctions de base pour parametrer l'envoi et la réception de données sur un port
//				série ainsi que l'encodage de trames.
//
// Created by DY David
//
// !!!!!!!!!!!!!!!!! Only Windows compatible at the moment !!!!!!!!!!!!!!!!!
//
// The encode and decode algorithms are part of the FlightCtrl software and
// are therefore written by the MikroKopter team.
//
//==============================================================================

#include "serial.h"


// Initialisation des paramètres du port COM
// La FlightControl communique à:
// 57600 Baud (bps), 8 Data bits, no parity bit, 1 stop bit
int init_port_com(int comNumber) {
	
	DCB dcb;						 // Device Control Block, contient la Configuration pour le port COM
	char comPort[6] = {'C','O','M'}; // Numero et caractere de fin a ajouter
	int fSuccess;					 // Variable indiquant si l'action a reussi ou erreur
	COMMTIMEOUTS timeouts;	         // Structure contenant la valeur du timeout
							

	// Recuperation du numero de port
	if( (comNumber>0) && (comNumber<10) ) {

		comPort[3] = 48 + comNumber;	// COMx (x code en ASCII) - 0 a pour valeur decimale 48 en ascii
		comPort[4] = 0;					// Ajout du caractere de fin 'NUL' - 0 en ascii
		comPort[5] = 0;	
		printf("-->numero %s\n", comPort);
	}

	else {
		if(comNumber>=10 && comNumber<=99) {

		comPort[3] = 48 + (comNumber/10);	// Recupere le chiffre des dizaines
		comPort[4] = 48 + (comNumber%10);   // Recupere le chiffre des unites
		comPort[5] = 0;						// Ajout du caractere de fin 'NUL' - 0 en ascii
		printf("-->numero %s\n", comPort);
		}

		else { 
		printf("Le numero doit etre compris entre 1 et 99\n");
		return -1;
		}
	}

	// Creation du handler pour initialiser le port COM
	hCom = CreateFile( comPort,						// ouverture du port
					GENERIC_READ | GENERIC_WRITE,	// permissions de lecture et ecriture
					0,    							// doit etre ouvert en acces exclusif
					NULL, 							// securite par defaut
					OPEN_EXISTING,					// doit etre OPEN_EXISTING pour le peripherique
					0,    							// pas de specifications optionnelles
					NULL 							// doit etre a NULL pour les peripheriques
					);


	if (hCom == INVALID_HANDLE_VALUE) {
		printf ("***ERR: CreateFile echec de creation %d.\n", GetLastError());
		return -1;
	}

	// Recuperation des parametres de la structure DCB
	// La structure DCB est une sorte de fichier de configuration affecte au handler de port
	SecureZeroMemory(&dcb, sizeof(DCB)); // Initialise la structure avec des zeros
	dcb.DCBlength = sizeof(DCB);			
	fSuccess = GetCommState(hCom, &dcb); // Recuperation des valeurs a la structure DCB

	if (!fSuccess) {
		printf ("***ERR: GetCommState echec %d.\n", GetLastError());
		return -1;
	}


	// Configuration de la communication serie
	// Configuration MK : 57600 Baud (bps), 8 Data bits, no parity bit, 1 stop bit 
	dcb.BaudRate = CBR_57600;    
	dcb.ByteSize = 8;             
	dcb.Parity = NOPARITY;       
	dcb.StopBits = ONESTOPBIT;    
	
	// Ecriture des valeurs de la nouvelle configuration au handler du port
	fSuccess = SetCommState(hCom, &dcb);
	
	if (!fSuccess) {
		printf ("***ERR: SetCommState failed with error %d.\n", GetLastError());
      return -1;
	}

	// Configuration du timeout pour le port serie
	timeouts.ReadIntervalTimeout		= MAXDWORD; 
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant	= 0; 
	
	// Application de la configuration au port
	fSuccess = SetCommTimeouts (hCom, &timeouts);
	
	if(!fSuccess) {
		printf("***ERR: SetCommTimeouts failed with error %d.\n", GetLastError());
		return -1;
	}


	//Initialisation & Configuration terminee
	return 0;
}




// Ajout d'un CRC à la trame.
// AddCRC calcule les checksums de taille 2 octets pour la trame à transmettre 
// et ajoute un '\r' pour signifier la fin de la trame(selon le format de trame défini par Mikrokopter
void AddCRC(unsigned int frame_length) { // taille de "#,adr,cmd,data"
	
	unsigned int tmpCRC = 0;
	unsigned int i;

	for (i = 0; i < frame_length; i++)
	{
		tmpCRC += tx_buffer[i];
	}

	tmpCRC %= 4096;
	tx_buffer[i++] = '=' + tmpCRC / 64;
	tx_buffer[i++] = '=' + tmpCRC % 64;
	tx_buffer[i++] = '\r';				// symbole de fin

}





// Ecrit les donnees de la trame dans le port serie
int sendStringToCom (char* toSend, int length) {
	
	int numOfBytesWritten;

	return WriteFile (hCom, toSend, length, &numOfBytesWritten, NULL);
}



// Constitution de la trame. Le format de la trame est définie par Mikrokopter.
void SendData(unsigned char cmd,unsigned char addr, unsigned char *snd, unsigned char len) {

	unsigned int pt = 0;
	unsigned char a,b,c;
	unsigned char ptr = 0;

	tx_buffer[pt++] = '#';               // Octet de start
	tx_buffer[pt++] = 'a' + addr;        // Adresse
	tx_buffer[pt++] = cmd;               // Commande
	
	while(len)							 // Codage de l'information en Base64
	{
		if(len) { a = snd[ptr++]; len--;} else a = 0;
		if(len) { b = snd[ptr++]; len--;} else b = 0;
		if(len) { c = snd[ptr++]; len--;} else c = 0;
		tx_buffer[pt++] = '=' + (a >> 2);
		tx_buffer[pt++] = '=' + (((a & 0x03) << 4) | ((b & 0xf0) >> 4));
		tx_buffer[pt++] = '=' + (((b & 0x0f) << 2) | ((c & 0xc0) >> 6));
		tx_buffer[pt++] = '=' + ( c & 0x3f);
	}

	// Ajout du CRC
	AddCRC(pt);
	
	// Envoi de la trame de données à la Flight Control. 
	sendStringToCom(tx_buffer, pt+3);
}									  


// Réception
void Decode64(unsigned char *ptrOut, unsigned char len, unsigned char ptrIn,unsigned char max) 
{
 unsigned char a,b,c,d;
 unsigned char ptr = 0;
 unsigned char x,y,z;
 
 while(len)
 {
 	a = rx_buffer[ptrIn++] - '=';
 	b = rx_buffer[ptrIn++] - '=';
	c = rx_buffer[ptrIn++] - '=';
	d = rx_buffer[ptrIn++] - '=';
	
 	if(ptrIn > max - 2) break;	   // Ne traite pas plus de donnees qu'il ne faut

 	x = (a << 2) | (b >> 4);
 	y = ((b & 0x0f) << 4) | (c >> 2);
 	z = ((c & 0x03) << 6) | d;

 	if(len--) ptrOut[ptr++] = x; else break;
	if(len--) ptrOut[ptr++] = y; else break;
	if(len--) ptrOut[ptr++] = z; else break;
 }
}



