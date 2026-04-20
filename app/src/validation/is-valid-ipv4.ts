export const isValidIPv4 = (value: string): boolean => {
  const parts = value.split(".");
  if (parts.length !== 4) return false;

  return parts.every((p) => {
    if (!/^\d+$/.test(p)) return false;
    const n = Number(p);
    return n >= 0 && n <= 255;
  });
};
