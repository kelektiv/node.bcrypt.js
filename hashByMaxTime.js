// lib/hashByMaxTime.js
const crypto = require('crypto');

/**
 * Hash a numeric "max time allotted" into a stable bucket index.
 *
 * Purpose:
 * - Deterministically map a max-time number to a bucket for grouping/sharding.
 * - Stable across runs (uses sha256) and safe for variable numeric input.
 *
 * @param {number|string} maxTime - the max time allotted (seconds or any numeric-like value)
 * @param {object} [opts]
 * @param {number} [opts.buckets=1024] - number of buckets (must be > 0)
 * @returns {number} bucket index in range [0, buckets-1]
 *
 * Examples:
 *   hashByMaxTime(30)       // => 0..1023
 *   hashByMaxTime('30')     // => same as above
 *   hashByMaxTime(null)     // => 0 (defaults)
 */
function hashByMaxTime(maxTime, opts = {}) {
  const buckets = Number.isInteger(opts.buckets) && opts.buckets > 0 ? opts.buckets : 1024;

  // Normalize input: empty or invalid -> '0'
  let normalized;
  if (maxTime === undefined || maxTime === null || maxTime === '') {
    normalized = '0';
  } else {
    // If numeric-like, convert to canonical integer string (rounding to integer seconds)
    const n = Number(maxTime);
    if (!Number.isFinite(n)) {
      // fallback to string representation
      normalized = String(maxTime);
    } else {
      // Use integer seconds - floor to keep stable bucket for close floats
      normalized = String(Math.floor(n));
    }
  }

  // Compute SHA-256 digest, then convert first 8 bytes to a Number (safe up to 2^53)
  const hash = crypto.createHash('sha256').update(normalized).digest();

  // read first 6-7 bytes to get a 48/56-bit integer safely
  // we'll use 6 bytes (48 bits) which is safely less than JS Number precision limit
  let int = 0;
  for (let i = 0; i < 6; i++) {
    int = (int << 8) + hash[i];
  }

  return int % buckets;
}

module.exports = hashByMaxTime;
