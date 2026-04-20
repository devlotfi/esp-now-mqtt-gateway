export const isUnicastMac = (mac: string) => {
  if (!mac) return false;

  const normalized = mac.toUpperCase();
  const parts = normalized.split(":");

  if (parts.length !== 6) return false;

  const bytes = parts.map((p) => parseInt(p, 16));
  if (bytes.some((b) => isNaN(b))) return false;

  const firstByte = bytes[0];

  // --- Special cases ---
  const isBroadcast = normalized === "FF:FF:FF:FF:FF:FF";
  const isZero = normalized === "00:00:00:00:00:00";

  // Bit checks
  const isMulticast = (firstByte & 1) === 1; // LSB = 1

  return !isBroadcast && !isZero && !isMulticast;
};
