
float signNotZero(in float k) {
	return (k >= 0.0) ? 1.0 : -1.0;
}


float2 signNotZero(in float2 v) {
	return float2(signNotZero(v.x), signNotZero(v.y));
}


/** Assumes that v is a unit vector. The result is an octahedral vector on the [-1, +1] square. */
float2 octEncode(in float3 v) {
	float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
	float2 result = v.xz * (1.0 / l1norm);
	if (v.y < 0.0) {
		result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
	}
	return result;
}


/** Returns a unit vector. Argument o is an octahedral vector packed via octEncode,
on the [-1, +1] square*/
float3 octDecode(float2 o) {
	float3 v = float3(o.x, 1.0 - abs(o.x) - abs(o.y), o.y);
	if (v.y < 0.0) {
		v.xz = (1.0 - abs(v.zx)) * signNotZero(v.xz);
	}
	return normalize(v);
}

/* The caller should store the return value into a GL_RGB8 texture
or attribute without modification. */
float3 snorm12x2_to_unorm8x3(float2 f)
{
    float2 u = float2(round(clamp(f, -1.0, 1.0) * 2047 + 2047));
    float t = floor(u.y / 256.0);
    // If storing to GL_RGB8UI, omit the final division
    return floor(float3(u.x / 16.0,
    frac(u.x / 16.0) * 256.0 + t,
    u.y - t * 256.0)) / 255.0;
}

float2 unorm8x3_to_snorm12x2(float3 u)
{
    u *= 255.0;
    u.y *= (1.0 / 16.0);
    float2 s = float2(u.x * 16.0 + floor(u.y),
    frac(u.y) * (16.0 * 256.0) + u.z);
    return clamp(s * (1.0 / 2047.0) - 1.0, float2(-1.0), float2(1.0));
}