#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define M b->dimM
#define N b->dimN

/*
Programmieren I - Projekt Springerproblem INF16B

benötigte Compiler Flags: -std=c99

Zum starten im Debug Modus indem alle Startfelder bei einem 8x8 Feld
auf geschlossene Routen getestet werden als Argument -d angeben

Die Fensterbreite der Konsole muss unter Umständen angepasst werden um größere Felder korrekt anzuzeigen.
*/

//Moves verlaufen im Uhrzeigersinn beginnend oben rechts
/*
   7     0
6           1
      S
5           2
   4     3
*/
//                    0   1   2   3   4   5   6   7
const short xM[8] = { 1,  2,  2,  1, -1, -2, -2, -1};
const short yM[8] = {-2, -1,  1,  2,  2,  1, -1, -2};

typedef struct board
{
    int* pField;     //Pointer auf dynamisch allokierten Speicher
    int  dimM;       //m = Zeilen
    int  dimN;       //n = Spalten
    int  startX;     //Startfeld
    int  startY;
    int  currentX;   //aktuelles Feld
    int  currentY;
    bool closed;     //Routentyp geschlossen/offen
} bParams;

void logo()
{
    printf(" ____ ____ ____ ____ ____ ____ ____ ____ _________ ____ ____ ____ ____ ____ ____ ____ \n");
    printf("||S |||p |||r |||i |||n |||g |||e |||r |||       |||P |||r |||o |||b |||l |||e |||m ||\n");
    printf("||__|||__|||__|||__|||__|||__|||__|||__|||_______|||__|||__|||__|||__|||__|||__|||__||\n");
    printf("|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/_______\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|/__\\|\n");
    for(int i = 0; i < 82; i++) printf("_");
    printf("v1.1\n\n");
    //created with http://www.kammerl.de/ascii/AsciiSignature.phpa
}

/*
Diese Methode ist für die Ausgabe in der Konsole verantwortlich
Dabei erstellt sie dynamisch je nach Spielfeldgröße eine korrekt skalierte Ansicht
inkl. Reihen- und Spaltenbeschriftung mit Nummern bzw Buchstaben gemäß einem originalen Schachbrett
*/
void displayBoard(bParams *b)
{
    printf("\n      ");
    for(int i = 0; i < N; i++)
        printf("  -- ");
    printf("\n");
    for(int i = 0; i < M; i++) {
        if(M-i < 10)                                                              //Gebe Zeilennummer absteigend aus
            printf("<%d>   |", M-i);
        else
            printf("<%d>  |", M-i);
        for(int j = 0; j < N; j++) {
            if     (b->pField[i*N+j] < 10)  printf("  %d |", b->pField[i*N+j]);   //Gebe Nummer aus Feld aus, gemäß ihrer Stellen
            else if(b->pField[i*N+j] < 100) printf(" %d |", b->pField[i*N+j]);
            else                            printf("%d |", b->pField[i*N+j]);
        }
        printf("\n      ");
        for(int j = 0; j < N; j++)
            printf("  -- ");
        printf("\n");
    }
    printf("\n        ");
    for(int i = 0; i < N; i++)                                                  //Gebe Spaltenbeschriftung mit Buchstaben aus
        printf("<%c>  ", 'a' + i);
    printf("\n\n");
}

/*
Allokiert dynamisch einen passenden Speicherbereich auf dem Heap in Abhängigkeit
von den eingegebenen Dimensionen
*/
void allocMemory(bParams *b)
{
    b->pField = calloc(M*N, M*N*sizeof(int));
}

/*
Gibt den dynamisch erzeugten Speicher wieder frei
*/
void freeMomory(bParams *b)
{
    free(b->pField);
}

/*
Methode zur Überprüfung ob eine gschlossene Route auf gegebenen Brett nach
dem Schenkschen Theorem möglich ist
*/
bool checkClosedTourPossible(bParams *b)
{
    int m = M;
    int n = N;
    if(m >= n) {                        //Wenn m>=n tausche m & n
        int temp = m;
        m = n;
        n = temp;
    }
    if(m%2 == 1 && n%2 == 1)            //Bedingungen siehe https://de.wikipedia.org/wiki/Springerproblem#Schwenksches_Theorem
        return false;
    if(m == 1 || m == 2 || m ==4)
        return false;
    if((m == 3 && n == 4) || (m == 3 && n == 6) || (m == 3 && n == 8))
        return false;
    return true;                        //Falls keine der obigen Bedingungen erfüllt sind, ist eine geschlossene Route möglich
}

/*
Methode zur manuellen Eingabe des Startfeldes in alphanumerischer Art um
die Benutzerfreundlichkeit zu steigern (a1 besser als 7:0)
Da leeres Spielfeld nach Dimensionseingabe dargestellt wird
*/
void selectSartPos(bParams *b)
{
    char sx = 'a';
    int  sy = 0;
    int  x = 0, y = 0;
    do {
        printf("\nStartposition eingeben Bsp.(a1) :: ");
        fflush(stdin);
        scanf("%c %d", &sx, &sy);
        y = M - sy;
        x = ((int)sx - 97);
        if(y < 0 || y > M-1 || x < 0 || x > N-1)
            printf("Ung%cltige Eingabe!\n", 129);
    }while(y < 0 || y > M-1 || x < 0 || x > N-1);
    b->startX = x;
    b->startY = y;
    b->pField[y*N+x] = 1;
}

/*
Generiert automatisch mit Hilfe der rand() Methode 2 Koordinaten für das
zufällige Startfeld
*/
void generateStartPos(bParams *b)
{
    int x = 0, y = 0;
    srand(time(NULL));
    x = b->startX = rand() % N;   //N = Spalten
    y = b->startY = rand() % M;   //M = Zeilen
    b->pField[y*N+x] = 1;
    printf("\n      Startfeld: <%c%d>\n", 97+x, M-y);     //Ausgabe des Startfeld damit Benutzer es schneller findet
}

/*
Kümmert sich um die Benutzereingaben bei Programmstart
1. Dimensionen des Spielfeldes m x n
2. Art der Route (offen/geschlossen)
3. Bestimmung des Startfeldes (automatisch/manuell)
*/
void userInput(bParams *b)
{
    char sTyp, rTyp;
    do {
        printf("Bitte Dimensionen von Spielfeld eingeben (MAX 26,26) Bsp. 8,8 :: ");
        fflush(stdin);
        scanf("%d,%d", &M, &N);
        if(M <= 0 || M > 26 || N <= 0 || N > 26)
            printf("Ung%cltige Dimensionen\n", 129);
    }while(M <= 0 || M > 26 || N <= 0 || N > 26);
    allocMemory(b);                                 //Allokiert den Speicher direkt nach Dimensionseingabe
    displayBoard(b);                                //Gibt das leere Spielfeld aus
    do {
        printf("Soll die Route geschlossen sein? (j/n) :: ");
        fflush(stdin);
        scanf("%c", &rTyp);
        if(rTyp != 'j' && rTyp != 'n')
            printf("Ung%cltige Eingabe!\n", 129);
    }while(rTyp != 'j' && rTyp != 'n');
    if(rTyp == 'n')
        b->closed = false;
    else {
        if(rTyp == 'j' && checkClosedTourPossible(b))
            b->closed = true;
        else {
            printf("Geschlossene Tour laut Schwenkschem Theorem nicht m%cglich!\n\n", 148);
            b->closed = false;
        }
    }
    do {
        printf("Soll ein Startfeld zuf%cllig gew%chlt werden? (j/n) :: ", 132, 132);
        fflush(stdin);
        scanf("%c", &sTyp);
        if(sTyp != 'j' && sTyp != 'n')
            printf("Ung%cltige Eingabe!\n", 129);
    }while(sTyp != 'j' && sTyp != 'n');
    if(sTyp == 'j')
        generateStartPos(b);
    else
        selectSartPos(b);
}

/*
Diese Methode arbeitet analog zu displayBoard() nur das hier das Spielfeld
in die Datei ausgabe.txt gespeichert wird
*/
bool writeToFile(bParams *b)
{
    FILE *out = fopen("ausgabe.txt", "w");
    time_t t;
    time(&t);
    fprintf(out, "Ausgabe wurde erstellt am %s\n", ctime(&t));
    fprintf(out, "\n      ");
    for(int i = 0; i < N; i++)
        fprintf(out, "  -- ");
    fprintf(out, "\n");
    for (int i = 0; i < M; i++) {
        if(M-i < 10)
            fprintf(out, "<%d>   |", M-i);
        else
            fprintf(out, "<%d>  |", M-i);
        for(int j = 0; j < N; j++) {
            if     (b->pField[i*N+j] < 10)  fprintf(out, "  %d |", b->pField[i*N+j]);
            else if(b->pField[i*N+j] < 100) fprintf(out, " %d |", b->pField[i*N+j]);
            else                            fprintf(out, "%d |", b->pField[i*N+j]);
        }
        fprintf(out, "\n      ");
        for(int h = 0; h < N; h++)
            fprintf(out, "  -- ");
        fprintf(out, "\n");
    }
    fprintf(out, "\n        ");
    for(int i = 0; i < N; i++)
        fprintf(out, "<%c>  ", 'a' + i);
    fprintf(out, "\n\n");
    if(b->closed)
        fprintf(out, "\nArt der Route: geschlossen\n");
    else
        fprintf(out, "\nArt der Route: offen\n");
    fclose(out);
    return true;
}

/*
Auswahlmethode für das Speichern des ausgefüllten Spielfeldes
*/
void saveToFile(bParams *b)
{
    char save;
    do {
        printf("Soll das Ergebnis in eine Textdatei gespeichert werden? (j/n) :: ");
        fflush(stdin);
        scanf("%c", &save);
        if(save != 'j' && save != 'n')
            printf("Ung%cltige Eingabe!\n", 129);
    }while(save != 'j' && save != 'n');
    if(save == 'j')
        if(writeToFile(b))
            printf("Datei wurde erfolgreich gespeichert\n");
}

/*
Gibt als Wahrheitswert zurück, ob ein Feld mit den Koordinaten (y;x) innerhalb des Spielfeldes ist
und das Feld noch frei ist (Wert 0)
*/
bool isMovePossible(int x, int y, bParams *b)
{
    return (x >= 0 && x < N && y >= 0 && y < M && b->pField[y*N+x] == 0);
}

/*
Gibt die Anzahl der freien Felder um das gesetzte Startfeld zurück
0 bedeutet, das Startfeld ist vom Endfeld aus erreichbar
bzw. nicht mehr erreichbar wenn der Springer sich noch in seiner Route befindet
Relevant für geschlossene Routen
*/
short getFreeFieldAroundStart(bParams *b)
{
    short count = 0;
    for(int i = 0; i < 8; i++) {
        if(isMovePossible(b->startX+xM[i], b->startY+yM[i], b))
            count++;
    }
    return count;
}

/*
Methode bekommt einen Pointer auf eine in nextMove() erzeugten Liste, welche in der Art transformiert wird,
dass die Züge mit den wenigsten Nachfolgemöglichkeiten aufsteigend sortiert abgespeichert werden.
Diese Heuristik dient dazu die Laufzeit des Backtracking erheblich zu verbessern, da so weniger häufig zurückgesprungen
werden muss.
*/
short warnsdorf(int* list, bParams *b)
{
    short idx = 0;                                              //Index zum Navigieren innerhalb der list[]
    short count = 0;                                            //Zählt die Anzahl der möglichen Moves in List[]
    int tempList[8] = {-1,-1,-1,-1,-1,-1,-1,-1};                //temporäre Liste zum Zwischenspeichern der nachfolgenden Felder
    for(int i = 0; i < 8; i++) {                                //Teste alle 8 möglichen nachfolge Züge
        int nextX = b->currentX+xM[i];
        int nextY = b->currentY+yM[i];
        if(isMovePossible(nextX, nextY, b) == false)
            list[i] = -1;                                       //speichere -1 zum indentifizieren eines ungültigen Zuges
        else {
            int rating = 0;                                     //speichert die Anzahl der Nachfolgefelder
            for(int k = 0; k < 8; k++) {                        //Teste alle 8 nachfolgenden Felder des nächsten Zuges
                int nnX = nextX + xM[k];
                int nnY = nextY + yM[k];
                if(isMovePossible(nnX, nnY, b))
                    rating++;
            }
            list[i] = rating;                                   //Speichere Rating in list[]
        }
    }
    for(int i = 0; i < 8; i++) {                                //sortiert list[] nach günstigsten Zug und speichert den Index in templist[]
        for(int j = 0; j < 8; j++) {
            if(list[j] == i) {
                tempList[idx] = j;
                idx++;
            }
        }
    }
    for(int i = 0; i < 8; i++) {                                //Kopiert templist[] nach list[] und zählt dabei die Anzahl der möglichen Move
        list[i] = tempList[i];
        if(list[i] != -1)
            count++;
    }
    return count;                                               //Gebe Anzahl der möglichen Moves zurück
}

/*
Dies ist die Hauptmethode zur Bestimmung der Lösung
Sie schreibt die Schrittnummer in das Spielfeld und erhöht diese beim nächsten Rekusiven Aufruf
*/
bool nextMove(int moveNr, int x, int y, bParams *b)
{
    b->currentX = x;                                            //Legt die Parameter in der Struktur ab
    b->currentY = y;
    if(M*N+1 == moveNr)                                         //Wenn alle Felder belegt sind breche die Rekursion ab mit Wert true
        return true;
    else {
        int warnsList[8] = {-1,-1,-1,-1,-1,-1,-1,-1};           //Speichert die Züge bewertet nach der Warnsdorf Heuristik
        short count = warnsdorf(warnsList, b);                  //rufe warnsdorf() auf, um Züge zu bewerten
        for(short i = 0; i < count; i++) {                      //Teste alle nach warnsdorf besten Züge nacheinander count Hierbei
            int newX = x+xM[warnsList[i]];                      //Erzeuge neue Koordinaten mit Hilfe der aus warnslist[] generierten Moveindex
            int newY = y+yM[warnsList[i]];
            if(b->closed) {                                     //Check auf geschlossene oder offene Route (abhängig für prüfung ob freie felder um start)
                if(isMovePossible(newX, newY, b) && getFreeFieldAroundStart(b) != 0) {
                    b->pField[newY*N+newX] = moveNr;            //Schreibe Movenummer in feld
                    if(nextMove(moveNr+1, newX, newY, b)==true) //Rekurisver Aufruf von nextMove()
                        return true;
                    else
                        b->pField[newY*N+newX] = 0;             //Falls move nicht möglich setzte zurück und schreibe wieder 0 ins Feld
                }
            }
            else {                                              //Variante für offene Route ohne check auf freie nachbarfelder
                if(isMovePossible(newX, newY, b)) {             //ansonsten analog zu geschlossener Route
                    b->pField[newY*N+newX] = moveNr;
                    if(nextMove(moveNr+1, newX, newY, b)==true)
                        return true;
                    else
                        b->pField[newY*N+newX] = 0;
                }
            }
        }
    }
    return false;                                               //falls keine Lösung gefunden wurde wird false zurückgegeben
}

/*
Startet die Lösungsfindung
*/
void findSolution(bParams *b)
{
    if(nextMove(2, b->startX, b->startY, b)==false)
        printf("Es gibt keine L%csung f%cr die gegebenen Parameter\n", 148, 129);
    else
        displayBoard(b);            //Ausgabe des fertigen Feldes
}

/*
Hauptprogramm
*/
int main(int argc, char* argv[])
{
    logo();
    const char* a = argv[argc-1];
    const char* s = "-d";
    bool debug = false;
    if(strcmp(a, s) == 0)
        debug = true;
    if(debug) {                             //1 für Test um alle Startfelder auf einmal zu testen
        bParams b;                          //Erzeuge Struktur
        b.dimM = 8;                         //Manuelle Eingabe der Dimensionen zu Debug Zwecken
        b.dimN = 8;
        for(int i = 0; i<b.dimM; i++) {     //Gehe alle Starfelder durch und finde eine Lösung
            for(int j = 0; j<b.dimN; j++) {
                printf("Solution :: %d\n", i*b.dimM+j+1);
                allocMemory(&b);
                b.pField[j*b.dimN+i]=1;
                b.startX = i;
                b.startY = j;
                b.closed = true;            //Hier kann der Routentyp zwecks Debug gewählt werden
                findSolution(&b);
                freeMomory(&b);
            }
        }
    }
    else {                                  //Regulärer Programmablauf
        char repeat;
        bool again;
        do {
            bParams b;                      //Ereuge Struktur
            userInput(&b);                  //Starte Benutzereingaben
            findSolution(&b);               //Finde Lösung
            saveToFile(&b);                 //Abspeichern in .txt Datei
            freeMomory(&b);                 //Speicher auf dem Heap freigeben
            do {
                printf("Soll ein neues Feld berechnet werden? (j/n) :: ");
                fflush(stdin);
                scanf("%c", &repeat);
                if(repeat != 'j' && repeat != 'n')
                    printf("Ung%cltige Eingabe!\n", 129);
            }while(repeat != 'j' && repeat != 'n');
            if(repeat == 'j')
                again = true;
            if(repeat == 'n')
                again = false;
            printf("\n\n");
        }while(again);                       //Wiederhole solange der Benutzer es wünscht
    }
    return EXIT_SUCCESS;
}
