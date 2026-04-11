export function generateUnicastMac() {
  const mac = Array.from({ length: 6 }, () => Math.floor(Math.random() * 256));

  mac[0] = (mac[0] & 0b11111110) | 0b00000010;

  return mac
    .map((b) => b.toString(16).padStart(2, "0"))
    .join(":")
    .toUpperCase();
}
