const speakeasy = require("speakeasy");
const QRCode = require("qrcode");

try {
  const secret = speakeasy.generateSecret({ name: "MyApp 2FA" });
  console.log("Secret Base32:", secret.base32);

  QRCode.toDataURL(secret.otpauth_url, (err, data_url) => {
    if (err) throw new Error("Failed to generate QR Code: " + err.message);

    console.log("QR Code URL: ", data_url);
    console.log("Scan this QR code in Google Authenticator to add the 2FA account.");
  });
} catch (error) {
  console.error("Error during setup:", error.message);
}
