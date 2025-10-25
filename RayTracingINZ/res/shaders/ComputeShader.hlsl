static const float MAX_FLOAT = 100000;
static const int MAX_OBJ = 100;

RWTexture2D<float4> outputTex : register(u0);


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
    //int objectIndex;
    //int materialIndex;
};

struct Sphere
{
    float4 position;
    float radius;
    //int materialIndex;
    //int type;
};

float3 RayAt(Ray ray, float t)
{
    return ray.direction * t + ray.origin;
}

float SphereIntersection(Ray ray, Sphere sphere)
{
    float3 oc = ray.origin - sphere.position.xyz;
    
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    
    float discriminant = b * b - 4.0 * a * c;
    
    if (discriminant < 0.0)
    {
        return -1.0;
    }
    
    float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
    float t2 = (-b + sqrt(discriminant)) / (2.0 * a);
    
    if (t1 > 0.0 && t2 > 0.0)
    {
        return min(t1, t2);
    }
    else if (t1 > 0.0)
    {
        return t1;
    }
    else if (t2 > 0.0)
    {
        return t2;
    }
    else
    {
        return -1.0;
    }
}

HitInfo Miss()
{
    HitInfo info;
    info.hitDistance = -1.0;
    info.hitPoint = float3(-1.0, -1.0, -1.0);
    info.normal = float3(0.0, 0.0, 0.0);
    info.t = 0.0;
    
    return info;
}


HitInfo CheckIntersection(Ray ray)
{
    HitInfo info = (HitInfo)0;
    info.hitDistance = MAX_FLOAT;
    //info.objectIndex = -1.0;
    
    Sphere spheres[2];
    spheres[0].position = float4(2.0, 1.0, 0.0, 1.0);
    spheres[0].radius = 1.0;
    spheres[1].position = float4(-1.0, 0.0, 5.0, 1.0);
    spheres[1].radius = 1.0;
    
    for (int i = 0; i < 2; i++)
    {
        float t = SphereIntersection(ray, spheres[i]);
        
        if (t < 0.0)
        {
            continue;
        }
        
        if (t < info.hitDistance)
        {
            info.hitDistance = t;
            info.t = t;
            info.hitPoint = RayAt(ray, t);
            info.normal = normalize(info.hitPoint - spheres[i].position.xyz);
        }
    }
    
    if (info.hitDistance == MAX_FLOAT)
    {
        info = Miss();
    }

    return info;
}

float3 TraceRay(Ray ray)
{
    float3 color;
    
    HitInfo info = CheckIntersection(ray);
    
    if (info.hitDistance < 0.0)
    {
        float3 unitDir = normalize(ray.direction);
        float a = 0.5 * (unitDir.y + 1.0f);
        return float3(lerp(float3(1.0f, 1.0f, 1.0f), float3(0.5f, 0.7f, 1.0f), a));
    }
    
    color = info.normal * 0.5 + 0.5;
    
    return color;
}

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint width, height;
    outputTex.GetDimensions(width, height);
    
    uint2 pixel = DTid.xy;
    
    if (pixel.x >= width || pixel.y >= height)
        return;
    
    float2 texSize = float2(width, height);
    float2 normalizedCord = float2(pixel) / float2(texSize);
    float aspectRatio = texSize.x / texSize.y;
    
    normalizedCord = normalizedCord * 2.0f - 1.0f;
    
    //normalizedCord.y = 1.0f - normalizedCord.y;
    normalizedCord.x *= aspectRatio;

    Ray ray;
    ray.origin = float3(0.0, 0.0, -2.0);
    ray.direction = normalize(float3(normalizedCord.x, normalizedCord.y, 1.0));
    
    Sphere sphere;
    sphere.position = float4(0.0, 0.0, 1.0, 0.0);
    sphere.radius = 1.0;
      
    float3 color = TraceRay(ray);
    
    //outputTex[pixel] = float4(1.0f - normalizedCord.x, normalizedCord.x, 0.0f, 1.0f);

    outputTex[pixel] = float4(color, 1.0);
}