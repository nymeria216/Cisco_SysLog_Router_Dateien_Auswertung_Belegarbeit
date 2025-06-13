// Programm Beleg Programmierung I

#define _CRT_SECURE_NO_WARNINGS  // Unterdrückt Warnungen für "unsichere" Funktionen wie fopen() in Visual Studio
#include <stdio.h>
#include <string.h>
// #include <windows.h>    // Für SetConsoleOutputCP() und CP_UTF8 und Windows ONLY

int main() {
   // SetConsoleOutputCP(CP_UTF8);  // Konsole auf UTF-8 stellen und Windows ONLY
    char dateiname[256];      // Variable zum Speichern des Dateipfads
    char suchbegriff[256];    // Variable zum Speichern des Suchbegriffs
    char zeile[1024];         // Puffer zum Einlesen einer Zeile aus der Datei
    int zeilennummer = 0;     // Zähler für die aktuelle Zeilennummer
    int treffer = 0;          // Zählt die Anzahl gefundener Treffer

    //Erklärung des Programms
    printf("Dies ist ein Programm zur Auswertung eines CISCO-Logfiles.\n\n");

    // Benutzer gibt den Dateipfad ein
    printf("Bitte geben Sie den Dateipfad ohne Anführungszeichen ein und drücken Sie die Enter-Taste: \n\n");
    fgets(dateiname, sizeof(dateiname), stdin);
    dateiname[strcspn(dateiname, "\n")] = '\0';

    // Fragestellung, ob die Datei existiert
    char janein;
    int versuch = 0;  // Zähler für JaNein-Versuche
    do {
        printf("Ist der Pfad und Name der Datei korrekt (y/n)? ");
        scanf(" %c", &janein);
        while (getchar() != '\n')

        if (janein == 'y' || janein == 'Y') {
        break;                     
    }

        versuch++;
    
        if (versuch == 2 && (janein != 'y' || janein != 'Y')) {
            printf("Zu viele Versuche. Das Programm wird beendet.\n");
            return 0;
        }

    } while (janein != 'y' || janein != 'y');  // Solange die Datei nicht existiert, wird nachgefragt


    //Hier eine Auswahl programmieren mit Eingabe von Ziffern, was der Benutzer mit der Datei machen will


    // Benutzer gibt den Suchbegriff ein
    printf("\n Bitte geben Sie den Suchbegriff ein und drücken Sie die Enter-Taste: ");
    fgets(suchbegriff, sizeof(suchbegriff), stdin);

    // Ausgabe zur Kontrolle
    printf("\n Dateiname: %s\n", dateiname);
    printf("\n Suchbegriff: %s\n", suchbegriff);
    suchbegriff[strcspn(suchbegriff, "\n")] = '\0';

    // Datei im Lesemodus öffnen
    FILE* datei = fopen(dateiname, "r");
    if (datei == NULL) {
        perror("Fehler beim Öffnen der Datei");  
        return -2;
    }

    printf("Die Datei wurde gefunden und die Suchtreffer werden nachfolgend ausgegeben:\n\n");

    // Zeilenweise die Datei lesen und nach dem Suchbegriff durchsuchen
    while (fgets(zeile, sizeof(zeile), datei)) {
        zeilennummer++;  // Aktuelle Zeilennummer erhöhen

        // Überprüfen, ob der Suchbegriff in der aktuellen Zeile enthalten ist
        if (strstr(zeile, suchbegriff)) {
            printf("Zeile %d: %s", zeilennummer, zeile);  // Treffer ausgeben
            treffer++;  // Trefferzähler erhöhen
        }
    }

    // Zusammenfassung der Ergebnisse
    if (treffer == 0) {
        printf("Keine Treffer für '%s' gefunden.\n", suchbegriff);
    }
    else {
        printf("\nInsgesamt wurden %d Treffer gefunden.\n", treffer);
    }

    fclose(datei);  // Datei schließen
    return 0;
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