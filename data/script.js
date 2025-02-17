document.getElementById("wifi-form").addEventListener("submit", function (event) {
	event.preventDefault();

	const ssid = document.getElementById("ssid").value;
	const password = document.getElementById("password").value;

	const formData = new URLSearchParams();
	formData.append("ssid", ssid);
	formData.append("password", password);

	fetch("/connect", {
		method: "POST",
		headers: { "Content-Type": "application/x-www-form-urlencoded" },
		body: formData.toString(), // Correctly formatted data
	})
		.then((response) => response.text())
		.then((data) => {
			document.getElementById("status").innerText = data;
			setTimeout(checkESP32IP, 8000);
		})
		.catch((error) => console.error("Error:", error));
});

function checkESP32IP() {
	fetch("/get-ip")
		.then((response) => response.text())
		.then((ip) => {
			if (ip.startsWith("⚠️")) {
				// Not connected yet, retry after 3 seconds
				setTimeout(checkESP32IP, 3000);
			} else {
				window.location.href = `http://${ip}`;
			}
		})
		.catch((error) => console.error("Error fetching IP:", error));
}

setInterval(
	() =>
		fetch("/temp")
			.then((res) => res.text())
			.then((temp) => (document.getElementById("temp").innerText = temp)),
	20000
);
