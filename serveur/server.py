"""
============================================================
Système de Surveillance d'une Échographie - CHU Mohammed VI
============================================================
Fichier    : server.py
Description: Serveur Flask qui reçoit les données du capteur
             DHT22 via HTTP POST et les expose via HTTP GET
             pour l'application Android.
             Détermine l'état de l'échographe selon la température.
Auteurs    : Safae El Attar, Jihane Bouras, Hafsa El Jaroudi
Encadrante : Madame Sanae Lamti
Stage      : CHU Mohammed VI Oujda — Avril/Mai 2024
============================================================
Dépendances : pip install flask flask-cors
Lancement   : python server.py
============================================================
"""

from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)  # Autorise les requêtes cross-origin (application mobile)

# Dictionnaire global pour stocker les dernières données du capteur
latest_data = {
    "temperature": None,
    "humidity": None,
    "status": "Unknown"
}

# ─── Seuil de température pour déterminer l'état de l'échographe ─
TEMPERATURE_SEUIL = 34.0  # °C


# ════════════════════════════════════════════════════════════
# Route racine — vérification du fonctionnement du serveur
# ════════════════════════════════════════════════════════════
@app.route("/", methods=["GET"])
def home():
    return "DHT22 Sensor Data Server — CHU Mohammed VI", 200


# ════════════════════════════════════════════════════════════
# Route POST — réception des données de l'ESP32
# ════════════════════════════════════════════════════════════
@app.route("/post-data", methods=["POST"])
def post_data():
    """
    Reçoit les données température et humidité de l'ESP32.
    Détermine l'état de l'échographe :
      - température > 34°C → "In Use"
      - sinon             → "Not In Use"
    """
    try:
        temperature = request.form.get("temperature")
        humidity    = request.form.get("humidity")

        if temperature is None or humidity is None:
            return jsonify({"success": False, "error": "Données manquantes"}), 400

        temperature = float(temperature)
        humidity    = float(humidity)

        # Détermination de l'état de l'échographe
        if temperature > TEMPERATURE_SEUIL:
            statut = "In Use"
        else:
            statut = "Not In Use"

        # Mise à jour des données globales
        latest_data["temperature"] = temperature
        latest_data["humidity"]    = humidity
        latest_data["status"]      = statut

        print(f"[DATA] Temp: {temperature}°C | Hum: {humidity}% | État: {statut}")

        return jsonify({
            "success": True,
            "temperature": temperature,
            "humidity": humidity,
            "status": statut
        }), 200

    except Exception as e:
        print(f"[ERREUR] {str(e)}")
        return jsonify({"success": False, "error": str(e)}), 500


# ════════════════════════════════════════════════════════════
# Route GET — envoi des données à l'application Android
# ════════════════════════════════════════════════════════════
@app.route("/get-data", methods=["GET"])
def get_data():
    """
    Renvoie les dernières données de température, humidité
    et état de l'échographe au format JSON.
    """
    return jsonify({
        "temperature": latest_data["temperature"],
        "humidity":    latest_data["humidity"],
        "status":      latest_data["status"]
    }), 200


# ════════════════════════════════════════════════════════════
# Point d'entrée de l'application
# ════════════════════════════════════════════════════════════
if __name__ == "__main__":
    print("=== Serveur Flask — Surveillance Échographie ===")
    print(f"Seuil température : {TEMPERATURE_SEUIL}°C")
    print("Écoute sur : http://0.0.0.0:5000")
    print("================================================")
    app.run(host="0.0.0.0", port=5000, debug=True)
