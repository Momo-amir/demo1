document.addEventListener("DOMContentLoaded", function () {
	fetch("header.html")
		.then((response) => response.text())
		.then((html) => {
			document.getElementById("hamburgerMenu").innerHTML = html;
			// Now that the header is loaded, attach the menu toggle listener.
			const menuBtn = document.getElementById("menuBtn");
			if (menuBtn) {
				menuBtn.addEventListener("click", function () {
					const menu = document.getElementById("navMenu");
					if (menu) {
						menuBtn.classList.toggle("active");
						menu.classList.toggle("active");
					}
				});
			}
		})
		.catch((error) => console.error("Error loading header:", error));

	let tempChart = null;

	// Try instantiating the chart. If it fails, we display an error and continue.
	try {
		const canvas = document.getElementById("tempChart");
		if (canvas) {
			const ctx = canvas.getContext("2d");
			if (typeof Chart !== "undefined") {
				tempChart = new Chart(ctx, {
					type: "bar",
					data: {
						labels: [],
						datasets: [
							{
								label: "Temperature (°C)",
								data: [],

								backgroundColor: "#007bff",
							},
						],
					},
					options: {
						scales: {
							x: { title: { display: true, text: "Time" } },
							y: { title: { display: true, text: "Temperature (°C)" } },
						},
					},
				});
			} else {
				throw new Error("Chart is not defined.");
			}
		} else {
			console.warn("No canvas element found with id 'tempChart'.");
		}
	} catch (err) {
		console.error("Chart initialization failed - running without chart:", err);
	}

	// Load historical data
	fetch("/tempdata")
		.then((response) => response.text())
		.then((data) => {
			data.split("\n").forEach((line) => {
				if (line.trim()) {
					const parts = line.split(",");
					if (parts.length >= 2) {
						const [timestamp, temp] = parts;

						const timeLabel = new Date(timestamp).toLocaleTimeString();

						if (tempChart) {
							tempChart.data.labels.push(timeLabel);
							tempChart.data.datasets[0].data.push(parseFloat(temp));
						}
					}
				}
			});

			if (tempChart) {
				tempChart.update();
			}
		})
		.catch((err) => console.error("Error loading historical data:", err));

	// Open the WebSocket connection
	const webSocket = new WebSocket(`ws://${location.host}/ws`);

	webSocket.onmessage = function (event) {
		try {
			const data = JSON.parse(event.data);
			const timeLabel = new Date(data.timestamp).toLocaleTimeString();
			const temp = parseFloat(data.temp);

			if (tempChart) {
				tempChart.data.labels.push(timeLabel);
				tempChart.data.datasets[0].data.push(temp);
				tempChart.update();
			}

			const tempEl = document.getElementById("temp");
			if (tempEl) tempEl.innerText = temp.toFixed(2);
		} catch (error) {
			console.error("Error processing WebSocket message:", error);
		}
	};

	// WiFi configuration form submission handler
	document.getElementById("wifi-form").addEventListener("submit", function (event) {
		event.preventDefault();
		const ssid = document.getElementById("ssid").value;
		const password = document.getElementById("password").value;
		const formData = new URLSearchParams();
		formData.append("ssid", ssid);
		formData.append("password", password);

		fetch("/connect", {
			method: "POST",
			headers: {
				"Content-Type": "application/x-www-form-urlencoded",
			},
			body: formData.toString(),
		})
			.then((response) => response.text())
			.then((data) => {
				document.getElementById("status").innerText = data;
			})
			.catch((error) => console.error("Error submitting WiFi form:", error));
	});
});
