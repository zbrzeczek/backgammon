#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define TRUE 1
#define FALSE 0
#define P1 1
#define P2 2
#define ROWS 24
#define COL 80

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

typedef struct GameState{
    char gracz1[10];
    char gracz2[10];
    int kolejka;
    int pionki[2][24];
    int pionkiWDomuP1;
    int pionkiWDomuP2;
    int bar[2];
    int kostki[2];
} GameState;

void saveGame(GameState *stanGry, WINDOW *info) {
    char iSave;
    wclear(info); 
    mvwprintw(info, 2, 0, "Wybierz zapis gry\n\n1. \n\n2. \n\n3. ");
    wrefresh(info);
    iSave = getch();

    char plikN[6];

    sprintf(plikN, "%c.bin", iSave);

    FILE *plik = fopen(plikN, "wb"); 

    if (plik != NULL) {
        fwrite(stanGry, sizeof(GameState), 1, plik);
        fclose(plik);

        mvprintw(ROWS/2-10,COL/2-10, "Gra zapisana %s\n", plikN);
    } else {
        mvprintw(ROWS/2-10,COL/2-10, "Nie dalo sie zapisac\n");
    }
}

void loadGameState(GameState *stanGry, char *nazwaPliku) {
    char plikN[6];

    sprintf(plikN, "%c.bin", *nazwaPliku);

    FILE *plik = fopen(plikN, "rb");  

    if (plik != NULL) {
        fread(stanGry, sizeof(GameState), 1, plik);
        fclose(plik);

        mvprintw(ROWS/2-10,COL/2-10, "Gra wczytana z zapisu %s\n", nazwaPliku);
    } else {
        mvprintw(ROWS/2-10,COL/2-10, "Nie dalo sie wczytac\n");
    }
}

void ustawNazwy(GameState *stanGry){
    echo();
    mvprintw(ROWS/2-1,(COL-25)/2, "Ustaw nazwe gracza 1 (O) \n");
    move(ROWS/2,COL/2-1);
    getstr(stanGry->gracz1);
    clear();
    refresh();
    mvprintw(ROWS/2-1,(COL-25)/2, "Ustaw nazwe gracza 2 (X) \n");
    move(ROWS/2,COL/2-1);
    getstr(stanGry->gracz2);
    clear();
    refresh();
}

void displayStatic(WINDOW *plansza){
    mvwprintw(plansza, 0, 0, "1 1 1 1 1 1     1 2 2 2 2 2");
    mvwprintw(plansza, 1, 0, "3 4 5 6 7 8     9 0 1 2 3 4");
    mvwprintw(plansza, 2, 0, "-----------|   |-----------");
    mvwprintw(plansza, 16, 0, "-----------|   |-----------");
    mvwprintw(plansza, 17, 0, "1 1 1 9 8 7     6 5 4 3 2 1");
    mvwprintw(plansza, 18, 0, "2 1 0");
}

void displayPlansza(GameState *stanGry, WINDOW *plansza) {
    int z = 0;
    wclear(plansza);
    displayStatic(plansza);

    for (int i = 0; i < 12; i++) {
        if (i == 6) z += 4;
        
        for (int j = 0; j < 2; j++) {
            int pUp = stanGry->pionki[j][23 - i];
            int pD = stanGry->pionki[j][11 - i];
            int bar = stanGry->bar[j];

            for (int k = 0; k < pUp; k++)
                mvwprintw(plansza, k + 3, 26 - (2 * i + z), (j == 0) ? "O" : "X");

            for (int k = 0; k < pD; k++)
                mvwprintw(plansza, 15 - k, 2 * i + z, (j == 0) ? "O" : "X");

            for (int k = 0; k < bar; k++)
                mvwprintw(plansza, (j == 0) ? 4 + k : 13 - k, 13, (j == 0) ? "O" : "X");
        }
    }

    wrefresh(plansza);
}


void zbicie(GameState *stanGry, WINDOW *info, int kosc, int pole){
    int kol = stanGry->kolejka-1;
    int opp = (kol+1)%2;
    int dod = stanGry->kostki[kosc-1];
    
    if (pole == 100){
        int opp1 = stanGry->kolejka==P1 ? 24-dod : dod;
        stanGry->pionki[opp][opp1]--;
        stanGry->bar[opp]++;
        stanGry->bar[kol]--;
        stanGry->pionki[kol][opp1]++;
        wclear(info);
        mvwprintw  (info, 10, 0, "Ruch obowiazkowy");
    }
    else {
        int opp1 = stanGry->kolejka==P1 ? pole-dod : pole+dod;
        stanGry->pionki[opp][opp1]--;
        stanGry->pionki[kol][pole]--;
        stanGry->pionki[kol][opp1]++;
        stanGry->bar[opp]++;
    }
    stanGry->kostki[kosc-1]=0;
}

void ruchNzbicie(GameState *stanGry, WINDOW *info, int kosc, int pole){
    int kol = stanGry->kolejka-1;
    int opp = (kol+1)%2;
    
    if (pole == 100){
        int opp1 = stanGry->kolejka==P1 ? 24-stanGry->kostki[kosc-1] : stanGry->kostki[kosc-1];
        stanGry->bar[kol]--;
        stanGry->pionki[kol][opp1]++;
        wclear(info);
        mvwprintw  (info, 10, 0, "Ruch obowiazkowy");
        wrefresh(info);
        sleep(1);
    }
    else {
        int opp1 = stanGry->kolejka==P1 ? pole-stanGry->kostki[kosc-1] : pole+stanGry->kostki[kosc-1];
        stanGry->pionki[kol][pole]--;
        stanGry->pionki[kol][opp1]++;
    }
    stanGry->kostki[kosc-1]=0;
}

int wybierzK(WINDOW *info) {
    int kosc;
    mvwprintw(info, 12, 0, "Wybierz kostke");
    wmove(info, 13, 0);
    wrefresh(info);
    wscanw(info, "%d", &kosc);
    return kosc;
}

int infoZbicie(GameState *stanGry, WINDOW *info, int kosc, int pole, int opp, int opp1){
    if (stanGry->pionki[opp][opp1] != 1) {
        mvwprintw(info, 15, 0, "Nakaz zbicia!");
        return TRUE;
    }
    else {
        zbicie(stanGry, info, kosc, pole);
        return FALSE;
    }
}

int infoNzbicie(GameState *stanGry, WINDOW *info, int kosc, int pole, int opp, int opp1){
    if (stanGry->pionki[opp][opp1] > 1 || opp1>23 || opp1 < 0 || stanGry->kostki[kosc-1] == 0){
        mvwprintw(info, 15, 0, "Ruch nie wykonalny");
        return TRUE;
    }
    else if (stanGry->pionki[opp][opp1] == 1) {
        zbicie(stanGry, info, kosc, pole);
        return FALSE;
    }
    else {
        ruchNzbicie(stanGry, info, kosc,  pole); 
        return FALSE;
    }
}

void wyswietlI(GameState *stanGry, WINDOW *info){
    mvwprintw(info, 4, 0, "Gracz %d \n\nPozostale ruchy \n1. %d\n2. %d", stanGry->kolejka,stanGry->kostki[0],stanGry->kostki[1]);
}

void wybKostki(GameState *stanGry,  int pole, WINDOW *info, int *ruch, int pZdej){
    int wybKosc=TRUE, kosc, kol = stanGry->kolejka-1;
    while (wybKosc){
        wyswietlI(stanGry, info);
        kosc = wybierzK(info);

        if (stanGry->kostki[kosc-1] == 0) {
            wclear(info);
            mvwprintw(info, 12, 0, "Ruch wykorzystany");
        }
        else {
            int opp1 = stanGry->kolejka==P1 ? pole-stanGry->kostki[kosc-1] : pole+stanGry->kostki[kosc-1];
            int opp = (kol+1)%2;

            if (pZdej > 0){
                wybKosc = infoZbicie(stanGry, info, kosc, pole, opp, opp1);
            }
            else {
                wybKosc = infoNzbicie(stanGry, info, kosc, pole, opp, opp1);
            }
        }
        wrefresh(info);
    }
}

void checkIfGra (GameState *stanGry, WINDOW *info, int *ruch) {
    int kol = stanGry->kolejka-1;
    //int opp1 = stanGry->kolejka==P1 ? pole-stanGry->kostki[0] : pole+stanGry->kostki[0];
    //int opp2 = stanGry->kolejka==P1 ? pole-stanGry->kostki[1] : pole+stanGry->kostki[1];
    if (stanGry->kostki[0] == 0 && stanGry->kostki[1] == 0) {
        wclear(info);
        mvwprintw(info, 8, 0, "Koniec ruchu");
        wrefresh(info);
        sleep(1);
        wclear(info);
        *ruch = FALSE;
    }
    //if (stanGry->kostki[0] == 0 && )
}

//liczba mozliwa do zdjecia

int zdejOgol(GameState *stanGry){
    int kol = stanGry->kolejka-1;
    int opp = (kol+1)%2;
    int zbicie = 0;

    for (int i = 0; i<24;i++){
        if (stanGry->pionki[kol][i] >0){
            int pole1 = stanGry->kolejka==P1 ? i-stanGry->kostki[0] : i+stanGry->kostki[0];
            int pole2 = stanGry->kolejka==P1 ? i-stanGry->kostki[1] : i+stanGry->kostki[1];
            if (stanGry->pionki[opp][pole1] == 1 && pole1 >= 0 && pole1 <24){
                zbicie++;
            }
            else if (stanGry->pionki[opp][pole2] == 1 && pole2 >= 0 && pole2 <24){
                zbicie++;
            }

        }
    }
    return zbicie;
}

int zdejPole(GameState *stanGry, int pole){
    int kol = stanGry->kolejka-1;
    int opp = (kol+1)%2;
    int zbicie = 0;

    int pole1 = stanGry->kolejka==P1 ? pole-stanGry->kostki[0] : pole+stanGry->kostki[0];
    int pole2 = stanGry->kolejka==P1 ? pole-stanGry->kostki[1] : pole+stanGry->kostki[1];
    if (stanGry->pionki[opp][pole1] == 1 && pole1 >= 0 && pole1 <24){
        zbicie++;
    }
    else if (stanGry->pionki[opp][pole2] == 1 && pole2 >= 0 && pole2 <24){
        zbicie++;
    }
    return zbicie;
}

void barDis(WINDOW *info){
    wclear(info);
    mvwprintw  (info, 10, 0, "Musisz wyjsc z baru");
    wrefresh(info);
    sleep(1);
}

void barObw(GameState *stanGry, WINDOW *plansza, WINDOW *info, int *ruch){
    int kol = stanGry->kolejka-1, opp = (kol+1)%2;
    int pos1 = kol+1==P1 ? 24-stanGry->kostki[0] : stanGry->kostki[0]-1;
    int pos2 = kol+1==P1 ? 24-stanGry->kostki[1] : stanGry->kostki[1]-1;
    int poz1 = stanGry->pionki[opp][pos1], poz2 = stanGry->pionki[opp][pos2];


    if (poz1 == 1 || poz2 == 1) {
        int pos = pos1 == 1 ? pos1 : pos2;
        int kosc = pos1 == 1 ? 1 : 2;
        zbicie(stanGry, info, kosc, 100);
    }
    else if (poz1 == 0 || poz2 == 0){
        int pos = pos1 == 0 ? pos1 : pos2;
        int kosc = pos1 == 0 ? 1 : 2;
        ruchNzbicie(stanGry, info, kosc, 100);
    }
    else if (poz1 >1 && poz2 > 1){
        barDis(info);
        *ruch = FALSE;
    }
}

int wGranicach(int poz) {
    return (poz >= 0 && poz <= 23);
}

int condBar (GameState *stanGry, int opp, int k1, int k2){
    int kol = stanGry->kolejka - 1;
    int pos1 = kol+1==P1 ? 24-k1 : k1-1;
    int pos2 = kol+1==P1 ? 24-k2 : k2-1;
    if (stanGry->pionki[opp][pos1] > 1 && stanGry->pionki[opp][pos2] > 1) return FALSE;
    else if (stanGry->pionki[opp][pos1] > 1 && k2 == 0) return FALSE;
    else if (stanGry->pionki[opp][pos2] > 1 && k1 == 0) return FALSE;
    return TRUE;
}

int maszRuch(GameState *stanGry, int opp, int opp1, int opp2) {
    int kol = stanGry->kolejka-1, k1 = stanGry->kostki[0], k2 = stanGry->kostki[1];
    int poz1 = stanGry->pionki[opp][opp1], poz2 = stanGry->pionki[opp][opp2];

    if ((poz1 > 1 &&  poz2 > 1) || (poz1 > 1 &&  k2 == 0) || (poz2 > 1 &&  k1 == 0) || (!wGranicach(poz1) && !wGranicach(poz2))) return FALSE;

    if (stanGry->bar[kol] > 0 && !condBar(stanGry, opp, k1, k2)){
        return FALSE;
    }
    return TRUE;
}

void zrobRuch(GameState *stanGry, WINDOW *info, int pole, int *ruch){
    int kol = stanGry->kolejka-1;

    int opp1 = kol+1==P1 ? pole-stanGry->kostki[0] : pole+stanGry->kostki[0];
    int opp2 = kol+1==P1 ? pole-stanGry->kostki[1] : pole+stanGry->kostki[1];
    
    if ((stanGry->pionki[kol][pole] != 0 && !maszRuch(stanGry, (kol+1)%2, opp1, opp2)) || stanGry->pionki[kol][pole] == 0){
        mvwprintw(info, 8, 0, "Wybierz inne pole");
    }
    else if (zdejOgol(stanGry) > 0 && zdejPole(stanGry, pole) == 0){
        mvwprintw(info, 10, 0, "Nakaz zbicia!");
    }
    else {
        wybKostki(stanGry, pole, info, ruch, zdejOgol(stanGry));
    }
    wrefresh(info);
    sleep(1);
    wclear(info);
}

void wykonajR(GameState *stanGry, WINDOW *info, WINDOW *plansza){
    int pole, ruch = TRUE, kostka;
    
    while (ruch){
        wclear(info);
        mvwprintw(info, 4, 0, "Gracz %d \n\nPozostale ruchy \n1. %d\n2. %d", stanGry->kolejka,stanGry->kostki[0],stanGry->kostki[1]);
        wrefresh(info);
        if (stanGry->bar[stanGry->kolejka-1] > 0) barObw(stanGry, plansza, info, &ruch);
        else {
            mvwprintw(info, 10, 0, "Wybierz pole");
            wmove(info, 12, 0);
            wrefresh(info);
            wscanw(info, "%d", &pole);
            wclear(info);
            zrobRuch(stanGry, info, pole-1, &ruch);
        }
        displayPlansza(stanGry, plansza);
        checkIfGra(stanGry, info, &ruch);
    }
}

void display(GameState *stanGry, WINDOW *info, int *graAktywna, WINDOW *plansza){
    char iInfo;
    mvwprintw(info, 0, 0, "Backgammon\n\nP1(O): %s \n \nP2(X): %s\n\nkolej gracza %d",stanGry->gracz1, stanGry->gracz2, stanGry->kolejka);
    mvwprintw(info, 9, 0, "Wykonaj ruch -> c\nZapisz stan gry -> s");
    wrefresh(info);
    iInfo = getch();
    if(iInfo == 's'){
        saveGame(stanGry, info);
        *graAktywna = FALSE;
    }
    else if (iInfo == 'c'){
        wclear(info);
        echo();
        wykonajR(stanGry, info, plansza);
        noecho();
    }
    else {
        wclear(info); 
        mvwprintw(info, 2, 0, "Zly input");
    }
    wrefresh(info);
}

void losowanieKostek(GameState *stanGry){
    mvprintw(ROWS/2-2,COL/2-5,"Losowanie kostek...");
    refresh();
    sleep(2);
    stanGry->kostki[0] = rand() % 6 + 1;
    stanGry->kostki[1] = rand() % 6 + 1;
    clear();
    refresh();
}

void wyswietlKos(GameState *stanGry){
    mvprintw(ROWS/2-2,COL/2-10,"Wynik:");
    mvprintw(ROWS/2,COL/2-10,"P1: %d   P2: %d", stanGry->kostki[0], stanGry->kostki[1]);
    refresh();
    sleep(1);
    clear();
}

void losowanieKolejki(GameState *stanGry){
    while (!stanGry->kolejka){
        mvprintw(ROWS/2-2,COL/2-10,"Losowanie kostek...");
        refresh();
        sleep(2);
        clear();
        stanGry->kostki[0] = rand() % 6 + 1;
        stanGry->kostki[1] = rand() % 6 + 1;

        wyswietlKos(stanGry);

        if (stanGry->kostki[0]>stanGry->kostki[1]){
            stanGry->kolejka = P1;
            mvprintw(ROWS/2-2,COL/2-10, "Zaczyna %s", stanGry->gracz1);
        }
        else if (stanGry->kostki[1]>stanGry->kostki[0]){
            stanGry->kolejka = P2;
            mvprintw(ROWS/2-2,COL/2-10, "Zaczyna %s", stanGry->gracz2);
        }
        else {
            mvprintw(ROWS/2-2,COL/2-10, "Remis");
        }
        clear();
        refresh();
        sleep(1);
    }
}

void granie(GameState *stanGry, int *graAktywna, WINDOW *plansza, WINDOW *info) {
    char input;
    while (*graAktywna) {
        if(stanGry->kolejka == 0){
            losowanieKolejki(stanGry);
        }
        else losowanieKostek(stanGry);
        displayPlansza(stanGry, plansza);
        display(stanGry, info, graAktywna, plansza);
        stanGry->kolejka = stanGry->kolejka==P1 ? P2:P1;
    }


    refresh();
}

void nowaGra(int *graAktywna, WINDOW *plansza, WINDOW *info){
    GameState game;

    for (int i=0; i<24; i++){
        game.pionki[0][i] = 0;
        game.pionki[1][i] = 0;
    }
    game.pionki[1][23] = 2;
    game.pionki[1][12] = 5;
    game.pionki[1][7] = 3;
    game.pionki[1][5] = 5;
    game.pionki[0][0] = 2;
    game.pionki[0][11] = 5;
    game.pionki[0][16] = 3;
    game.pionki[0][18] = 5;
    game.kolejka = 0;
    game.bar[0] = 0;
    game.bar[1] = 0;
    
    refresh();

    ustawNazwy(&game);
    noecho();
    granie(&game, graAktywna, plansza, info);
    refresh();
    *graAktywna = FALSE;
}

void wczytajGre(int *graAktywna, WINDOW *plansza, WINDOW *info){
    GameState game;
    char input;
    
    mvprintw(ROWS/2-10,COL/2-10, "Z ktorego zapisu chcesz wczytac");
    mvprintw(ROWS/2-8,COL/2-10, "1.");
    mvprintw(ROWS/2-6,COL/2-10, "2.");
    mvprintw(ROWS/2-10,COL/2-10, "3.");
    refresh();
    input = getch();
    loadGameState(&game, &input);
    clear();
    refresh();

    granie(&game, graAktywna, plansza, info);
    refresh();
    *graAktywna = FALSE;
}

void displayInterfaceFromFile(const char *filename) {
    int ch;
    FILE *fp = fopen(filename, "r"); 
    
    while ((ch = fgetc(fp)) != EOF) {
        printw("%c", ch);
    }

    refresh();  
    fclose(fp);
}

void interface(){
    WINDOW *plansza, *info;
    plansza = create_newwin(19, 27, 2, 4);
    info = create_newwin(24, 40, 4, 40);
    
    char input; 
    int graAktywna = TRUE;

    noecho();

    do {
        displayInterfaceFromFile("interface.txt");
        input = getch();

        clear();
        refresh();
        if(input == '1'){
            nowaGra(&graAktywna, plansza, info);
        }
        else if (input == '2'){
            wczytajGre(&graAktywna, plansza, info);
        }
        else if (input == '3') {
            graAktywna = FALSE;
            endwin();
        }
        else {
            mvprintw(ROWS/2-1, (COL-28)/2,"Zly input, wpisz liczbe <1,4>");
            refresh();
            sleep(1);
            clear();
            refresh();
        }
    } while (graAktywna);
}

int main() {
    srand(time(NULL));
    int row,col;

    initscr();	
    noecho();	
    clear();

    interface();

    refresh();
    getch();
    endwin();
    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	//box(local_win, 0 , 0);	
    	/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, '|', '|', '-','-','*','*','*','*');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}
