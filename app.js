const express = require("express");
const speakeasy = require("speakeasy");
const qrcode = require("qrcode");

const app = express();
const port = 3000;

// Middleware for parsing JSON and serving static files
app.use(express.json());
app.use(express.urlencoded({ extended: true }));
app.use(express.static("public")); // Serve static files from the public folder

// Route to serve the home page
app.get("/", (req, res) => {
  res.sendFile(__dirname + "/index.html");
});

// Route to generate a 2FA secret and QR Code
app.get("/generate", async (req, res) => {
  try {
    const secret = speakeasy.generateSecret({ length: 20 });
    const qrCodeDataURL = await qrcode.toDataURL(secret.otpauth_url);

    res.json({
      secret: secret.base32,
      qrCode: qrCodeDataURL,
    });
  } catch (error) {
    console.error("Error generating QR Code:", error.message);
    res.status(500).send("Failed to generate QR Code");
  }
});

// Route to verify the 2FA token
app.post("/verify", (req, res) => {
  try {
    const { token, secret } = req.body;

    const verified = speakeasy.totp.verify({
      secret: secret,
      encoding: "base32",
      token: token,
      window: 1, // Allows slight time drift
    });

    if (verified) {
      res.json({ valid: true, message: "Token is valid!" });
    } else {
      res.json({ valid: false, message: "Token is invalid." });
    }
  } catch (error) {
    console.error("Error verifying token:", error.message);
    res.status(500).send("Failed to verify token");
  }
});

// Start the server
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
