/*
  MKR1000 - Gombmátrix Web Szerver (Access Point mód)
  A hivatalos tutorial alapján átalakítva a Digital Twin projekthez.
*/

#include <SPI.h>
#include <WiFi101.h>
#include "index_page.h" // Ez tartalmazza a vizuális PCB HTML-t

// --- PIN DEFINÍCIÓK (D2-D9 tartomány a Serial stabilitásért) ---
#define R1 2
#define R2 3
#define R3 4
#define R4 5
#define C1 6
#define C2 7
#define C3 8
#define C4 9

// Wi-Fi Access Point beállítások
char ssid[] = "MKR1000_Matrix_Szerver";
char pass[] = "laborjelszo2024";
int status = WL_IDLE_STATUS;
WiFiServer server(80);

int utolsoGomb = 0; // Itt tároljuk az aktív gombot (1-16)

const int sorok[] = {R1, R2, R3, R4};
const int oszlopok[] = {C1, C2, C3, C4};

void setup() {
  Serial.begin(9600);

  // Mátrix lábak inicializálása
  for (int i = 0; i < 4; i++) {
    pinMode(sorok[i], OUTPUT);
    digitalWrite(sorok[i], HIGH);
    pinMode(oszlopok[i], INPUT_PULLUP);
  }

  // Ellenőrizzük a Wi-Fi shield jelenlétét
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield nem található!");
    while (true);
  }

  // Access Point indítása
  Serial.print("Hálózat indítása: ");
  Serial.println(ssid);
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("AP indítása sikertelen!");
    while (true);
  }

  server.begin();
  printWiFiStatus();
}

void loop() {
  // 1. Mátrix beolvasása (Folyamatos pásztázás)
  int lenyomott = olvasMatrix();
  if (lenyomott > 0) {
    utolsoGomb = lenyomott;
    Serial.print("Gombnyomás: ");
    Serial.println(utolsoGomb);
  }

  // 2. Kliens kezelése (mint a tutorialban)
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Új kliens csatlakozott");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Itt küldjük a választ a kliensnek
            szolgaltatWeboldal(client);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Kliens lecsatlakozott");
  }
}

// --- FÜGGVÉNYEK ---

int olvasMatrix() {
  for (int s = 0; s < 4; s++) {
    digitalWrite(sorok[s], LOW); // Adott sor lehúzása
    for (int o = 0; o < 4; o++) {
      if (digitalRead(oszlopok[o]) == LOW) { // Ha az oszlop is LOW
        delay(50); // Pergésmentesítés
        while(digitalRead(oszlopok[o]) == LOW); // Várunk az elengedésig
        digitalWrite(sorok[s], HIGH);
        return (s * 4) + (o + 1); // 1-16 index
      }
    }
    digitalWrite(sorok[s], HIGH);
  }
  return 0;
}

void szolgaltatWeboldal(WiFiClient client) {
  // HTTP Fejlécek küldése
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Refresh: 1"); // 1 mp-enkénti auto-frissítés
  client.println("Connection: close");
  client.println();

  // HTML tartalom összeállítása a sablon alapján
  String response = String(INDEX_HTML);
  for (int i = 1; i <= 16; i++) {
    String placeholder = "@C" + String(i) + "@";
    if (utolsoGomb == i) {
      response.replace(placeholder, "active");
    } else {
      response.replace(placeholder, "");
    }
  }

  // Teljes oldal elküldése
  client.print(response);
  client.println();
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("Weboldal elérhető itt: http://");
  Serial.println(ip);
}