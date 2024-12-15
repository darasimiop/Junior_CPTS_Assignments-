const speakeasy = require("speakeasy");

try {
  const base32secret = "GBWTIUBQFYWGMLRJMZXUORZQONKV2ODGGNGTKNCRJYSTI5KKJM7Q";
  const userToken = "605646";

  const verified = speakeasy.totp.verify({
    secret: base32secret,
    encoding: "base32",
    token: userToken,
  });

  console.log("Token is valid:", verified);
} catch (error) {
  console.error("Error during verification:", error.message);
}
