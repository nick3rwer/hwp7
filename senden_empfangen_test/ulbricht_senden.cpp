#include <iostream>
#include <b15f/b15f.h>
#include <random>
#include <cstdint>
#include <bitset>
#include <thread>
#include <chrono>
#include <atomic>
using namespace std ;
B15F & drv = B15F :: getInstance () ; //drv wird ein Objekt einer Klasse


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

        result.push_back(nibble1);
        result.push_back(nibble2);
    }
    return result;
}

void write(uint8_t byte){
	drv.delay_ms(50);
	drv.setRegister(& PORTA , byte) ;
}

int main ()
{
	drv.setRegister(& DDRA , 0b11111111);
	string input_data = "lbrecht Dürer der Jüngere (auch Duerer; latinisiert Albertus Durerus; * 21. Mai 1471 in Nürnberg; † 6. April 1528 ebenda) war ein deutscher Maler, Grafiker, Mathematiker und Kunsttheoretiker. Mit seinen konsequent signierten Gemälden, Zeichnungen sowie den große Verbreitung findenden Kupferstichen und Holzschnitten zählt er zu den herausragenden Vertretern der Renaissance. Seine Landschaftsaquarelle zählen zu den frühesten ihrer Gattung; gleiches gilt für seine Selbstporträts und seine Aktzeichnungen. ";
	vector<uint8_t> data_to_send;
    data_to_send = fragmentiere(input_data);
	for(auto byte : data_to_send)
	{
        //cout << "Sende Byte: " << bitset<8>(byte) << endl;
		write(byte);
	}
}
