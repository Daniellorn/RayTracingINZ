#include "Utils.hlsli"

cbuffer CameraBuffer : register(b0)
{
    float4x4 invProjectionMartix;
    float4x4 invViewMatrix;
    float3 cameraPosition;
};

cbuffer SceneConfiguration : register(b1)
{
    int numOfSpheres;
    int numOfBounce;
}

cbuffer RenderData : register(b2)
{
    uint frameIndex;
    uint raysPerPixel;
}

struct Ray
{
    float3 origin;
    float3 direction;
};

struct HitInfo
{
    float3 normal;
    float3 hitPoint;
    float t;
    float hitDistance;
    int objectIndex;
    int materialIndex;
};

struct Sphere
{
    float4 position;
    float radius;

    int materialIndex;
    int type;
};

struct Material
{
    float4 albedo;
    float4 EmissionColor;
    float roughness;
    float glossiness;
    float EmissionPower;
};


StructuredBuffer<Sphere> g_Spheres : register(t0);
StructuredBuffer<Material> g_Materials : register(t1);
RWTexture2D<float4> outputTex : register(u0);


float3 RayAt(Ray ray, float t)
{
    return ray.direction * t + ray.origin;
}

float3 GetEmission(Material material)
{
    float4 result = material.EmissionColor * material.EmissionPower;

    return float3(result.x, result.y, result.z);
}

float SphereIntersection(Ray ray, Sphere sphere)
{
    float3 spherePosition = float3(sphere.position.xyz);
    float3 oc = ray.origin - spherePosition;
    
    float a = 1; //dot(ray.direction, ray.direction);
    float b = 2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    
    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f)
    {
        return -1.0f;
    }
    
    float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
    
    if (t1 > 0.0f && t2 > 0.0f)
    {
        return min(t1, t2);
    }
    else if (t1 > 0.0f)
    {
        return t1;
    }
    else if (t2 > 0.0f)
    {
        return t2;
    }
    else
    {
        return -1.0f;
    }
}

HitInfo Miss()
{
    HitInfo info = (HitInfo)0;
    info.hitDistance = -1.0f;
    info.hitPoint = float3(-1.0f, -1.0f, -1.0f);
    info.normal = float3(0.0f, 0.0f, 0.0f);
    info.t = 0.0;
    
    return info;
}


HitInfo CheckIntersection(Ray ray)
{
    HitInfo info = (HitInfo)0;
    info.hitDistance = MAX_FLOAT;
    info.objectIndex = -1.0;
   
    
    for (int i = 0; i < numOfSpheres; i++)
    {
        float t = SphereIntersection(ray, g_Spheres[i]);
        
        if (t < 0.0)
        {
            continue;
        }
        
        if (t < info.hitDistance)
        {
            info.hitDistance = t;
            info.t = t;
            info.hitPoint = RayAt(ray, t);
            info.normal = normalize(info.hitPoint - float3(g_Spheres[i].position.xyz));
            info.objectIndex = i;
            info.materialIndex = g_Spheres[i].materialIndex;
        }
    }
    
    if (info.hitDistance == MAX_FLOAT)
    {
        info = Miss();
    }
    
    return info;
}

float3 TraceRay(Ray ray, inout uint seed)
{
    float3 light = float3(0.0f, 0.0f, 0.0f);
    float3 contribution = float3(1.0f, 1.0f, 1.0f);
    
    for (int i = 0; i < numOfBounce; i++)
    {
        HitInfo info = CheckIntersection(ray);
        
        if (info.hitDistance < 0.0f)
        {
            break;
            float3 unitDir = normalize(ray.direction);
            float a = 0.5 * (unitDir.y + 1.0f);
            return float3(lerp(float3(1.0f, 1.0f, 1.0f), float3(0.5f, 0.7f, 1.0f), a));
        }
        
        Sphere closestSphere = g_Spheres[info.objectIndex];
        Material sphereMaterial = g_Materials[info.materialIndex];
        
        light += GetEmission(sphereMaterial) * contribution;
        
        ray.origin = info.hitPoint + info.normal * EPSILON;
        
        float3 diffuseDir = normalize(RandomVec3OnUnitHemiSphere(seed, info.normal));
        
        ray.direction = diffuseDir;
        contribution *= sphereMaterial.albedo.xyz;
        
    }
    
    return light;
}

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    outputTex.GetDimensions(width, height);
    
    uint2 pixel = DTid.xy;
    
    if (pixel.x >= width || pixel.y >= height)
        return;
    
    uint seed = pixel.x + pixel.y * width;
    seed *= frameIndex;
    
    float2 texSize = float2(width, height);
    float2 normalizedCord = float2(pixel) / float2(texSize);
    float aspectRatio = texSize.x / texSize.y;
    
    normalizedCord = normalizedCord * 2.0f - 1.0f;
    normalizedCord *= aspectRatio;
    
    float4 clip = float4(normalizedCord, 1.0f, 1.0f);
    float4 target = mul(invProjectionMartix, clip);
    target.xyz /= target.w;
    
    float4 worldDir4 = mul(invViewMatrix, float4(target.xyz, 0.0f));
    float3 worldDir = normalize(worldDir4.xyz);
    
    Ray ray;
    ray.origin = cameraPosition;
    ray.direction = worldDir;
      
    float3 totalColor = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < int(raysPerPixel); i++)
    {
        totalColor += TraceRay(ray, seed);
    }

    float3 finalColor = totalColor.xyz / float(raysPerPixel);
    
    outputTex[pixel] = float4(finalColor, 1.0f);
}