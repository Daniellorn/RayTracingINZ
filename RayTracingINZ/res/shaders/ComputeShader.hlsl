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
    int numOfModels;
}

cbuffer RenderConfiguration : register(b2)
{
    int frameIndex;
    int raysPerPixel;
    int numOfBounces;
    int accumulate;
    int numOfNodes;
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
    float metalness;
    float EmissionPower;
};

struct Triangle
{
    float4 v1;
    float4 v2;
    float4 v3;

    float4 n1;
    float4 n2;
    float4 n3;
    
    float4 MyCentroid;
};

struct Model
{
    int startTriangle;
    int triangleCount;
    int materialIndex;
};

struct BVHNode
{
    float4 aabbMin;
    float4 aabbMax;
    uint leftFirst;
    uint triangleCount;
};


StructuredBuffer<Sphere> g_Spheres : register(t0);
StructuredBuffer<Material> g_Materials : register(t1);
StructuredBuffer<Triangle> g_Triangles : register(t2);
StructuredBuffer<Model> g_Models : register(t3);
StructuredBuffer<int> g_TriIndexes : register(t4);
StructuredBuffer<BVHNode> g_BVHNodes : register(t5);
RWTexture2D<float4> outputTex : register(u0);
RWTexture2D<float4> accumulationTex : register(u1);


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

float TriangleIntersection(Ray ray, Triangle tri)
{
    float3 e1 = tri.v2.xyz - tri.v1.xyz;
    float3 e2 = tri.v3.xyz - tri.v1.xyz;
    
    float3 q = cross(ray.direction, e2);
    float a = dot(e1, q);
    
    if (a > -EPSILON && a < EPSILON)
    {
        return -1.0f;
    }
    
    float f = 1 / a;
    
    float3 s = ray.origin - tri.v1.xyz;
    float u = f * dot(s, q);
    
    if (u < 0.0f)
    {
        return -1.0f;
    }
    
    float3 r = cross(s, e1);
    float v = f * dot(ray.direction, r);
    
    if (v < 0.0f || u + v > 1.0f)
    {
        return -1.0f;
    }
    
    float t = f * dot(e2, r);
    
    if (t > EPSILON)
    {
        return t;
    }
    
    return -1.0f;
}

float IntersectAABB(const Ray ray, const BVHNode node)
{   
    float3 invDir = 1 / ray.direction.xyz;
    float3 t1 = (node.aabbMin.xyz - ray.origin.xyz) * invDir;
    float3 t2 = (node.aabbMax.xyz - ray.origin.xyz) * invDir;
    
    float3 tmin = min(t1, t2);
    float3 tmax = max(t1, t2);
    
    float tNear = max(max(tmin.x, tmin.y), tmin.z);
    float tFar = min(min(tmax.x, tmax.y), tmax.z);
    
    if (tFar <= tNear || tFar < 0.0f)
    {
        return -1.0f;
    }
    
    return tNear;
}

HitInfo
    Miss()
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
   
    int stack[32];
    int stackPtr = 0;
    stack[stackPtr++] = 0;
    
    while (stackPtr > 0)
    {
        int nodeIdx = stack[--stackPtr];
        BVHNode node = g_BVHNodes[nodeIdx];
        
        float aabbDist = IntersectAABB(ray, node);
        
        if (aabbDist < 0.0f || aabbDist >= info.hitDistance)
        {
            continue;
        }
        
        if (node.triangleCount > 0)
        {
            for (int k  = 0; k < node.triangleCount; k++)
            {
                int triIdx = g_TriIndexes[node.leftFirst + k];
                Triangle tri = g_Triangles[triIdx];
                
                float t = TriangleIntersection(ray, tri);
                
                if (t < 0.0f)
                {
                    continue;
                }
                
                if (t < info.hitDistance)
                {
                    info.hitDistance = t;
                    info.t = t;
                    info.hitPoint = RayAt(ray, t);
                    
                    float3 e1 = tri.v2.xyz - tri.v1.xyz;
                    float3 e2 = tri.v3.xyz - tri.v1.xyz;
                    info.normal = normalize(cross(e1, e2));
                    
                    info.objectIndex = triIdx;
                    info.materialIndex = 3;
                }
            }

        }
        else
        {
            int leftChild = node.leftFirst;
            int rightChild = node.leftFirst + 1;
            
            if (rightChild < numOfNodes)
            {
                stack[stackPtr++] = rightChild;
            }
            if (leftChild < numOfNodes)
            {
                stack[stackPtr++] = leftChild;
            }
        }
    }
    
    
    //for (int index = 0; index < numOfModels; index++)
    //{
    //    Model model = g_Models[index];
    //    
    //    for (int j = 0; j < model.triangleCount; j++)
    //    {
    //        Triangle tri = g_Triangles[model.startTriangle + j];
    //    
    //        float t = TriangleIntersection(ray, tri);
    //    
    //        if (t < 0.0f)
    //        {
    //            continue;
    //        }
    //    
    //        if (t < info.hitDistance)
    //        {
    //            info.hitDistance = t;
    //            info.t = t;
    //            info.hitPoint = RayAt(ray, t);
    //        
    //            float3 e1 = tri.v2.xyz - tri.v1.xyz;
    //            float3 e2 = tri.v3.xyz - tri.v1.xyz;
    //            info.normal = normalize(cross(e1, e2));
    //        
    //            info.objectIndex = model.startTriangle + j;
    //            info.materialIndex = model.materialIndex;
    //
    //        }
    //    }
    //}
    
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
    
    for (int i = 0; i < numOfBounces; i++)
    {
        HitInfo info = CheckIntersection(ray);
        
        if (info.hitDistance < 0.0f)
        {
            break;
            float3 unitDir = normalize(ray.direction);
            float t = 0.5f * (unitDir.y + 1.0f);
            light += lerp(float3(1.0f, 1.0f, 1.0f), float3(0.5f, 0.7f, 1.0f), t) * contribution;
            break;
        }

        Material material = g_Materials[info.materialIndex];
        
        light += GetEmission(material) * contribution;
        
        ray.origin = info.hitPoint + info.normal * EPSILON;
        
        
        float3 Fdielectics = float3(0.04f, 0.04f, 0.04f);
        float3 F0 = lerp(Fdielectics, material.albedo.xyz, material.metalness);
        
        float cosTheta = dot(info.normal, -ray.direction);
        float3 F = FresnelSchlick(max(cosTheta, 0.0f), F0);
        
        float reflectionChance = max(F.r, max(F.g, F.b));
        float randomValue = RandomFloat(seed);
        
        
        if (randomValue < reflectionChance)
        {
            float3 reflectionDir = reflect(ray.direction, info.normal);
            float3 diffuseDir = normalize(RandomVec3OnUnitHemiSphere(seed, reflectionDir));
            
            ray.direction = lerp(reflectionDir, diffuseDir, material.roughness * material.roughness);
            
            contribution *= F / reflectionChance;
        }
        else
        {
            if (material.metalness >= 1.0f)
            {
                return light;
            }
            
            ray.direction = normalize(RandomVec3OnUnitHemiSphere(seed, info.normal));
            float3 kd = (1.0f - F) * (1.0f - material.metalness);
            contribution *= (material.albedo.xyz * kd) / (1.0f - reflectionChance);
        
        }
        
        //float3 diffuseDir = normalize(RandomVec3OnUnitHemiSphere(seed, info.normal));
        //float3 specularDir = reflect(ray.direction, info.normal); // ray.direction - 2.0 * dot(normal, ray.direction) * normal;
        //
        //bool isSpecularBounce = material.metalness >= RandomFloat(seed); //metalness
        //
        //ray.direction = lerp(diffuseDir, specularDir, material.roughness * int(isSpecularBounce));
        //contribution *= lerp(material.albedo.xyz, float3(1.0f, 1.0f, 1.0f), int(isSpecularBounce)); //* invPI;
        
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
    seed *= PCG_Hash(seed + frameIndex * 719393);
    
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
    
    for (int i = 0; i < raysPerPixel; i++)
    {
        float3 jitteredDirection = ray.direction;
        jitteredDirection += RandomVec3(seed, -0.001, 0.001);
        
        Ray jitteredRay;
        jitteredRay.origin = ray.origin;
        jitteredRay.direction = normalize(jitteredDirection);
        
        totalColor += TraceRay(jitteredRay, seed);
    }
    
    //for (int i = 0; i < raysPerPixel; i++)
    //{        
    //    totalColor += TraceRay(ray, seed);
    //}

    float3 finalColor = totalColor.xyz / float(raysPerPixel);
    
    if (accumulate)
    {
        float4 prev = accumulationTex[pixel];
        float t = 1 / float(frameIndex);
        
        float3 blend = lerp(prev.xyz, finalColor, t);
        
        accumulationTex[pixel] = float4(blend, 1.0f);
        outputTex[pixel] = float4(blend, 1.0f);
    }
    else
    {
        //if (g_BVHNodes[0].triangleCount > 100)
        //{
        //    finalColor = float3(1.0f, 0.0f, 0.0f);
        //}
        
        outputTex[pixel] = float4(finalColor, 1.0f);
    }
    
}