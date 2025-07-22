// Programm Beleg Programmierung I

#define _CRT_SECURE_NO_WARNINGS  // Unterdrückt Warnungen für "unsichere" Funktionen wie fopen() in Visual Studio
#include <stdio.h>
#include <string.h>
#include <stdlib.h>             // Für das Löschen des Terminaltextes
#include <ctype.h>              // Buchstaben Bibliothek
// #include <windows.h>         // Für SetConsoleOutputCP() und CP_UTF8 und Windows ONLY

char suchbegriff[256];    // Variable zum Speichern des Suchbegriffs
char zeile[1024];         // Puffer zum Einlesen einer Zeile aus der Datei
int zeilennummer = 0;     // Zähler für die aktuelle Zeilennummer
int treffer = 0;          // Zählt die Anzahl gefundener Treffer
int begriff = 1;          // Suchbegriff Auswahlliste
int zeitauswahl = 1;
// SetConsoleOutputCP(CP_UTF8);  // Konsole auf UTF-8 stellen und Windows ONLY
int tag, versuch = 0;
char monat[4];
char uhrzeit[9];
int jahr = 0;
int minJahr = 9999, maxJahr = 0;
char dateiname[256] = "/Volumes/HSMW_MacOS/Programmierung/2._Semester/Programmierung_I/Cisco_SysLog_Router_Dateien_Auswertung_Belegarbeit/logs/syslog2.log";
int stunde;
int minute;
int sekunde;
FILE* datei;
int monatZuZahl(const char *monat);
int zeitZuSekunden(int tag, const char *monat, int jahr, int stunde, int minute, int sekunde);


int log_dateiendung(const char *log_dateiname) {
    size_t laenge = strlen(log_dateiname);
    return (laenge >= 4 && strcmp(log_dateiname + laenge - 4, ".log") == 0);
}

int dateiOeffnen() {
    datei = fopen(dateiname, "r");
    if (!datei) {
        perror("Fehler beim Öffnen der Datei");
    }
    return 0;
}

int suche(char *dateiname, char *suchbegriff, char *zeile, int zeilennummer, int treffer) {
    // Ausgabe zur Kontrolle
    printf("\n Dateiname: %s\n", dateiname);
    printf("\n Suchbegriff: %s\n", suchbegriff);
    suchbegriff[strcspn(suchbegriff, "\n")] = '\0';

    dateiOeffnen();

    printf("Die Datei wurde gefunden und die Suchtreffer werden nachfolgend ausgegeben:\n\n");

    // Zeilenweise die Datei lesen und nach dem Suchbegriff durchsuchen
    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;  // Aktuelle Zeilennummer erhöhen

        // Überprüfen, ob der Suchbegriff in der aktuellen Zeile enthalten ist
        if (strstr(zeile, suchbegriff)) {
            printf("Zeile %d: %s", zeilennummer, zeile);  // Treffer ausgeben
            treffer++;  // Trefferzähler erhöhen
            return 0;
        }
    } return 0;

    // Zusammenfassung der Ergebnisse
    if (treffer == 0) {
        printf("Keine Treffer für '%s' gefunden.\n", suchbegriff);
    }
    else {
        printf("\nInsgesamt wurde(n) %d Treffer gefunden.\n", treffer);
    }

    fclose(datei);  // Datei schließen
    return 0;
}

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

int monatDefinition() {
    do {
        versuch = 0;
        printf("\n Monat (MMM): ");
        if (scanf("%3s", monat) != 1) {
            printf("Fehler bei der Eingabe.\n");
            while (getchar() != '\n');
            
        } else {
            while (getchar() != '\n'); // Rest der Zeile löschen
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

int jahrDefinition() {
    
    dateiOeffnen();

    do {
        if (!fgets(zeile, sizeof(zeile), datei)) break;

        char logMonat[4];
        int logTag, logJahr;

        if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3) {
            if (logJahr < minJahr) minJahr = logJahr;
            if (logJahr > maxJahr) maxJahr = logJahr;
        }
    } while (1);

    fclose(datei);

    do {
        versuch = 0;
        printf("\n! Wähle eine Jahreszahl aus der kleinsten (%d) und größten Jahreszahl (%d).\n", minJahr, maxJahr);
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
    return 0;
}

int uhrzeitDefinition() {
    do {
        printf("\n Uhrzeit (HH:MM:SS): ");

        if (scanf("%d:%d:%d", &stunde, &minute, &sekunde) != 3) {
            printf("Ungültiges Format. Bitte HH:MM:SS eingeben.\n");
            versuch++;

            // Eingabepuffer leeren
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        } else if (stunde < 0 || stunde > 23 || minute < 0 || minute > 59 || sekunde < 0 || sekunde > 59) {
            printf("Ungültige Uhrzeitformat: %02d:%02d:%02d\n", stunde, minute, sekunde);
            versuch++;
        } else {
            // printf("Gültige Uhrzeit: %02d:%02d:%02d\n", stunde, minute, sekunde);
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

int zeitZuSekunden(int tag, const char *monat, int jahr, int stunde, int minute, int sekunde) {
    int mon = monatZuZahl(monat);
    if (mon == 0) {
        return -1; // ungültiger Monat
    }
    
    int sekunden = sekunde
                  + 60 * minute
                  + 3600 * stunde
                  + 86400 * (tag - 1)
                  + 86400 * 31 * (mon - 1)
                  + 86400 * 31 * 12 * jahr;
    return sekunden;
}

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

int zeitraum() {
    int startzeit;  // nur deklarieren

    printf("\nWähle die erste Zeit aus:");

    tagDefinition();
    monatDefinition();
    jahrDefinition();
    uhrzeitDefinition();

    // Erst hier, nach Eingabe, wird startzeit berechnet
    startzeit = zeitZuSekunden(tag, monat, jahr, stunde, minute, sekunde);

    printf("\nErste Zeit: %d. %s %d um %02d:%02d:%02d Uhr\n\n\n", tag, monat, jahr, stunde, minute, sekunde);

    printf("Wie sollen die Logs betrachtet werden?");
    printf("\n1: Ab der ersten Zeit.");
    printf("\n2: Bis zur ersten Zeit.");
    printf("\n3: Zeitraum zwischen der ersten und zweiten Zeit.");
    printf("\n4: Programm beenden.");

    printf("\n\nAuswahl: ");
    scanf("%d", &zeitauswahl);
    while (getchar() != '\n');  // Eingabepuffer leeren

    switch (zeitauswahl)
    {
    case 1: {
        dateiOeffnen();

        while (fgets(zeile, sizeof(zeile), datei)) {
            char stern;
            int lTag, lJahr, lStunde, lMinute, lSekunde, millisek;
            char lMonat[4];

            if (sscanf(zeile, "%c%3s %d %d %d:%d:%d.%d", &stern, lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde, &millisek) == 8) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit) {
                    printf("%s", zeile);
                }
            }
        }

        fclose(datei);
        break;
    }
    case 2: {
        dateiOeffnen();

        while (fgets(zeile, sizeof(zeile), datei)) {
            int lTag, lJahr, lStunde, lMinute, lSekunde;
            char lMonat[4];

            if (sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit <= startzeit) {
                    printf("%s", zeile);
                }
            }
        }
        fclose(datei);
        break;
    }
    case 3: {
        int endTag, endJahr, endStunde, endMinute, endSekunde;
        char endMonat[4];
        int endzeit;

        printf("\nGib die zweite Zeit ein (Tag, Monat, Jahr, Uhrzeit):\n");

        // zweiter Tag
        do {
            versuch = 0;
            printf("\n Tag (DD): ");
            int falscheEingabe = scanf("%d", &endTag);
            while (getchar() != '\n');

            if (falscheEingabe != 1 || endTag < 1 || endTag > 31) {
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

        //  zweiter Monat
        do {
            versuch = 0;
            printf("\n Monat (MMM): ");
            if (scanf("%3s", endMonat) != 1) {
                printf("Fehler bei der Eingabe.\n");
                while (getchar() != '\n');
                
            } else {
                while (getchar() != '\n'); // Rest der Zeile löschen
                if (!alleMonate(endMonat)) {
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

        //  zweites Jahr
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

        do {
            if (!fgets(zeile, sizeof(zeile), datei)) break;

            char logMonat[4];
            int logTag, logJahr;

            if (sscanf(zeile, "*%3s %d %d", logMonat, &logTag, &logJahr) == 3) {
                if (logJahr < minJahr) minJahr = logJahr;
                if (logJahr > maxJahr) maxJahr = logJahr;
            }
        } while (1);

        fclose(datei);

        // zweite Uhrzeit
        do {
            printf("\n Uhrzeit (HH:MM:SS): ");

            if (scanf("%d:%d:%d", &endStunde, &endMinute, &endSekunde) != 3) {
                printf("Ungültiges Format. Bitte HH:MM:SS eingeben.\n");
                versuch++;

                // Eingabepuffer leeren
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
            } else if (endStunde < 0 || endStunde > 23 || endMinute < 0 || endMinute > 59 || endSekunde < 0 || endSekunde > 59) {
                printf("Ungültige Uhrzeitformat: %02d:%02d:%02d\n", endStunde, endMinute, endSekunde);
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
        while (getchar() != '\n');

        endzeit = zeitZuSekunden(endTag, endMonat, endJahr, endStunde, endMinute, endSekunde);

        if (endzeit < startzeit) {
            printf("\nDie zweite Zeit muss nach der ersten Zeit liegen.\n");
            break;
        }

        dateiOeffnen();

        printf("\nLogs zwischen %d.%s.%d %02d:%02d:%02d und %d.%s.%d %02d:%02d:%02d:\n\n",
            tag, monat, jahr, stunde, minute, sekunde,
            endTag, endMonat, endJahr, endStunde, endMinute, endSekunde);

        while (fgets(zeile, sizeof(zeile), datei)) {
            int lTag, lJahr, lStunde, lMinute, lSekunde;
            char lMonat[4];

            if (sscanf(zeile, "*%3s %d %d %d:%d:%d", lMonat, &lTag, &lJahr, &lStunde, &lMinute, &lSekunde) == 6) {
                int logZeit = zeitZuSekunden(lTag, lMonat, lJahr, lStunde, lMinute, lSekunde);
                if (logZeit >= startzeit && logZeit <= endzeit) {
                    printf("%s", zeile);
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
    // Löscht den Terminaltext
    system("clear");

    //Erklärung des Programms
    // printf("Dies ist ein Programm zur Auswertung eines CISCO-Logfiles.\n\n");
    printf("\n#####################################################");
    printf("\n      Auswertungsprogramm für CISCO-Logdateien");
    printf("\n#####################################################");

    // Benutzer gibt den Dateipfad ein
    printf("\n\nBitte geben Sie den Dateipfad ein: %s", dateiname);
    // fgets(dateiname, sizeof(dateiname), stdin);
    // dateiname[strcspn(dateiname, "\n")] = '\0';

    // .log-Dateiendung Fehlermeldung
    if (!log_dateiendung(dateiname)) {
        printf("\nDie Datei muss die Endung .log haben. Programm wird beendet.\n");
        return 0;
    }

    printf("\nWähle ein Suchbegriff aus:");
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

    switch (begriff)
    {
    case 1:
        zeitraum();
        break;
    case 2:
        printf("Test");
        break;
    case 3:
        /* code */
        break;
    case 4:
        /* code */
        break;
    case 5:
        /* code */
        break;
    case 6:
        /* code */
        break;
    case 7:
        /* code */
        break;
    case 8:
        /* code */
        break;
    case 9:
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


// Programm ausführen: STRG+F5 oder Menüeintrag "Debuggen" > "Starten ohne Debuggen starten"
// Programm debuggen: F5 oder "Debuggen" > Menü "Debuggen starten"

// Tipps für den Einstieg: 
//   1. Verwenden Sie das Projektmappen-Explorer-Fenster zum Hinzufügen/Verwalten von Dateien.
//   2. Verwenden Sie das Team Explorer-Fenster zum Herstellen einer Verbindung mit der Quellcodeverwaltung.
//   3. Verwenden Sie das Ausgabefenster, um die Buildausgabe und andere Nachrichten anzuzeigen.
//   4. Verwenden Sie das Fenster "Fehlerliste", um Fehler anzuzeigen.
//   5. Wechseln Sie zu "Projekt" > "Neues Element hinzufügen", um neue Codedateien zu erstellen, bzw. zu "Projekt" > "Vorhandenes Element hinzufügen", um dem Projekt vorhandene Codedateien hinzuzufügen.
//   6. Um dieses Projekt später erneut zu öffnen, wechseln Sie zu "Datei" > "Öffnen" > "Projekt", und wählen Sie die SLN-Datei aus.