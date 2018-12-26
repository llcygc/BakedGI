
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
