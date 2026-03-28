/*
 * ============================================================
 * Système de Surveillance d'une Échographie - CHU Mohammed VI
 * ============================================================
 * Fichier    : surveillance_temperature.ino
 * Description: Lecture de la température et humidité via DHT22,
 *              envoi des données au serveur Flask via HTTP POST
 * Matériel   : ESP32 + Capteur DHT22
 * Auteurs    : Safae El Attar, Jihane Bouras, Hafsa El Jaroudi
 * Encadrante : Madame Sanae Lamti
 * Stage      : CHU Mohammed VI Oujda — Avril/Mai 2024
 * ============================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ─── Configuration Wi-Fi ────────────────────────────────────
const char* ssid     = "VOTRE_SSID";       // Nom du réseau Wi-Fi
const char* password = "VOTRE_MOT_DE_PASSE"; // Mot de passe Wi-Fi

// ─── Configuration Serveur Flask ────────────────────────────
const char* serverName = "http://VOTRE_IP_SERVEUR:5000/post-data";
// Exemple : "http://192.168.1.100:5000/post-data"

// ─── Configuration Capteur DHT22 ────────────────────────────
#define DHTPIN  4          // Broche GPIO D4 de l'ESP32
#define DHTTYPE DHT22      // Type de capteur : DHT22
DHT dht(DHTPIN, DHTTYPE);

// ─── Variables globales ──────────────────────────────────────
float temp = 0.0;
float hum  = 0.0;

// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Démarrage du système de surveillance ===");

  // Connexion Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✓ Wi-Fi connecté !");
  Serial.print("✓ Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Initialisation du capteur DHT22
  dht.begin();
  Serial.println("✓ Capteur DHT22 initialisé.");
  Serial.println("============================================\n");
}

// ════════════════════════════════════════════════════════════
void loop() {
  // Lecture des données du capteur DHT22
  temp = dht.readTemperature();
  hum  = dht.readHumidity();

  // Affichage sur le moniteur série
  Serial.print("[DHT22] Température : ");
  Serial.print(temp);
  Serial.print(" °C | Humidité : ");
  Serial.print(hum);
  Serial.println(" %");

  // Vérification de la validité des données
  if (isnan(temp) || isnan(hum)) {
    Serial.println("[ERREUR] Lecture DHT22 invalide. Nouvelle tentative...");
    delay(2000);
    return;
  }

  // Envoi des données au serveur Flask via HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Formatage des données
    String httpRequestData = "temperature=" + String(temp)
                           + "&humidity="    + String(hum);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("[HTTP] Code réponse : ");
      Serial.println(httpResponseCode);
      Serial.print("[HTTP] Réponse serveur : ");
      Serial.println(response);
    } else {
      Serial.print("[ERREUR HTTP] Code : ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("[ERREUR] Wi-Fi déconnecté. Reconnexion...");
    WiFi.begin(ssid, password);
  }

  delay(2000); // Délai de 2 secondes entre chaque envoi
}
