#define MAX_SPHERES 10

// A material that dictates how a ray should interact when an intersection occurs
struct RayTracingMaterial
{
    // The colour of the mesh
    float4 colour;    
    // The colour of the meshes emission, should be black if no emmission is intended
    float4 emissionColour;    
    // The colour of the specular highlights
    float4 specularColour;
    
    
    // The stregth of the light emmited, should be set to be greater than the bloom threshold
    float emissionStrenth;    
    // value from 0 to 1, controlls the weighting between a diffuse bounce and a specular ray bounce
    float smoothness;    
    // value from 0 to 1, controlls how likely a specular bounce occurs
    float specularPorbability;    
    // Padds the struct to be byte safe
    float padding;
};

// Describes a ray, isn't 16 byte safe but is not passed from the cpu
struct Ray
{    
    float3 origin;
    float3 direction;
};

// Describes a ray intersection, isn't 16 byte safe but is not passed from the cpu
struct HitInfo
{
    // Tracks wether the ray intersected with some geometry
    bool didHit;
    // The distance between the ray origin and the hitPoint
    float distance;
    
    // The position in world space that the ray intersection occured
    float3 hitPoint;    
    // The tangent normal from the hitPoint 
    float3 normal;   
    
    // The material of the mesh that the ray hit
    RayTracingMaterial material;
};
    
// Describes a sphere in the scene
struct SphereData
{
    // The position of the sphere
    float3 position;
    // The radius of the sphere
    float radius;
    
    // The material attached to the sphere
    RayTracingMaterial material;
};

// Describes a triangle
struct Triangle
{
    float3 positionA;
    float3 positionB;
    float3 positionC;
};

// Translates the pixel into world space in front of the camera
float3 CalcualteViewPointLocation(float2 _uv, float2 _planeSize, float3 _cameraForward, float3 _cameraRight, float3 _cameraWorldPosition, float _nearClipPlane)
{
    // camera up vector is calculated here on the gpu using the forward and right vectors as opposed to calculating it on the cpu and passing it through a buffer
    float3 cameraUp = cross(_cameraRight, _cameraForward);
    
    // Bottom left of the plane in world space
    float3 bottomLeft = float3(-_planeSize.x / 2, -_planeSize.y / 2, _nearClipPlane);
    // Local position of the pixel in world space on the plane
    float3 viewPointLocal = bottomLeft + float3(_planeSize.x * _uv.x, _planeSize.y * _uv.y, 0);
    // Returns the pixel in world space
    return _cameraWorldPosition + _cameraRight * viewPointLocal.x + cameraUp * viewPointLocal.y + _cameraForward * _nearClipPlane;
}

// Using the pixel index generates a random seed, is constant each frame unless multipass rendering is enabled
uint GenerateRandomSeed(float2 _uv, uint2 _screenSize, int _multiPassFramesRendered)
{
    uint2 pixelCoordinates = _uv * _screenSize;
    uint pixelIndex =  pixelCoordinates.y * _screenSize.x + pixelCoordinates.x;
    
    // Returns an offsetted random seed if multipass rendering is enabled
    return pixelIndex + _multiPassFramesRendered * 719393;
}

// PCG (permuted congruential generator).
// www.pcg-random.org && www.shadertoy.com/view/XlGcRh
float RandomValue(inout uint _seed)
{
    _seed = _seed * 747796405 + 2891336453;
    uint result = ((_seed >> ((_seed >> 28) + 4)) ^ _seed) * 277803737;
    result = (result >> 22) ^ result;
    return result / 4294967295.0; // (2^32 - 1)
}

// Influences the random direction generation to be cosign weighted, making ray bounces towards the pole of the sphere more likely
float RandomValueNormalDistribution(inout uint _seed)
{
    // stackoverflow.com/a/6178290
    float theta = 2 * 3.1415926 * RandomValue(_seed);
    float rho = sqrt(-2 * log(RandomValue(_seed)));
    return rho * cos(theta);
}

// Generates a random direction
float3 RandomDirection(inout uint _seed)
{
    // function has a ~50% of succeding in generating a point in a sphere when generating a point in a unit cube
    // Safety loop is used in stead of a while true loop to avoid potential graphical artefacts
    for (int safetyLimit = 0; safetyLimit < 100; safetyLimit++)
    {        
    // Point in cube
        float x = RandomValueNormalDistribution(_seed) * 2 - 1;
        float y = RandomValueNormalDistribution(_seed) * 2 - 1;
        float z = RandomValueNormalDistribution(_seed) * 2 - 1;
    
        float3 pointInCube = float3(x, y, z);
        float squareDistanceFromCentre = dot(pointInCube, pointInCube);
    
    // If point is *inside* sphere, scale it so that the point is *on* the sphere
        if (squareDistanceFromCentre <= 1)
        {
            // normalise
            return pointInCube / sqrt(squareDistanceFromCentre);
        }
    }
    
    return 0;
}

// Random direction in the hemisphere oriented around the given normal vector
float3 RandomHemisphereDirection(float3 _normal, inout uint _seed)
{
    float3 direction = RandomDirection(_seed);
    return direction * sign(dot(_normal, direction));
}

// Returns an initialised RayTracingMaterial
RayTracingMaterial CreateRayTracingMaterial()
{
    RayTracingMaterial material;
    material.colour = float4(1, 1, 1, 1);
    material.emissionColour = float4(0, 0, 0, 1);
    material.specularColour = float4(1, 1, 1, 1);
    material.emissionStrenth = 0;
    material.smoothness = 0;
    material.specularPorbability = 0;
    material.padding = 0;
    return material;
}

// Returns an initialised HitInfo
HitInfo CreateHitInfo()
{    
    HitInfo hitInfo;
    hitInfo.didHit = false;
    hitInfo.distance = 1.#INF;
    hitInfo.hitPoint = float3(0, 0, 0);
    hitInfo.material = CreateRayTracingMaterial();
    hitInfo.normal = float3(0, 1, 0); 

    return hitInfo;
}

// https://raytracing.github.io/
// Ray intersection with a sphere
HitInfo RaySphere(Ray _ray, SphereData _sphere)
{
    // offset the ray, so it can be tested against a sphere at the origin
    float3 offsetRayOrigin = _ray.origin - _sphere.position;
    
    // x² + y² + z² = r²
    // RayOrigion + RayDirection * Distance
    //When the squared length of that resulting vector is equal to the radius² then the ray intersects with the sphere. 
    //Rearranging the equation to solve for distance, if the square root is negative then there is no intersection. 
    //If the square root is positive then the ray intersects with the sphere twice, once entering it and the other leaving it. 
    //If the square root is exactly 0 then there is only one intersection as the ray is at a tangent to the sphere point it intersects.
    
    float a = dot(_ray.direction, _ray.direction);
    float b = 2 * dot(offsetRayOrigin, _ray.direction);
    float c = dot(offsetRayOrigin, offsetRayOrigin) - _sphere.radius * _sphere.radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant >= 0)
    {
        float distance = (-b - sqrt(discriminant)) / (2 * a);
        
        if (distance >= 0)
        {
            HitInfo hitInfo = CreateHitInfo();

            hitInfo.didHit = true;
            hitInfo.distance = distance;
            hitInfo.hitPoint = _ray.origin + _ray.direction * distance;
            hitInfo.normal = normalize(hitInfo.hitPoint - _sphere.position);
            hitInfo.material = _sphere.material;
            return hitInfo;
        }
    }
    
    HitInfo hitInfo = CreateHitInfo();
    return hitInfo;
}

// https://raytracing.github.io/
// Ray intersection with a triangle
HitInfo RayTriagnle(Ray _ray, Triangle _triangle)
{
    float3 edgeAB = _triangle.positionB - _triangle.positionA;
    float3 edgeAC = _triangle.positionC - _triangle.positionA;
    float3 normalVector = cross(edgeAB, edgeAC);
    float3 ao = _ray.origin - _triangle.positionA;
    float3 dao = cross(ao, _ray.direction);
    
    float determinant = -dot(_ray.direction, normalVector);
    float invertedDeterminant = 1 / determinant;
    
    // Calculate distance to triangle & barycentric coordinates of intersection point
    float distance = dot(ao, normalVector) * invertedDeterminant;
    float u = dot(edgeAC, dao) * invertedDeterminant;
    float v = -dot(edgeAB, dao) * invertedDeterminant;
    float w = 1 - u - v;
    
    HitInfo hitInfo = CreateHitInfo();
    hitInfo.didHit = determinant >= 1E-6 && distance >= 0 && u >= 0 && v >= 0 && w >= 0;
    hitInfo.hitPoint = _ray.origin + _ray.direction * distance;
    hitInfo.normal = normalize(_triangle.positionA * w + _triangle.positionB * u + _triangle.positionC * v);
    hitInfo.distance = distance;
    return hitInfo;
}

// Loops through each sphere and calcualtes if the ray has hit one of them or bounced into the surounding environment
HitInfo CalculateRayCollision(Ray _ray, SphereData _spheres[MAX_SPHERES], int _sphereCount)
{
    // Save the closest hit
    HitInfo closestHit = CreateHitInfo();
    
    for (int sphere = 0; sphere < _sphereCount; sphere++)
    {
        // Check for intersection
        HitInfo hitInfo = RaySphere(_ray, _spheres[sphere]);
        
        // If there was one and it's closer than the already found one, then that is the new closest hit
        if (hitInfo.didHit && hitInfo.distance < closestHit.distance)
        {
            closestHit = hitInfo;
        }
    }
    
    return closestHit;
}

// Traces a ray through the scene, if intersections occour bouncing several times
float3 Trace(Ray _ray, int _maxRayBounce, SphereData _spheres[MAX_SPHERES], int _sphereCount, inout uint _seed)
{
    // Track the incoming light and ray colour
    float3 incomingLight = float3(0, 0, 0);
    float3 rayColour = float3(1, 1, 1);
    
    // For each bounce
    for (int bounce = 0; bounce < _maxRayBounce; bounce++)
    {
        // Check for intersections
        HitInfo hitInfo = CalculateRayCollision(_ray, _spheres, _sphereCount);
        
        // Bounce
        if (hitInfo.didHit)
        {
            // The material of the mesh that was hit
            RayTracingMaterial material = hitInfo.material;
            
            // The origin of the ray is set to the hit point as that's where the bounced ray is measured from
            _ray.origin = hitInfo.hitPoint;
            
            // A random direction is generated in a hemispshere oriontate about the tangent normal of the intersection
            float3 diffuseDirection = RandomHemisphereDirection(hitInfo.normal, _seed);
            // The speuclar direciont is a perfect reflection about the hit normal
            float3 specularDirection = reflect(_ray.direction, hitInfo.normal);
            // Using the Random seed, calculate if the ray is a specular bounce or not
            bool isSpecularBounce = material.specularPorbability >= RandomValue(_seed);
            
            // The bounced direction is interpolated from the diffuse (random) bounce and the specular (mirrored) bounce by the smoothness value of the material
            // Only if a specular bounce occured, otherwise smoothness is multiplied by 0 and only the pure diffuseDirection is used
            _ray.direction = lerp(diffuseDirection, specularDirection, material.smoothness * isSpecularBounce);
            
            // Tracks the light being emitted from the mesh that was hit
            float3 emittedLight = (float3) material.emissionColour * (float3) material.emissionStrenth;
            // The smaller the angle between the ray direction and the normal of the hit point the stronger the light the ray recieves
            float lightStrength = dot(hitInfo.normal, _ray.direction);
            
            incomingLight += emittedLight * rayColour;
            
            // The light strength calcualtion roughly halfs the light in the scene, so it's doubled here
            rayColour *= (float3)lerp(material.colour, material.specularColour, isSpecularBounce) * lightStrength * 2;
        }
        else
        {
        // Ray Missed
            incomingLight += float3(0.2, 0.2, 0.2);
            break;
        }
    }
    
    // Either max bounces has been reached or the ray missed all the geometry in the scene
    return incomingLight;
}