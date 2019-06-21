# RotaCam

Mit dem Projekt <strong>RotaCam</strong> soll eine drehbare WabCam erstellt werden. 
Basierend auf einem <em>ESP32CAM Modul</em> in Verbindung mit einem RC-Servo soll zunächst ein um 180° schwenkbares Kameramodul entstehen. Die Befehle zur Steuerung soll via Telnet gesendet werden können.


<strong>Motivation:</strong>  
Zwar gibt es eine Vielzahl an Webcams auf dem Markt, doch sind die allermeisten Modelle inzwischen nur mit einer herstellereigenen Cloud nutzbar. Eine Eigenheit die mir persönlich überhaupt nicht zusagt ;-)

<strong>Idee:</strong>  
Seit einiger Zeit ist nun das ESP32Cam-Modul verfügbar (z.B. bei Banggods)
Die kompakte Bauform ist ideal um sie, in Kombination mit einem RC-Servo, eine drehbare Webcam aufzubauen.

<strong>Umsetzung:</strong>  
Die eigentliche WebCam-Funktionalität ist bereits in diversen Repositories zur Verfügung gestellt worden. In der ersten Version greife ich auf SourceCode von RandomNerd zurück. An dieser Stelle vielen Dank an Rui Santos!

Die Steuerung des Servos erfolgt über eine Telnet-Verbindung. Nach Eingabe eines ‚?‘ erscheint die aktuelle Befehlsliste:

ESP32 - Cam Controller V0.1.1 (04-06-2019)  
______________________________________________________  
Befehle  
Bxxx    - Blitz (Intensität 0 < xxx < 100)  
Rxxx    - Rotation (Winkel 0 < xxx < 180  
q       - Telnet-Sitzung beenden  
l       - Zustandslog aktivieren/deaktivieren (für Debugging)  
i       - Initialisierung  
?       - Diese Liste  

<em>Hinweis:  
Die Kommando-Buchstaben (zur Steuerung des Moduls) sind in Großbuchstaben, gefolgt von einem Parameter, Sitzungsbefehle in Kleinbuchstaben einzugeben.</em>

Um eine solide Einheit zu bilden, braucht die ganze Geschichte natürlich ein kompaktes Gehäuse. Das STL-Files des Moduls liegt hier ab lässt sich mit einem 3D-Drucker leicht herstellen. Es dient lediglich als Basisträger. Eine Abdeckung die das Ganze wetterfest und damit für den Außeneinsatz tauglich machen wird, ist in Planung…

<strong>Technischen Details:</strong>  
Um den Aufbau möglichst schlank zu gestalten, also ohne zusätzliche Platine (die weiteren Platz gebraucht hätte) ist das Gehäuse so entworfen, dass es auf der einen Seite das ESP32CAM-Modul aufnimmt und den Servo auf der Anderen. 
Ein Durchbruch zu Vcc GND und Pin12 des ESP32CAM, in passender Größe für den Stecker des Servos ermöglichen die Verbindung von Beiden. 
Leider passt die Pinbelegung des Steckers am Kabel Servomotor ab Werk nicht zu der am ESP32 CAM. Darum muss Vcc und GND am Stecker vertauscht werden. Es ist jedoch recht einfach die Kunststoffnasen des Steckers leicht aufzubiegen. Dann können die beiden Metallstecker nach hinten herausgezogen und in der passenden Position wieder eingeschoben werden.
An die Anschlussleitung des Servus wird nun noch die Stromzuführung gelötet und durch die Bohrung des Gehäuses nach außen geführt. Der Rest des Kabels findet in der Aussparung des Gehäuses Platz.
Das Drehkreuz des Servos dient zur Befestigung des gesamten Moduls. Es sollte also ausreichend Anschlussleitung eingeplant werden, damit die Drehung um 180° möglich ist.
