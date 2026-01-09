#include <iostream>
#include <string>
#include <b15f/b15f.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <limits>
#include <atomic>
#include <cstdint>

using namespace std;
using namespace std::chrono_literals;

B15F& drv = B15F::getInstance();
std::atomic<bool> clkRunning{false};
bool flanke_wechselt_nicht = false;

void HandshakeWarteschleife() {
    uint8_t Ueberpruefen;
    drv.setRegister(&DDRA, 0b11110000);
    drv.setRegister(&PORTA, 0b11110000);
    drv.getRegister(&DDRA);
    while (true) {
        this_thread::sleep_for(10ms);
        Ueberpruefen = drv.getRegister(&PINA);
        Ueberpruefen &= 0b00001111;

        if (Ueberpruefen == 0b00001111) {
            cerr << "Anderer PC gefunden!" << endl;
            break;
        }
    }
}

void Handshake(int& WelcherAnschluss) {
    uint8_t Ueberpruefen;

    drv.setRegister(&DDRA, 0b00001111);
    Ueberpruefen = drv.getRegister(&PINA);

    if ((int)(Ueberpruefen & 0b11110000) > 0) {
        cerr << "Anderer PC gefunden: Anschluss 0 wird verwendet." << endl;
        WelcherAnschluss = 0;
        drv.setRegister(&PORTA, (Ueberpruefen | 0b00001111));
        cerr << "Bestaetigung verschickt." << endl;
    } else {
        cerr << "Kein anderer PC gefunden; Wartesignal verschickt (1)" << endl;
        WelcherAnschluss = 1;
        HandshakeWarteschleife();
    }
}

string input() {
    cout << "Schreibe 1 (Datei) oder 0 (String): " << endl;

    int string_or_file;
    if (!(cin >> string_or_file)) {
        cerr << "Ungueltige Eingabe.\n";
        return "";
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (string_or_file == 1) {
        cout << "Gib den Dateipfad ein:" << endl;
        string path;
        getline(cin, path);

        ifstream file(path);
        if (!file.is_open()) {
            cerr << "Fehler: Datei konnte nicht geoeffnet werden: " << path << endl;
            return "";
        }

        string content, line;
        while (getline(file, line)) {
            content += line + "\n";
        }
        return content;

    } else if (string_or_file == 0) {
        cout << "Gib einen String ein:" << endl;
        string s;
        getline(cin, s);
        return s;
    }

    cerr << "Ungueltige Eingabe. Bitte 0 oder 1 eingeben.\n";
    return "";
}

void clockSignal() {
    drv.setRegister(&DDRA, 0b10000000); // Bit 7 als Ausgang (dein Signal)

    while (clkRunning.load()) {
        flanke_wechselt_nicht = false;
        drv.setRegister(&PORTA, 0b10000000); // Takt auf HIGH setzen
        flanke_wechselt_nicht = true;
        this_thread::sleep_for(2000ms);
        flanke_wechselt_nicht = false;
        drv.setRegister(&PORTA, 0b00000000); // Takt auf LOW setzen
        flanke_wechselt_nicht = true;
        this_thread::sleep_for(2000ms);
    }
}


void fragmentiere(string data){
    for (int i = 0; i < data.size(); i++) {
        unsigned char letter = data[i];
        uint8_t nibble1 = letter & 0b00001111;
        uint8_t nibble2 = letter >> 4;
        nibble2 |= (uint8_t)(1u << 5);
        if(i + 1 == data.size()){
           nibble2 |= (uint8_t)(1u << 6); // Setze das vorletzte Bit von nibble2, sonst 0
        }
        cout << "Character: " << letter << endl;
        cout << "Unsigned 8-bit: " << bitset<8>(letter) << endl;  
        cout << "Nibble 1: " << bitset<8>(nibble1) << endl;
        cout << "Nibble 2: " << bitset<8>(nibble2) << endl;
    }
}

void write(uint8_t byte) {
}

int main() {
    int Anschluss = 0;
    Handshake(Anschluss);
    std::thread t;   // clock thread

    if (Anschluss == 1) {
        clkRunning = true;
        t = std::thread(clockSignal);
    }

    string input_data = input();
    cout << "Eingegebene Daten: " << input_data << endl;

    if (Anschluss == 1) {
        // Master-Modus
        while (true) {
            if(flanke_wechselt_nicht){
                uint8_t empfangen = drv.getRegister(&PINA);
                if (empfangen & 0b10000000) cout << "write\n";
                else                        cout << "read\n";
            }
        }
    } else {
        // Slave-Modus
        while (true) {
            //this_thread::sleep_for(100ms);
            uint8_t empfangen = drv.getRegister(&PINA);
            if (empfangen & 0b10000000) cout << "read\n";
            else                        cout << "write\n";
        }
    }

    // hier kommst du praktisch nie hin
    if (t.joinable()) {
        clkRunning = false;
        t.join();
    }
    return 0;
}