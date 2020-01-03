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

# Geplante Änderungen
- Abspielen ganzer Ordner über DFPlayerMini Methoden ohne nextTrack(),
  falls möglich auch Shuffle über das MP3 Modul
    - Kürzere Pausen zwischen den Tracks
    - Vereinfachung des Codes
- Weitere Ansagen (z. B. beim Einschalten), ggf. andere Stimme?
- Fun Sounds wenn ohne Karte die Buttons gedrückt werden
- Weitere Maßnahmen zur Steigerung der Akku Laufzeit
    - Kartenleser mit Interrupts, dann muss er nicht mehr gepollt werden
      (dazu muss das Software Serial für den MP3 Player auf andere Pins)
