/**
 * LIFT VEZÉRLŐ SZOFTVER - OKTATÁSI VERZIÓ
 * Architektúra: Eseményvezérelt Állapotgép (FSM)
 * Logika: Down-Collective (Lefelé gyűjtő prioritás)
 */

// --- Konfiguráció és Konstansok (Clean Code alapok) ---
#define CALLER_MCU_PREFIX    'A' // Külső hívógombok (emeletenként)
#define CONTROL_PANEL_PREFIX 'B' // Belső panel gombjai (kabinban)
#define DOOR_MCU_PREFIX      'C' // Ajtóvezérlés parancs/visszajelzés
#define MOTOR_MCU_PREFIX     'D' // Motorvezérlés parancs/visszajelzés
#define STATION_WAIT_TIME    3000 // Mennyit álljon a lift nyitott ajtóval (ms)
#define TOTAL_FLOORS         7    // Az épület szintjeinek száma

// Állapotok definiálása (Enumerációk) - Olvashatóbbá teszik a logikát, mint a számok
enum CabinState { CABIN_IDLE, CABIN_MOVING };
enum DoorState  { DOOR_IDLE, DOOR_OPENING, DOOR_CLOSING, DOOR_WAITING };

// --- Globális változók (A lift "világképe") ---
CabinState cabinState = CABIN_IDLE;
DoorState  doorState  = DOOR_IDLE;

// 2D Tömb a kéréseknek: [0] sor a külső hívások, [1] sor a belső hívások
bool requests[2][TOTAL_FLOORS]; 
int  currentFloor = 1;      // Hol van éppen a lift
bool movingUp     = true;   // Aktuális irány (felfelé = true)
bool motorACK = false;      // Megérkezett-e a motor visszaigazolása
bool doorACK = false;       // Befejeződött-e az ajtó mozgása
unsigned long timeOfArrival = 0; // Időbélyeg az ajtó várakoztatásához

// --- Segédfüggvények (Üzleti logika / Logic Helpers) ---

/**
 * Ellenőrzi, hogy van-e aktív hívás egy adott emeleti tartományban.
 * Ezzel látja a lift, hogy van-e még dolga "előtte" vagy "mögötte".
 */
bool hasRequest(int start, int end) {
    for (int i = max(1, start); i <= min(TOTAL_FLOORS, end); i++) {
        // Ha bármelyik típusú hívás (kinti/benti) él az adott emeleten
        if (requests[0][i - 1] || requests[1][i - 1]) return true;
    }
    return false;
}

/**
 * A lift legfontosabb döntése: Megálljunk-e ezen az emeleten?
 * Logika: Megállunk, ha valaki ki akar szállni (B), VAGY ha lefelé megyünk és be akarnak szállni (A),
 * VAGY ha nincs több dolgunk abban az irányban, de itt be akarnak szállni.
 */
bool shouldStop() {
    bool internalHere = requests[1][currentFloor - 1]; // külső itt
    bool externalHere = requests[0][currentFloor - 1]; // belső itt
    
    // Van-e még bármi abba az irányba, amerre nézünk?
    bool moreInDir = movingUp ? hasRequest(currentFloor + 1, TOTAL_FLOORS) : hasRequest(1, currentFloor - 1);
    
    // Megállunk, ha:
    // - Bárki ki akar szállni (B:x)
    // - Jó irányba megyünk és be akarnak szállni (A:x)
    // - Végállomáshoz értünk (nincs több kérés előttünk az adott irányban)
    return internalHere || 
           (!movingUp && externalHere) || // Lefelé gyűjtő (Down-collective)
           (!moreInDir && externalHere);  // Nincs már további hívás az adottirányban
}

// --- Fő vezérlő logika ---
void setup() { 
    Serial.begin(9600); // Soros kommunikáció indítása a Python szimulátorral
}

void loop() {
    // A négy fő funkcionális blokk folyamatos futtatása (nem blokkoló módon)
    processInput();     // Bemenetek fogadása
    
    updateCabinFSM();   // Kabin mozgásának kezelése
    updateDoorFSM();    // Ajtó ciklusának kezelése
    updateDirection();  // Irány meghatározása (stratégia)
}

/**
 * Soros porton érkező parancsok feldolgozása.
 * Toggle logikát használ: egy gombnyomás bekapcsol, a következő ki (ha még nem ért oda a lift).
 */
void processInput() {
    if (!Serial.available()) return;
    
    String input = Serial.readStringUntil('\n');
    char prefix = input[0];
    int floorNum = input.substring(2).toInt();
    int idx = floorNum - 1;

    // Külső hívás (A) vagy Belső hívás (B)
    if (prefix == CALLER_MCU_PREFIX || prefix == CONTROL_PANEL_PREFIX) {
        int reqType = (prefix == CALLER_MCU_PREFIX) ? 0 : 1;
        requests[reqType][idx] = true; // Állapot megfordítása (Toggle)
        
        // Visszajelzés a Pythonnak a gomb színezéséhez (0 = kikapcsolás, floorNum = bekapcsolás)
        Serial.print(prefix); 
        Serial.print(":"); 
        Serial.println(requests[reqType][idx] ? String(floorNum) : "0"); 
    }
    
    // Nyugtázó jelek fogadása (ACK)
    if (prefix == DOOR_MCU_PREFIX)  doorACK = true;
    if (prefix == MOTOR_MCU_PREFIX) { 
        motorACK = true; 
        currentFloor = floorNum; // Pozíció frissítése a motor visszajelzése alapján
    }
}

/**
 * Intelligens irányváltás: A lift tartja az irányt, amíg van előtte dolga.
 * Ha elfogy, megfordul, ha van dolga a háta mögött.
 */
void updateDirection() {
    // Csak akkor foglalkozunk irányváltással, ha a lift áll és az ajtó sem mozog
    if (cabinState != CABIN_IDLE || doorState != DOOR_IDLE) return;

    // Megvárjuk, amíg az updateCabinFSM megállítja a liftet és törli a hívást.
    if (requests[0][currentFloor - 1] || requests[1][currentFloor - 1]) return;
    
    if (hasRequest(1, TOTAL_FLOORS)) {
        // van-e még dolgunk előre?
        // felfelé megyünk ? ha igen->van még kérés felfelé: ha nem-> van még kérés lefelé
        bool moreAhead = movingUp ? hasRequest(currentFloor + 1, TOTAL_FLOORS) : hasRequest(1, currentFloor - 1);
        
        if (!moreAhead) {
            // Csak akkor nézzük a hátunk mögé, ha előttünk már nincs semmi
            bool moreBehind = movingUp ? hasRequest(1, currentFloor - 1) : hasRequest(currentFloor + 1, TOTAL_FLOORS);
            if (moreBehind) {
                movingUp = !movingUp; 
                // Itt érdemes egy debug üzenetet küldeni (opcionális)
                // Serial.println("DEBUG: Irányváltás");
            }
        }
    }
}

/**
 * Kabin Állapotgép: Kezeli az indulást, mozgást és a megállás kezdeményezését.
 */
void updateCabinFSM() {
    // Biztonsági retesz: Ha az ajtó nincs nyugalomban, a kabin nem indulhat/módosulhat
    if (doorState != DOOR_IDLE) return;

    if (cabinState == CABIN_IDLE) {
        // IDLE helyzetben döntünk: Megállunk (nyitunk) vagy Megyünk tovább
        if (shouldStop()) {
            // Megállás: Töröljük a hívásokat ezen a szinten
            requests[0][currentFloor - 1] = false;
            requests[1][currentFloor - 1] = false;
            
            // Értesítjük a Pythont, hogy oltsa el a gombok fényeit (0 küldésével)
            Serial.println("A:0"); 
            Serial.println("B:0"); 
            
            // Ajtónyitási parancs küldése
            Serial.println("C:1");
            doorACK = false;
            doorState = DOOR_OPENING;
        } 
        // vagy van-e dolgunk bárhol máshol?
        else if (hasRequest(1, TOTAL_FLOORS)) {
            // Van hívás, de nem itt: Indulás a következő szintre
            int next = movingUp ? currentFloor + 1 : currentFloor - 1;
            Serial.print("D:"); 
            Serial.println(next);
            motorACK = false;
            cabinState = CABIN_MOVING;
        }
    } 
    else if (cabinState == CABIN_MOVING && motorACK) {
        // Ha mozgásban voltunk és megkaptuk a motor jelzését az érkezésről
        cabinState = CABIN_IDLE;
    }
}

/**
 * Ajtó Állapotgép: Sorrendi folyamat: NYIT -> VÁR -> ZÁR.
 * Itt használjuk a millis() függvényt a delay() helyett, hogy a lift maradék része ne fagyjon le.
 */
void updateDoorFSM() {
    switch (doorState) {
        case DOOR_IDLE:
          break;
          
        case DOOR_OPENING: 
            if (doorACK) { 
                timeOfArrival = millis(); // Megérkezés időpontjának rögzítése
                doorState = DOOR_WAITING; 
            } 
            break;
            
        case DOOR_WAITING: 
            // Csak akkor megyünk tovább, ha letelt a várakozási idő (nem blokkoló időzítés)
            if (millis() - timeOfArrival >= STATION_WAIT_TIME) { 
                Serial.println("C:0"); // Ajtózárás parancs küldése
                doorACK = false; 
                doorState = DOOR_CLOSING; 
            } 
            break;
            
        case DOOR_CLOSING: 
            if (doorACK) { 
                doorState = DOOR_IDLE; // Ajtó újra nyugalomban, mehet tovább a kabin
            } 
            break;
    }
}
