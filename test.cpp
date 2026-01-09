#include<iostream>
#include<fstream>
#include <stdint.h>
#include <bitset>
#include <vector>

using namespace std;

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


uint8_t control_bit_setzten(uint8_t byte){
    if (byte & 0x01) {
        return byte | (1u << 4);
    } else {
        return byte | (0u << 4);
    }
}

vector<uint8_t> fragmentiere(string data){
    vector<uint8_t> result;
    for (int i = 0; i < data.size(); i++) {
        unsigned char letter = data[i];
        uint8_t nibble1 = letter & 0b00001111;
        uint8_t nibble2 = letter >> 4;
        nibble1 = control_bit_setzten(nibble1);
        nibble2 = control_bit_setzten(nibble2);
        //das 6. Bit wird auf eins gesetzt, um zu signalisieren, dass es sich um den 2. Nibble handelt
        nibble2 |= (uint8_t)(1u << 5);  
        //die letzte Sendung wird codiert vorletzte Bit auf 1
        if(i + 1 == data.size()){
           nibble2 |= (uint8_t)(1u << 6);
        }
        /*cout << "Character: " << letter << endl;
        cout << "Unsigned 8-bit: " << bitset<8>(letter) << endl;  
        cout << "Nibble 1: " << bitset<8>(nibble1) << endl;
        cout << "Nibble 2: " << bitset<8>(nibble2) << endl;
        cout << "--- Ueberpruefung ---" << endl;*/
        if(kontrolle_letzte_sendung(nibble2)){
            cout << "Letzte Sendung dieses Zeichens." << bitset<8>(nibble2) << endl;
        }
        result.push_back(nibble1);
        result.push_back(nibble2);
    }
    return result;
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

int main(){
    vector<uint8_t> data;
    string input_data = "Samira ist wunderschoen!";
    data = fragmentiere(input_data);
    string original_data = read(data);
    if(original_data == input_data){
        cout << "Die Übertragung war erfolgreich: " << endl;        
        cout << "Eingabedaten: " << input_data << endl;
        cout << original_data << endl;
    } else {
        cout << "Die Übertragung ist fehlgeschlagen." << endl;
    }
    return 0;
}
//test für git