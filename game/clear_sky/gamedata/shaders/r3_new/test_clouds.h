static const float fieldOfView = 55.0;
 
static const float lightAngle = 70.0;
 
static const uint raySteps = 10u;
static const uint noiseIterations = 7u;
 
static const float cloudsFrequency = 1.0;
static const float cloudsThickness = 0.15;
static const float cloudsAltitude = 1.0;

float GetCloudNoise(float2 position, float e0) {
    position *= cloudsFrequency;
    // position /= iChannelResolution[0].xy;
    float noise = 0.0, weightSum = 0.0;
    for (uint i = 0u; i < noiseIterations; ++i) {
        float iterationWeight = exp2(-float(i));
        // float2 noisePosition = position * exp2(float(i)) + 0.1 * iTime / iChannelResolution[0].xy;
        float2 noisePosition = position * exp2(float(i)) + 0.1 * timers.x;
        noise += noise::hash32(noisePosition).r * iterationWeight;
        weightSum += iterationWeight;
    } noise /= weightSum;
    return smoothstep(e0, 1.0, noise);
}

void PlanarClouds(
    float3 viewPosition,
    float3 viewDirection,
    float3 lightDirection,
    float rand,
    out float scattering,
    out float transmittance
) {
    scattering = 0.0;
    transmittance = 1.0;
    
    //--// Find current point on cloud
    
    float tPlane = (cloudsAltitude - viewPosition.z) / viewDirection.z;
    if (tPlane < 0.0) { return; }
    float2 cloudPosition = viewPosition.xy + viewDirection.xy * tPlane;
    
    //--// Set up self-shadowing raymarch
    
    float rayLength3D = cloudsThickness / lightDirection.z;
    float3 rayfloattor3D = rayLength3D * lightDirection;
    
    float2 rayStep  = lightDirection.xy * cloudsThickness / lightDirection.z;
         rayStep -= viewDirection.xy  * cloudsThickness / viewDirection.z;
         rayStep /= float(raySteps);
    float2 rayPosition = cloudPosition + rayStep * rand;
    
    //--// Self-shadowing raymarch
    
    float e0 = 0.5 / (0.5 * cloudsThickness / (cloudsAltitude * viewDirection.z) + (1.0 - 0.5 * cloudsThickness / cloudsAltitude));
    
    float lightOpticalDepth = 0.0;
    for (uint i = 0u; i < raySteps; ++i, rayPosition += rayStep) {
        float cloudNoise = GetCloudNoise(rayPosition, e0);
        lightOpticalDepth += max(cloudNoise, 0.0);
    }
    lightOpticalDepth *= 4.0 * cloudsFrequency / cloudsThickness * rayLength3D / float(raySteps);
    
    float cloudNoise = GetCloudNoise(cloudPosition, e0);
    float cloudOpticalDepth = 4.0 * cloudsFrequency * cloudNoise / abs(viewDirection.z);
        
    transmittance = exp(-cloudOpticalDepth);
    scattering = exp(-lightOpticalDepth) * (1.0 - transmittance);
}

// mat3 GetRotationMatrix(float3 unitAxis, float angle) {
	// float cosine = cos(angle);

	// float3 axis = unitAxis * sin(angle);
	// float3 tmp = unitAxis - unitAxis * cosine;

	// return mat3(
		// unitAxis.x * tmp.x + cosine, unitAxis.x * tmp.y - axis.z, unitAxis.x * tmp.z + axis.y,
		// unitAxis.y * tmp.x + axis.z, unitAxis.y * tmp.y + cosine, unitAxis.y * tmp.z - axis.x,
		// unitAxis.z * tmp.x - axis.y, unitAxis.z * tmp.y + axis.x, unitAxis.z * tmp.z + cosine
	// );
// }
float3 LinearToSrgb(float3 color) {
	return lerp(1.055 * pow(color, 1.0 / 2.4) - 0.055, color * 12.92, step(color, 0.0031308));
}
void mainClouds(out float4 fragColor, in float2 fragCoord) {
	float2 iResolution = screen_res.xy;
    // float zrot =  radians(360.0) * iMouse.x / iResolution.x;
    // float xrot = -radians(90.0) * ((iMouse.x + iMouse.y) < 1.5 ? 0.2 : (iMouse.y / iResolution.y)) - radians(90.0);
    // mat3 viewRot = GetRotationMatrix(float3(0.0, 0.0, 1.0), zrot)
                 // * GetRotationMatrix(float3(1.0, 0.0, 0.0), xrot);
    float3 viewDirection;
    // viewDirection.xy = (2.0 * fragCoord - iResolution.xy) * tan(radians(fieldOfView) / 2.0) / iResolution.y;
    // viewDirection.z = -1.0;
    // viewDirection = viewRot * normalize(viewDirection);
	viewDirection = G_BUFFER::unpack_position(0, fragCoord, 1);
    viewDirection = normalize(viewDirection);
    float3 viewPosition = 0.0;
    
    // float3 lightDirection = float3(float2(cos(iTime + 2.0), sin(iTime + 2.0)) * sin(radians(lightAngle)), cos(radians(lightAngle)));
    float3 lightDirection = Ldynamic_dir.xyz;
    
    // sky
    const float3 rlHor = float3(0.3, 0.4, 0.5), rlUp = float3(0.1, 0.2, 0.4);
    const float3 mieHor = 0.2, mieUp = 0.05;
    float phaseRl = pow(dot(viewDirection, lightDirection), 2.0) * 0.5 + 1.0;
    float phaseMie = pow((dot(viewDirection, lightDirection) + 1.0) * 0.55, 16.0) * 0.5 + 1.0;
    fragColor.rgb  = rlHor;
    fragColor.rgb += phaseMie * lerp(mieUp, mieHor, pow(1.0 - max(viewDirection.z, 0.0), 4.0));
    
    // clouds
    // float2 noise = texture(iChannel0, fragCoord / iChannelResolution[0].xy).xy;
    float2 noise = noise::hash32(fragCoord);
    
    float cloudsScattering; float cloudsTransmittance;
    PlanarClouds(viewPosition, viewDirection, lightDirection, noise.x, cloudsScattering, cloudsTransmittance);
    
    // fade distant clouds
    cloudsScattering *= exp(-0.05 / abs(viewDirection.z));
    cloudsTransmittance = lerp(1.0, cloudsTransmittance, exp(-0.05 / abs(viewDirection.z)));
    
    fragColor.rgb = fragColor.rgb * cloudsTransmittance + phaseMie * cloudsScattering;
    
    // basic tonemap
    // fragColor.rgb *= inversesqrt(fragColor.rgb * fragColor.rgb + 1.0);
    fragColor.rgb *= rsqrt(fragColor.rgb * fragColor.rgb + 1.0);
    
    // // convert to srgb & dither
    fragColor.rgb = LinearToSrgb(fragColor.rgb);
    fragColor.rgb += (noise.x - noise.y) / 255.0;
    fragColor.a = 1.0;
}