#pragma once

namespace AngleUtils {

inline float normalizeAz(float az) {
  while (az < 0.0f) az += 360.0f;
  while (az >= 360.0f) az -= 360.0f;
  return az;
}

inline float clampEl(float el) {
  if (el < 0.0f) return 0.0f;
  if (el > 90.0f) return 90.0f;
  return el;
}

inline float shortestAzDelta(float target, float current) {
  float d = target - current;
  while (d > 180.0f) d -= 360.0f;
  while (d < -180.0f) d += 360.0f;
  return d;
}

}  // namespace AngleUtils
