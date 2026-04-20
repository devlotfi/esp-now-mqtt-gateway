import { isValidIPv4 } from "./is-valid-ipv4";

export const isValidSubnetMask = (mask?: string): boolean => {
  if (!mask || !isValidIPv4(mask)) return false;

  const parts = mask.split(".").map((p) => Number(p));

  // Ensure all are numbers
  if (parts.some((n) => Number.isNaN(n))) return false;

  const num: number =
    ((parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3]) >>> 0;

  const inverted: number = ~num >>> 0;

  return ((inverted + 1) & inverted) === 0;
};
