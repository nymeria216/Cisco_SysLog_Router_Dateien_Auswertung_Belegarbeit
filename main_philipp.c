// Programm Beleg Programmierung I


// Präprozessoranweisungen

#define _CRT_SECURE_NO_WARNINGS     // Unterdrückt Warnungen für "unsichere" Funktionen wie fopen() in Visual Studio
#include <stdio.h>                  // Einbindung der Standard-Ein-/Ausgabe-Bibliothek
#include <string.h>                 // Einbindung der String-Bibliothek für Funktionen zur Zeichenkettenverarbeitung
// #include <windows.h>             // Für SetConsoleOutkputCP() und CP_UTF8 und Windows ONLY



// zunächst werden die Funktionen definiert, die für das Programm benötigt werden

// 1. Funktion zur Eingabe des Dateipfads durch den Benutzer

void Dateipfad_Eingabe(char* dateiname, size_t groesse) {
    printf("\nBitte geben Sie den korrekten Dateipfad ein: \n\n");
    if (fgets(dateiname, groesse, stdin) != NULL) {
        dateiname[strcspn(dateiname, "\n")] = '\0';
    }
}


// 2. Funktion zur Bestätigung der Eingabe des Dateipfads

void Bestaetigung_des_Dateipfads(char* dateiname) {
    char janein;
    int versuch = 0;

    do {
        // Fragt den Benutzer, ob Pfad und Dateiname korrekt sind
        printf("\nIst der Pfad und Name der Datei korrekt (y/n)?\n\n");
        scanf("%c", &janein);
        while (getchar() != '\n');

        // Überprüfen der Eingabe und Ausgeben entsprechender Ja/Nein-Antworten
        if (janein == 'y' || janein == 'Y') {
            break;
        }
        else if (janein == 'n' || janein == 'N') {
            Dateipfad_Eingabe(dateiname, sizeof(dateiname));       // bei "Nein" wird die Funktion zur Eingabe des Dateipfads erneut aufgerufen
        }
        else {
            // Ungültige Eingabe
            printf("Ungültige Eingabe. Bitte geben Sie 'y' für Ja oder 'n' für Nein ein.\n");
            versuch++;
        }

        // Zählt die verbleibenden Versuche
        if (versuch > 0 && versuch < 3) {
            printf("Noch %d Versuch(e) übrig\n", 3 - versuch);
        }

        // Programm beenden nach zu vielen Fehlversuchen
        if (versuch >= 3) {
            printf("Zu viele ungültige Versuche. Das Programm wird beendet.\n");
        }

    } while (1);
}


// Hier folgt das Hauptprogramm

int main() {
    // SetConsoleOutputCP(CP_UTF8);  // Konsole auf UTF-8 stellen und Windows ONLY
    char dateiname[256];      // Variable zum Speichern des Dateipfads
    char suchbegriff[256];    // Variable zum Speichern des Suchbegriffs
    char zeile[1024];         // Puffer zum Einlesen einer Zeile aus der Datei
    int zeilennummer = 0;     // Zähler für die aktuelle Zeilennummer
    int treffer = 0;          // Zählt die Anzahl gefundener Treffer

    printf("Dies ist ein Programm zur Auswertung eines CISCO-Logfiles.\n\n");       // Kurze Erklärung was die Aufgabe des Programms ist

    Dateipfad_Eingabe(dateiname, sizeof(dateiname));        // Aufrufen der zuvor definierten Funktion "Dateipfad_Eingabe"

    Bestaetigung_des_Dateipfads(dateiname);                  // Aufrufen der zuvor definierten Funktion "Bestätigung_des_Dateipfads"

    // Filterliste
    // printf("\n Wonach soll gefiltert werden? Wähle eine Zahl aus. \n");
    // printf("1: \n");

    //Eingabe des Suchbegriffs durch Benutzer
    printf("\n\nBitte geben Sie den Suchbegriff ein: ");
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