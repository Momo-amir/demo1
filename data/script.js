document.getElementById("wifi-form").addEventListener("submit", function (event) {
	event.preventDefault();

	const ssid = document.getElementById("ssid").value;
	const password = document.getElementById("password").value;

	fetch("/connect", {
		method: "POST",
		headers: { "Content-Type": "application/json" },
		body: JSON.stringify({ ssid, password }),
	})
		.then((response) => response.text())
		.then((data) => {
			document.getElementById("status").innerText = data;
		});
});
