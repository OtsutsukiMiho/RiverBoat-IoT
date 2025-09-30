from flask import Flask, render_template_string, request, jsonify
import requests

app = Flask(__name__)

# ================= Arduino IoT Configuration ==================
CLIENT_ID = "lM2Ai4kLxcOP48BsVqwvxToABqGuDIGI"
CLIENT_SECRET = "WXMsuI2Xc2H0fxYSRafMXpbxX5W408ke3fX2HcMqngja9dD1AUKaTxw6xVO9i01a"
THING_ID = "d28b3c5e-9e12-4740-b6c0-a11ca6de204b"

# Map each button to its Arduino IoT Cloud Property ID
PROPERTY_IDS = {
    "W": "6d519ee6-4762-4c30-9445-83d652785341",
    "A": "65923002-6995-4d79-878a-73cd98b01118",
    "D": "e36f0146-01d8-43dc-a836-79fe03cf27a4",
    "S": "4265ace7-6c4d-4d4b-b870-3980764053ca",
}

# ================= Button State Storage ==================
button_states = {
    "W": False,
    "A": False,
    "S": False,
    "D": False
}

# ================= Arduino IoT Token & Property Update ==================
def get_access_token():
    url = "https://api2.arduino.cc/iot/v1/clients/token"
    data = {
        "grant_type": "client_credentials",
        "client_id": CLIENT_ID,
        "client_secret": CLIENT_SECRET,
        "audience": "https://api2.arduino.cc/iot"
    }
    r = requests.post(url, data=data)
    r.raise_for_status()
    return r.json()["access_token"]

def update_property(token, thing_id, property_id, value):
    url = f"https://api2.arduino.cc/iot/v2/things/{thing_id}/properties/{property_id}/publish"
    headers = {
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json"
    }
    data = {"value": value}
    r = requests.put(url, headers=headers, json=data)
    r.raise_for_status()
    return r.json()

# ================== HTML + JS Web UI ==================
HTML_PAGE = '''
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>RiverBoat</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="{{ url_for('static', filename='style.css') }}">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <div class="container">
        
        <div class="card">
            <h2>Movement</h2>
            <div id="movement" style="font-size: 22px; margin: 15px 0; color: #FFFFFF;">
                Loading movement...
            </div>
            <div class="row">
                <button id="W" class="control-button off" onclick="toggleButton('W')">W</button>
            </div>
            <div class="row">
                <button id="A" class="control-button off" onclick="toggleButton('A')">A</button>
                <button id="S" class="control-button off" onclick="toggleButton('S')">S</button>
                <button id="D" class="control-button off" onclick="toggleButton('D')">D</button>
            </div>
        </div>

        <div class="card">
            <h2>Temperature</h2>
            <div id="temperature" style="font-size: 22px; margin: 15px 0; color: #FFFFFF;">
                Loading temperature...
            </div>
            <canvas id="tempChart" width="350" height="200"></canvas>
        </div>
    </div>

    <script>
    
    let tempData = [];
    let tempLabels = [];

    const ctx = document.getElementById('tempChart').getContext('2d');
    const tempChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: tempLabels,
            datasets: [{
                label: 'Temperature (°C)',
                data: tempData,
                borderColor: '#FFFFFF',
                backgroundColor: 'rgba(255, 215, 0, 0.2)',
                borderWidth: 2,
                tension: 0.3,
                fill: true,
                pointRadius: 3
            }]
        },
        options: {
            responsive: true,
            plugins: {
                legend: { labels: { color: 'white' } }
            },
            scales: {
                x: { ticks: { color: 'white' } },
                y: { ticks: { color: 'white' } }
            }
        }
    });
    
        async function fetchData() {
            try {
                const res = await fetch("/status");
                const data = await res.json();

                document.getElementById("movement").textContent =
                    data.ui_movement || "No data";

                document.getElementById("temperature").textContent =
                    data.temperature || "No data";
            } catch (e) {
                document.getElementById("movement").textContent = "Error fetching data";
                document.getElementById("temperature").textContent = "Error fetching data";
            }
        }
    
        async function toggleButton(key) {
            await fetch("/toggle", {
                method: "POST",
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ key: key })
            });
        }
        
        setInterval(fetchData, 2000);

        window.onload = async () => {
            fetchData();
        };
    </script>
</body>
</html>
'''

@app.route("/")
def home():
    return render_template_string(HTML_PAGE)

@app.route("/toggle", methods=["POST"])
def toggle():
    data = request.get_json()
    key = data.get("key")
    
    if key in button_states:
        button_states[key] = not button_states[key]
        new_state = button_states[key]

        try:
            token = get_access_token()

            prop_id = PROPERTY_IDS.get(key)
            if prop_id:
                value_to_send = 1 if new_state else 0
                update_property(token, THING_ID, prop_id, value_to_send)
        except Exception as e:
            print(f"Error updating Arduino: {e}")
            return jsonify({"error": "Failed to update Arduino IoT"}), 500

        return jsonify({"key": key, "state": new_state})
    
    return jsonify({"error": "Invalid key"}), 400

@app.route("/status")
def get_status():
    try:
        token = get_access_token()
        url = f"https://api2.arduino.cc/iot/v2/things/{THING_ID}"
        headers = {"Authorization": f"Bearer {token}"}
        r = requests.get(url, headers=headers)
        r.raise_for_status()
        thing_data = r.json()

        properties = thing_data.get("properties", [])
        ui_movement_value = None
        temperature_value = None

        for prop in properties:
            if prop.get("name") == "ui_movement":
                ui_movement_value = prop.get("last_value")
            elif prop.get("name") == "temperature":
                temperature_value = prop.get("last_value")

        return jsonify({
            "ui_movement": ui_movement_value,
            "temperature": f"Temperature: {temperature_value}°C" if temperature_value is not None else "Temperature: ?°C"
        })

    except Exception as e:
        print(f"Error fetching status: {e}")
        return jsonify({"ui_movement": None, "temperature": "Temperature: ?°C"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)