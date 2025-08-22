// Programm Beleg Programmierung I

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>             // Löschen des Terminals
#include <ctype.h>              // Buchstaben-Bibliothek
// #include <windows.h>
#define RED     "\033[31;91m"
#define YELLOW  "\033[33;93m"     
#define RESET   "\033[0m"

#ifdef _WIN32                   
#define strcasecmp _stricmp
#endif

/* ##############################
     VARIABLENDEKLARATION
###############################*/

FILE* datei;
int stunde;
int minute;
int sekunde;
int zeilennummer = 0;
int treffer = 0;
int begriff = 1;
int zeitauswahl = 1;
int tag, versuch = 0;
int jahr = 0;
int minJahr = 9999, maxJahr = 0;
char suchbegriff[256];
char zeile[1024];
char monat[4];
char uhrzeit[9];
char sevLevel[8];
char exitEingaben[6];
char dateiname[256];
// char dateiname[256] = "/Volumes/HSMW_MacOS/Programmierung/2._Semester/Programmierung_I/Cisco_SysLog_Router_Dateien_Auswertung_Belegarbeit/logs/syslog2.log";
// char dateiname[256] = "C:\\Users\\katha\\OneDrive\\Philipp\\HSMW Cybercrime, IT-Forensik\\2. Semester\\Programmierung I\\06 Beleg\\CICSO-Logfiles\\syslog_generic.log";
FILE* outputDatei = NULL;

void hauptmenue(void);
void auswahlnachSuche(int funktionID);
int speichersuche(const char* zielDateiname);
int ipSuche(void);
int zeitraum(void);
int eigenerSuchbegriff(void);
void eigeneFacilitySuche(void);
void facilitySuche(void);
void eigeneUserSuche(void);
void userSuche(void);
int severityLevel(void);
int neueDateiAuswaehlen(void);
void ipFilterSucheEinfach(int privat);
void eigeneMnemonicSuche(void);
void mnemonicSuche(void);

int monatZuZahl(const char* monat);
int zeitZuSekunden(int tag, const char* monat, int jahr, int stunde, int minute, int sekunde);
int zeitZuSekundenOhneJahr(int tag, const char* monat, int stunde, int minute, int sekunde);


/* ##############################
      METHODENDEKLARATION
###############################*/

// Funktion: Programmende nach Exit-Eingabe
int exitEingabe(char* exitEingaben) {
    if (strcmp(exitEingaben, "exit") == 0) {
        printf(RED "\nProgramm wird beendet.\n" RESET);
        exit(0);
    }
    return 0;
}

//  Funktion: Prüfung auf Dateinnamenende (".log")
int log_dateiendung(const char* log_dateiname) {
    size_t laenge = strlen(log_dateiname);
    return (laenge >= 4 && strcmp(log_dateiname + laenge - 4, ".log") == 0);
}

// Funktion: Öffnet die Datei
int dateiOeffnen() {
    datei = fopen(dateiname, "r");
    if (!datei) {
        perror(RED "Fehler beim Öffnen der Datei" RESET);
    }
    return 0;
}

// Funktion: Prüft, ob eine IPv4-Adresse gültig ist (vier Zahlen zwischen 0–255)
int istGueltigeIPv4(const char* ip) {
    int a, b, c, d;

    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        if (a >= 0 && a <= 255 &&
            b >= 0 && b <= 255 &&
            c >= 0 && c <= 255 &&
            d >= 0 && d <= 255) {
            return 1;
        }
    }
    return 0;
}

// Funktion: Prüft, ob eine IP-Adresse im privaten Bereich liegt
int istPrivateIP(const char* ip) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) != 4) return 0;

    // Private IP-Bereiche:
    // 10.0.0.0 – 10.255.255.255
    if (a == 10) return 1;

    // 172.16.0.0 – 172.31.255.255
    if (a == 172 && b >= 16 && b <= 31) return 1;

    // 192.168.0.0 – 192.168.255.255
    if (a == 192 && b == 168) return 1;

    return 0;
}

// Funktion: Suche nach einer IP-Adresse im Logfile
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
            printf(RED "\n\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
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
        printf(YELLOW "\nKeine Einträge mit IP-Adresse '%s' gefunden.\n" RESET, suchbegriff);
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }

    auswahlnachSuche(1);

    return 0;
}

// Funktion: Definition aller gültigen Monate
int alleMonate(const char* monat) {
    const char* gueltigeMonate[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    for (int i = 0; i < 12; i++) {
        if (strcasecmp(monat, gueltigeMonate[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Funktion: Definition der und aller Tage
int tagDefinition() {
    int tagVorhanden = 0;
    int vorhandeneTage[31] = { 0 }; // Index 0 bis 30 → Tag 1 bis 31
    int versuch = 0;
    char tagEingabe[16];

    // Datei scannen und vorhandene Tage erfassen
    dateiOeffnen();
    while (fgets(zeile, sizeof(zeile), datei)) {
        char logMonat[4];
        int logTag, logJahr, dummy;

        if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3 ||
            sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4) {

            if (logTag >= 1 && logTag <= 31) {
                if (!vorhandeneTage[logTag - 1]) {
                    vorhandeneTage[logTag - 1] = 1;
                    tagVorhanden = 1;
                }
            }
        }
    }
    fclose(datei);

    if (tagVorhanden) {
        printf("\nVerfügbare Tage in den Logdaten: ");
        for (int i = 0; i < 31; i++) {
            if (vorhandeneTage[i]) {
                printf("%d ", i + 1);
            }
        }

        do {
            printf("\nTag (DD): \n");
            fgets(tagEingabe, sizeof(tagEingabe), stdin);
            tagEingabe[strcspn(tagEingabe, "\n")] = '\0';
            exitEingabe(tagEingabe);

            // Prüfen, ob nur Ziffern eingegeben wurden
            int gueltig = 1;
            for (int i = 0; tagEingabe[i] != '\0'; i++) {
                if (!isdigit((unsigned char)tagEingabe[i])) {
                    gueltig = 0;
                    break;
                }
            }

            if (!gueltig || strlen(tagEingabe) == 0) {
                printf(YELLOW "\nUngültige Eingabe – gib einen Tag zwischen 1 und 31 ein." RESET);
                versuch++;
            }
            else {
                tag = atoi(tagEingabe);
                if (tag < 1 || tag > 31 || !vorhandeneTage[tag - 1]) {
                    printf(YELLOW "\nDieser Tag ist nicht in den Logdaten enthalten oder ungültig." RESET);
                    versuch++;
                }
                else {
                    break; // gültiger Tag
                }
            }

            if (versuch < 3) {
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "\n\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }

        } while (1);
    }
    else {
        printf("Es ist keine Abfrage für den Tag nötig.\n");
        tag = -1;
    }

    return 0;
}

// Funktion: Definition der Monate
int monatDefinition() {
    int monatVorhanden = 0;
    int versuch = 0;
    char monatEingabe[16];
    char vorhandeneMonate[12][4]; // Maximal 12 gültige Monate
    int anzahlVorhandeneMonate = 0;

    // Datei scannen und vorhandene Monate erfassen
    dateiOeffnen();
    while (fgets(zeile, sizeof(zeile), datei)) {
        char logMonat[4];
        int logTag, logJahr, dummy;

        if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3 ||
            sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4) {

            // Formatierung: erster Buchstabe groß, Rest klein
            logMonat[0] = toupper(logMonat[0]);
            logMonat[1] = tolower(logMonat[1]);
            logMonat[2] = tolower(logMonat[2]);
            logMonat[3] = '\0';

            if (alleMonate(logMonat)) {
                // Prüfen, ob der Monat schon gespeichert wurde
                int bereitsVorhanden = 0;
                for (int i = 0; i < anzahlVorhandeneMonate; i++) {
                    if (strcmp(vorhandeneMonate[i], logMonat) == 0) {
                        bereitsVorhanden = 1;
                        break;
                    }
                }
                if (!bereitsVorhanden && anzahlVorhandeneMonate < 12) {
                    strncpy(vorhandeneMonate[anzahlVorhandeneMonate], logMonat, 4);
                    anzahlVorhandeneMonate++;
                    monatVorhanden = 1;
                }
            }
        }
    }
    fclose(datei);

    if (monatVorhanden) {
        printf("\nVerfügbare Monate in der Datei: ");
        for (int i = 0; i < anzahlVorhandeneMonate; i++) {
            printf("%s ", vorhandeneMonate[i]);
        }
        printf("\n");

        do {
            printf("Monat (MMM): \n");
            fgets(monatEingabe, sizeof(monatEingabe), stdin);
            monatEingabe[strcspn(monatEingabe, "\n")] = '\0';
            exitEingabe(monatEingabe);

            if (strlen(monatEingabe) != 3) {
                printf(YELLOW "\nUngültige Eingabe (z. B. Jan, Feb, Mar...).\n" RESET);
                versuch++;
            }
            else {
                // Formatieren
                monatEingabe[0] = toupper(monatEingabe[0]);
                monatEingabe[1] = tolower(monatEingabe[1]);
                monatEingabe[2] = tolower(monatEingabe[2]);
                monatEingabe[3] = '\0';

                // Überprüfen, ob gültig UND in Datei enthalten
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
                    strncpy(monat, monatEingabe, 4);
                    break;
                }
            }

            if (versuch < 3) {
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        } while (1);
    }
    else {
        printf("\nEs ist keine Abfrage für den Monat nötig.\n");
        monat[0] = '\0'; // leer setzen
    }

    return 0;
}

// Funktion: Definition der Jahre
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
            printf("\nBitte wählen Sie eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl aus (%d).\n", minJahr, maxJahr);
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
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n\n" RESET);
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

// Funktion: Definition der Uhrzeit
int uhrzeitDefinition() {
    int versuch = 0;
    char uhrzeitEingabe[16];

    do {
        printf("\nUhrzeit (HH:MM:SS): \n");
        fgets(uhrzeitEingabe, sizeof(uhrzeitEingabe), stdin);
        uhrzeitEingabe[strcspn(uhrzeitEingabe, "\n")] = '\0';
        exitEingabe(uhrzeitEingabe);
        sscanf(uhrzeitEingabe, "%d:%d:%d", &stunde, &minute, &sekunde);

        if (scanf("%d:%d:%d", &stunde, &minute, &sekunde) != 3) {
            printf(YELLOW "\nUngültiges Format. Bitte HH:MM:SS eingeben.\n" RESET);
            versuch++;
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        else if (stunde < 0 || stunde > 23 || minute < 0 || minute > 59 || sekunde < 0 || sekunde > 59) {
            printf(YELLOW "\nUngültige Uhrzeitformat: %02d:%02d:%02d\n" RESET, stunde, minute, sekunde);
            versuch++;
        }
        else {
            break;
        }
        if (versuch < 3) {
            printf(YELLOW "Noch %d Versuch(e) übrig\n\n" RESET, 3 - versuch);
        }
        else {
            printf(RED "Zu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            exit(1);
        }
    } while (1);
    return 0;
}

// Funktion: Zeitpunktberechnung für Zeitraum mit Jahr
int zeitZuSekunden(int tag, const char* monat, int jahr, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) return -1;
    int sekunden = sekunde + 60 * minute + 3600 * stunde + 86400 * (tag - 1) + 86400 * 31 * (mon - 1) + 86400 * 31 * 12 * jahr;
    return sekunden;
}

// Funktion: Zeitpunktberechnung für Zeitraum ohne Jahr
int zeitZuSekundenOhneJahr(int tag, const char* monat, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) return -1;
    int sekunden = sekunde + 60 * minute + 3600 * stunde + 86400 * (tag - 1) + 86400 * 31 * (mon - 1);
    return sekunden;
}

// Funktion: Zur Zeitstempelberechnung
int monatZuZahl(const char* monat) {
    if (strcmp(monat, "Jan") == 0) return 1;
    if (strcmp(monat, "Feb") == 0) return 2;
    if (strcmp(monat, "Mar") == 0) return 3;
    if (strcmp(monat, "Apr") == 0) return 4;
    if (strcmp(monat, "May") == 0) return 5;
    if (strcmp(monat, "Jun") == 0) return 6;
    if (strcmp(monat, "Jul") == 0) return 7;
    if (strcmp(monat, "Aug") == 0) return 8;
    if (strcmp(monat, "Sep") == 0) return 9;
    if (strcmp(monat, "Oct") == 0) return 10;
    if (strcmp(monat, "Nov") == 0) return 11;
    if (strcmp(monat, "Dec") == 0) return 12;
    return 0;
}

//Funktion Begrenzung Versuche
int begrenzungversuche(int min, int max, int maxVersuche) {
    int eingabe;
    int versuche = 0;
    char buffer[64];

    do {
        printf("\n\nAuswahl (%d-%d):\n", min, max);

        // Ganze Zeile einlesen statt scanf direkt
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf(RED "Fehler beim Lesen der Eingabe.\n" RESET);
            exit(1);
        }

        // Entferne \n am Ende
        buffer[strcspn(buffer, "\n")] = '\0';

        exitEingabe(buffer);

        // Prüfen, ob leer
        if (strlen(buffer) == 0) {
            printf(YELLOW "Keine Eingabe erkannt. Bitte eine Zahl zwischen %d und %d eingeben.\n" RESET, min, max);
            versuche++;
        }
        // Prüfen, ob Zahl gültig
        else if (sscanf(buffer, "%d", &eingabe) != 1 || eingabe < min || eingabe > max) {
            printf(YELLOW "\nUngültige Eingabe. Bitte eine Zahl zwischen %d und %d eingeben.\n" RESET, min, max);
            versuche++;
        }
        else {
            return eingabe; // gültige Zahl → zurückgeben
        }

        if (versuche < maxVersuche) {
            printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
        }
        else {
            printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            exit(1);
        }

    } while (1);
}

//Funktion Ergebnis der Suche speichern
int speichersuche(const char* zielDateiname) {
    char speichern;
    int versuche = 0;
    const int maxVersuche = 3;

    do {
        printf("\nMöchten Sie die Ergebnisse in einer Datei speichern? (j/n):\n");
        speichern = getchar();
        while (getchar() != '\n'); // Eingabepuffer leeren

        if (speichern == 'j' || speichern == 'J') {
            outputDatei = fopen(zielDateiname, "w");
            if (!outputDatei) {
                perror(RED "Fehler beim Öffnen der Datei" RESET);
                return 0;
            }
            printf("\n");
            return 1;
        }
        else if (speichern == 'n' || speichern == 'N') {
            return 0; // Nicht speichern
        }
        else {
            versuche++;
            if (versuche < maxVersuche) {
                printf(YELLOW "\nUngültige Eingabe.\n" RESET);
                printf(YELLOW "Noch %d Versuch(e) übrig.\n" RESET, maxVersuche - versuche);
            }
            else {
                printf(RED "\nZu viele ungültige Eingaben. Das Programm wird beendet.\n" RESET);
                exit(1);
            }
        }
    } while (1);
}

// Funktion: Auswahl nach Suche
void auswahlnachSuche(int funktionID) {
    char wahl;

    // Falls noch offen: Speicherdatei schließen
    if (outputDatei) {
        fclose(outputDatei);
        outputDatei = NULL;
        printf("\nErgebnisse wurden in 'Suchergebnisse.txt' gespeichert.\n");
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
        printf("\nProgramm wird beendet.\n");
        exit(0);
    default:
        printf("Ungültige Eingabe. Zurück ins Hauptmenü.\n");
        hauptmenue();
    }
}

// Funktion 0: eigene Suchbegriffeingabe
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

    printf("\nDateiname: %s\n", dateiname);
    printf("\nSuchbegriff: %s\n", suchbegriff);
    speichersuche("Suchergebnisse.txt");

    dateiOeffnen();
    treffer = 0;
    zeilennummer = 0;

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
    if (treffer == 0) {
        printf(YELLOW "\nKeine Treffer für '%s' gefunden.\n" RESET, suchbegriff);
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }

    fclose(datei);
    auswahlnachSuche(4);

    return 0;
}

// Funktion: 1. Auswahl, Zeitraumberechnung der Logs
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
        while (getchar() != '\n');
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
            printf(YELLOW "\nKeine Logs gefunden.\n" RESET);
        }
        else {
            printf("\nEs wurden %d Logs gefunden.\n", treffer);
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
        while (getchar() != '\n');
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
            printf(YELLOW "\nKeine Logs gefunden.\n" RESET);
        }
        else {
            printf("\nEs wurden %d Logs gefunden.\n", treffer);
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
        printf("\nBitte wählen Sie die zweite Zeit aus:");
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
        while (getchar() != '\n');
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
            printf(YELLOW "\nKeine Logs gefunden.\n" RESET);
        }
        else {
            printf("\nEs wurden %d Logs gefunden.\n", treffer);
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

// Funktion 2: IP-Suche/Filterung
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
        printf(YELLOW "\nKeine %s IP-Adressen gefunden.\n" RESET, privat ? "privaten" : "öffentlichen");
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }

    auswahlnachSuche(privat ? 9 : 10);
}

// Funktion 3: Facility-Suche/Filterung
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

    // Muster erstellen, z. B. "%STP-"
    char muster[70];
    snprintf(muster, sizeof(muster), "%%%s-", eingabe);
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    zeilennummer = 0;
    treffer = 0;

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
        printf(YELLOW "\nKeine Logzeilen zur Facility '%s' gefunden.\n" RESET, eingabe);
    }
    else {
        printf("\nInsgesamt %d Treffer für Facility '%s'.\n", treffer, eingabe);
    }

    auswahlnachSuche(5);
}

#define MAX_FACILITIES 1000

int compareStrings(const void* a, const void* b) {
    const char* sa = *(const char**)a;
    const char* sb = *(const char**)b;
    return strcmp(sa, sb);
}

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
        printf(YELLOW "\nKeine Facilities gefunden.\n" RESET);
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
        printf(YELLOW "\nKeine Treffer für Facility '%s' gefunden.\n" RESET, muster);
    }
    else {
        printf("\nInsgesamt %d Treffer für Facility '%s'.\n", treffer, muster);
    }

    // Speicher freigeben
    for (int i = 0; i < anzahlFacilities; i++) {
        free(facilities[i]);
    }

    auswahlnachSuche(6);
}

// Funktion 3: Facility-Suche/Filterung
void eigeneUserSuche() {
    char eingabe[64];
    int versuche = 0;
    const int maxVersuche = 3;

    do {
        printf("\nBitte geben Sie einen Usernamen ein (z. B. admin, datacadmin):");
        fgets(eingabe, sizeof(eingabe), stdin);
        eingabe[strcspn(eingabe, "\n")] = '\0'; // Zeilenumbruch entfernen

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

    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    zeilennummer = 0;
    treffer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        if (strstr(zeile, eingabe)) {
            printf("Zeile %d: %s", zeilennummer, zeile);
            if (outputDatei) {
                fprintf(outputDatei, "Zeile %d: %s", zeilennummer, zeile);
            }
            treffer++;
        }
    }


    fclose(datei);

    if (treffer == 0) {
        printf(YELLOW "\nKeine Logzeilen zum User '%s' gefunden.\n" RESET, eingabe);
    }
    else {
        printf("\nInsgesamt %d Treffer für User '%s'.\n", treffer, eingabe);
    }
    auswahlnachSuche(3);
}

#define MAX_USER 100

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
        printf(YELLOW "\nKeine User gefunden.\n" RESET);
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
        printf(YELLOW "\nKeine Treffer für Benutzer '%s' gefunden.\n" RESET, muster);
    }
    else {
        printf("\nInsgesamt %d Treffer für Benutzer '%s'.\n", treffer, muster);
    }

    auswahlnachSuche(7);

    // Speicher freigeben wie gehabt ...


    // Speicher freigeben
    for (int i = 0; i < anzahlUser; i++) {
        free(userNamen[i]);
    }
}

#define MAX_MNEMONICS 1000

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
        printf(YELLOW "\nKeine Mnemonics gefunden.\n" RESET);
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
        printf(YELLOW "\nKeine Treffer für Mnemonic '%s' gefunden.\n" RESET, muster);
    }
    else {
        printf("\nInsgesamt %d Treffer für Mnemonic '%s'.\n", treffer, muster);
    }

    for (int i = 0; i < anzahlMnemonics; i++) {
        free(mnemonics[i]);
    }

    auswahlnachSuche(11);
}

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

    // Muster erstellen, z. B. "-CONFIG_I:"
    char muster[70];
    snprintf(muster, sizeof(muster), "-%s:", eingabe);
    speichersuche("Suchergebnisse.txt");
    dateiOeffnen();
    zeilennummer = 0;
    treffer = 0;

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
        printf(YELLOW "\nKeine Logzeilen zum Mnemonic '%s' gefunden.\n" RESET, eingabe);
    }
    else {
        printf("\nInsgesamt %d Treffer für Mnemonic '%s'.\n", treffer, eingabe);
    }

    auswahlnachSuche(12);
}

// Funktion 7: Severity-Level-Suche/Filterung
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
            printf(YELLOW "Keine Logs gefunden.\n" RESET);
        else
            printf(YELLOW "Keine Logs mit Severity Level %d gefunden.\n" RESET, sevLevelAuswahl);
    }
    else {
        if (sevLevelAuswahl == 8)
            printf("\nInsgesamt wurden %d Logs gefunden.\n", treffer);
        else
            printf("\nInsgesamt wurden %d Logs mit Severity Level %d gefunden.\n", treffer, sevLevelAuswahl);
    }

    auswahlnachSuche(8);
    return 0;
}

// Funktion 8: Auswahl neue Datei
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
            printf(YELLOW "Datei konnte nicht geöffnet werden. Bitte erneut versuchen.\n" RESET);
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

/* ##############################
      MAIN-/ HAUPTMETHODE
###############################*/

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
