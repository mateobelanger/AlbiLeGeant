#define F_CPU 8000000
#include "header.h"

#define VERT 'v'    //lors du choix de la couleur
#define ROUGE 'r'   //lors du choix de la couleur
#define GAUCHE 'g'  //lors du choix du coté avec la photorésistance
#define DROIT 'd'   //lors du choix du coté avec la photorésistance

ISR(INT0_vect);
ISR(INT1_vect);
ISR(TIMER1_COMPA_vect);

enum Etats {
    COULEUR, UTURN, TOGA, TOGAB, COMPTEURLIGNE_1, PARKING_1, TOABC, COMPTEURLIGNE_2, ALLERETOUR, CINQ40, PHOTORESISTANCE, INTERMITTENCE, TOAGC, TOGAH, PARKING_2 
    };

volatile char    couleurChoisie = '\0'; //NUL (pas encore choisi)
volatile uint8_t etat = COULEUR;
volatile bool    commencerParking = false;
char cote;

ISR(INT0_vect);
ISR(INT1_vect);
ISR(INT2_vect);
ISR(TIMER1_COMPA_vect);

int main(){
    for(;;){
        switch(etat) {
            case COULEUR:
                DDRC = 0xff;    //mode ecriture pour la DEL
                DDRD = 0x00;    //mode lecture pour lire les interrupts
                initialisationINT2(1,0);    //falling edge activates interrupt.
                while(etat == COULEUR){
                    if (!(PINB & (1<<3))){
                        if(couleurChoisie){  //ne fait rien si la couleur n'a pas encore été choisie.
                            etat = PHOTORESISTANCE;
                            EIMSK &= ~(1 << INT2);   //INT2 désactivé, le choix de couleur ne peut plus être changé.
                        }   
                    }
                }
            case UTURN: //tourner 180 degres
                    DDRD = 0xFF; //PORT D en sortie pour le signal des moteurs
					initialisationPwmMoteurs(); // Configure les registres d'initialisation du timer1 pour le PWM moteur.
					capteur.tourner180();
					etat++;
					break;
                    
            case TOGA: //linetracking() jusqu'a intersection du segment GA
					while (!capteur.estIntersection())		//tant qu'on ne detecte pas d'intersection
					{
						capteur.lecture();
						capteur.lineTracking();
					}
					etat++;
					break;
                    
            case TOGAB:
                //linetracking() jusqu'a l'intersection GAB et tourne a gauche sur le segment AB
				while (!capteur.estIntersection())		//tant qu'on ne detecte pas d'intersection
				{
					capteur.lecture();
					capteur.lineTracking();
				}
				capteur.intersectionGauche();
				etat++;
                break;
                
            case COMPTEURLIGNE_1:
                //linetracking() intermittent avec un compteur d'intersection jusqu'a l'emplacement (rouge ou vert) theorique
                //du parking
                uint8_t triggerParking = 0;
                if(couleurChoisie == VERT)
                    triggerParking = 3;
                else
                    triggerParking = 6;

                uint8_t compteurIntersection = 0;

                while(!capteur.estI}}ntersection()){

                    capteur.lecture();
                    capteur.lineTra}cking();
                }
                compteurIntersection++;
                while(capteur.estIntersection()) {
                  capteur.lecture();
                  ajustementPwmMoteurs(50,50);
                }

                if(compteurIntersection == triggerParking) {
                    etat++;
                }
            }
                break;
            case PArKING_1:
                //linetracking() durant une minuterie prédéterminée
                //la fin de la minuterie active commencerParking, ce qui initialise la Séquence de parking

                minuterie(30000);
                while(!commencerParking) {
                capteur.lecture();
                capteur.lineTracking();
                }
                capteur.tournerGauche();
                _delay_ms(2000);
                ajustementPwmMoteurs(0,0);
                PORTA = 0x01;
                _delay_ms(500);
                PORTA = 0x00;
                _delay_ms(2000);


                do {
                    capteur.lecture();
                    capteur.tournerDroite();
                } while(!capteur.getSensor(2));

                etat++;
                break;
                
            case TOABC:
                //linetracking() jusqu'a l'intersection ABC et tourne a gauche sur le segment BC
				while (!capteur.estIntersection())		//tant qu'on ne detecte pas d'intersection
				{
					capteur.lecture();
					capteur.lineTracking();
				}
				capteur.intersectionGauche();
				etat++;
                break;
                
                
            case COMPTEURLIGNE_2:
            {
                //
                // Compte le nombre d'intersection en fonction de la couleur choisie
                //
                uint8_t triggerBonneIntersection = 0;
                if(couleurChoisie == VERT)
                    triggerParking = 2;
                else
                    triggerParking = 4;

                uint8_t compteurIntersection = 0;

                while(!capteur.estIntersection()){

                    capteur.lecture();
                    capteur.lineTracking();
                }
                compteurIntersection++;
                while(capteur.estIntersection()) {
                  capteur.lecture();
                  ajustementPwmMoteurs(50,50);
                }

                if(compteurIntersection == triggerBonneIntersection) {
                    etat++;
                }
            }

            case ALLERETOUR:
                {
                    uint8_t triggerSwitchEtat = 0;
                    capteur.intersectionGauche();
                            
                    while(!capteur.estPerdu()) {
                        capteur.lecture();
                        capteur.lineTracking();
                    }
                    
                    if(couleurChoisie == VERT) {
                        capteur.tourner180Gauche();
                        triggerSwitchEtat = 3;
                    }
                    else {
                        capteur.tourner180Droite();
                        triggerSwitchEtat = 1;
                    }
                    
                    while(!capteur.estIntersection) {
                        capteur.lecture();
                        capteur.lineTracking();
                    }
                    
                    capteur.intersectionGauche();
                    
                    for (uint8_t i = 0; i < triggerSwitchEtat; i++){
                    
                        while(!capteur.estIntersection()) {
                        capteur.lecture();
                        capteur.lineTracking();
                        }
                        while(capteur.estIntersection)
                            ajustementPWMMoteurs(60,60);
                    }
                    
                    etat++;
                }
                
            case CINQ40:
                
                while(!capteurs.estPerdu()){
                    capteurs.lecture();
                    capteurs.lineTracking();
                }
                
                ecrire1('D',2);
                ecrire1('D',3);        
                
                //
                // On fait avancer le robot jusqu'à ce qu'il arrive au bout. Puis on le fait reculer de maniere manuelle
                // jusqu'à ce que son centre de rotation soit au dessus de C (j'ai pas trouvé de meilleur moyen :/)
                //
                
                ajustementPwmMoteurs(20,20);
                _delay_ms(200);
                ecrire0('D',2);
                ecrire0('D',3);            // Ici je fais freiner le robot pour être sur de s'arreter
                ajustementPwmMoteurs(10,10);
                _delay_ms(50);
                ajustementPwmMoteurs(0,0);
                
                for(uint8_t compteurLignes = 0; i < 9; i++) {
                
                    while(!capteurs.estPerdu())
                        capteurs.tournerGauche();
                    
                    while(capteurs.estPerdu()
                    capteurs.tournerGauche();
                }S
                etat++;                     

            case PHOTORESISTANCE:
                uint16_t valeurCan16bitDroit;
                uint16_t valeurCan16bitGauche;
                DDRC = 0xff;    /* A RETIRER */
                DDRA = 0x00;    //portA en lecture pour les photoresitances.
                can convertisseur;
                initialisationINT0(1,1);   //rising edge
                while(etat==PHOTORESISTANCE) {
                    valeurCan16bitDroit = convertisseur.lecture(PINA1);   //lecture pin1 portA
                    valeurCan16bitDroit >>= 2;
                    valeurCan16bitGauche = convertisseur.lecture(PINA0);    //lecture pin0 portA
                    valeurCan16bitGauche >>= 2;
                    
                    if(valeurCan16bitDroit > 230) {
                        cote = DROIT;
                        _delay_ms(50);
                    }
                    else if(valeurCan16bitGauche > 230 ) {
                        cote = GAUCHE;
                        _delay_ms(50);
                    }
                }
            case INTERMITTENCE:
            case TOAGC:
            case TOGAH:
            case PARKING_2:
        }

    }
}

ISR(INT0_vect){
    if (etat == PHOTORESISTANCE){
        switch(cote){
            case DROIT:
                /* etat = TOURNER_DROIT_1 */
                
                ecrire1('C', 0);    /* A RETIRER */
                ecrire0('C', 1);    /* A RETIRER */
                
                EIMSK &= ~(1 << INT0);  //désactive INT0 si le coté a été choisi
                etat++;     //passe à l'état suivant
                break;
            case GAUCHE:
                /* etat = TOURNER_GAUCHE_1 */
                
                ecrire1('C', 1);    /* A RETIRER */
                ecrire0('C', 0);    /* A RETIRER */
                
                EIMSK &= ~(1 << INT0);  //désactive INT0 si le coté a été choisi
                etat++;     //passe à l'état suivant
                break;  
        }
    }
}

ISR(INT1_vect){ 
    if(couleurChoisie){  //ne fait rien si la couleur n'a pas encore été choisie.
        EIMSK &= ~(1 << INT0) & ~(1 << INT1);   //interruptions désactivées pour INT0 et INT1, le choix de couleur ne peut plus être changé.
        etat = PHOTORESISTANCE; //etat suivant.
    }
}

ISR(INT2_vect){
    if(etat == COULEUR){
        _delay_ms(30);
        if(! (PINB & (1 << 2)) ){   //anti-rebond
            switch(couleurChoisie){
                case VERT:
                    couleurChoisie = ROUGE;
                    ecrire1('C', 0);
                    ecrire0('C', 1);
                    break;  
                case ROUGE: 
                    couleurChoisie = VERT;
                    ecrire1('C', 1);
                    ecrire0('C', 0);
                    break;
                default:        //quand couleurChoisie est null.
                    couleurChoisie = VERT;
                    ecrire1('C', 1);
                    ecrire0('C', 0);
                    break;
            }
        } 
    }
}
