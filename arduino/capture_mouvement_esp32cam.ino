/*
 * ============================================================
 * Système de Surveillance d'une Échographie - CHU Mohammed VI
 * ============================================================
 * Fichier    : capture_mouvement_esp32cam.ino
 * Description: Détection de mouvement via capteur PIR HC-SR501,
 *              capture photo avec ESP32-CAM et sauvegarde sur SD
 * Matériel   : ESP32-CAM + Capteur PIR HC-SR501 + Carte SD
 * Câblage    :
 *   PIR HC-SR501 → GPIO 13 (signal), 5V (VCC), GND (GND)
 *   Carte SD    → intégrée à l'ESP32-CAM (SD_MMC)
 * Auteurs    : Safae El Attar, Jihane Bouras, Hafsa El Jaroudi
 * Encadrante : Madame Sanae Lamti
 * Stage      : CHU Mohammed VI Oujda — Avril/Mai 2024
 * ============================================================
 */

#include "esp_camera.h"
#include "SD_MMC.h"
#include "FS.h"

// ─── Configuration des broches ESP32-CAM (modèle AI-Thinker) ─
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ─── Configuration Capteur PIR ───────────────────────────────
#define brochePIR 13      // GPIO 13 → sortie du capteur PIR

// ─── Compteur de fichiers photos ─────────────────────────────
int numero_fichier = 0;

// ════════════════════════════════════════════════════════════
// Initialisation de la caméra
// ════════════════════════════════════════════════════════════
bool initialiser_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERREUR] Initialisation caméra : 0x%x\n", err);
    return false;
  }
  return true;
}

// ════════════════════════════════════════════════════════════
// Capture et sauvegarde d'une photo sur la carte SD
// ════════════════════════════════════════════════════════════
void enregistrer_photo() {
  // Capture de l'image
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("[ERREUR] Échec de la capture photo.");
    return;
  }

  // Génération du nom de fichier unique
  numero_fichier++;
  char adresse[20];
  sprintf(adresse, "/%d.jpg", numero_fichier);

  // Sauvegarde sur la carte SD
  File file = SD_MMC.open(adresse, FILE_WRITE);
  if (!file) {
    Serial.println("[ERREUR] Impossible d'ouvrir le fichier sur la SD.");
    esp_camera_fb_return(fb);
    return;
  }

  file.write(fb->buf, fb->len);
  file.close();

  Serial.print("[PHOTO] Photo n°");
  Serial.print(numero_fichier);
  Serial.print(" enregistrée → ");
  Serial.println(adresse);

  // Libération du tampon image
  esp_camera_fb_return(fb);
}

// ════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Démarrage ESP32-CAM + Capteur PIR ===");

  // Initialisation de la caméra
  if (!initialiser_camera()) {
    Serial.println("[ERREUR] Caméra non initialisée. Arrêt.");
    while (true); // Arrêt du programme
  }
  Serial.println("✓ Caméra initialisée.");

  // Initialisation de la carte SD
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("[ERREUR] Carte SD non détectée.");
    while (true); // Arrêt du programme
  }
  Serial.println("✓ Carte SD montée.");

  // Configuration de la broche PIR en entrée
  pinMode(brochePIR, INPUT);
  Serial.println("✓ Capteur PIR prêt.");
  Serial.println("=========================================\n");
  Serial.println("En attente de détection de mouvement...");
}

// ════════════════════════════════════════════════════════════
void loop() {
  // Lecture de l'état du capteur PIR
  int etatPIR = digitalRead(brochePIR);

  if (etatPIR == HIGH) {
    Serial.println("[PIR] Mouvement détecté ! Capture en cours...");
    enregistrer_photo();
    delay(300); // Délai anti-rafale
  }
}
