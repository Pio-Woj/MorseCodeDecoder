/*-------------------------------------------------------------------------------------
					Technika Mikroprocesorowa 2 - projekt
					Dekoder alfabetu Morse`a z wykorzystaniem czujnika światła.
					autor: Piotr Wojsa
----------------------------------------------------------------------------*/
					
#include "MKL05Z4.h"
#include "ADC.h"
#include "pit.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


float adc_volt_coeff = ((float)(((float)2.91) / 4095) );			// Współczynnik korekcji wyniku, w stosunku do napięcia referencyjnego przetwornika
uint8_t result_ok=0;
uint16_t temp;
float	result=0;
float sum_of_samples=0;
float number_of_samples=0;

void ADC0_IRQHandler()
{	
	temp = ADC0->R[0];	// Odczyt danej i skasowanie flagi COCO
	if (!result_ok){
		sum_of_samples+=temp;
		number_of_samples++;
	}
	
}
void PIT_IRQHandler() //przerwanie zgłaszane co 100ms
{
	
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
	result_ok=1;
}

char* morse[] = {".","-",
		 "..",".-","-.","--",
		 "...","..-",".-.",".--","-..","-.-","--.","---",
		 "....","...-","..-.",".-..",".--.",".---","-...","-..-","-.-.","-.--","--..","--.-",
		 "-----",".----","..---","...--","....-",".....","-....","--...","---..","----."};
char decoded_char[] = {'E','T',
		  	'I','A','N','M',
			'S','U','R','W','D','K','G','O',
			'H','V','F','L','P','J','B','X','C','Y','Z','Q',
			'0','1','2','3','4','5','6','7','8','9'};

											 
//Dekodowanie ciągu znaków w kodzie Morse`a na zasadzie LUT (Lookup Table)
//Funkcja porównuje dany ciąg znaków z gotowymi kombinacjami z tablicy morse[]
//Jeśli nastąpi zgodność dla któregoś indeksu tej tablicy to zostaje wyświetlony
//znak skryty pod indeksem o tym samym numerze ale już z tablicy decoded_char[]
void decode_letter(char* coded_letter){
		int i;
			for(i=0; i<37; i++){
				if(strcmp(morse[i],coded_letter)==0){
						break;
				}
			}
			char t[]={0,0};
			t[0]=decoded_char[i];
			LCD1602_Print(t);
	
}

void light_error_msg(void){
	
		LCD1602_ClearAll();
		LCD1602_SetCursor(0,0);
		LCD1602_Print("Problem z");
		LCD1602_SetCursor(0,1);
		LCD1602_Print("oswietelniem");
		DELAY(4000);
		LCD1602_ClearAll();
		LCD1602_SetCursor(0,0);
		LCD1602_Print("Zrestartuj");
		LCD1602_SetCursor(0,1);
		LCD1602_Print("dekoder");
		DELAY(4000);
	
}

void too_much_light_msg(void){
	
		LCD1602_ClearAll();
		LCD1602_SetCursor(0,0);
		LCD1602_Print("Zbyt jasne");
		LCD1602_SetCursor(0,1);
		LCD1602_Print("otoczenie");
		DELAY(4000);
		LCD1602_ClearAll();
		LCD1602_SetCursor(0,0);
		LCD1602_Print("Przestaw dekoder");
		LCD1602_SetCursor(0,1);
		LCD1602_Print("w ciemne miejsce");
		DELAY(4000);
		LCD1602_ClearAll();
		LCD1602_SetCursor(0,0);
		LCD1602_Print("Zrestartuj");
		LCD1602_SetCursor(0,1);
		LCD1602_Print("dekoder");
		DELAY(4000);
	
}


void no_transmission_msg(void){
		LCD1602_ClearAll();
		LCD1602_SetCursor(0,0);
		LCD1602_Print("Brak transmisji");
		LCD1602_SetCursor(0,1);
		LCD1602_Print("---------------");
	
}

float get_average_light(){
	int exit=0;
	float average = 0;
	LCD1602_SetCursor(0,0);
	LCD1602_Print("Kalibracja");
	
	//Kalibracja trwa około 4s
	while(1)
	{		
			
		if(result_ok)
		{
			average += sum_of_samples/number_of_samples*adc_volt_coeff; // Wyliczenie średniej i dostosowanie wyniku do zakresu napięciowego
			number_of_samples=0;
			sum_of_samples=0;
			result_ok=0;
			exit++;
		}
		
		if(exit>=40)
			break;
}
	return average/40;

}


int main (void)
{
	uint8_t	kal_error;
	uint8_t up=0;
	uint8_t down=0;
	uint8_t space_flag=1;
	uint8_t no_transmission_flag=0;
	uint8_t row=0;
	uint8_t column=0;
	
	char morse_message[6]="";
	
	
	LCD1602_Init();		 					// Inicjalizacja wyświetlacza LCD
	LCD1602_Backlight(TRUE);	
	LCD1602_ClearAll();
	PIT_Init();
	kal_error=ADC_Init();				// Inicjalizacja i kalibracja przetwornika A/C
	if(kal_error)
	{	
		while(1);									// Kalibracja się nie powiodła
	}
	ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(12);
	
	float average=get_average_light(); // Zbadanie oświetlenia otoczenia, jako punkt odniesienia
	//Sprawdzenie, czy oświetlenie czujnika światła nie jest za duże
	//2.3 to około 83% wartości maskymalnej czujnika możliwej do osiągnięcia
	//a wygenerowanie up następuje, gdy wartość napięcia jest większa o 20% niż wartość
	//referencyjna, co jest niemożliwe do spełnienia dla napięcia odniesienia większego,
	//bądź równego 83% wartości napięcia referencyjnego dla przetwornika
	if(average>=2.3){
	
		while(1){
			too_much_light_msg();
		}
	}
	LCD1602_ClearAll();
	LCD1602_SetCursor(0,0);
	int index = 0;
	while(1)
	{		
			
		if(result_ok)
		{
			result = sum_of_samples/number_of_samples*adc_volt_coeff; // Wyliczenie średniej i dostosowanie wyniku do zakresu napięciowego
   		
			if(result>(1.2*average)){
				
				space_flag=0;
				down=0;
				up++;
				
				//Jeśli flaga była ustawiona to znaczy
				//że na wyświetlaczu znajduje się napis dla stanu braku nadawania
				//Natomiast teraz zaczyna się nadawanie
				if(no_transmission_flag==1){
					LCD1602_ClearAll();
					LCD1602_SetCursor(0,0);
					no_transmission_flag=0;
				}
				//Średnio na znak 0.5-0.6s plus widełki +/-100% 
				if(up>=1&&up<=12){
					if(morse_message[4]==0)
						morse_message[index]='.';
				}
				//Kreska powinna zająć 3 razy tyle co czas na jeden znak/kropkę
				//czyli mniej więcej 1.8s, ale założyłem grubszy margines czasowy z góry
				else if (up>=13&&up<=40){
					if(morse_message[4]==0)
						morse_message[index]='-';
				}
				//Bardziej oświetlony czujnik przez więcej niż 4s może być skutkiem zmiany
				//otoczenia pracy dekodera, od użytkownika powinna zależeć odpowiednia reakcja
				//na taką sytuację, a następnie restart dekodera.
				else if(up>=41){
					while(1){
						light_error_msg();
					}
				}
			}else{
				
				
				down++;
				//Przerwa między nadawanymi znakami powinna trwać jeden okres znaku
				//czyli około 0.5-0.6s natomiast dodane zostały odpowiednie marginesy czasowe
				if(down<=12&&up!=0){
				index++;
				up=0;
				}
				//Ten warunek czasowy oznacza zakończenie kodowania danej litery/cyfry
				else if(down>=13&&down<=34&&morse_message[0]!=0){
					//Odpowiednie przesunięcie kursora w razie skończenia się miejsca na wierszu wyświetlacza
					if(column==16&&row==0){
						LCD1602_SetCursor(0,1);
						column=0;
						row=1;
					}else if(column==16&&row==1){
						LCD1602_SetCursor(0,0);
						column=0;
						row=0;
					}
					decode_letter(morse_message);
					column++;
					
					morse_message[0]=0;
					morse_message[1]=0;
					morse_message[2]=0;
					morse_message[3]=0;
					morse_message[4]=0;
					index=0;
					
				}
				//Odpowienio długa cisza w nadawaniu oznacza zakończenie nadawania słowa
				else if(down>=35&&space_flag==0&&column!=0){
					LCD1602_Print(" ");
					space_flag=1;
					column++;
				}
				//Cisza na wejściu odbiornika jest tak długa że użytkownik zostaje poinformowany o jej zaistnieniu
				else if(down>=120&&no_transmission_flag==0){
					
					no_transmission_flag=1;
					no_transmission_msg();
				}
			}
			number_of_samples=0;
			sum_of_samples=0;
			result_ok=0;
		}
		
}
}
