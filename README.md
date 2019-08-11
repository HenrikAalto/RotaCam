# RotaCam

Mit dem Projekt <strong>RotaCam</strong> soll eine drehbare WabCam erstellt werden. 
Basierend auf einem <em>ESP32CAM Modul</em> in Verbindung mit einem RC-Servo soll zunächst ein um 180° schwenkbares Kameramodul entstehen. Die Befehle zur Steuerung soll via Telnet gesendet werden können.


<strong>Motivation:</strong>  
Zwar gibt es eine Vielzahl an Webcams auf dem Markt, doch sind die allermeisten Modelle inzwischen nur mit einer herstellereigenen Cloud nutzbar. Eine Eigenheit die mir persönlich überhaupt nicht zusagt ;-)

<strong>Idee:</strong>  
Seit einiger Zeit ist nun das ESP32Cam-Modul verfügbar (z.B. bei <a href="https://www.banggood.com/Geekcreit-ESP32-CAM-WiFi-bluetooth-Camera-Module-Development-Board-ESP32-With-Camera-Module-OV2640-p-1394679.html?rmmds=search&cur_warehouse=CN">Banggods</a>)
Die kompakte Bauform ist ideal um sie, in Kombination mit einem <a href="https://www.banggood.com/4-X-Towerpro-MG90S-Metal-Gear-RC-Micro-Servo-p-961967.html?rmmds=search&cur_warehouse=CN">RC-Servo</a>, eine drehbare Webcam aufzubauen.

<strong>Umsetzung:</strong>  
Die eigentliche WebCam-Funktionalität ist bereits in diversen Repositories zur Verfügung gestellt worden. In der ersten Version greife ich auf SourceCode von <a href="https://randomnerdtutorials.com">RandomNerd</a> zurück. An dieser Stelle vielen Dank an Rui Santos!

Die Steuerung des Servos erfolgt über eine Telnet-Verbindung. Nach Eingabe eines ‚?‘ erscheint die aktuelle Befehlsliste:

<pre><code>ESP32 - Cam Controller V0.1.1 (04-06-2019)  
_________________________________________________________  
Befehle  
Bxxx    - Blitz (Intensität 0 < xxx < 100)  
Rxxx    - Rotation (Winkel 0 < xxx < 180  
q       - Telnet-Sitzung beenden  
l       - Zustandslog aktivieren/deaktivieren 
i       - Initialisierung  
?       - Diese Liste  
</code></pre>

<em>Hinweis:  
Die Kommando-Buchstaben (zur Steuerung des Moduls) sind in Großbuchstaben, gefolgt von einem Parameter, Sitzungsbefehle in Kleinbuchstaben einzugeben.</em>

Um eine solide Einheit zu bilden, braucht die ganze Geschichte natürlich ein kompaktes Gehäuse. Das STL-Files des Moduls liegt <a href="https://github.com/HenrikAalto/RotaCam/blob/master/Webcam-Träger%20V1.4.stl">hier</a> ab lässt sich mit einem 3D-Drucker leicht herstellen. Es dient lediglich als Basisträger. Eine Abdeckung die das Ganze wetterfest und damit für den Außeneinsatz tauglich machen wird, ist in Planung…

<strong>Technischen Details:</strong>  
Um den Aufbau möglichst schlank zu gestalten, also ohne zusätzliche Platine (die weiteren Platz gebraucht hätte), ist das Gehäuse so entworfen, dass es auf der einen Seite das ESP32CAM-Modul aufnimmt 
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/Gehaeuse_ESP-seitig.jpg)  
und den Servo 
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/Gehaeuse_Servo-seitig.jpg)  
auf der Anderen.  
Ein Durchbruch zu Vcc GND und Pin12 des ESP32CAM, in passender Größe für den Stecker des Servos ermöglichen die Verbindung von Beiden.  
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/ESP32-CAM.png)  
<em>(Danke an Jan Jezek für das 3D-Modell des ESP32-CAM-Moduls, es hat mit den Entwurf des Halters sehr erleichtert)</em>
Leider passt die Pinbelegung des Steckers am Kabel Servomotor, bei diesem Modell nicht ab Werk nicht zu der am ESP32 CAM. Darum muss Vcc und GND am Stecker vertauscht werden. Es ist jedoch recht einfach die Kunststoffnasen des Steckers leicht aufzubiegen. Dann können die beiden Metallstecker nach hinten herausgezogen und in der passenden Position wieder eingeschoben werden.
An die Anschlussleitung des Servos wird nun noch die Stromzuführung gelötet und durch die Bohrung des Gehäuses nach außen geführt. Der Rest des Kabels findet in der Aussparung des Gehäuses Platz.  
Das Drehkreuz des Servos wird nicht benötigt, da seine Achse später in die Grunplatte gesteckt und dort verschraubt wird. Es sollte also ausreichend Anschlussleitung eingeplant werden, damit die Drehung um 180° möglich ist.  
Dieses Foto zeigt nicht den letzten Designstand. Die aktuelle Version des Halters hat zusätzlich eine Öffnung um ggf. die Anschlussleitung einer externe Antenne nach außen führen zu können.
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/WebCam_offen.png)  
Um das Trägermodul im Gehäuse zu befestigen wird es mit einer Innensechskantschraube und eine Unterlagsscheibe fixiert. Das Gehäuse bietet auch die möglichkeit eine Kunststoffscheibe besser gesagt -folie einzuschieben, damit die Kameraoptik und die LED nicht der Witterung ausgesetzt sind. Um das erkennbar zu machen, habe ich die Scheibe für das Foto ein Stück weit herausgezogen. Eine Nut zum einschieben eines umlaufenden Dichtungsgummies (hier mit Flexifilament ausgeführt) ist ebenfalls vorhanden.  
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/WebCam.png)  
Hier das fertige, geschlossene Gehäuse.  

<strong>Hinweise:</strong>  
Bei dem von mir getesteten ESP32-Cam Modell war das WiFi-Signal sehr schwach. Entgegen meiner Erwartung war die interne Antenne nicht aktiv. Nach Verbinden mit einer externen WiFi-Antenne über den IPX-Stecker klappte alles prima und die Bildübertragung lief deutlich flüssiger (das war auch der Grund, das Design des Haltes nochmal anzupassen und eine Öffnung für das Antennenkaben hinzuzufügen). Auf der Seite von <a href="https://robotzero.one/esp32-cam-arduino-ide/">Robot Zero One</a> fand ich dann auch einen Hinweis, wie man erkennen kann welche Antennenkonfiguration einem geliefert wurde (On-Board vs IPEX Antenna). Bei einem anderem ESP32-Modul, das ich inzwischen erhalten habe, war (wie erwartet) die interne Antenne aktiv. Es war der gleiche Artikel, vom gleichen Liferanten - also besser immer erst mal unterm Mikroskop nachsehen:
So siehts aus wenn der 0 Ohm Widerstand den externen Antennenanschlussaktiviert:
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/ESP32%20mit%20aktiver%20externer%20Antenne.png)  
...und so wenn die interne Antenne aktiv ist:  
![Title](https://github.com/HenrikAalto/RotaCam/blob/master/ESP32%20mit%20aktiver%20interner%20Antenne.png)  
Umlöten ist bei dem SMD-Fisselkram schwierig. Da es sich aber um einen 0 Ohm Widerstand handeln soll (also eine simple Verbindung), könnte ein Brücke aus Lötizinn zu internen Antenne genügen. Natürlichch nur wenn zuvor das SMB-Bauteil ausgelötet wurde. Ich hab das aber nicht getestet sondern werde noch mit einigen externe Antennen testen. Wenn sich was brauchbares ergibt schreib ich's hier rein...  
Sehr hilfreich kann in dem Zusammenhang auch der unter 
https://github.com/SeeedDocument/forum_doc/raw/master/reg/ESP32_CAM_V1.6.pdf 
abliegende Schaltplan sein. Anhand dessen auch die etwas sonderbar anmutende Funktion des Pins4 (3,3V/5V) erklärlich wird. Es wirkt auf den ersten Blick unsinnig, dass im Plan der 3,3V- und der 5V-Anschluss jeweils über einen 0 Ohm Widerstand zusammengeschaltet werden. Schaut man sich das Board jedoch genauer an, erkennt man, dass der Hersteller nur einen (oder auch keinen) der 0 Ohm Widerstände eingelötet hat.  
Ursprünglich hatte ich überlegt dies zur Stromzuführung zu nutzen. Da dies trotz der oben erwähnten Erkentnisse jedoch nicht so richtig funktioniert wurde die Idee wieder verworfen. 
