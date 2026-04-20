export const isValidPrivateIPv4 = (value: string): boolean => {
  const parts = value.split(".");
  if (parts.length !== 4) return false;

  const nums = parts.map((p) => {
    if (!/^\d+$/.test(p)) return NaN;
    return Number(p);
  });

  if (nums.some((n) => Number.isNaN(n) || n < 0 || n > 255)) {
    return false;
  }

  const [a, b, c, d] = nums;

  if (a === 0) return false; // 0.0.0.0
  if (a === 127) return false; // loopback
  if (a === 169 && b === 254) return false; // link-local
  if (a >= 224) return false; // multicast + reserved
  if (a === 255 && b === 255 && c === 255 && d === 255) return false;

  if (a === 10) return true;
  if (a === 172 && b >= 16 && b <= 31) return true;
  if (a === 192 && b === 168) return true;

  return false;
};
