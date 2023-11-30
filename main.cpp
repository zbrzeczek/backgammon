#include <iostream>
#include <fstream>
#include <ncurses.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

int main() {
    FILE *fp;
    int ch, prev, y, x;
    char mesg[]="Just a string";		
    int row,col;				
    initscr();	
    noecho();	

    WINDOW *my_win;
    my_win = newwin(100, 100, 50, 50);
    box(my_win, 0, 0);
    fp = fopen("interface.txt", "r+");
    if(fp == NULL){
        perror("Cannot open input file");
        exit(1);
    }

    while ((ch = fgetc(fp)) != EOF){
        getyx(stdscr, y, x);		/* get the current curser position */
        
        //check if its at the end of screen
        if(y == (row - 1)){
            wprintw(my_win, "<-Press Any Key->");	/* tell the user to press a key */
            getch();
            clear();				/* clear the screen */
            move(0, 0);			/* start at the beginning of the screen */
        }

        if(prev == '/' && ch == '*')    	/* If it is / and * then only switch bold on */    
        {
            attron(A_BOLD);			/* cut bold on */
            getyx(stdscr, y, x);		/* get the current curser position */
            move(y, x - 1);			/* back up one space */
            wprintw(my_win, "%c%c", '/', ch); 		/* The actual printing is done here */
        }

        else wprintw(my_win, "%c", ch);

        wrefresh(my_win);

        if(prev == '*' && ch == '/') attroff(A_BOLD);        		/* Switch it off once we got *
                                 	 * and then / */
        prev = ch;
    }
    		
    mvwprintw(my_win, row/2,(col-strlen(mesg))/2,"%s",mesg);
    mvwprintw(my_win, row-2,0,"This screen has %d rows and %d columns\n",row,col);
    printw("Try resizing your window(if possible) and then run this program again");
    wrefresh(my_win);
    getch();
    endwin();
    return 0;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

/*
void interface() {
    char interfaceInput;
    bool wyborInterface = true;

    while (wyborInterface){
        wyswietleniePliku("interface.txt");
        if (interfaceInput != '1' || interfaceInput != '2' || interfaceInput != '3'){
            printf("Nie ma takiej opcji, podaj numer");
        }
        else {
            printf("cos nie pasi");
            wyborInterface = false;
        }
    }
}

int main (){
    int x;
    interface();
    return 0;
}*/