// Programm Beleg Programmierung I

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>             // Löschen des Terminals
#include <ctype.h>              // Buchstaben-Bibliothek
#define RED     "\033[31;91m"
#define YELLOW  "\033[33;93m"     
#define RESET   "\033[0m"

#ifdef _WIN32                   
#define strcasecmp _stricmp
#endif

/* ##############################
     VARIABLENDEKLARATION
###############################*/

// SetConsoleOutputCP(CP_UTF8);  // Konsole auf UTF-8 stellen und Windows ONLY
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
char dateiname[256] = "/Volumes/HSMW_MacOS/Programmierung/2._Semester/Programmierung_I/Cisco_SysLog_Router_Dateien_Auswertung_Belegarbeit/logs/syslog_generic.log";
// char dateiname[256] = "C:\\Users\\katha\\OneDrive\\Philipp\\HSMW Cybercrime, IT-Forensik\\2. Semester\\Programmierung I\\06 Beleg\\CICSO-Logfiles\\syslog_generic.log";
FILE* outputDatei = NULL;  

void hauptmenue(void);
void auswahlnachSuche(int funktionID);
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

int monatZuZahl(const char* monat);
int zeitZuSekunden(int tag, const char* monat, int jahr, int stunde, int minute, int sekunde);
int zeitZuSekundenOhneJahr(int tag, const char* monat, int stunde, int minute, int sekunde);


/* ##############################
      METHODENDEKLARATION
###############################*/

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
        printf("\n Gib eine IP-Adresse ein (Format: XXX.XXX.XXX.XXX): ");
        fgets(suchbegriff, sizeof(suchbegriff), stdin);
        suchbegriff[strcspn(suchbegriff, "\n")] = '\0';

        // Prüfe, ob die Eingabe leer ist
        if (strlen(suchbegriff) == 0) {
            versuche++;
            printf(YELLOW "Keine IP-Adresse eingegeben." RESET);
        }
        // Prüfe das Format der IP-Adresse
        else if (!istGueltigeIPv4(suchbegriff)) {
            versuche++;
            printf(YELLOW "Ungültiges IP-Adressformat." RESET);

        }
        else {
            break; // Eingabe ist gültig → Schleife verlassen
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
    printf("\n Suche nach IP-Adresse: %s\n", suchbegriff);

    dateiOeffnen(); // Öffne die Logdatei

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
    auswahlnachSuche(1);

    // Zusammenfassung ausgeben
    if (treffer == 0) {
        printf(YELLOW "\nKeine Einträge mit IP-Adresse '%s' gefunden.\n" RESET, suchbegriff);
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }


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
    versuch = 0;
    do {
        printf("\n Tag (DD): ");
        int falscheEingabe = scanf("%d", &tag);
        while (getchar() != '\n');
        if (falscheEingabe != 1 || tag < 1 || tag > 31) {
            printf(YELLOW "\nUngültige Eingabe – gib einen Tag zwischen 1 und 31 ein." RESET);
            versuch++;
        }
        else {
            break;
        }
        if (versuch < 3) {
            printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, 3 - versuch);
        }
        else {
            printf(RED "Zu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            return 1;
        }
    } while (1);
    return 0;
}

// Funktion: Definition der Monate
int monatDefinition() {
    do {
        versuch = 0;
        printf(" Monat (MMM): ");
        if (scanf("%3s", monat) != 1) {
            printf(YELLOW "Fehler bei der Eingabe.\n" RESET);
            while (getchar() != '\n');
        }
        else {
            while (getchar() != '\n');
            monat[0] = toupper(monat[0]);
            monat[1] = tolower(monat[1]);
            monat[2] = tolower(monat[2]);
            monat[3] = '\0';
            if (!alleMonate(monat)) {
                printf(YELLOW "\nUngültige Eingabe (z. B. Jan, Feb, Mar...).\n" RESET);
                versuch++;
            }
            else {
                break;
            }
        }
        if (versuch < 3) {
            printf(YELLOW "Noch %d Versuch(e) übrig\n" RESET, 3 - versuch);
        }
        else {
            printf(RED "\nZu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            return 1;
        }
    } while (1);
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
        do {
            versuch = 0;
            printf("Wähle eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl (%d).", minJahr, maxJahr);
            printf("\n Jahreszahl (YYYY): ");
            int falscheEingabe = scanf("%d", &jahr);
            if (falscheEingabe != 1) {
                printf(YELLOW "\nKeine gültige Jahreszahl." RESET);
                versuch++;
                while (getchar() != '\n');
            }
            else if (jahr < minJahr || jahr > maxJahr) {
                printf(YELLOW "Jahreszahl außerhalb des gültigen Bereichs." RESET);
                versuch++;
            }
            else {
                break;
            }
            if (versuch < 3) {
                printf(YELLOW "\nNoch %d Versuch(e) übrig\n" RESET, 3 - versuch);
            }
            else {
                printf(RED "Zu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
                return 1;
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
    do {
        printf(" Uhrzeit (HH:MM:SS): ");
        if (scanf("%d:%d:%d", &stunde, &minute, &sekunde) != 3) {
            printf(YELLOW "Ungültiges Format. Bitte HH:MM:SS eingeben.\n" RESET);
            versuch++;
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        else if (stunde < 0 || stunde > 23 || minute < 0 || minute > 59 || sekunde < 0 || sekunde > 59) {
            printf(YELLOW "Ungültige Uhrzeitformat: %02d:%02d:%02d\n" RESET, stunde, minute, sekunde);
            versuch++;
        }
        else {
            break;
        }
        if (versuch < 3) {
            printf(YELLOW "Noch %d Versuch(e) übrig\n" RESET, 3 - versuch);
        }
        else {
            printf(RED "Zu viele ungültige Versuche. Das Programm wird beendet.\n" RESET);
            return 1;
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

// Funktion: Auswahl nach Suche
void auswahlnachSuche(int funktionID) {
    char wahl;

    // Nur wenn mindestens ein Treffer gefunden wurde → Frage stellen
    if (treffer > 0) {
        char speichern;
        printf("\n\nMöchten Sie die Ergebnisse in eine Datei speichern? (j/n): ");
        speichern = getchar();
        while (getchar() != '\n');  // Eingabepuffer leeren

        if (speichern == 'j' || speichern == 'J') {
            outputDatei = fopen("Suchergebnisse.txt", "w");
            if (!outputDatei) {
                perror(YELLOW "Fehler beim Öffnen der Datei" RESET);
            }
            else {
                printf("\nErgebnisse werden gespeichert\n");
            }
        }
    }

    printf("\n\nWas möchten Sie tun?");
    printf("\n1: Suche wiederholen");
    printf("\n2: Zurück ins Hauptmenü");
    printf("\n3: Programm beenden");
    printf("\nBitte auswählen: ");
    wahl = getchar();
    while (getchar() != '\n');

    // Datei schließen, wenn offen
    if (outputDatei) {
        fclose(outputDatei);
        outputDatei = NULL;
        printf("\nErgebnisse wurden in 'Suchergebnisse.txt' gespeichert.\n");
    }

    switch (wahl) {
    case '1':
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
        default:
            printf("Unbekannte Funktion.\n");
            hauptmenue();
        }
        break;
    case '2':
        hauptmenue();
        break;
    case '3':
        printf("\nProgramm wird beendet.\n");
        exit(0);
    default:
        printf("Ungültige Eingabe. Zurück ins Hauptmenü.\n");
        hauptmenue();
    }
}

// Funktion 0: eigene Suchbegriffeingabe
int eigenerSuchbegriff() {

    printf("\n Gib einen beliebigen Suchbegriff und drücke die Enter-Taste: ");
    fgets(suchbegriff, sizeof(suchbegriff), stdin);
    suchbegriff[strcspn(suchbegriff, "\n")] = '\0'; // Zeilenumbruch entfernen

    // Prüfen, ob der Suchbegriff leer ist
    if (strlen(suchbegriff) == 0) {
        printf(RED "\nDu hast keinen Suchbegriff eingegeben. Abbruch.\n" RESET);
        return 1;
    }

    printf("\n Dateiname: %s\n", dateiname);
    printf("\n Suchbegriff: %s\n", suchbegriff);

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

    printf("\nWie sollen die Logs betrachtet werden?\n");
    printf("\n1: Ab der ersten eingegebenen Zeit.");
    printf("\n2: Bis zur ersten eingegebenen Zeit.");
    printf("\n3: Zeitraum zwischen der ersten und zweiten Zeit.");
    printf("\n4: Zurück ins Hauptmenü");
    printf("\n5: Programm beenden.\n");

    printf("\n\nAuswahl: ");
    scanf("%d", &zeitauswahl);
    while (getchar() != '\n');

    switch (zeitauswahl) {
    case 1: {
        treffer = 0;
        printf("\nWähle die erste Zeit aus:");
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
        printf("\nWähle die erste Zeit aus:");
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

        printf("\nWähle die erste Zeit aus:");
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
        printf("\nWähle die zweite Zeit aus:");
        tagDefinition();
        monatDefinition();
        jahrDefinition();
        uhrzeitDefinition();

        if (jahr == -1) {
            endzeit = zeitZuSekundenOhneJahr(tag, monat, stunde, minute, sekunde);
        } else {
            endzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);
        }

        if (endzeit < startzeit) {
            printf(YELLOW "\nDie zweite Zeit muss nach der ersten Zeit liegen.\n" RESET);
            break;
        }

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
    printf("\nGib einen Facility-Begriff ein (z. B. STP, LINK, DHCP): ");
    fgets(eingabe, sizeof(eingabe), stdin);
    eingabe[strcspn(eingabe, "\n")] = '\0'; // Zeilenumbruch entfernen

    if (strlen(eingabe) == 0) {
        printf(YELLOW "Kein Begriff eingegeben. Zurück...\n" RESET);
        return;
    }

    // Muster erstellen, z. B. "%STP-"
    char muster[70];
    snprintf(muster, sizeof(muster), "%%%s-", eingabe);

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

    printf("\nGefundene Facilities:\n");
    for (int i = 0; i < anzahlFacilities; i++) {
        printf("%2d: %s\n", i + 1, facilities[i]);
    }
    printf(" 0: Zurück\n");

    int auswahl;
    printf("\nWähle eine Facility aus: ");
    scanf("%d", &auswahl);
    while (getchar() != '\n');

    if (auswahl == 0 || auswahl > anzahlFacilities) {
        printf("Zurück...\n");
        return;
    }

    const char* muster = facilities[auswahl - 1];
    char suchmuster[70];
    snprintf(suchmuster, sizeof(suchmuster), "%%%s-", muster); // z. B. %LINK-

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
    printf("\nGib einen Usernamen ein (z. B. admin, datacadmin): ");
    fgets(eingabe, sizeof(eingabe), stdin);
    eingabe[strcspn(eingabe, "\n")] = '\0'; // Zeilenumbruch entfernen

    if (strlen(eingabe) == 0) {
        printf(YELLOW "Keinen User eingegeben. Zurück...\n" RESET);
        return;
    }

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
    auswahlnachSuche(6);
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
        return;
    }

    // sortieren
    qsort(userNamen, anzahlUser, sizeof(char*), compareStrings);

    printf("\nGefundene User:\n");
    for (int i = 0; i < anzahlUser; i++) {
        printf("%2d: %s\n", i + 1, userNamen[i]);
    }
    printf(" 0: Zurück\n");

    int auswahl;
    printf("\nWähle einen User aus: ");
    scanf("%d", &auswahl);
    while (getchar() != '\n');

    if (auswahl == 0 || auswahl > anzahlUser) {
        printf(YELLOW "Zurück...\n" RESET);
    }
    else {
        const char* muster = userNamen[auswahl - 1];

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
    }

    auswahlnachSuche(7);

    // Speicher freigeben
    for (int i = 0; i < anzahlUser; i++) {
        free(userNamen[i]);
    }
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

    printf("\n Wähle ein Severity Level aus.");
    for (int i = 0; i < 8; ++i)
        printf("\n %d: %s", i, sevLevellNamen[i]);
    printf("\n 8: Alle");
    printf("\n 9: Zurück in das Hauptmenü");
    printf("\n 10: Programm beenden\n");

    int sevLevelAuswahl;
    printf("\n Ausgewähltes Severity Level: ");
    scanf("%d", &sevLevelAuswahl);
    while (getchar() != '\n'); // Eingabepuffer leeren

    if (sevLevelAuswahl == 9) {
        hauptmenue();
    }

    if (sevLevelAuswahl == 10) {
        printf(RED "\nProgramm wird beendet.\n" RESET);
        exit(0);
    }

    dateiOeffnen();
    int treffer = 0, zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        char* prozentZeichen = strchr(zeile, '%');
        if (prozentZeichen) {
            char* minusZeichen = strchr(prozentZeichen, '-');
            if (minusZeichen && isdigit(*(minusZeichen + 1)) && !isdigit(*(minusZeichen + 2))) {
                int sev = *(minusZeichen + 1) - '0';
                if ((sevLevelAuswahl == 8) || (sev == sevLevelAuswahl)) {
                    if (treffer == 0 && sevLevelAuswahl != 8) {
                        printf("\nAlle %s Logs werden angezeigt (%s)\n\n", sevLevellNamen[sevLevelAuswahl], sevLevelBeschreibung[sevLevelAuswahl]);
                    }
                }
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
        printf("\n\nBitte geben Sie den Pfad zur neuen Logdatei ein (mit .log-Endung):\n> ");
        fgets(neuerDateiname, sizeof(neuerDateiname), stdin);
        neuerDateiname[strcspn(neuerDateiname, "\n")] = '\0'; // Zeilenumbruch entfernen

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

    printf("\n\nWähle ein Suchbegriff aus:\n");
    printf("\n0: Eigene Eingabe");
    printf("\n1: Zeitraum");
    printf("\n2: IP Adresse");
    printf("\n3: Facilities");
    printf("\n4: User");
    printf("\n5: Ereignis");
    printf("\n6: Fehlermeldung");
    printf("\n7: Severity Level");
    printf("\n8: Neue Datei auswählen");
    printf("\n9: Programm beenden");

    printf("\n\nAusgewählter Suchbegriff: ");
    scanf("%d", &begriff);
    while (getchar() != '\n');  // Eingabepuffer leeren

    switch (begriff) {
    case 0:
        eigenerSuchbegriff();
        break;
    case 1:
        zeitraum();
        break;
    case 2: {
        int wahl;
        printf("\nWähle die Art der IP-Suche:\n");
        printf("1: Manuelle Eingabe einer IP-Adresse\n");
        printf("2: Nur private IP-Adressen anzeigen\n");
        printf("3: Nur öffentliche IP-Adressen anzeigen\n");
        printf("4: Zurück in das Hauptmenü\n");
        printf("5: Programm beenden");
        printf("\n\nAuswahl: ");
        scanf("%d", &wahl);
        while (getchar() != '\n');

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
        int wahl;
        printf("\nWähle die Art der Facility-Suche:\n");
        printf("1: Eigene Suche nach Facility-Begriff\n");
        printf("2: Alle vorhandenen Facilities anzeigen und auswählen\n");
        printf("3: Zurück in das Hauptmenü\n");
        printf("4: Programm beenden\n");
        printf("\nAuswahl: ");
        scanf("%d", &wahl);
        while (getchar() != '\n');

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
        int wahl;
        printf("\nWähle die Art der User-Suche:\n");
        printf("1: Eigene Suche nach User\n");
        printf("2: Alle User anzeigen und auswählen\n");
        printf("3: Zurück in das Hauptmenü\n");
        printf("4: Programm beenden\n");
        printf("\nAuswahl: ");
        scanf("%d", &wahl);
        while (getchar() != '\n');

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
    case 5: case 6:
    case 7:
        severityLevel();
        break;
    case 8:
        neueDateiAuswaehlen();
        hauptmenue();
        break;
    case 9:
        printf(RED "Programm wird beendet.\n" RESET);
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
    printf("\n\nBitte geben Sie den Dateipfad ein: %s\n", dateiname);

    if (!log_dateiendung(dateiname)) {
        printf(RED "\nDie Datei muss die Endung .log haben. Programm wird beendet.\n" RESET);
        return 0;
    }

    hauptmenue();  // Menü einmalig aufrufen

    return 0;
}

