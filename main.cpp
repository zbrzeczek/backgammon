#include <iostream>
#include <fstream>


using namespace std;

void wyswietleniePliku(string fileName) {
    string line;
    ifstream myfile(fileName);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
        cout << line << '\n';
        }
        myfile.close();
    }
}

void interface() {
    char interfaceInput;
    bool wyborInterface = true;

    while (wyborInterface){
        wyswietleniePliku("interface.txt");
        interfaceInput = getch();
        if (interfaceInput != '1' || interfaceInput != '2' || interfaceInput != '3'){
            clrscr();
            cout << "Nie ma takiej opcji, podaj numer";
        }
        else {
            clrscr();
            cout << "cos nie pasi";
            wyborInterface = false;
        }
    }
}

int main (){
    int x;
    interface();
    cin >> x;
    return 0;
}