module Intersect;

import Ray;

namespace Raycast {
  public float hit_sphere(float3 center, float radius, Ray r) {
      float3 oc = r.origin - center;
      float b = dot(oc, r.direction);
      float c = dot(oc, oc) - radius * radius;
      float h = b * b - c;
      if (h < 0.0) return -1.0;
      return -b - sqrt(h);
  }
  
  public float2 hit_aabb(float3 boxMin, float3 boxMax, Ray r) {
      float3 invDir = 1.0 / r.direction;
      float3 t0 = (boxMin - r.origin) * invDir;
      float3 t1 = (boxMax - r.origin) * invDir;
  
      float3 tmin = min(t0, t1);
      float3 tmax = max(t0, t1);
  
      float start = max(max(tmin.x, tmin.y), tmin.z);
      float end = min(min(tmax.x, tmax.y), tmax.z);
  
      return float2(start, end);
  }

  public float3 get_aabb_normal(float3 p, float3 boxMin, float3 boxMax) {
      const float epsilon = 0.001;
      if (abs(p.x - boxMin.x) < epsilon) return float3(-1, 0, 0);
      if (abs(p.x - boxMax.x) < epsilon) return float3(1, 0, 0);
      if (abs(p.y - boxMin.y) < epsilon) return float3(0, -1, 0);
      if (abs(p.y - boxMax.y) < epsilon) return float3(0, 1, 0);
      if (abs(p.z - boxMin.z) < epsilon) return float3(0, 0, -1);
      if (abs(p.z - boxMax.z) < epsilon) return float3(0, 0, 1);
      return float3(0);
  }

  public struct RayHit {
      public float tNear;
      public float tFar;
      public bool hit;
  };

  public RayHit intersectAABB(Ray ray, float3 boxMin, float3 boxMax) {
      float3 invDir = 1.0 / ray.direction;
      float3 t0 = (boxMin - ray.origin) * invDir;
      float3 t1 = (boxMax - ray.origin) * invDir;
      float3 tMin = min(t0, t1);
      float3 tMax = max(t0, t1);

      float tNear = max(max(tMin.x, tMin.y), tMin.z);
      float tFar = min(min(tMax.x, tMax.y), tMax.z);

      return { tNear, tFar, tFar >= tNear && tFar > 0 };
  }
}