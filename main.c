// Programm Beleg Programmierung I

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>             // Löschen des Terminals
#include <ctype.h>              // Buchstaben-Bibliothek
// #include <windows.h>         // Für SetConsoleOutputCP() und CP_UTF8 und Windows ONLY

/* ##############################
     VARIABLENDEKLARATION
###############################*/

// globale Variablendefinition
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
char dateiname[256] = "/Volumes/HSMW_MacOS/Programmierung/2._Semester/Programmierung_I/Cisco_SysLog_Router_Dateien_Auswertung_Belegarbeit/logs/syslog_generic.log";

int monatZuZahl(const char *monat);
int zeitZuSekunden(int tag, const char *monat, int jahr, int stunde, int minute, int sekunde);
int zeitZuSekundenOhneJahr(int tag, const char *monat, int stunde, int minute, int sekunde);


/* ##############################
      METHODENDEKLARATION
###############################*/

//  Funktion: Prüfung auf Dateinnamenende (".log")
int log_dateiendung(const char *log_dateiname) {
    size_t laenge = strlen(log_dateiname);
    return (laenge >= 4 && strcmp(log_dateiname + laenge - 4, ".log") == 0);
}

// Funktion: Öffnet die Datei
int dateiOeffnen() {
    datei = fopen(dateiname, "r");
    if (!datei) {
        perror("Fehler beim Öffnen der Datei");
    }
    return 0;
}

// Funktion: eigene Suchbegriffeingabe
int eigenerSuchbegriff() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF); // Eingabepuffer leeren

    printf("\n Gib einen beliebigen Suchbegriff und drücke die Enter-Taste: ");
    fgets(suchbegriff, sizeof(suchbegriff), stdin);
    suchbegriff[strcspn(suchbegriff, "\n")] = '\0'; // Zeilenumbruch entfernen

    // Prüfen, ob der Suchbegriff leer ist
    if (strlen(suchbegriff) == 0) {
        printf("Du hast keinen Suchbegriff eingegeben. Abbruch.\n");
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
            treffer++;
        }
    }

    if (treffer == 0) {
        printf("Keine Treffer für '%s' gefunden.\n", suchbegriff);
    } else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }

    fclose(datei);
    return 0;
}

// Funktion: Definition aller gültigen Monate
int alleMonate(const char *monat) {
    const char* gueltigeMonate[] = {
        "Jan", "Feb", "Mar", "Apr", "Mai", "Jun",
        "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"
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
    do {
        versuch = 0;
        printf("\n Tag (DD): ");
        int falscheEingabe = scanf("%d", &tag);
        while (getchar() != '\n');
        if (falscheEingabe != 1 || tag < 1 || tag > 31) {
            printf("\nUngültige Eingabe – gib einen Tag zwischen 1 und 31 ein.");
            versuch++;
        } else {
            break;
        }
        if (versuch < 3) {
            printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
        } else {
            printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n");
            return 1;
        }
    } while (1);
    return 0;
}

// Funktion: Definition der Monate
int monatDefinition() {
    do {
        versuch = 0;
        printf("\n Monat (MMM): ");
        if (scanf("%3s", monat) != 1) {
            printf("Fehler bei der Eingabe.\n");
            while (getchar() != '\n');
        } else {
            while (getchar() != '\n');
            if (!alleMonate(monat)) {
                printf("\nUngültige Eingabe (z. B. Jan, Feb, Mar...).\n");
                versuch++;
            } else {
                break;
            }
        }
        if (versuch < 3) {
            printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
        } else {
            printf("Zu viele ungültige Versuche. Das Programm wird beendet.\n");
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
        } else if (sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4 && logJahr >= 1970 && logJahr <= 2100) {
            jahrVorhanden = 1;
            if (logJahr < minJahr) minJahr = logJahr;
            if (logJahr > maxJahr) maxJahr = logJahr;
        }
    }
    fclose(datei);
    if (jahrVorhanden) {
        do {
            versuch = 0;
            printf("\n! Wähle eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl (%d).", minJahr, maxJahr);
            printf(" Jahreszahl (YYYY): ");
            int falscheEingabe = scanf("%d", &jahr);
            if (falscheEingabe != 1) {
                printf("\nKeine gültige Jahreszahl.");
                versuch++;
                while (getchar() != '\n');
            } else if (jahr < minJahr || jahr > maxJahr) {
                printf(" Jahreszahl außerhalb des gültigen Bereichs.");
                versuch++;
            } else {
                break;
            }
            if (versuch < 3) {
                printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
            } else {
                printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n");
                return 1;
            }
        } while (1);
    } else {
        jahr = -1;
        printf("\nEs ist keine Abfrage für das Jahr nötig.\n");
    }
    return 0;
}

// Funktion: Definition der Uhrzeit
int uhrzeitDefinition() {
    do {
        printf("\n Uhrzeit (HH:MM:SS): ");
        if (scanf("%d:%d:%d", &stunde, &minute, &sekunde) != 3) {
            printf("Ungültiges Format. Bitte HH:MM:SS eingeben.\n");
            versuch++;
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        } else if (stunde < 0 || stunde > 23 || minute < 0 || minute > 59 || sekunde < 0 || sekunde > 59) {
            printf("Ungültige Uhrzeitformat: %02d:%02d:%02d\n", stunde, minute, sekunde);
            versuch++;
        } else {
            break;
        }
        if (versuch < 3) {
            printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
        } else {
            printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n");
            return 1;
        }
    } while (1);
    return 0;
}

// Funktion: Zeitpunktberechnung für Zeitraum mit Jahr
int zeitZuSekunden(int tag, const char *monat, int jahr, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) return -1;
    int sekunden = sekunde + 60 * minute + 3600 * stunde + 86400 * (tag - 1) + 86400 * 31 * (mon - 1) + 86400 * 31 * 12 * jahr;
    return sekunden;
}

// Funktion: Zeitpunktberechnung für Zeitraum ohne Jahr
int zeitZuSekundenOhneJahr(int tag, const char *monat, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) return -1;
    int sekunden = sekunde + 60 * minute + 3600 * stunde + 86400 * (tag - 1) + 86400 * 31 * (mon - 1);
    return sekunden;
}

// Funktion: Zur Zeitstempelberechnung
int monatZuZahl(const char *monat) {
    if (strcmp(monat, "Jan") == 0) return 1;
    if (strcmp(monat, "Feb") == 0) return 2;
    if (strcmp(monat, "Mar") == 0) return 3;
    if (strcmp(monat, "Apr") == 0) return 4;
    if (strcmp(monat, "Mai") == 0) return 5;
    if (strcmp(monat, "Jun") == 0) return 6;
    if (strcmp(monat, "Jul") == 0) return 7;
    if (strcmp(monat, "Aug") == 0) return 8;
    if (strcmp(monat, "Sep") == 0) return 9;
    if (strcmp(monat, "Okt") == 0) return 10;
    if (strcmp(monat, "Nov") == 0) return 11;
    if (strcmp(monat, "Dez") == 0) return 12;
    return 0;
}

// Funktion: 1. Auswahl, Zeitraumberechnung der Logs
int zeitraum() {
    int startzeit;

    printf("\nWähle die erste Zeit aus:");

    tagDefinition();
    monatDefinition();
    jahrDefinition();
    uhrzeitDefinition();

    // Prüfung, ob ein Jahr vorhanden oder nicht, dann Ausgabe der Startzeit
    if (jahr == -1) {
        startzeit = zeitZuSekundenOhneJahr(tag, monat, stunde, minute, sekunde);
        printf("\nErste Zeit: %d. %s um %02d:%02d:%02d Uhr\n\n\n", tag, monat, stunde, minute, sekunde);
    } else {
        startzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);
        printf("\nErste Zeit: %d. %s %d um %02d:%02d:%02d Uhr\n\n\n", tag, monat, jahr, stunde, minute, sekunde);
    }

    printf("Wie sollen die Logs betrachtet werden?");
    printf("\n1: Ab der ersten Zeit.");
    printf("\n2: Bis zur ersten Zeit.");
    printf("\n3: Zeitraum zwischen der ersten und zweiten Zeit.");
    printf("\n4: Programm beenden.");

    printf("\n\nAuswahl: ");
    scanf("%d", &zeitauswahl);
    while (getchar() != '\n');

    switch (zeitauswahl) {
    // Ausgabe aller Logs vor dem ersten Zeitpunkt
    case 1: {
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            char stern;
            int lTag, lJahr, lStunde, lMinute, lSekunde, millisek;
            char lMonat[4];
            if (jahr == -1) {
                if (sscanf(zeile, "%c%3s %d %d:%d:%d.%d", &stern, lMonat, &lTag, &lStunde, &lMinute, &lSekunde, &millisek) == 7) {
                    int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit) printf("%s", zeile);
                }
            } else {
                if (sscanf(zeile, "%c%3s %d %d %d:%d:%d.%d", &stern, lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde, &millisek) == 8) {
                    int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit) printf("%s", zeile);
                }
            }
        }
        fclose(datei);
        break;
    }
    // Ausgabe aller Logs nach dem ersten Zeitpunkt
    case 2: {
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            int lTag, lJahr, lStunde, lMinute, lSekunde;
            char lMonat[4];
            if (jahr == -1) {
                if (sscanf(zeile, "*%3s %d %d:%d:%d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                    int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                    if (logZeit <= startzeit) printf("%s", zeile);
                }
            } else {
                if (sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                    int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                    if (logZeit <= startzeit) printf("%s", zeile);
                }
            }
        }
        fclose(datei);
        break;
    }
    // Ausgabe aller Logs zwischen zwei Zeitpunkten
    case 3: {
        int endTag, endJahr, endStunde, endMinute, endSekunde;
        char endMonat[4];
        int endzeit;
        printf("\nGib die zweite Zeit ein (Tag, Monat, Jahr, Uhrzeit):\n");
        // Zweiter Tag
        do {
            versuch = 0;
            printf("\n Tag (DD): ");
            int falscheEingabe = scanf("%d", &endTag);
            while (getchar() != '\n');
            if (falscheEingabe != 1 || endTag < 1 || endTag > 31) {
                printf("\nUngültige Eingabe – gib einen Tag zwischen 1 und 31 ein.");
                versuch++;
            } else break;
            if (versuch < 3) printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
            else { printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n"); return 1; }
        } while (1);
        // Zweiter Monat
        do {
            versuch = 0;
            printf("\n Monat (MMM): ");
            if (scanf("%3s", endMonat) != 1) {
                printf("Fehler bei der Eingabe.\n");
                while (getchar() != '\n');
            } else {
                while (getchar() != '\n');
                if (!alleMonate(endMonat)) {
                    printf("\nUngültige Eingabe (z. B. Jan, Feb, Mar...).\n");
                    versuch++;
                } else break;
            }
            if (versuch < 3) printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
            else { printf("Zu viele ungültige Versuche. Das Programm wird beendet.\n"); return 1; }
        } while (1);
        // Zweites Jahr
        int endJahrVorhanden = 0;
        minJahr = 9999; maxJahr = 0;
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            char logMonat[4];
            int logTag, logJahr, dummy;
            if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3 && logJahr >= 1970 && logJahr <= 2100) {
                endJahrVorhanden = 1;
                if (logJahr < minJahr) minJahr = logJahr;
                if (logJahr > maxJahr) maxJahr = logJahr;
            } else if (sscanf(zeile, "<%d>: %3s %d %d", &dummy, logMonat, &logTag, &logJahr) == 4 && logJahr >= 1970 && logJahr <= 2100) {
                endJahrVorhanden = 1;
                if (logJahr < minJahr) minJahr = logJahr;
                if (logJahr > maxJahr) maxJahr = logJahr;
            }
        }
        fclose(datei);
        if (endJahrVorhanden) {
            do {
                versuch = 0;
                printf("\n! Wähle eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl (%d).\n", minJahr, maxJahr);
                printf(" Jahreszahl (YYYY): ");
                int falscheEingabe = scanf("%d", &endJahr);
                if (falscheEingabe != 1) {
                    printf("\nKeine gültige Jahreszahl.");
                    versuch++;
                    while (getchar() != '\n');
                } else if (endJahr < minJahr || endJahr > maxJahr) {
                    printf(" Jahreszahl außerhalb des gültigen Bereichs.");
                    versuch++;
                } else break;
                if (versuch < 3) printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
                else { printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n"); return 1; }
            } while (1);
        } else {
            endJahr = -1;
            printf("\nEs ist keine Abfrage für das Jahr nötig.\n");
        }
        // Zweite Uhrzeit
        do {
            printf("\n Uhrzeit (HH:MM:SS): ");
            if (scanf("%d:%d:%d", &endStunde, &endMinute, &endSekunde) != 3) {
                printf("Ungültiges Format. Bitte HH:MM:SS eingeben.\n");
                versuch++;
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
            } else if (endStunde < 0 || endStunde > 23 || endMinute < 0 || endMinute > 59 || endSekunde < 0 || endSekunde > 59) {
                printf("Ungültige Uhrzeitformat: %02d:%02d:%02d\n", endStunde, endMinute, endSekunde);
                versuch++;
            } else break;
            if (versuch < 3) printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
            else { printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n"); return 1; }
        } while (1);
        while (getchar() != '\n');
        if (jahr == -1 || endJahr == -1) {
            endzeit = zeitZuSekundenOhneJahr(endTag, endMonat, endStunde, endMinute, endSekunde);
            if (endzeit < startzeit) { printf("\nDie zweite Zeit muss nach der ersten Zeit liegen.\n"); break; }
            dateiOeffnen();
            printf("\nLogs zwischen %d. %s um %02d:%02d:%02d Uhr und %d. %s um %02d:%02d:%02d Uhr\n\n",
                tag, monat, stunde, minute, sekunde,
                endTag, endMonat, endStunde, endMinute, endSekunde);
            while (fgets(zeile, sizeof(zeile), datei)) {
                int lTag, lStunde, lMinute, lSekunde;
                char lMonat[4];
                if (sscanf(zeile, "*%3s %d %d:%d:%d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                    int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit && logZeit <= endzeit) printf("%s", zeile);
                }
            }
        } else {
            endzeit = zeitZuSekunden(endTag, endMonat, endJahr, endStunde, endMinute, endSekunde);
            if (endzeit < startzeit) { printf("\nDie zweite Zeit muss nach der ersten Zeit liegen.\n"); break; }
            dateiOeffnen();
            printf("\nLogs zwischen %d. %s %d um %02d:%02d:%02d Uhr und %d. %s %d um %02d:%02d:%02d Uhr\n\n",
                tag, monat, jahr, stunde, minute, sekunde,
                endTag, endMonat, endJahr, endStunde, endMinute, endSekunde);
            while (fgets(zeile, sizeof(zeile), datei)) {
                int lTag, lJahr, lStunde, lMinute, lSekunde;
                char lMonat[4];
                if (sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                    int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit && logZeit <= endzeit) printf("%s", zeile);
                }
            }
        }
        fclose(datei);
        break;
    }
    case 4:
        printf("Programm wird beendet.\n");
        exit(0);
        break;
    default:
        printf("Ungültige Auswahl.\n");
        break;
    }
    return 0;
}

int main() {
    system("clear");
    printf("\n#####################################################");
    printf("\n      Auswertungsprogramm für CISCO-Logdateien");
    printf("\n#####################################################");
    printf("\n\nBitte geben Sie den Dateipfad ein: %s", dateiname);
    if (!log_dateiendung(dateiname)) {
        printf("\nDie Datei muss die Endung .log haben. Programm wird beendet.\n");
        return 0;
    }
    printf("\nWähle ein Suchbegriff aus:");
    printf("\n0: Eigene Eingabe");
    printf("\n1: Zeitraum");
    printf("\n2: IP Adresse");
    printf("\n3: Interfaces");
    printf("\n4: User");
    printf("\n5: Ereignis");
    printf("\n6: Fehlermeldung");
    printf("\n7: MAC-Adresse");
    printf("\n8: Severity Level");
    printf("\n9: Neue Datei auswählen");
    printf("\n10: Programm beenden");

    printf("\n\nAusgewählter Suchbegriff: ");
    scanf("%d", &begriff);

    switch (begriff) {
    case 0:
        eigenerSuchbegriff();
        break;
    case 1:
        zeitraum();
        break;
    case 2:
        printf("Test");
        break;
    case 3: case 4: case 5: case 6: case 7: case 8: case 9:
        /* code */
        break;
    case 10:
        printf("Programm wird beendet.\n");
        exit(0);
        break;
    default:
        break;
    }
}
