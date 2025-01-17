function fetchData() {
    fetch('/temperature')
        .then(response => response.json())
        .then(data => {
            // Mettre à jour les valeurs
            document.getElementById('temperature').textContent = data.temperature.toFixed(1);
            document.getElementById('humidity').textContent = data.humidity.toFixed(1);
            document.getElementById('doorStatus').textContent = data.doorState ? 'Ouverte' : 'Fermée';
            document.getElementById('doorImage').src = data.doorState ? 'open.png' : 'closed.png';
            document.getElementById('gas').textContent = data.gas.toFixed(1);  // Ajouter la mise à jour du gaz

            // Mettre à jour les jauges circulaires
            updateGauge('temperatureGauge', data.temperature, 50); // Température max à 50°C
            updateGauge('humidityGauge', data.humidity, 100);      // Humidité max à 100%
            updateGauge('gasGauge', data.gas, 1000);               // Gaz max à 1000 ppm
        })
        .catch(error => {
            console.error('Erreur lors de la récupération des données :', error);
        });
}

function updateGauge(elementId, value, max) {
    const gauge = document.getElementById(elementId);
    const percentage = Math.min(100, Math.max(0, (value / max) * 100));
    gauge.style.background = `conic-gradient(#0078d7 ${percentage}%, #eee ${percentage}%)`;
}

// Actualiser toutes les 5 secondes
setInterval(fetchData,500);

// Charger les données immédiatement
window.onload = fetchData;
