#include<iostream>
#include<string>
#include<b15f/b15f.h>
#include<thread>
#include<chrono>
#include<fstream>
#include<vector>
#include<sys/socket.h>

using namespace std;
B15F& drv = B15F::getInstance();

void HandshakeWarteschleife(){ //Master wartet auf Slave
        bool Warteschleife = true;
        uint8_t Ueberpruefen;

        drv.setRegister(&DDRA, 0b11110000);
        drv.setRegister(&PORTA, 0b11110000);

        while (Warteschleife == true){
                this_thread::sleep_for(10ms);
                Ueberpruefen = (drv.getRegister(&PINA));
                Ueberpruefen &= 0b00001111;
                if (Ueberpruefen == 0b00001111){ //Slave gefunden
                        cerr << "Anderer PC gefunden!" << endl;
                        Warteschleife = false;
                }
        }
    return;
}

void Handshake(int& WelcherAnschluss){
        bool Warteschleife = false;
        uint8_t Ueberpruefen;

        drv.setRegister(&DDRA, 0b00001111);
        Ueberpruefen = drv.getRegister(&PINA);
        if ((int)(Ueberpruefen & 0b11110000) > 0){ //Wird Slave
                cerr << "Anderer PC gefunden: Anschluss 0 wird verwendet." << endl;
                WelcherAnschluss = 0;
                drv.setRegister(&PORTA, (Ueberpruefen | 0b00001111));
                cerr << "Bestätigung verschickt." << endl;
        return;
        }
        else { //Wird Master
                cerr << "Kein anderer PC gefunden; Wartesignal verschickt (1)" << endl;
                WelcherAnschluss = 1;
                HandshakeWarteschleife();
        return;
        }
}

int main(){
    vector<uint8_t> ZuVerschicken;
    vector<uint8_t> ZuErhalten;
    uint8_t Nibble1S;
    uint8_t Nibble0S;
    uint8_t Nibble1E;
    uint8_t Nibble0E;
    uint8_t Erhalten;
    int Anschluss;
    bool Kommunikation = true;
    bool SendungLaeuft = true;
    int i = 0;
    int Nibble = 1;
    uint8_t NibbleZuSenden;
    uint8_t NibbleGesendetBekommen;
    bool FehlererkennungS;
    bool FehlererkennungE;
    string Takt = "Senden";
    uint8_t LetzteSendung;

    Handshake(Anschluss);

    ZuVerschicken.assign(istreambuf_iterator<char>(cin), istreambuf_iterator<char>());
/*
    cout.write(reinterpret_cast<const char*>(ZuVerschicken.data()), ZuVerschicken.size());
    cout.flush();

    cout << endl;*/

    while(Kommunikation){
        Nibble1S = (ZuVerschicken[i] & 0b11110000);
        Nibble1S = (Nibble1S >> 4);
        Nibble0S = (ZuVerschicken[i] & 0b00001111);

        if(Takt == "Senden"){
            if(Nibble == 1){
                NibbleZuSenden = Nibble1S;
            }
            else{
                NibbleZuSenden = Nibble0S;
            }
        }

        if(Takt == "Prüfen"){
            NibbleZuSenden = NibbleGesendetBekommen;
        }

        if(Anschluss == 1){
            drv.setRegister(&PORTA, (NibbleZuSenden << 4)); //!
        }
        else{
            drv.setRegister(&PORTA, NibbleZuSenden);
        }

        this_thread::sleep_for(50ms);

        if(Anschluss == 1){
            NibbleGesendetBekommen = (drv.getRegister(&PINA) & 0b00001111);
        }
        else{
            NibbleGesendetBekommen = (drv.getRegister(&PINA) & 0b11110000); //!
            NibbleGesendetBekommen = (NibbleGesendetBekommen >> 4);
        }

        if(Takt == "Senden"){
            if(Nibble == 1){
                Nibble1E = NibbleGesendetBekommen;
            }
            else{
                Nibble0E = NibbleGesendetBekommen;
            }
        }
        if(Takt == "Prüfen"){
            if(NibbleGesendetBekommen != LetzteSendung){
                cerr << "Fehler in der Übertragung! Stelle: " << i << endl;
            }

            if(Nibble == 0){
            Erhalten = (Nibble1E << 4 | Nibble0E);
            ZuErhalten.push_back(Erhalten);
            }

            if (Nibble == 1){
                Nibble = 0;
            }
            else{
                i++;
                Nibble = 1;
            }
        }

        if(Takt == "Senden"){
            Takt = "Prüfen";
            LetzteSendung = NibbleZuSenden;
        }
        else{
            Takt = "Senden";
        }

        if (i > ZuVerschicken.size()){
            SendungLaeuft = false;
        }
        Kommunikation = SendungLaeuft;
        this_thread::sleep_for(50ms);
    }

    ZuErhalten.pop_back();

    std::cout.write(reinterpret_cast<const char*>(ZuErhalten.data()), ZuErhalten.size());
    std::cout.flush();
}