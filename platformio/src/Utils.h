#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "esp_system.h"
#include "Properties.h"

/*
Token format:
base64(payload).base64(signature)
*/

#define TOKEN_PAYLOAD_SIZE 16
#define TOKEN_SIGNATURE_SIZE 32

//--------------------------------------------------
// HMAC SHA256
//--------------------------------------------------

static void hmac_sha256(
    const uint8_t *key, size_t key_len,
    const uint8_t *input, size_t input_len,
    uint8_t *output)
{
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t *info;

  info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, info, 1);

  mbedtls_md_hmac_starts(&ctx, key, key_len);
  mbedtls_md_hmac_update(&ctx, input, input_len);
  mbedtls_md_hmac_finish(&ctx, output);

  mbedtls_md_free(&ctx);
}

////////////////////////////////////////////////////
// constant-time compare (prevents timing attacks)
////////////////////////////////////////////////////

static bool secure_compare(const uint8_t *a, const uint8_t *b, size_t len)
{
  uint8_t diff = 0;

  for (size_t i = 0; i < len; i++)
    diff |= a[i] ^ b[i];

  return diff == 0;
}

////////////////////////////////////////////////////
// TOKEN GENERATION
////////////////////////////////////////////////////

bool generateToken(char *out_token, size_t out_size)
{
  uint8_t payload[TOKEN_PAYLOAD_SIZE];
  uint8_t signature[TOKEN_SIGNATURE_SIZE];

  char payload_b64[64];
  char sig_b64[64];

  size_t payload_b64_len;
  size_t sig_b64_len;

  // random payload
  esp_fill_random(payload, sizeof(payload));

  // sign payload
  hmac_sha256(
      (const uint8_t *)jwt_secret,
      strlen(jwt_secret),
      payload,
      sizeof(payload),
      signature);

  // base64 payload
  if (mbedtls_base64_encode(
          (uint8_t *)payload_b64,
          sizeof(payload_b64),
          &payload_b64_len,
          payload,
          sizeof(payload)) != 0)
    return false;

  payload_b64[payload_b64_len] = '\0';

  // base64 signature
  if (mbedtls_base64_encode(
          (uint8_t *)sig_b64,
          sizeof(sig_b64),
          &sig_b64_len,
          signature,
          sizeof(signature)) != 0)
    return false;

  sig_b64[sig_b64_len] = '\0';

  // final token
  int written = snprintf(
      out_token,
      out_size,
      "%s.%s",
      payload_b64,
      sig_b64);

  if (written <= 0 || (size_t)written >= out_size)
    return false;

  return true;
}

////////////////////////////////////////////////////
// TOKEN VERIFICATION
////////////////////////////////////////////////////

bool verifyToken(const char *token)
{
  const char *sep = strchr(token, '.');
  if (!sep)
    return false;

  size_t payload_b64_len = sep - token;
  const char *sig_b64 = sep + 1;

  char payload_b64[64];
  char sig_b64_copy[64];

  if (payload_b64_len >= sizeof(payload_b64))
    return false;

  memcpy(payload_b64, token, payload_b64_len);
  payload_b64[payload_b64_len] = '\0';

  strncpy(sig_b64_copy, sig_b64, sizeof(sig_b64_copy) - 1);
  sig_b64_copy[sizeof(sig_b64_copy) - 1] = '\0';

  uint8_t payload[32];
  uint8_t sig[32];

  size_t payload_len;
  size_t sig_len;

  // decode payload
  if (mbedtls_base64_decode(
          payload,
          sizeof(payload),
          &payload_len,
          (const uint8_t *)payload_b64,
          payload_b64_len) != 0)
    return false;

  // decode signature
  if (mbedtls_base64_decode(
          sig,
          sizeof(sig),
          &sig_len,
          (const uint8_t *)sig_b64_copy,
          strlen(sig_b64_copy)) != 0)
    return false;

  if (sig_len != TOKEN_SIGNATURE_SIZE)
    return false;

  uint8_t expected_sig[TOKEN_SIGNATURE_SIZE];

  hmac_sha256(
      (const uint8_t *)jwt_secret,
      strlen(jwt_secret),
      payload,
      payload_len,
      expected_sig);

  return secure_compare(sig, expected_sig, TOKEN_SIGNATURE_SIZE);
}

bool checkAuthorization(WebServer &server)
{
  if (!server.hasHeader("Authorization"))
  {
    server.send(401);
    return false;
  }

  const String &header = server.header("Authorization");
  const char *auth = header.c_str();

  const char prefix[] = "Bearer ";

  if (strncmp(auth, prefix, sizeof(prefix) - 1) != 0)
  {
    server.send(401);
    return false;
  }

  const char *token = auth + sizeof(prefix) - 1;

  Serial.print("Extracted Token: ");
  Serial.println(token);

  if (!verifyToken(token))
  {
    server.send(401);
    return false;
  }

  return true;
}

static bool isHexChar(char c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

bool isValidEspNowKey(const char *key)
{
  if (!key)
    return false;

  size_t len = strlen(key);

  if (len != 32)
    return false;

  for (size_t i = 0; i < 32; i++)
  {
    if (!isHexChar(key[i]))
      return false;
  }

  return true;
}

bool isValidMacAddress(const char *mac)
{
  if (!mac)
    return false;

  if (strlen(mac) != 17)
    return false;

  for (int i = 0; i < 17; i++)
  {
    if ((i + 1) % 3 == 0)
    {
      if (mac[i] != ':')
        return false;
    }
    else
    {
      if (!isHexChar(mac[i]))
        return false;
    }
  }

  return true;
}

bool isUnicastMacAddress(const char *mac)
{
  // Validate format first
  if (!isValidMacAddress(mac))
    return false;

  // Reject broadcast address
  if (strcasecmp(mac, "FF:FF:FF:FF:FF:FF") == 0)
    return false;

  // Convert first byte
  char byteStr[3];
  byteStr[0] = mac[0];
  byteStr[1] = mac[1];
  byteStr[2] = '\0';

  uint8_t firstByte = strtoul(byteStr, nullptr, 16);

  // Check I/G bit (bit 0)
  // 0 = unicast
  // 1 = multicast
  if (firstByte & 0x01)
    return false;

  return true;
}

bool keyHexToBytes(const char *hex, uint8_t key[16])
{
  if (!isValidEspNowKey(hex))
    return false;

  for (int i = 0; i < 16; i++)
  {
    char tmp[3];
    tmp[0] = hex[i * 2];
    tmp[1] = hex[i * 2 + 1];
    tmp[2] = 0;

    key[i] = strtoul(tmp, NULL, 16);
  }

  return true;
}
bool keyBytesToHex(const uint8_t key[16], char hex[33])
{
  if (!key || !hex)
    return false;

  for (int i = 0; i < 16; i++)
  {
    sprintf(hex + (i * 2), "%02X", key[i]);
  }

  hex[32] = '\0';

  return true;
}

bool macStringToBytes(const char *macStr, uint8_t mac[6])
{
  if (!isValidMacAddress(macStr))
    return false;

  for (int i = 0; i < 6; i++)
  {
    mac[i] = strtoul(macStr + (i * 3), NULL, 16);
  }

  return true;
}
bool macBytesToString(const uint8_t mac[6], char macStr[18])
{
  if (!mac || !macStr)
    return false;

  sprintf(macStr,
          "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);

  return true;
}

bool hashPassword(
    const char *password,
    uint8_t salt[PASSWORD_SALT_SIZE],
    uint8_t hash[PASSWORD_HASH_SIZE])
{
  if (!password)
    return false;

  // Generate random salt
  esp_fill_random(salt, PASSWORD_SALT_SIZE);

  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);

  mbedtls_sha256_update(&ctx, salt, PASSWORD_SALT_SIZE);
  mbedtls_sha256_update(&ctx,
                        (const uint8_t *)password,
                        strlen(password));

  mbedtls_sha256_finish(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  return true;
}

bool verifyPassword(
    const char *password,
    const uint8_t salt[PASSWORD_SALT_SIZE],
    const uint8_t stored_hash[PASSWORD_HASH_SIZE])
{
  if (!password)
    return false;

  uint8_t computed_hash[PASSWORD_HASH_SIZE];

  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);

  mbedtls_sha256_update(&ctx, salt, PASSWORD_SALT_SIZE);
  mbedtls_sha256_update(&ctx,
                        (const uint8_t *)password,
                        strlen(password));

  mbedtls_sha256_finish(&ctx, computed_hash);
  mbedtls_sha256_free(&ctx);

  // constant-time comparison
  uint8_t diff = 0;
  for (int i = 0; i < PASSWORD_HASH_SIZE; i++)
    diff |= computed_hash[i] ^ stored_hash[i];

  return diff == 0;
}

void uuid(char uuid[37])
{
  uint8_t data[16];

  for (int i = 0; i < 16; i++)
    data[i] = esp_random() & 0xFF;

  // UUID v4 adjustments
  data[6] = (data[6] & 0x0F) | 0x40; // version 4
  data[8] = (data[8] & 0x3F) | 0x80; // variant

  // Changed %02X to %02x for lowercase output
  sprintf(uuid,
          "%02x%02x%02x%02x-"
          "%02x%02x-"
          "%02x%02x-"
          "%02x%02x-"
          "%02x%02x%02x%02x%02x%02x",
          data[0], data[1], data[2], data[3],
          data[4], data[5],
          data[6], data[7],
          data[8], data[9],
          data[10], data[11], data[12], data[13], data[14], data[15]);
}