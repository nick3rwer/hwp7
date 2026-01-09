# include <iostream>
# include <b15f/b15f.h>
#include <cstdint>
#include <vector>
using namespace std ;

bool kontrolle_letzte_sendung(uint8_t byte){
    return (byte & (1u << 6)) != 0;
}

bool controlbit_pruef(uint8_t byte) {
    return ((byte ^ (byte >> 4)) & 0x01) == 0;
}


bool vollständige_überprüfung(uint8_t nibble1, uint8_t nibble2){
    //ueberpuefe das 5. bit, welches das Kontrollbit ist
    if(controlbit_pruef(nibble1) && controlbit_pruef(nibble2)){
        //ueberpruef ob das nibbl1 als 6. bit eine 0 hat und nibble2 eine 1
        if((nibble1 & (1u << 5)) == 0 && ((nibble2 & (1u << 5)) != 0 )){
            return true;
    }
}    return false;
}

string read(vector<uint8_t> received_nibbles){
    string original_data;
    for (int i = 0; i < received_nibbles.size(); i += 2) {
        uint8_t nibble1 = received_nibbles[i];
        uint8_t nibble2 = received_nibbles[i + 1];
        if(vollständige_überprüfung(nibble1, nibble2)){
            uint8_t original_byte = (nibble2 & 0b00001111) << 4 | (nibble1 & 0b00001111);
            original_data += static_cast<char>(original_byte);
        } else {
            cout << "Fehler bei der Übertragung der Nibbles!" << endl;
            break;
        }
    }
    return original_data;
}

int main ()
{
B15F & drv = B15F :: getInstance() ; //drv wird ein Objekt einer Klasse
vector<uint8_t> data_to_receive;
cout << "Warte auf Daten..." << endl;
uint8_t vorgaenger = 0b11111111;
	while (1)
	{   
		uint8_t empfangen = (( int ) drv.getRegister (& PINA ) ) ;
		if(vorgaenger != empfangen && empfangen != 0b0){
                if(vollständige_überprüfung(vorgaenger, empfangen)){
                    uint8_t original_byte = (empfangen & 0b00001111) << 4 | (vorgaenger & 0b00001111);
                    cout<<static_cast<char>(original_byte);
                }
            }
        
            if(empfangen & (1u << 6)){
			break;
		}
		drv.delay_ms(50);
        vorgaenger = empfangen;
	}
    cout << endl << "Datenübertragung abgeschlossen." << endl;
}
//Ende main main ende