# JoniBOX
Basierend auf TonUINO, die DIY Musikbox (nicht nur) für Kinder

## Motivation zum Fork
Die Box sollte nur spielen, wenn eine Karte aufgelegt ist.
So ist die Bedeutung und Funktion der Karten auch für die ganz Kleinen zu verstehen.

Es gibt dazu einen [Pull Request](https://github.com/xfjx/TonUINO/pull/36) für den DEV Branch von [mcgreg1](https://github.com/mcgreg1).
Die Umsetzung habe ich zum Teil übernommen.
Der DEV Branch enthält viel Code für die Modifier Karten, daran habe ich zur Zeit kein Interesse.

Insgesamt fehlt mir im Code eine klare Linie, sodass ich auch einiges umstrukturieren möchte.
Erfahrungsgemäß erleichtert dies Erweiterungen enorm und natürlich auch spätere Änderungen, wenn man den Code einige Zeit nicht angeschaut hat.


# Bereits umgesetzte Änderungen gegenüber TonUINO
Stand 26.12.2019
- Wiedergabe hält an, wenn die Karte entfernt wird
- Dazu wird alle 150 ms der Kartenleser abgefragt
- Maßnahmen zur Verlängerung der Akku Laufzeit:
    - Nach 20 Minuten wird der DFPlayerMini in den Sleep Modus versetzt
    - Weiterhin erfolgt die Abfrage des Kartenlesers nur noch alle 1,5 s
- Refactoring der Software begonnen
- Abbruch des Anlernens durch Wegnehmen der Karte möglich

Stand 29.12.2019
- Weiteres Refactoring, Player und Kartenleser in separaten Files
- Ansagen beim Anlernen werden komplett ausgesprochen  

Stand 03.01.2020
- Buttons als Sound Board wenn Karte weg  

Stand 04.01.2020
- Separate Timeouts für MP3-Modul und Kartenleser
- Durch deaktivieren des Kartenlesers bei Pause und ohne Karte in Reichweite kann die Leistungsaufnahme um etwa 40 % reduziert werden. Intenso S10000 schaltet dann bereits ab.
- Optimierung der Lautstärke Regelung (zunehmende Geschwindigkeit der Änderung)

Stand 14.01.2020
- Zurück auf einen Timeout, Abschalten der Powerbank wird bewusst provoziert. Mit einem Taster (Öffner) lässt sich die Powerbank wieder reaktivieren.
- LED Ausgang für den Taster. An beim abspielen, pulsiert wenn der Timeout läuft, aus wenn aus.
- Track und Volume Funktion können per Config getauscht werden

Stand 29.01.2020
- Initialisierung des Zufallsgenerators repariert
- Party Modus (Zufallswiedergabe) verbessert: Merkt sich bereits gespielte Tracks und vermeidet Wiederholungen, bis jeder Track aus dem Ordner gespielt wurde

Stand 01.02.2020
- Hörspiel Modus (Zufallswiedergabe) verbessert: Spielt einen neuen Track, wenn der vorherige zu Ende war und man die Karte neu auflegt oder Pause drückt. Merkt sich bereits gespielte Tracks wie im Party Modus.
- Karte löschen komplizierter gemacht (Kindersicherung). Wenn pausiert (Karte liegt auf): Pause Button 8 s gedrückt halten, dann + Button kurz drücken. Wenn die Ansage kommt, Buttons los lassen.


# Geplante Änderungen
- Weitere Ansagen (z. B. beim Einschalten), ggf. andere Stimme?
- Abspielen ganzer Ordner über DFPlayerMini Methoden ohne nextTrack()
    - Kürzere Pausen zwischen den Tracks
    - Vereinfachung des Codes
- Weitere Maßnahmen zur Steigerung der Akku Laufzeit
    - ~~Kartenleser mit Interrupts, dann muss er nicht mehr gepollt werden
      (dazu muss das Software Serial für den MP3 Player auf andere Pins)~~
      Hat im Test keinen nennenswerten Effekt gehabt
    - Testen, wann die Powerbank geladen werden muss, wenn sie nur beim Abspielen eingeschaltet ist
