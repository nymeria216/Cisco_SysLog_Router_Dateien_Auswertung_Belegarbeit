/* 
Hochschule Mittweida
University of Applied Sciences
 
Studiengang: Bachelor of Science IT-Forensik/Cybercrime
Seminargruppe: CC24w1-B
Modul: Programmierung I
Dozent: Prof. Dirk Pawlaszczyk

Belegarbeit: Auswertung von CISCO-Syslog-Routerdateien

Autoren: Melissa Futtig, Philipp Dellwo
Datum: 30.08.2025

Beschreibung: Das folgende Programm dient der Auswertung von CISCO-Syslog-Routerdateien.
              Es ermöglicht die Suche nach Begriffen und die Filterung nach Bestandteilen der Dateien.
              Die daraus gewonnenen Ergebnisse können in einer txt.-Datei gespeichert werden.
*/


/* ===== Präprozessor-Direktiven ===== */

#define _CRT_SECURE_NO_WARNINGS     // Unterdrückt Warnungen zu unsicheren Funktionen bei MS Visual Studio
#include <stdio.h>                  // Standard-Ein-/Ausgabe
#include <string.h>                 // Stringfunktionen
#include <stdlib.h>                 // Speicherverwaltung, Systemaufrufe
#include <ctype.h>                  // Buchstaben-Bibliothek
// #include <windows.h>             // Für Windows-spezifische Funktionen Kommentar entfernen


/* ===== Farbdefinitionen (Konsole) ===== */

#define RED     "\033[31;91m"   // Rot für Fehlermeldungen
#define YELLOW  "\033[33;93m"   // Gelb für Warnungen
#define RESET   "\033[0m"       // Reset auf Standardfarbe



/* ===== Plattformabhängige Anpassungen ===== */

#ifdef _WIN32                   
#define strcasecmp _stricmp     // Windows: Ersatz für fehlendes strcasecmp
#endif


/* ===== Variablendeklaration ===== */

/* --- Dateien --- */

FILE* datei;                 // Dateizeiger für Eingabedatei
FILE* outputDatei = NULL;    // Dateizeiger für Ausgabedatei (für Treffer), NULL wenn keine Datei geöffnet
char dateiname[256];         // Pfad/Name der Eingabedatei

/* --- Zähler / Steuerung --- */

int zeilennummer = 0;        // Aktuell verarbeitete Zeilennummer
int treffer = 0;             // Anzahl der gefundenen Treffer
int begriff = 1;             // Suchmodus/Begriffs-Auswahl
int zeitauswahl = 1;         // Auswahl der Zeitfilter
int versuch = 0;             // Zähler für Versuche/Eingaben

/* --- Zeitangaben --- */
int tag;                     // Tag aus Zeitstempel;
char monat[4];               // Monat als 3-Buchstaben-Kürzel
int jahr = 0;                // Jahr aus Zeitstempel
int minJahr = 9999,          // Kleinstes im Log gefundenes Jahr
maxJahr = 0;                 // Größtes im Log gefundenes Jahr
char uhrzeit[9];             // Uhrzeit "HH:MM:SS"
int stunde;                  // Stunde aus Zeitstempel
int minute;                  // Minute aus Zeitstempel
int sekunde;                 // Sekunde aus Zeitstempel

/* --- Suche --- */

char suchbegriff[256];       // Aktueller Suchbegriff
char zeile[1024];            // Puffer für eine eingelesene Zeile
char sevLevel[8];            // Severity-Level als Text
char exitEingaben[6];        // Benutzereingabe zum Beenden


/* ===== Funktionsdeklarationen ===== */

/* --- Menü --- */

void hauptmenue(void);                                   // Hauptmenü anzeigen/steuern
void auswahlnachSuche(int funktionID);                   // Optionen nach einer Suche

/* --- Suchfunktionen --- */

int  eigenerSuchbegriff(void);                           // Benutzerdefinierten Begriff abfragen
int  zeitraum(void);                                     // Suche/Filter nach Zeitraum
int  ipSuche(void);                                      // Suche nach IP-Adressen
void ipFilterSucheEinfach(int privat);                   // IP-Filter (private/öffentliche IP-Adressen)
void eigeneFacilitySuche(void);                          // Benutzerdefinierte Facility-Suche
void facilitySuche(void);                                // Vordefinierte Facility-Suche
void eigeneUserSuche(void);                              // Benutzerdefinierte User-Suche
void userSuche(void);                                    // Vordefinierte User-Suche
void eigeneMnemonicSuche(void);                          // Benutzerdefinierte Mnemonic-Suche
void mnemonicSuche(void);                                // Vordefinierte Mnemonic-Suche
int  severityLevel(void);                                // Filter nach Severity-Level

/* --- Datei/Ausgabe --- */

int  speichersuche(const char* zielDateiname);           // Treffer in Datei schreiben
int  neueDateiAuswaehlen(void);                          // Neue Eingabedatei wählen

/* --- Hilfsfunktionen --- */

int monatZuZahl(const char* monat);                                                             // Umwandlung Monat in Zahl
int zeitZuSekunden(int tag, const char* monat, int jahr, int stunde, int minute, int sekunde);  // Konvertiert vollständiges Datum + Uhrzeit in Sekunden seit Jahresbeginn
int zeitZuSekundenOhneJahr(int tag, const char* monat, int stunde, int minute, int sekunde);    // Wie zeitZuSekunden, jedoch ohne Berücksichtigung des Jahres   
int exitEingabe(char* exitEingaben);                                                            // Eingabe von "exit", um das Programm frühzeitig zu verlassen

/* ===== Funktionsdefinitionen ===== */

/* --- Hilfsfunktionen --- */

// Prüft, ob die Datei eine .log-Datei ist.
int log_dateiendung(const char* log_dateiname) {                                // Eingabe eines Dateinamen
    size_t laenge = strlen(log_dateiname);                                      // ermittelt die Länge des Strings log_dateiname
    return (laenge >= 4 && strcmp(log_dateiname + laenge - 4, ".log") == 0);    // Rückgabe: 1, wenn Endung ".log", sonst 0
}


// Prüft, ob ein String ein gültiger Monatsname ist.
int alleMonate(const char* monat) {                         // Eingabe von Monatskürzel
    const char* gueltigeMonate[] = {                        // Liste der erlaubten Monatskürzel
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    for (int i = 0; i < 12; i++) {                          // geht alle Monate in der Liste der gültigen Monate durch
        if (strcasecmp(monat, gueltigeMonate[i]) == 0) {    // prüft, ob Eingabe und Monat aus der Liste übereinstimmen (Groß-/Kleinschreibung egal)
            return 1;                                       // gültiger Monat gefunden
        }
    }
    return 0;                                               // kein gültiger Monat
}


// Wandelt einen Monatsnamen in die entsprechende Zahl (1-12) um.
int monatZuZahl(const char* monat) {            // Eingabe von Monatskürzel                    
    if (strcmp(monat, "Jan") == 0) return 1;    // Januar
    if (strcmp(monat, "Feb") == 0) return 2;    // Februar
    if (strcmp(monat, "Mar") == 0) return 3;    // März
    if (strcmp(monat, "Apr") == 0) return 4;    // April
    if (strcmp(monat, "May") == 0) return 5;    // Mai
    if (strcmp(monat, "Jun") == 0) return 6;    // Juni
    if (strcmp(monat, "Jul") == 0) return 7;    // Juli
    if (strcmp(monat, "Aug") == 0) return 8;    // August
    if (strcmp(monat, "Sep") == 0) return 9;    // September
    if (strcmp(monat, "Oct") == 0) return 10;   // Oktober
    if (strcmp(monat, "Nov") == 0) return 11;   // November
    if (strcmp(monat, "Dec") == 0) return 12;   // Dezember
    return 0;                                   // Rückgabe 0, wenn kein gültiger Monat übergeben wurde
}


// Prüft, ob eine IPv4-Adresse gültig ist
int istGueltigeIPv4(const char* ip) {                       // Eingabe einer IP-Adresse
    int a, b, c, d;

    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {    // Einlesen von vier Ganzzahlen im Format "a.b.c.d"
        if (a >= 0 && a <= 255 &&                            // Prüft, ob alle Zahlen im gültigen Bereich liegen (0-255)
            b >= 0 && b <= 255 &&
            c >= 0 && c <= 255 &&
            d >= 0 && d <= 255) {
            return 1;                                       // gültige IPv4-Adresse
        }
    }
    return 0;                                               // ungültige IPv4-Adresse
}


// Prüft, ob eine IP-Adresse im privaten Bereich liegt.
int istPrivateIP(const char* ip) {                                  // Übergabe einer IP-Adresse
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) return 0;   // ungültiges Format → nicht privat
    if (a == 10) return 1;                                          // 10.0.0.0 – 10.255.255.255 
    if (a == 172 && b >= 16 && b <= 31) return 1;                   // 172.16.0.0 – 172.31.255.255       
    if (a == 192 && b == 168) return 1;                             // 192.168.0.0 – 192.168.255.255  
    return 0;                                                       // sonst: öffentliche Adresse
}

// Vergleichsfunktion für die alphabetische Sortierung von Strings
int compareStrings(const void* a, const void* b) {                  // Übergabe von zwei Strings
    const char* sa = *(const char**)a;                              // erster String
    const char* sb = *(const char**)b;                              // zweiter String
    return strcmp(sa, sb);                                          // Vergleich der Strings
}


// Liest eine Zahl ein und prüft, ob diese gültig ist. Die Anzahl der Versuche ist begrenzt; bei Überschreitung wird das Programm beendet.
int begrenzungversuche(int min, int max, int maxVersuche) {         // Übergabe gültiger Werte
    int eingabe;
    int versuche = 0;
    char buffer[64];

    do {
        printf("\n\nAuswahl (%d-%d):\n", min, max);

        if (!fgets(buffer, sizeof(buffer), stdin)) {                // Ganze Zeile einlesen
            printf(RED "Fehler beim Lesen der Eingabe.\n" RESET);
            exit(1);                                                // beendet Programm bei Lesefehler
        }

        buffer[strcspn(buffer, "\n")] = '\0';                       // Zeilenumbruch am Ende entfernen

        exitEingabe(buffer);                                        // Exitfunktion

        if (strlen(buffer) == 0) {                                                                                          // prüft, ob Eingabe leer ist
            printf(YELLOW "Keine Eingabe erkannt. Bitte geben Sie eine Zahl zwischen %d und %d ein.\n" RESET, min, max);
            versuche++;
        }
        
        else if (sscanf(buffer, "%d", &eingabe) != 1 || eingabe < min || eingabe > max) {                                   // prüft, ob Zahlen im gültigen Bereich liegen
            printf(YELLOW "\nUngültige Eingabe. Bitte geben Sie eine Zahl zwischen %d und %d ein.\n" RESET, min, max);
            versuche++;
        }
        else {
            return eingabe; // gültige Zahl wird zurückgegeben
        }

        if (versuche < maxVersuche) {                                                           // wertet Fehlversuche aus
            printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
        }
        else {
            printf(RED "Zu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            exit(1);
        }

    } while (1);       // wiederholt, bis eine gültige Eingabe erfolgt oder max. Versuche erreicht sind
}


// Beendet jederzeit das Programm, wenn "exit" eingegeben wird.
int exitEingabe(char* exitEingaben) {                       // Übergabe zu prüfender String
    if (strcmp(exitEingaben, "exit") == 0) {                // prüft, ob Eingabe = "exit"              
        printf(RED "\nProgramm wird beendet.\n" RESET);     // gibt eine Meldung aus
        exit(0);                                            // Programm wird sofort beendet
    }
    return 0;
}


/* --- Datei/Ausage-Funktionen --- */


// Öffnet die Datei und gibt bei einem Fehler eine Meldung aus.
int dateiOeffnen() {
    datei = fopen(dateiname, "r");
    if (!datei) {
        perror(RED "Fehler beim Öffnen der Datei" RESET);
    }
    return 0;
}


// Fragt, ob die Suchergebnisse gespeichert werden sollen.
int speichersuche(const char* zielDateiname) {           // Übergabe der Datei, in die die Suchergebnisse gespeichert werden sollen
    char speichern[16];                                  // Variable für Antwort des Benutzers
    int versuche = 0;                                    // Zählt die Anzahl der Eingabeversuche
    const int maxVersuche = 3;                           // Maximale erlaubte Anzahl an Versuchen, bevor die Funktion abbricht

    do {
        printf("\nMöchten Sie die Ergebnisse in einer Datei speichern? (j/n):\n");
        
        if (!fgets(speichern, sizeof(speichern), stdin)) {
            printf(RED "Fehler beim Lesen der Eingabe.\n" RESET);
            exit(1);
        }

        speichern[strcspn(speichern, "\n")] = '\0';     // Zeilenumbruch entfernen
        exitEingabe(speichern); 

        if (strcmp(speichern,"j") == 0 || strcmp(speichern, "J") == 0) {     // Datei zum Schreiben öffnen
            outputDatei = fopen(zielDateiname, "w");
            if (!outputDatei) {                         // Fehler beim Öffnen
                perror(RED "Fehler beim Öffnen der Datei" RESET);
                return 0;
            }
            printf("\n");
            return 1;   // Ergebnisse werden gespeichert
        }
        else if (strcmp(speichern, "n") == 0 || strcmp(speichern, "N") == 0) {    // keine Speicherung
            printf("\n");
            return 0; 
        }
        else {      // ungültige Eingabe
            versuche++;
            if (versuche < maxVersuche) {
                printf(YELLOW "\nUngültige Eingabe.\n" RESET);
                printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
            }
            else {  // beendet nach zu vielen Fehlversuchen
                printf(RED "Zu viele ungültige Eingaben. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        }
    } while (1);    // wiederholen, bis gültige Eingabe erfolgt ist
}


/* --- Zeit/Datum-Funktionen --- */

// Liest die Logdatei, ermittelt vorhandene Tage und fragt interaktiv einen gültigen Tag ab.
int tagDefinition() {
    int tagVorhanden = 0;               // Gibt an, ob der eingegebene Tag in der Logdatei gefunden wurde
    int vorhandeneTage[31] = { 0 };     // Array für mögliche Tage 
    int versuch = 0;                    // Zählt die Eingabeversuche des Benutzers
    char tagEingabe[16];                // Puffer für die Eingabe des Benutzers

    // Logdatei öffnen und nach Tagen durchsuchen
    dateiOeffnen();                                 
    while (fgets(zeile, sizeof(zeile), datei)) {    // zeilenweise lesen
        char logMonat[4];
        int logTag, logJahr, dummy;

        // zwei mögliche Formate für das Datum prüfen
        if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3 ||                
            sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4) {

            // prüfen, ob Tag im gültigen Bereich liegt
            if (logTag >= 1 && logTag <= 31) {           
                if (!vorhandeneTage[logTag - 1]) {
                    vorhandeneTage[logTag - 1] = 1;
                    tagVorhanden = 1;
                }
            }
        }
    }
    fclose(datei);  // Datei wieder schließen

    // nur wenn Tage im Log gefunden wurden
    if (tagVorhanden) {                                 
        printf("\nVerfügbare Tage in den Logdaten: ");
        for (int i = 0; i < 31; i++) {
            if (vorhandeneTage[i]) {
                printf("%d ", i + 1);                   // zeigt gefundene Tage an
            }
        }

        do {                                                // Eingabeaufforderung für gültigen Tag
            printf("\nTag (DD): \n");
            fgets(tagEingabe, sizeof(tagEingabe), stdin);
            tagEingabe[strcspn(tagEingabe, "\n")] = '\0';   // Zeilenumbruch entfernen
            exitEingabe(tagEingabe);                        // Exitfunktion

            int gueltig = 1;    // Prüft, ob nur Ziffern eingegeben wurden
            for (int i = 0; tagEingabe[i] != '\0'; i++) {
                if (!isdigit((unsigned char)tagEingabe[i])) {
                    gueltig = 0;
                    break;
                }
            }

            if (!gueltig || strlen(tagEingabe) == 0) {      // Eingabe nicht gültig
                printf(YELLOW "\nUngültige Eingabe – geben Sie bitte einen Tag zwischen 1 und 31 ein." RESET);
                versuch++;
            }
            else {
                tag = atoi(tagEingabe);     // Eingabe in Zahl umwandeln
                if (tag < 1 || tag > 31 || !vorhandeneTage[tag - 1]) {      // prüft, ob dieser Tag im Log enthalten ist
                    printf(YELLOW "\nDieser Tag ist nicht in den Logdaten enthalten oder ungültig." RESET);
                    versuch++;
                }
                else {
                    break; // gültiger Tag
                }
            }

            if (versuch < 3) {      // begrenzt Fehlerversuche
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }

        } while (1);    // solange wiederholen, bis gültiger Tag eingegeben wurde
    }
    else {
        printf("Es ist keine Abfrage für den Tag nötig.\n");
        tag = -1;   // kein Tag gefunden
    }

    return 0;       // Funktion beendet
}


// Liest die Logdatei, ermittelt vorhandene Monate und fragt interaktiv einen gültigen Monat ab.
int monatDefinition() {
    int monatVorhanden = 0;             // Gibt an, ob der eingegebene Monat in der Logdatei gefunden wurde
    int versuch = 0;                    // Zählt Eingabeversuche des Benutzers
    char monatEingabe[16];              // Eingabepuffer für den vom Benutzer eingegebenen Monat
    char vorhandeneMonate[12][4];       // speichert maximal 12 gültige Monate
    int anzahlVorhandeneMonate = 0;     // Anzahl der tatsächlich in der Logdatei gefundenen Monate

    dateiOeffnen();     // öffnet Logdatei und sucht nach Monaten
    while (fgets(zeile, sizeof(zeile), datei)) {    // liest Daten zeilenweise ein
        char logMonat[4];
        int logTag, logJahr, dummy;

        // prüft zwei mögliche Formate für das Datum
        if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3 ||                   
            sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4) {

            // formatiert Monatskürzel einheitlich: Erster Buchstabe groß, Rest klein
            logMonat[0] = toupper(logMonat[0]);     
            logMonat[1] = tolower(logMonat[1]);
            logMonat[2] = tolower(logMonat[2]);
            logMonat[3] = '\0';

            // prüft, ob Monatskürzel gültig ist
            if (alleMonate(logMonat)) {

                // prüft, ob der Monat bereits gespeichert wurde
                int bereitsVorhanden = 0;
                for (int i = 0; i < anzahlVorhandeneMonate; i++) {
                    if (strcmp(vorhandeneMonate[i], logMonat) == 0) {
                        bereitsVorhanden = 1;
                        break;
                    }
                }
                // falls noch nicht enthalten: speichert neuen Monat
                if (!bereitsVorhanden && anzahlVorhandeneMonate < 12) {
                    strncpy(vorhandeneMonate[anzahlVorhandeneMonate], logMonat, 4);
                    anzahlVorhandeneMonate++;
                    monatVorhanden = 1;     // mindestens einen Monat gefunden
                }
            }
        }
    }
    fclose(datei);      // Datei wieder schließen

    // wenn Monate gefunden wurden, zeigt Auswahl an und fordert Eingabe
    if (monatVorhanden) {
        printf("\nVerfügbare Monate in der Datei: ");
        for (int i = 0; i < anzahlVorhandeneMonate; i++) {
            printf("%s ", vorhandeneMonate[i]);
        }
        printf("\n");

        do {
            printf("Monat (MMM): \n");
            fgets(monatEingabe, sizeof(monatEingabe), stdin);
            monatEingabe[strcspn(monatEingabe, "\n")] = '\0';       // entfernt Zeilenumbruch
            exitEingabe(monatEingabe);                              // Exitfunktion

            if (strlen(monatEingabe) != 3) {        // Eingabe muss genau 3 Zeichen haben
                printf(YELLOW "\nUngültige Eingabe (z. B. Jan, Feb, Mar...)." RESET);
                versuch++;
            }
            else {
                // Eingabe einheitlich formatieren
                monatEingabe[0] = toupper(monatEingabe[0]);
                monatEingabe[1] = tolower(monatEingabe[1]);
                monatEingabe[2] = tolower(monatEingabe[2]);
                monatEingabe[3] = '\0';

                // prüft, ob der eingegebene Monat in den Logdaten enthalten ist
                int gueltig = 0;
                for (int i = 0; i < anzahlVorhandeneMonate; i++) {
                    if (strcmp(monatEingabe, vorhandeneMonate[i]) == 0) {
                        gueltig = 1;
                        break;
                    }
                }

                if (!gueltig) {
                    printf(YELLOW "\nDieser Monat ist nicht in den Logdaten enthalten oder ungültig.\n" RESET);
                    versuch++;
                }
                else {
                    strncpy(monat, monatEingabe, 4);    // gültigen Monat in Variable schreiben
                    break;
                }
            }

            // wertet Fehlversuche aus
            if (versuch < 3) {
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        } while (1);    // wiederholen, bis eine gültige Eingabe erfolgt
    }
    else {
        printf("\nEs ist keine Abfrage für den Monat nötig.\n");
        monat[0] = '\0'; // Variable leer setzen
    }

    return 0;
}


// Liest die Logdatei, ermittelt vorhandene Jahre und fragt interaktiv ein gültiges Jahr ab.
int jahrDefinition() {
    int jahrVorhanden = 0;
    minJahr = 9999; maxJahr = 0;

    dateiOeffnen();
    while (fgets(zeile, sizeof(zeile), datei)) {
        char logMonat[4];
        int logTag, logJahr, dummy;
        if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3 && logJahr >= 1970 && logJahr <= 2100) {
            jahrVorhanden = 1;
            if (logJahr < minJahr) minJahr = logJahr;
            if (logJahr > maxJahr) maxJahr = logJahr;
        }
        else if (sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4 && logJahr >= 1970 && logJahr <= 2100) {
            jahrVorhanden = 1;
            if (logJahr < minJahr) minJahr = logJahr;
            if (logJahr > maxJahr) maxJahr = logJahr;
        }
    }
    fclose(datei);

    if (jahrVorhanden) {
        int versuch = 0;
        char jahrEingabe[13];
        char* endptr;

        do {
            printf("\nBitte wählen Sie eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl aus (%d).", minJahr, maxJahr);
            printf("\nJahreszahl (YYYY): \n");
            fgets(jahrEingabe, sizeof(jahrEingabe), stdin);
            jahrEingabe[strcspn(jahrEingabe, "\n")] = '\0';
            exitEingabe(jahrEingabe);

            jahr = (int)strtol(jahrEingabe, &endptr, 10);
            if (endptr == jahrEingabe || *endptr != '\0') {
                printf(YELLOW "\nKeine gültige Jahreszahl." RESET);
                versuch++;
            }
            else if (jahr < minJahr || jahr > maxJahr) {
                printf(YELLOW "\nDie Jahreszahl liegt außerhalb des gültigen Bereichs." RESET);
                versuch++;
            }
            else {
                break;
            }
            if (versuch < 3) {
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        } while (1);
    }
    else {
        jahr = -1;
        printf("\nEs ist keine Abfrage für das Jahr nötig.\n");
    }
    return 0;
}


// Liest eine Uhrzeit im Format HH:MM:SS ein und prüft ihre Gültigkeit.
int uhrzeitDefinition() {
    int versuch = 0;
    char uhrzeitEingabe[64];   // größerer Puffer

    do {
        printf("\nUhrzeit (HH:MM:SS): \n");
        
        if (!fgets(uhrzeitEingabe, sizeof(uhrzeitEingabe), stdin)) {
            printf(RED "Fehler beim Lesen der Eingabe.\n" RESET);
            exit(1);
        }

        // Zeilenumbruch am Ende entfernen
        uhrzeitEingabe[strcspn(uhrzeitEingabe, "\n")] = '\0';

        // Exit erlauben
        exitEingabe(uhrzeitEingabe);

        // Uhrzeit parsen
        if (sscanf(uhrzeitEingabe, "%d:%d:%d", &stunde, &minute, &sekunde) != 3) {
            printf(YELLOW "\nUngültiges Format. Bitte HH:MM:SS eingeben.\n" RESET);
            versuch++;
        }
        else if (stunde < 0 || stunde > 23 || minute < 0 || minute > 59 || sekunde < 0 || sekunde > 59) {
            printf(YELLOW "\nUngültige Uhrzeit: %02d:%02d:%02d\n" RESET, stunde, minute, sekunde);
            versuch++;
        }
        else {
            break;
        }

        if (versuch < 3) {
            printf(YELLOW "Noch %d Versuch(e) übrig\n" RESET, 3 - versuch);
        }
        else {
            printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            exit(1);
        }
    } while (1);

    return 0;
}

// Wandelt Datum und Uhrzeit (Tag, Monat, Jahr, Stunde, Minute, Sekunde) in Sekunden um.
int zeitZuSekunden(int tag, const char* monat, int jahr, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) return -1;
    int sekunden = sekunde + 60 * minute + 3600 * stunde + 86400 * (tag - 1) + 86400 * 31 * (mon - 1) + 86400 * 31 * 12 * jahr;
    return sekunden;
}


// Wandelt Datum ohne Jahr und Uhrzeit in Sekunden um.
int zeitZuSekundenOhneJahr(int tag, const char* monat, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) return -1;
    int sekunden = sekunde + 60 * minute + 3600 * stunde + 86400 * (tag - 1) + 86400 * 31 * (mon - 1);
    return sekunden;
}


/* --- Such/Filter-Funktionen --- */

// Liest einen Suchbegriff ein, prüft die Eingabe, durchsucht die Logdatei und gibt die Treffer aus.
int eigenerSuchbegriff() {
    int versuche = 0;
    const int maxVersuche = 3;

    do {
        printf("\nBitte geben Sie einen beliebigen Suchbegriff ein:\n");
        fgets(suchbegriff, sizeof(suchbegriff), stdin);
        suchbegriff[strcspn(suchbegriff, "\n")] = '\0';
        exitEingabe(suchbegriff);

        if (strlen(suchbegriff) == 0) {
            printf(YELLOW "\nUngültige Eingabe. Bitte einen Suchbegriff eingeben.\n" RESET);
            versuche++;
            if (versuche < maxVersuche) {
                printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        }
        else {
            break; // gültige Eingabe → Schleife verlassen
        }
    } while (1);

    // Suchbegriff in Kleinbuchstaben umwandeln (für case-insensitive Vergleich)
    for (int i = 0; suchbegriff[i]; i++) {
        suchbegriff[i] = (char)tolower((unsigned char)suchbegriff[i]);
    }

    printf("\nDateiname: %s\n", dateiname);
    printf("\nSuchbegriff: %s\n", suchbegriff);
    speichersuche("Suchergebnisse.txt");

    dateiOeffnen();
    treffer = 0;
    zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;

        // Zeile in Kleinbuchstaben kopieren
        char zeile_klein[1024];
        strcpy(zeile_klein, zeile);
        for (int i = 0; zeile_klein[i]; i++) {
            zeile_klein[i] = (char)tolower((unsigned char)zeile_klein[i]);
        }

        // Case-insensitive Teilstring-Suche
        if (strstr(zeile_klein, suchbegriff)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs für '%s' gefunden.\n" RESET, suchbegriff);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
    }

    fclose(datei);
    auswahlnachSuche(4);
    return 0;
}


// Ermöglicht die Betrachtung von Logs ab/bis zu einem Zeitpunkt oder zwischen zwei Zeitpunkten.
int zeitraum() {
    int startzeit;

    printf("\nWie wollen Sie die Logs betrachten?\n");
    printf("\n1: Ab der ersten eingegebenen Zeit.");
    printf("\n2: Bis zur ersten eingegebenen Zeit.");
    printf("\n3: Zeitraum zwischen der ersten und zweiten Zeit.");
    printf("\n4: Zurück ins Hauptmenü");
    printf("\n5: Programm beenden.");

    int zeitauswahl = begrenzungversuche(1, 5, 3);

    switch (zeitauswahl) {
    case 1: {
        treffer = 0;
        printf("\nWählen Sie bitte die erste Zeit aus:\n");
        tagDefinition();
        monatDefinition();
        jahrDefinition();
        uhrzeitDefinition();

        if (jahr == -1) {
            startzeit = zeitZuSekundenOhneJahr(tag, monat, stunde, minute, sekunde);
            printf("\nErste Zeit: %d. %s um %02d:%02d:%02d Uhr\n\n", tag, monat, stunde, minute, sekunde);
        }
        else {
            startzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);
            printf("\nErste Zeit: %d. %s %d um %02d:%02d:%02d Uhr\n\n", tag, monat, jahr, stunde, minute, sekunde);
        }
        speichersuche("Suchergebnisse.txt");
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            char stern;
            int lTag = 0, lJahr = -1, lStunde = 0, lMinute = 0, lSekunde = 0;
            char lMonat[4];

            if (sscanf(zeile, "*%3s %d %d:%d:%d.%*d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }

            else if (sscanf(zeile, "*%3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }

            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }


            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }
        }
        fclose(datei);

        if (treffer == 0) {
            printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs gefunden.\n" RESET);
        }
        else {
            printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
        }
        break;
    }
    case 2: {
        treffer = 0;
        printf("\nWählen Sie bitte die erste Zeit aus:\n");
        tagDefinition();
        monatDefinition();
        jahrDefinition();
        uhrzeitDefinition();

        if (jahr == -1) {
            startzeit = zeitZuSekundenOhneJahr(tag, monat, stunde, minute, sekunde);
            printf("\nErste Zeit: %d. %s um %02d:%02d:%02d Uhr\n\n\n", tag, monat, stunde, minute, sekunde);
        }
        else {
            startzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);
            printf("\nErste Zeit: %d. %s %d um %02d:%02d:%02d Uhr\n\n\n", tag, monat, jahr, stunde, minute, sekunde);
        }
        speichersuche("Suchergebnisse.txt");
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            int lTag = 0, lJahr = -1, lStunde = 0, lMinute = 0, lSekunde = 0;
            char lMonat[4];

            if (jahr == -1 && sscanf(zeile, "*%3s %d %d:%d:%d.%*d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }

            else if (jahr != -1 && sscanf(zeile, "*%3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }

            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }

            else if (jahr == -1 && sscanf(zeile, "*%3s %d %d:%d:%d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }

            else if (jahr != -1 && sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }
        }
        fclose(datei);

        if (treffer == 0) {
            printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs gefunden.\n" RESET);
        }
        else {
            printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
        }
        break;
    }
    case 3: {
        treffer = 0;
        int endTag, endJahr, endStunde, endMinute, endSekunde;
        char endMonat[4];
        int endzeit;

        printf("\nBitte wählen Sie die erste Zeit aus:\n");
        tagDefinition();
        monatDefinition();
        jahrDefinition();
        uhrzeitDefinition();

        if (jahr == -1) {
            startzeit = zeitZuSekundenOhneJahr(tag, monat, stunde, minute, sekunde);
            printf("\nErste Zeit: %d. %s um %02d:%02d:%02d Uhr\n\n\n", tag, monat, stunde, minute, sekunde);
        }
        else {
            startzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);
            printf("\nErste Zeit: %d. %s %d um %02d:%02d:%02d Uhr\n\n\n", tag, monat, jahr, stunde, minute, sekunde);
        }

        // Eingabe zweite Zeit
        printf("\nBitte wählen Sie die zweite Zeit aus:\n");
        tagDefinition();
        monatDefinition();
        jahrDefinition();
        uhrzeitDefinition();

        if (jahr == -1) {
            endzeit = zeitZuSekundenOhneJahr(tag, monat, stunde, minute, sekunde);
        }
        else {
            endzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);
        }

        if (endzeit < startzeit) {
            printf(YELLOW "\nDie zweite Zeit muss nach der ersten Zeit liegen.\n" RESET);
            break;
        }
        // while (getchar() != '\n');
        speichersuche("Suchergebnisse.txt");
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            int lTag = 0, lJahr = -1, lStunde = 0, lMinute = 0, lSekunde = 0;
            char lMonat[4];

            if (sscanf(zeile, "*%3s %d %d:%d:%d.%*d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit && logZeit <= endzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            else if (sscanf(zeile, "*%3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit && logZeit <= endzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit && logZeit <= endzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit && logZeit <= endzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
        }
        fclose(datei);

        // Nach der Ausgabe:
        if (treffer == 0) {
            printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs gefunden.\n" RESET);
        }
        else {
            printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
        }
        break;
    }
    case 4:
        hauptmenue();
        break;
    case 5:
        printf(RED "\nProgramm wird beendet\n" RESET);
        exit(0);
        break;
    default:
        printf(RED "\nUngültige Auswahl.\n" RESET);
        break;
    }

    auswahlnachSuche(2);
    return 0;
}


// Liest eine IPv4-Adresse ein, prüft ihre Gültigkeit und durchsucht das Logfile nach Treffern.
int ipSuche() {

    int maxVersuche = 3;  // Maximal erlaubte Fehlversuche bei der IP-Eingabe
    int versuche = 0;

    // Eingabeschleife für die IP-Adresse
    while (versuche < maxVersuche) {
        printf("\nBitte geben Sie eine IP-Adresse ein (Format: XXX.XXX.XXX.XXX): ");
        fgets(suchbegriff, sizeof(suchbegriff), stdin);
        suchbegriff[strcspn(suchbegriff, "\n")] = '\0';
        exitEingabe(suchbegriff);
        // fgets(suchbegriff, sizeof(suchbegriff), stdin);
        // suchbegriff[strcspn(suchbegriff, "\n")] = '\0';

        // Prüfe, ob die Eingabe leer ist
        if (strlen(suchbegriff) == 0) {
            versuche++;
            printf(YELLOW "\nKeine IP-Adresse eingegeben." RESET);
        }
        // Prüfe das Format der IP-Adresse
        else if (!istGueltigeIPv4(suchbegriff)) {
            versuche++;
            printf(YELLOW "\nUngültiges IP-Adressformat." RESET);

        }
        else {
            break;
        }

        // Gib verbleibende Versuche aus oder beende bei zu vielen Fehlern
        if (versuche < maxVersuche) {
            printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, maxVersuche - versuche);
        }
        else {
            printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            exit(1);
        }
    }

    // Gültige IP-Adresse liegt vor – Suche starten
    printf("\nSuche nach IP-Adresse: %s\n", suchbegriff);
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();

    treffer = 0;
    zeilennummer = 0;

    // Durchsuche jede Zeile der Datei nach der eingegebenen IP-Adresse
    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        if (strstr(zeile, suchbegriff)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }

    fclose(datei); // Datei schließen
    
    // Zusammenfassung ausgeben
    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs mit IP-Adresse '%s' gefunden.\n" RESET, suchbegriff);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
    }

    auswahlnachSuche(1);

    return 0;
}


// Durchsucht das Logfile nach IPv4-Adressen und filtert private oder öffentliche Adressen.
void ipFilterSucheEinfach(int privat) {
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();

    zeilennummer = 0;
    treffer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;

        char* ptr = zeile;
        while ((ptr = strstr(ptr, ".")) != NULL) {
            // Rückwärtslauf zum Anfang der IP
            char* start = ptr;
            while (start > zeile && (isdigit(*(start - 1)) || *(start - 1) == '.')) start--;

            char ip[32] = { 0 };
            sscanf(start, "%31s", ip);

            // Kürzen bei nachfolgendem Text
            for (int i = 0; ip[i]; i++) {
                if (!isdigit(ip[i]) && ip[i] != '.') {
                    ip[i] = '\0';
                    break;
                }
            }

            if (istGueltigeIPv4(ip)) {
                int isPriv = istPrivateIP(ip);
                if ((privat && isPriv) || (!privat && !isPriv)) {
                    printf("Zeile %d: %s", zeilennummer, zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
                    }
                    treffer++;
                    break;
                }
            }
            ptr++;
        }
    }
    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs mit %s IP-Adressen gefunden.\n" RESET, privat ? "privaten" : "öffentlichen");
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
    }

    auswahlnachSuche(privat ? 9 : 10);
}


// Liest einen Facility-Begriff ein, durchsucht das Logfile und gibt passende Zeilen aus.
void eigeneFacilitySuche() {
    char eingabe[64];
    printf("\nBitte geben Sie einen Facility-Begriff ein (z. B. STP, LINK, DHCP):\n");
    fgets(eingabe, sizeof(eingabe), stdin);
    eingabe[strcspn(eingabe, "\n")] = '\0';
    exitEingabe(eingabe);

    if (strlen(eingabe) == 0) {
        printf(YELLOW "Kein Begriff eingegeben. Zurück...\n" RESET);
        return;
    }

    // Eingabe klein machen
    for (int i = 0; eingabe[i]; i++) {
        eingabe[i] = tolower((unsigned char)eingabe[i]);
    }

    // Muster erstellen, z. B. "%STP-"
    char muster[70];
    snprintf(muster, sizeof(muster), "%%%s-", eingabe);
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    zeilennummer = 0;
    treffer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;

        // Zeile in Kleinbuchstaben kopieren
        char zeile_klein[1024];
        strcpy(zeile_klein, zeile);
        for (int i = 0; zeile_klein[i]; i++) {
            zeile_klein[i] = (char)tolower((unsigned char)zeile_klein[i]);
        }

        // Case-insensitive Teilstring-Suche
        if (strstr(zeile_klein, muster)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs zur Facility '%s' gefunden.\n" RESET, eingabe);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs Facility '%s' gefunden.\n", treffer, eingabe);
    }

    auswahlnachSuche(5);
}

#define MAX_FACILITIES 1000

// Extrahiert alle Facilities aus dem Logfile, sortiert sie alphabetisch, fragt eine Facility ab und zeigt die Treffer.
void facilitySuche() {
    char* facilities[MAX_FACILITIES];
    int anzahlFacilities = 0;

    dateiOeffnen();
    zeilennummer = 0;

    for (int i = 0; i < MAX_FACILITIES; i++) {
        facilities[i] = NULL;
    }

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;

        char* start = strchr(zeile, '%');
        if (start) {
            start++; // nach dem '%'

            char facility[64];
            int i = 0;
            while (*start && *start != '-' && i < 63) {
                facility[i++] = *start++;
            }
            facility[i] = '\0';

            if (strlen(facility) == 0) continue;

            int vorhanden = 0;
            for (int j = 0; j < anzahlFacilities; j++) {
                if (strcmp(facilities[j], facility) == 0) {
                    vorhanden = 1;
                    break;
                }
            }

            if (!vorhanden && anzahlFacilities < MAX_FACILITIES) {
                facilities[anzahlFacilities] = malloc(strlen(facility) + 1);
                strcpy(facilities[anzahlFacilities], facility);
                anzahlFacilities++;
            }
        }
    }

    fclose(datei);

    if (anzahlFacilities == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Facilities gefunden.\n" RESET);
        return;
    }

    // Sortieren alphabetisch
    qsort(facilities, anzahlFacilities, sizeof(char*), compareStrings);

    printf("\nGefundene Facilities:\n\n");
    for (int i = 0; i < anzahlFacilities; i++) {
        if (i == anzahlFacilities - 1) {
            printf("%2d: %s", i + 1, facilities[i]);   // letzte Zeile OHNE \n
        }
        else {
            printf("%2d: %s\n", i + 1, facilities[i]);  // alle anderen MIT \n
        }
    }

    int auswahl = begrenzungversuche(0, anzahlFacilities, 3);

    const char* muster = facilities[auswahl - 1];
    char suchmuster[70];
    snprintf(suchmuster, sizeof(suchmuster), "%%%s-", muster); // z. B. %LINK-
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    treffer = 0;
    zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        if (strstr(zeile, suchmuster)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs für Facility '%s' gefunden.\n" RESET, muster);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs für Facility '%s' gefunden.\n", treffer, muster);
    }

    // Speicher freigeben
    for (int i = 0; i < anzahlFacilities; i++) {
        free(facilities[i]);
    }

    auswahlnachSuche(6);
}


// Liest einen Usernamen ein und durchsucht das Logfile nach Treffern. 
void eigeneUserSuche() {
    char eingabe[64];
    int versuche = 0;
    const int maxVersuche = 3;

    do {
        printf("\nBitte geben Sie einen Usernamen ein (z. B. admin, datacadmin):\n");
        fgets(eingabe, sizeof(eingabe), stdin);
        eingabe[strcspn(eingabe, "\n")] = '\0'; // Zeilenumbruch entfernen

        exitEingabe(eingabe);

        if (strlen(eingabe) == 0) {
            printf(YELLOW "\nUngültige Eingabe. Bitte geben Sie einen Usernamen ein.\n" RESET);
            versuche++;
            if (versuche < maxVersuche) {
                printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        }
        else {
            break; // gültige Eingabe → Schleife verlassen
        }
    } while (1);

    for (int i = 0; eingabe[i]; i++)
        eingabe[i] = (char)tolower((unsigned char)eingabe[i]);

    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    zeilennummer = 0;
    treffer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        char zeile_klein[1024];
        strcpy(zeile_klein, zeile);
        for (int i = 0; zeile_klein[i]; i++)
            zeile_klein[i] = (char)tolower((unsigned char)zeile_klein[i]);

        if (strstr(zeile_klein, eingabe)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            treffer++;
        }
    }


    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs zum User '%s' gefunden.\n" RESET, eingabe);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs für User '%s' gefunden.\n", treffer, eingabe);
    }
    auswahlnachSuche(3);
}

#define MAX_USER 100

// Extrahiert alle Usernamen aus dem Logfile, sortiert sie alphabetisch, lässt eine Auswahl treffen und gibt die passenden Logzeilen aus.
void userSuche() {
    char* userNamen[MAX_USER];
    int anzahlUser = 0;

    dateiOeffnen();
    zeilennummer = 0;

    for (int i = 0; i < MAX_USER; i++) {
        userNamen[i] = NULL;
    }

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        char* userStart = strstr(zeile, "user ");
        if (!userStart) userStart = strstr(zeile, "User ");

        if (userStart) {
            userStart += 5; // springe über "user "
            char name[64];
            int i = 0;
            while (*userStart && !isspace(*userStart) && i < 63) {
                name[i++] = *userStart++;
            }
            name[i] = '\0';

            if (strlen(name) == 0) continue;

            // schon vorhanden?
            int vorhanden = 0;
            for (int j = 0; j < anzahlUser; j++) {
                if (strcmp(userNamen[j], name) == 0) {
                    vorhanden = 1;
                    break;
                }
            }

            if (!vorhanden && anzahlUser < MAX_USER) {
                userNamen[anzahlUser] = malloc(strlen(name) + 1);
                strcpy(userNamen[anzahlUser], name);
                anzahlUser++;
            }
        }
    }

    fclose(datei);

    if (anzahlUser == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine User gefunden.\n" RESET);
        auswahlnachSuche(7);
        return;
    }

    // sortieren
    qsort(userNamen, anzahlUser, sizeof(char*), compareStrings);

    printf("\nGefundene User:\n");
    for (int i = 0; i < anzahlUser; i++) {
        printf("%d: %s", i, userNamen[i]);   // 0-basiert ausgeben
    }

    // KEINE "Zurück"-Zeile mehr!
    // printf("%2d: Zurück\n", anzahlUser);

    int auswahl = begrenzungversuche(0, anzahlUser - 1, 3);

    // Direkter Zugriff (0-basiert), NICHT auswahl-1
    const char* muster = userNamen[auswahl];

    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    treffer = 0;
    zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        if (strstr(zeile, muster)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }
    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs für Benutzer '%s' gefunden.\n" RESET, muster);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs für Benutzer '%s' gefunden.\n", treffer, muster);
    }

    auswahlnachSuche(7);

    // Speicher freigeben
    for (int i = 0; i < anzahlUser; i++) {
        free(userNamen[i]);
    }
}


// Liest ein Mnemonic ein und durchsucht das Logfile nach passenden Einträgen.
void eigeneMnemonicSuche() {
    char eingabe[64];
    int versuche = 0;
    const int maxVersuche = 3;

    do {
        printf("\nBitte geben Sie ein Mnemonic ein (z. B. CONFIG_I, UPDOWN, ADJCHANGE):\n");
        fgets(eingabe, sizeof(eingabe), stdin);
        eingabe[strcspn(eingabe, "\n")] = '\0';
        exitEingabe(eingabe);

        if (strlen(eingabe) == 0) {
            printf(YELLOW "\nUngültige Eingabe. Bitte geben Sie ein Mnemonic ein.\n" RESET);
            versuche++;
            if (versuche < maxVersuche) {
                printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        }
        else {
            break; // gültige Eingabe → Schleife verlassen
        }
    } while (1);

    for (int i = 0; eingabe[i]; i++)
        eingabe[i] = (char)tolower((unsigned char)eingabe[i]);

    // Muster erstellen, z. B. "-CONFIG_I:"
    char muster[70];
    snprintf(muster, sizeof(muster), "-%s:", eingabe);
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    zeilennummer = 0;
    treffer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        char zeile_klein[1024];
        strcpy(zeile_klein, zeile);
        for (int i = 0; zeile_klein[i]; i++)
            zeile_klein[i] = (char)tolower((unsigned char)zeile_klein[i]);

        if (strstr(zeile_klein, muster)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            treffer++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs zum Mnemonic '%s' gefunden.\n" RESET, eingabe);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs für Mnemonic '%s' gefunden.\n", treffer, eingabe);
    }

    auswahlnachSuche(12);
}


#define MAX_MNEMONICS 1000

// Liest alle Mnemonics aus dem Logfile, sortiert sie alphabetisch, lässt eine Auswahl treffen und gibt die passenden Logzeilen aus.
void mnemonicSuche() {
    char* mnemonics[MAX_MNEMONICS];
    int anzahlMnemonics = 0;

    dateiOeffnen();
    zeilennummer = 0;

    for (int i = 0; i < MAX_MNEMONICS; i++) {
        mnemonics[i] = NULL;
    }

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;

        char* prozent = strchr(zeile, '%');
        if (prozent) {
            char* minus = strrchr(prozent, '-');
            char* doppelpunkt = strchr(prozent, ':');
            if (!minus || !doppelpunkt || minus >= doppelpunkt) continue;

            char mnemonic[64];
            int len = doppelpunkt - minus - 1;
            if (len <= 0 || len >= sizeof(mnemonic)) continue;

            strncpy(mnemonic, minus + 1, len);
            mnemonic[len] = '\0';

            int vorhanden = 0;
            for (int j = 0; j < anzahlMnemonics; j++) {
                if (strcmp(mnemonics[j], mnemonic) == 0) {
                    vorhanden = 1;
                    break;
                }
            }

            if (!vorhanden && anzahlMnemonics < MAX_MNEMONICS) {
                mnemonics[anzahlMnemonics] = malloc(strlen(mnemonic) + 1);
                strcpy(mnemonics[anzahlMnemonics], mnemonic);
                anzahlMnemonics++;
            }
        }
    }

    fclose(datei);

    if (anzahlMnemonics == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs zu Mnemonics gefunden.\n" RESET);
        return;
    }

    qsort(mnemonics, anzahlMnemonics, sizeof(char*), compareStrings);

    printf("\nGefundene Mnemonics:\n\n");
    for (int i = 0; i < anzahlMnemonics; i++) {
        printf("%2d: %s\n", i + 1, mnemonics[i]);
    }

    printf("\nBitte wählen Sie ein Mnemonic aus: ");
    int auswahl = begrenzungversuche(1, anzahlMnemonics, 3);

    const char* muster = mnemonics[auswahl - 1];
    char suchmuster[70];
    snprintf(suchmuster, sizeof(suchmuster), "-%s:", muster);
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    treffer = 0;
    zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        if (strstr(zeile, suchmuster)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nIn der analysierten Log-Datei wurden keine Logs für Mnemonic '%s' gefunden.\n" RESET, muster);
    }
    else {
        printf("\nIn der analysierten Log-Datei wurden %d Logs für Mnemonic '%s' gefunden.\n", treffer, muster);
    }

    for (int i = 0; i < anzahlMnemonics; i++) {
        free(mnemonics[i]);
    }

    auswahlnachSuche(11);
}


//  Bietet eine Auswahl an Syslog-Severity-Leveln und filtert die Logdatei nach dem gewählten Level
int severityLevel() {
    const char* sevLevellNamen[] = {
        "EMERGENCIES", "ALERTS", "CRITICALS", "ERRORS", "WARNINGS",
        "NOTIFICATIONS", "INFORMATIONAL", "DEBUGGING"
    };
    const char* sevLevelBeschreibung[] = {
        "Ein System ist unbenutzbar",
        "Sofortiges Handeln erforderlich",
        "Kritische Zustände",
        "Errorwarnungen",
        "Warnhinweise",
        "Normale, aber signifikante Zustände",
        "Informierende Nachrichten/Logs",
        "Debugging Nachrichten/Logs"
    };

    printf("\nBitte wählen Sie ein Severity Level aus.\n");
    for (int i = 0; i < 8; ++i)
        printf("\n%d: %s", i, sevLevellNamen[i]);
    printf("\n8: Zurück in das Hauptmenü");
    printf("\n9: Programm beenden");

    int sevLevelAuswahl = begrenzungversuche(0, 9, 3);

    if (sevLevelAuswahl == 8) {
        hauptmenue();
    }

    if (sevLevelAuswahl == 9) {
        printf(RED "\nProgramm wird beendet.\n" RESET);
        exit(0);
    }

    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    int treffer = 0, zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        char* prozentZeichen = strchr(zeile, '%');
        if (prozentZeichen) {
            char* minusZeichen = strchr(prozentZeichen, '-');
            if (minusZeichen && isdigit(*(minusZeichen + 1)) && !isdigit(*(minusZeichen + 2))) {
                int sev = *(minusZeichen + 1) - '0';

                if (sev == sevLevelAuswahl) {
                    printf("%s", zeile);
                    if (outputDatei) {
                        fprintf(outputDatei, "%s", zeile);
                    }
                    treffer++;
                }
            }
        }
    }
    fclose(datei);

    if (treffer == 0) {
        if (sevLevelAuswahl == 8)
            printf(YELLOW "In der analysierten Log-Datei wurden keine Logs gefunden.\n" RESET);
        else
            printf(YELLOW "In der analysierten Log-Datei wurden keine Logs mit Severity Level %d gefunden.\n" RESET, sevLevelAuswahl);
    }
    else {
        if (sevLevelAuswahl == 8)
            printf("\nIn der analysierten Log-Datei wurden %d Logs gefunden.\n", treffer);
        else
            printf("\nIn der analysierten Log-Datei wurden %d Logs für Severity Level '%d' gefunden.\n", treffer, sevLevelAuswahl);
    }

    auswahlnachSuche(8);
    return 0;
}

/* ===== Navigationsfunktionen ===== */

//  Bietet nach einer Suche die Optionen: wiederholen, ins Hauptmenü zurückkehren oder Programm beenden.
void auswahlnachSuche(int funktionID) {
    char wahl;

    // Falls noch offen: Speicherdatei schließen
    if (outputDatei) {
        fclose(outputDatei);
        outputDatei = NULL;
        printf("\nDie Ergebnisse wurden in der 'Suchergebnisse.txt' Datei gespeichert.\n");
    }

    // Auswahl anzeigen
    printf("\n\nWas möchten Sie tun?\n");
    printf("\n1: Suche wiederholen");
    printf("\n2: Zurück ins Hauptmenü");
    printf("\n3: Programm beenden");
    wahl = begrenzungversuche(1, 3, 3);  // 1=Suche wiederholen, 2=Menü, 3=Beenden

    switch (wahl) {
    case 1:
        switch (funktionID) {
        case 1: ipSuche(); break;
        case 2: zeitraum(); break;
        case 3: eigeneUserSuche(); break;
        case 4: eigenerSuchbegriff(); break;
        case 5: eigeneFacilitySuche(); break;
        case 6: facilitySuche(); break;
        case 7: userSuche(); break;
        case 8: severityLevel(); break;
        case 9: ipFilterSucheEinfach(1); break;
        case 10: ipFilterSucheEinfach(0); break;
        case 11: mnemonicSuche(); break;
        case 12: eigeneMnemonicSuche(); break;
        default:
            printf("Unbekannte Funktion.\n");
            hauptmenue();
        }
        break;
    case 2:
        hauptmenue();
        break;
    case 3:
        printf(RED "\nProgramm wird beendet.\n" RESET);
        exit(0);
    default:
        printf("Ungültige Eingabe. Zurück ins Hauptmenü.\n");
        hauptmenue();
    }
}


// Liest einen neuen Logdateinamen ein, prüft ihn und übernimmt ihn als aktive Datei.
int neueDateiAuswaehlen() {
    char neuerDateiname[256];
    FILE* neueDatei;

    while (1) {
        printf("\n\nBitte geben Sie den Pfad zur neuen Datei ein (.log-Datei:\n");

        fgets(neuerDateiname, sizeof(neuerDateiname), stdin);
        neuerDateiname[strcspn(neuerDateiname, "\n")] = '\0'; // Zeilenumbruch entfernen

        exitEingabe(neuerDateiname);

        if (!log_dateiendung(neuerDateiname)) {
            printf(YELLOW "Die Datei muss die Endung .log haben.\n" RESET);
            continue;
        }

        neueDatei = fopen(neuerDateiname, "r");
        if (!neueDatei) {
            printf(YELLOW "Datei konnte nicht geöffnet werden. Bitte versuchen Sie es erneut.\n" RESET);
            continue;
        }
        fclose(neueDatei);

        // Dateiname übernehmen
        strcpy(dateiname, neuerDateiname);
        printf("\nNeue Datei: %s\n", dateiname);
        break;
    }
    return 0;
}

// Stellt das Hauptmenü dar, prüft die Eingabe und ruft die gewählte Suchfunktion oder Programmoption auf.
void hauptmenue() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printf("\n#####################################################");
    printf("\n      Auswertungsprogramm für CISCO-Logdateien");
    printf("\n#####################################################");
    printf("\n\nHinweise: Das Programm kann jederzeit mit der Eingabe von 'exit' beendet werden.\n          Alle Eingaben müssen mit der Enter-Taste bestätigt werden.");
    printf("\n\nAktuelle Logdatei: %s", dateiname);

    printf("\n\nBitte wählen Sie einen Suchbegriff aus:\n");
    printf("\n0: Eigene Eingabe");
    printf("\n1: Zeitraum");
    printf("\n2: IP-Adresse");
    printf("\n3: Facilities");
    printf("\n4: User");
    printf("\n5: Mnemonic");
    printf("\n6: Severity Level");
    printf("\n7: Neue Datei auswählen");
    printf("\n8: Programm beenden");

    begriff = begrenzungversuche(0, 8, 3);

    switch (begriff) {
    case 0:
        eigenerSuchbegriff();
        break;
    case 1:
        zeitraum();
        break;
    case 2: {
        printf("\nBitte wählen Sie die Art der IP-Suche:\n\n");
        printf("1: Manuelle Eingabe einer IP-Adresse\n");
        printf("2: Nur private IP-Adressen anzeigen\n");
        printf("3: Nur öffentliche IP-Adressen anzeigen\n");
        printf("4: Zurück in das Hauptmenü\n");
        printf("5: Programm beenden");

        int wahl = begrenzungversuche(1, 5, 3);


        switch (wahl) {
        case 1: ipSuche(); break;
        case 2: ipFilterSucheEinfach(1); break;
        case 3: ipFilterSucheEinfach(0); break;
        case 4:
            hauptmenue();
            break;
        case 5:
            printf(RED "Programm wird beendet.\n" RESET);
            exit(0);
        default:
            printf(YELLOW "Ungültige Auswahl.\n" RESET);
            break;
        }
        break;
    }
    case 3: {
        printf("\nBitte wählen Sie die Art der Facility-Suche:\n\n");
        printf("1: Eigene Suche nach Facility-Begriff\n");
        printf("2: Alle vorhandenen Facilities anzeigen und auswählen\n");
        printf("3: Zurück in das Hauptmenü\n");
        printf("4: Programm beenden");

        int wahl = begrenzungversuche(1, 4, 3);


        switch (wahl) {
        case 1: eigeneFacilitySuche(); break;
        case 2: facilitySuche(); break;
        case 3:
            hauptmenue();
            break;
        case 4:
            printf(RED "Programm wird beendet.\n" RESET);
            exit(0);
        default: printf(YELLOW "Ungültige Auswahl.\n" RESET); break;
        }
        break;
    }
    case 4: {
        printf("\n Bitte wählen Sie die Art der User-Suche:\n\n");
        printf("1: Eigene Suche nach User\n");
        printf("2: Alle User anzeigen und auswählen\n");
        printf("3: Zurück in das Hauptmenü\n");
        printf("4: Programm beenden");

        int wahl = begrenzungversuche(1, 4, 3);


        switch (wahl) {
        case 1: eigeneUserSuche(); break;
        case 2: userSuche(); break;
        case 3:
            hauptmenue();
            break;
        case 4:
            printf(RED "Programm wird beendet.\n" RESET);
            exit(0);
        default: printf(YELLOW "Ungültige Auswahl.\n" RESET); break;
        }
        break;
    }
    case 5: {
        printf("\nBitte wählen Sie die Art der Mnemonic-Suche:\n\n");
        printf("1: Eigene Suche nach Mnemonic\n");
        printf("2: Alle vorhandenen Mnemonics anzeigen und auswählen\n");
        printf("3: Zurück in das Hauptmenü\n");
        printf("4: Programm beenden");

        int wahl = begrenzungversuche(1, 4, 3);

        switch (wahl) {
        case 1: eigeneMnemonicSuche(); break;
        case 2: mnemonicSuche(); break;
        case 3: hauptmenue(); break;
        case 4:
            printf(RED "Programm wird beendet.\n" RESET);
            exit(0);
        default:
            printf(YELLOW "Ungültige Auswahl.\n" RESET);
            break;
        }
        break;
    }
    case 6:
        severityLevel();
        break;
    case 7:
        neueDateiAuswaehlen();
        hauptmenue();
        break;
    case 8:
        printf(RED "\nProgramm wird beendet.\n" RESET);
        exit(0);

    default:
        printf(YELLOW "Ungültige Auswahl.\n" RESET);
        break;
    }
}

/* ===== Programmstart ===== */

// Einstiegspunkt des Programms; prüft die Logdatei und ruft das Hauptmenü auf.
int main() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    printf("\n#####################################################");
    printf("\n      Auswertungsprogramm für CISCO-Logdateien");
    printf("\n#####################################################");
    printf("\n\nHinweise: Das Programm kann jederzeit mit der Eingabe von 'exit' beendet werden.\n          Alle Eingaben müssen mit der Enter-Taste bestätigt werden.");
    
    int maxVersuche = 3;
    int versuch = 0;
    int gueltig = 0;

    while (versuch < maxVersuche) {
        printf("\n\nBitte geben Sie den Dateipfad ein (.log-Datei):\n");
        fgets(dateiname, sizeof(dateiname), stdin);
        dateiname[strcspn(dateiname, "\n")] = '\0'; // Zeilenumbruch entfernen

        // Exit-Eingabe prüfen
        if (strcmp(dateiname, "exit") == 0) {
            printf(RED "\nProgramm wird beendet.\n" RESET);
            return 0;
        }

        if (!log_dateiendung(dateiname)) {
            versuch++;
            if (versuch < maxVersuche) {
                printf(RED "\nDie Datei muss die Endung .log haben." RESET);
                printf(YELLOW "\nNoch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuch);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                return 1;
            }
        }
        else {
            // Teste, ob Datei existiert und lesbar ist
            FILE* test = fopen(dateiname, "r");
            if (!test) {
                versuch++;
                if (versuch < maxVersuche) {
                    printf(RED "\nDatei konnte nicht geöffnet werden." RESET);
                    printf(YELLOW "\nNoch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuch);
                }
                else {
                    printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                    return 1;
                }
            }
            else {
                fclose(test);
                gueltig = 1;
                break;
            }
        }
    }

    if (!gueltig) {
        return 1;
    }

    hauptmenue();
    return 0;
}