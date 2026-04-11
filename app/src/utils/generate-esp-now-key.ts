export function generateEspNowKey() {
  const hex = "0123456789ABCDEF";
  let pmk = "";

  for (let i = 0; i < 32; i++) {
    pmk += hex[Math.floor(Math.random() * 16)];
  }

  return pmk;
}
