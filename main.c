// Programm Beleg Programmierung I

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>             // Löschen des Terminals
#include <ctype.h>              // Buchstaben-Bibliothek
// #include <windows.h>         // Für SetConsoleOutputCP() und CP_UTF8 und Windows ONLY

// #ifdef _WIN32                // Windows ONLY
// #define strcasecmp _stricmp  // Windows ONLY
// #endif                       // Windows ONLY

/* ##############################
     VARIABLENDEKLARATION
###############################*/

// globale Variablendefinition
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

    printf("\nGib einen beliebigen Suchbegriff und drücke die Enter-Taste: ");
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
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }

    fclose(datei);
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
    int ch;
    // Leere den Eingabepuffer, falls noch alte Zeichen vorhanden sind
    while ((ch = getchar()) != '\n' && ch != EOF);

    int maxVersuche = 3;  // Maximal erlaubte Fehlversuche bei der IP-Eingabe
    int versuche = 0;

    // Eingabeschleife für die IP-Adresse
    while (versuche < maxVersuche) {
        printf("\n Gib eine IP-Adresse ein (Format: XXX.XXX.XXX.XXX): ");
        fgets(suchbegriff, sizeof(suchbegriff), stdin);
        suchbegriff[strcspn(suchbegriff, "\n")] = '\0'; // Entferne das '\n' am Ende

        // Prüfe, ob die Eingabe leer ist
        if (strlen(suchbegriff) == 0) {
            versuche++;
            printf("Keine IP-Adresse eingegeben.");
        }
        // Prüfe das Format der IP-Adresse
        else if (!istGueltigeIPv4(suchbegriff)) {
            versuche++;
            printf("Ungültiges IP-Adressformat.");
        }
        else {
            break; // Eingabe ist gültig → Schleife verlassen
        }

        // Gib verbleibende Versuche aus oder beende bei zu vielen Fehlern
        if (versuche < maxVersuche) {
            printf("\nNoch %d Versuch(e) übrig\n", maxVersuche - versuche);
        }
        else {
            printf("\nZu viele ungültige Versuche. Das Programm wird beendet.\n");
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
            // IP-Adresse gefunden – Zeile ausgeben
            printf("Zeile %d: %s", zeilennummer, zeile);
            treffer++;
        }
    }

    fclose(datei); // Datei schließen

    // Zusammenfassung ausgeben
    if (treffer == 0) {
        printf("\nKeine Einträge mit IP-Adresse '%s' gefunden.\n", suchbegriff);
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }


    return 0;
}

// Funktion: Definition aller gültigen Monate
int alleMonate(const char *monat) {
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
            printf("\nUngültige Eingabe – gib einen Tag zwischen 1 und 31 ein.");
            versuch++;
        } 
        else {
            break;
        }
        if (versuch < 3) {
            printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
        } 
        else {
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
        printf(" Monat (MMM): ");
        if (scanf("%3s", monat) != 1) {
            printf("Fehler bei der Eingabe.\n");
            while (getchar() != '\n');
        } 
        else {
            while (getchar() != '\n');
            if (!alleMonate(monat)) {
                printf("\nUngültige Eingabe (z. B. Jan, Feb, Mar...).\n");
                versuch++;
            } 
            else {
                break;
            }
        }
        if (versuch < 3) {
            printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
        } 
        else {
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
            printf("! Wähle eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl (%d).", minJahr, maxJahr);
            printf("\n Jahreszahl (YYYY): ");
            int falscheEingabe = scanf("%d", &jahr);
            if (falscheEingabe != 1) {
                printf("\nKeine gültige Jahreszahl.");
                versuch++;
                while (getchar() != '\n');
            } 
            else if (jahr < minJahr || jahr > maxJahr) {
                printf(" Jahreszahl außerhalb des gültigen Bereichs.");
                versuch++;
            } 
            else {
                break;
            }
            if (versuch < 3) {
                printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
            } 
            else {
                printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n");
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
            printf("Ungültiges Format. Bitte HH:MM:SS eingeben.\n");
            versuch++;
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        } 
        else if (stunde < 0 || stunde > 23 || minute < 0 || minute > 59 || sekunde < 0 || sekunde > 59) {
            printf("Ungültige Uhrzeitformat: %02d:%02d:%02d\n", stunde, minute, sekunde);
            versuch++;
        } 
        else {
            break;
        }
        if (versuch < 3) {
            printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
        } 
        else {
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
        treffer = 0;
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            char stern;
            int lTag = 0, lJahr = -1, lStunde = 0, lMinute = 0, lSekunde = 0;
            char lMonat[4];

            // Altes Cisco-Format: *Sep 14 06:04:55.610: ...
            if (sscanf(zeile, "*%3s %d %d:%d:%d.%*d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            // Neues Cisco-Format: <189>: Sep 27 2023 10:19:18.4321 UTC: ...
            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            // Falls Zeile ohne Millisekunden (selten, aber möglich)
            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
        }
        fclose(datei);

        if (treffer == 0) {
            printf("\nKeine Logs gefunden.\n");
        } else {
            printf("\nEs wurden %d Logs gefunden.\n", treffer);
        }

        break;
    }
    // Ausgabe aller Logs nach dem ersten Zeitpunkt
    case 2: {
        treffer = 0;
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            int lTag = 0, lJahr = -1, lStunde = 0, lMinute = 0, lSekunde = 0;
            char lMonat[4];

            // Altes Format ohne Jahr, mit Millisekunden
            if (jahr == -1 && sscanf(zeile, "*%3s %d %d:%d:%d.%*d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            // Altes Format mit Jahr, mit Millisekunden
            else if (jahr != -1 && sscanf(zeile, "*%3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            // Neues Format mit Jahr, mit Millisekunden
            else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%*d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            // Optional: ohne Millisekunden
            else if (jahr == -1 && sscanf(zeile, "*%3s %d %d:%d:%d", lMonat, &lTag, &lStunde, &lMinute, &lSekunde) == 5) {
                int logZeit = zeitZuSekundenOhneJahr(lTag, lMonat, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
            else if (jahr != -1 && sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                    treffer++;
                }
            }
        }
        fclose(datei);

        if (treffer == 0) {
            printf("\nKeine Logs gefunden.\n");
        } else {
            printf("\nEs wurden %d Logs gefunden.\n", treffer);
        }
        break;
    }
    // Ausgabe aller Logs zwischen zwei Zeitpunkten
    case 3: {
        treffer = 0;
        int endTag, endJahr, endStunde, endMinute, endSekunde;
        double sekundenBruch = 0.0;
        char endMonat[4];
        int endzeit;

        // --- Eingabe und Validierung: Tag ---
        do {
            versuch = 0;
            printf(" Tag (DD): ");
            int falscheEingabe = scanf("%d", &endTag);
            while (getchar() != '\n');
            if (falscheEingabe != 1 || endTag < 1 || endTag > 31) {
                printf("\nUngültige Eingabe – gib einen Tag zwischen 1 und 31 ein.");
                versuch++;
            } else break;
            if (versuch < 3) printf("\nNoch %d Versuch(e) übrig\n", 3 - versuch);
            else { printf(" Zu viele ungültige Versuche. Das Programm wird beendet.\n"); return 1; }
        } while (1);

        // --- Eingabe und Validierung: Monat ---
        do {
            versuch = 0;
            printf(" Monat (MMM): ");
            if (scanf("%3s", endMonat) != 1) {
                printf("Fehler bei der Eingabe.\n");
                while (getchar() != '\n');
            } else {
                while (getchar() != '\n');
                if (!alleMonate(endMonat)) {
                    printf("\nUngültige Eingabe (z. B. Jan, Feb, Mar...).\n");
                    versuch++;
                } else break;
            }
            if (versuch < 3) printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
            else { printf("Zu viele ungültige Versuche. Das Programm wird beendet.\n"); return 1; }
        } while (1);

        // --- Jahrenerkennung im Log ---
        int endJahrVorhanden = 0;
        minJahr = 9999; maxJahr = 0;
        dateiOeffnen();
        while (fgets(zeile, sizeof(zeile), datei)) {
            char logMonat[4];
            int logTag, logJahr, logStunde, logMinute, logSekunde, dummy;
            // Cisco-Format mit Jahr
            if (sscanf(zeile, "*%3s %d %d %d:%d:%d", logMonat, &logTag, &logJahr, &logStunde, &logMinute, &logSekunde) == 6 && logJahr >= 1970 && logJahr <= 2100) {
                endJahrVorhanden = 1;
                if (logJahr < minJahr) minJahr = logJahr;
                if (logJahr > maxJahr) maxJahr = logJahr;
            }
            // Syslog-Priority-Format mit Jahr
            else if (sscanf(zeile, "<%d>: %3s %d %d %d:%d:%d", &dummy, logMonat, &logTag, &logJahr, &logStunde, &logMinute, &logSekunde) == 7 && logJahr >= 1970 && logJahr <= 2100) {
                endJahrVorhanden = 1;
                if (logJahr < minJahr) minJahr = logJahr;
                if (logJahr > maxJahr) maxJahr = logJahr;
            }
        }
        fclose(datei);

        // --- Eingabe und Validierung: Jahr ---
        if (endJahrVorhanden) {
            do {
                versuch = 0;
                printf("! Wähle eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl (%d).\n", minJahr, maxJahr);
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

        // --- Eingabe und Validierung: Uhrzeit ---
        do {
            versuch = 0;
            printf(" Uhrzeit (HH:MM:SS): ");
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

        // --- Zeitumrechnung und Bereichsprüfung ---
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
                    if (logZeit >= startzeit && logZeit <= endzeit) {
                        printf("%s", zeile);
                        treffer++;
                    }
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
                double sekundenBruch = 0.0;

                // Cisco-Standardformat mit Jahr
                if (sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                    int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit && logZeit <= endzeit) {
                        printf("%s", zeile);
                        treffer++;
                    }
                }
                // Syslog-Priority-Format mit Jahr und ggf. Zeitzone
                else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%lf %*s", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde, &sekundenBruch) >= 6) {
                    int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit && logZeit <= endzeit) {
                        printf("%s", zeile);
                        treffer++;
                    }
                }
                // Alternative: Priority-Format ohne Zeitzone
                else if (sscanf(zeile, "<%*d>: %3s %d %d %d:%d:%d.%lf", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde, &sekundenBruch) >= 6) {
                    int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                    if (logZeit >= startzeit && logZeit <= endzeit) {
                        printf("%s", zeile);
                        treffer++;
                    }
                }
            }
        }
        fclose(datei);

        // --- Ergebnis-Ausgabe ---
        if (treffer == 0) {
            printf("\nKeine Logs gefunden.\n");
        } else {
            printf("\nEs wurden %d Logs gefunden.\n", treffer);
        }
        break;
    }

    case 4:
        printf("Programm wird beendet\n");
        exit(0);
        break;
    default:
        printf("Ungültige Auswahl.\n");
        break;
    }
    return 0;
}

int severityLevel() {
    const char *sevLevellNamen[] = {
        "EMERGENCIES", "ALERTS", "CRITICALS", "ERRORS", "WARNINGS",
        "NOTIFICATIONS", "INFORMATIONAL", "DEBUGGING"
    };
    const char *sevLevelBeschreibung[] = {
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
    printf("\n 9: Programm beenden\n");

    int sevLevelAuswahl;
    printf("\n Ausgewähltes Severity Level: ");
    scanf("%d", &sevLevelAuswahl);
    while (getchar() != '\n'); // Eingabepuffer leeren

    if (sevLevelAuswahl == 9) {
        printf("\nProgramm wird beendet.\n");
        exit(0);
    }

    dateiOeffnen();
    int treffer = 0, zeilennummer = 0;

    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;
        char *prozentZeichen = strchr(zeile, '%');
        if (prozentZeichen) {
            char *minusZeichen = strchr(prozentZeichen, '-');
            if (minusZeichen && isdigit(*(minusZeichen + 1)) && !isdigit(*(minusZeichen + 2))) {
                int sev = *(minusZeichen + 1) - '0';
                if ((sevLevelAuswahl == 8) || (sev == sevLevelAuswahl)) {
                    if (treffer == 0 && sevLevelAuswahl != 8) {
                        printf("\nAlle %s Logs werden angezeigt (%s)\n\n", sevLevellNamen[sevLevelAuswahl], sevLevelBeschreibung[sevLevelAuswahl]);
                    }
                    printf("%s", zeile);
                    treffer++;
                }
            }
        }
    }
    fclose(datei);

    if (treffer == 0) {
        if (sevLevelAuswahl == 8)
            printf("Keine Logs gefunden.\n");
        else
            printf("Keine Logs mit Severity Level %d gefunden.\n", sevLevelAuswahl);
    } else {
        if (sevLevelAuswahl == 8)
            printf("\nInsgesamt wurden %d Logs gefunden.\n", treffer);
        else
            printf("\nInsgesamt wurden %d Logs mit Severity Level %d gefunden.\n", treffer, sevLevelAuswahl);
    }
    return 0;
}

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
                    treffer++;
                    break;
                }
            }

            ptr++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf("\nKeine %s IP-Adressen gefunden.\n", privat ? "privaten" : "öffentlichen");
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }
}

void eigeneFacilitySuche() {
    char eingabe[64];
    printf("\nGib einen Facility-Begriff ein (z. B. STP, LINK, DHCP): ");
    fgets(eingabe, sizeof(eingabe), stdin);
    eingabe[strcspn(eingabe, "\n")] = '\0'; // Zeilenumbruch entfernen

    if (strlen(eingabe) == 0) {
        printf("Kein Begriff eingegeben. Zurück...\n");
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
            treffer++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf("\nKeine Logzeilen zur Facility '%s' gefunden.\n", eingabe);
    }
    else {
        printf("\nInsgesamt %d Treffer für Facility '%s'.\n", treffer, eingabe);
    }
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
        printf("\nKeine Facilities gefunden.\n");
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
            treffer++;
        }
    }

    fclose(datei);

    if (treffer == 0) {
        printf("\nKeine Treffer für Facility '%s' gefunden.\n", muster);
    }
    else {
        printf("\nInsgesamt %d Treffer für Facility '%s'.\n", treffer, muster);
    }

    // Speicher freigeben
    for (int i = 0; i < anzahlFacilities; i++) {
        free(facilities[i]);
    }
}

/* ##############################
      MAIN-/ HAUPTMETHODE
###############################*/


int main() {
    //SetConsoleOutputCP(CP_UTF8);  // Windows: Konsole auf UTF-8

    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    
    printf("\n#####################################################");
    printf("\n      Auswertungsprogramm für CISCO-Logdateien");
    printf("\n#####################################################");
    printf("\n\nBitte geben Sie den Dateipfad ein: %s", dateiname);
    if (!log_dateiendung(dateiname)) {
        printf("\nDie Datei muss die Endung .log haben. Programm wird beendet.\n");
        return 0;
    }

    // Hauptmenü-Schleife
    while (1) {
        printf("\nWähle ein Suchbegriff aus:");
        printf("\n0: Eigene Eingabe");
        printf("\n1: Zeitraum");
        printf("\n2: IP Adresse");
        printf("\n3: Interfaces");
        printf("\n4: User");
        printf("\n5: Ereignis");
        printf("\n6: Fehlermeldung");
        printf("\n7: Severity Level");
        printf("\n8: Neue Datei auswählen");
        printf("\n9: Programm beenden");

        printf("\n\nAusgewählter Suchbegriff: ");
        scanf("%d", &begriff);

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
            printf("4: Zurück ins Hauptmenü\n");
            printf("5: Programm beenden\n");
            printf("\n\nAuswahl: ");
            scanf("%d", &wahl);
            while (getchar() != '\n');

            switch (wahl) {
                case 1: 
                    ipSuche(); 
                    break;
                case 2: 
                    ipFilterSucheEinfach(1); 
                    break;
                case 3: 
                    ipFilterSucheEinfach(0); 
                    break;
                case 4: 
                    printf("Zurück ins Hauptmenü...\n"); 
                    break;
                case 5: 
                    printf("Programm wird beendet.\n"); 
                    exit(0);
                default: 
                    printf("Ungültige Auswahl.\n"); 
                    break;
            }
            break;
        }
        case 3: {
            int wahl;
            printf("\nWähle die Art der Facility-Suche:\n");
            printf("1: Eigene Suche nach Facility-Begriff\n");
            printf("2: Alle vorhandenen Facilities anzeigen und auswählen\n");
            printf("3: Zurück ins Hauptmenü\n");
            printf("4: Programm beenden\n");
            printf("\nAuswahl: ");
            scanf("%d", &wahl);
            while (getchar() != '\n');

            switch (wahl) {
            case 1: eigeneFacilitySuche(); break;
            case 2: facilitySuche(); break;
            case 3: printf("Zurück ins Hauptmenü...\n"); break;
            case 4: printf("Programm wird beendet.\n"); exit(0);
            default: printf("Ungültige Auswahl.\n"); break;
            }
            break;
        }
        case 4: case 5: case 6: 
        case 7: 
            severityLevel();
            break;
        case 8:
            printf("\nFunktion zum Dateiwechsel ist noch nicht implementiert.\n");
            break;
        case 9:
            printf("Programm wird beendet.\n");
            exit(0);
            break;
        default:
            printf("Ungültige Auswahl. Bitte erneut versuchen.\n");
            break;
        }
    }
}
