// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "embree2/rtcore.h"
#include "tutorial/tutorial.h"
#include "kernels/xeon/bvh4/bvh4.h"
#include "kernels/xeon/geometry/triangle4v.h"

namespace embree
{
  /* scene data */
  RTCScene g_scene = nullptr;

  struct Triangle { int v0, v1, v2; };

  /* error reporting function */
  void error_handler(const RTCError code, const int8* str)
  {
    printf("Embree: ");
    switch (code) {
    case RTC_UNKNOWN_ERROR    : printf("RTC_UNKNOWN_ERROR"); break;
    case RTC_INVALID_ARGUMENT : printf("RTC_INVALID_ARGUMENT"); break;
    case RTC_INVALID_OPERATION: printf("RTC_INVALID_OPERATION"); break;
    case RTC_OUT_OF_MEMORY    : printf("RTC_OUT_OF_MEMORY"); break;
    case RTC_UNSUPPORTED_CPU  : printf("RTC_UNSUPPORTED_CPU"); break;
    case RTC_CANCELLED        : printf("RTC_CANCELLED"); break;
    default                   : printf("invalid error code"); break;
    }
    if (str) { 
      printf(" ("); 
      while (*str) putchar(*str++); 
      printf(")\n"); 
    }
    exit(1);
  }

  /* adds a cube to the scene */
  unsigned int addCube (RTCScene scene_i, const Vec3fa& pos)
  {
    /* create a triangulated cube with 12 triangles and 8 vertices */
    unsigned int mesh = rtcNewTriangleMesh (scene_i, RTC_GEOMETRY_STATIC, 12, 8);
    
    /* set vertices */
    Vec3fa* vertices = (Vec3fa*) rtcMapBuffer(scene_i,mesh,RTC_VERTEX_BUFFER); 
    vertices[0].x = pos.x + -1; vertices[0].y = pos.y + -1; vertices[0].z = pos.z + -1; 
    vertices[1].x = pos.x + -1; vertices[1].y = pos.y + -1; vertices[1].z = pos.z + +1; 
    vertices[2].x = pos.x + -1; vertices[2].y = pos.y + +1; vertices[2].z = pos.z + -1; 
    vertices[3].x = pos.x + -1; vertices[3].y = pos.y + +1; vertices[3].z = pos.z + +1; 
    vertices[4].x = pos.x + +1; vertices[4].y = pos.y + -1; vertices[4].z = pos.z + -1; 
    vertices[5].x = pos.x + +1; vertices[5].y = pos.y + -1; vertices[5].z = pos.z + +1; 
    vertices[6].x = pos.x + +1; vertices[6].y = pos.y + +1; vertices[6].z = pos.z + -1; 
    vertices[7].x = pos.x + +1; vertices[7].y = pos.y + +1; vertices[7].z = pos.z + +1; 
    rtcUnmapBuffer(scene_i,mesh,RTC_VERTEX_BUFFER); 
    
    /* set triangles */
    int tri = 0;
    Triangle* triangles = (Triangle*) rtcMapBuffer(scene_i,mesh,RTC_INDEX_BUFFER);
    
    // left side
    triangles[tri].v0 = 0; triangles[tri].v1 = 2; triangles[tri].v2 = 1; tri++;
    triangles[tri].v0 = 1; triangles[tri].v1 = 2; triangles[tri].v2 = 3; tri++;
    
    // right side
    triangles[tri].v0 = 4; triangles[tri].v1 = 5; triangles[tri].v2 = 6; tri++;
    triangles[tri].v0 = 5; triangles[tri].v1 = 7; triangles[tri].v2 = 6; tri++;
    
    // bottom side
    triangles[tri].v0 = 0; triangles[tri].v1 = 1; triangles[tri].v2 = 4; tri++;
    triangles[tri].v0 = 1; triangles[tri].v1 = 5; triangles[tri].v2 = 4; tri++;
    
    // top side
    triangles[tri].v0 = 2; triangles[tri].v1 = 6; triangles[tri].v2 = 3; tri++;
    triangles[tri].v0 = 3; triangles[tri].v1 = 6; triangles[tri].v2 = 7; tri++;
    
    // front side
    triangles[tri].v0 = 0; triangles[tri].v1 = 4; triangles[tri].v2 = 2; tri++;
    triangles[tri].v0 = 2; triangles[tri].v1 = 4; triangles[tri].v2 = 6; tri++;
    
    // back side
    triangles[tri].v0 = 1; triangles[tri].v1 = 3; triangles[tri].v2 = 5; tri++;
    triangles[tri].v0 = 3; triangles[tri].v1 = 7; triangles[tri].v2 = 5; tri++;
    
    rtcUnmapBuffer(scene_i,mesh,RTC_INDEX_BUFFER);
    
    return mesh;
  }

  /* adds a ground plane to the scene */
  unsigned int addGroundPlane (RTCScene scene_i)
  {
    /* create a triangulated plane with 2 triangles and 4 vertices */
    unsigned int mesh = rtcNewTriangleMesh (scene_i, RTC_GEOMETRY_STATIC, 2, 4);
    
    /* set vertices */
    Vec3fa* vertices = (Vec3fa*) rtcMapBuffer(scene_i,mesh,RTC_VERTEX_BUFFER); 
    vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10; 
    vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10; 
    vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10; 
    vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;
    rtcUnmapBuffer(scene_i,mesh,RTC_VERTEX_BUFFER); 
    
    /* set triangles */
    Triangle* triangles = (Triangle*) rtcMapBuffer(scene_i,mesh,RTC_INDEX_BUFFER);
    triangles[0].v0 = 0; triangles[0].v1 = 2; triangles[0].v2 = 1;
    triangles[1].v0 = 1; triangles[1].v1 = 2; triangles[1].v2 = 3;
    rtcUnmapBuffer(scene_i,mesh,RTC_INDEX_BUFFER);
    
    return mesh;
  }

  /* adds a hair to the scene */
  unsigned int addHair(RTCScene scene_i)
  {
    unsigned int geomID = rtcNewHairGeometry (scene_i, RTC_GEOMETRY_STATIC, 1, 4, 1);

    ssef* pos = (ssef*) rtcMapBuffer(scene_i,geomID,RTC_VERTEX_BUFFER);
    pos[0] = ssef(0.0f,0.0f,0.0f,0.1f);
    pos[1] = ssef(0.0f,1.0f,0.0f,0.1f);
    pos[2] = ssef(0.0f,2.0f,0.0f,0.1f);
    pos[3] = ssef(0.0f,3.0f,0.0f,0.1f);
    rtcUnmapBuffer(scene_i,geomID,RTC_VERTEX_BUFFER);

    int* index = (int*) rtcMapBuffer(scene_i,geomID,RTC_INDEX_BUFFER);
    index[0] = 0;
    rtcUnmapBuffer(scene_i,geomID,RTC_INDEX_BUFFER);
    
    return geomID;
  }

  /* prints the bvh4.triangle4v data structure */
  void print_bvh4_triangle4v(BVH4::NodeRef node, size_t depth)
  {
    if (node.isNode())
    {
      BVH4::Node* n = node.node();
      
      std::cout << "Node {" << std::endl;
      for (size_t i=0; i<BVH4::N; i++) 
      {
        for (size_t k=0; k<depth; k++) std::cout << "  ";
        std::cout << "  bounds" << i << " = " << n->bounds(i) << std::endl;
      }

      for (size_t i=0; i<BVH4::N; i++) 
      {
        if (n->child(i) == BVH4::emptyNode)
          continue;

        for (size_t k=0; k<depth; k++) std::cout << "  ";
        std::cout << "  child" << i << " = ";
        print_bvh4_triangle4v(n->child(i),depth+1); 
      }
      for (size_t k=0; k<depth; k++) std::cout << "  ";
      std::cout << "}" << std::endl;
    }
    else
    {
      size_t num; 
      const Triangle4v* tri = (const Triangle4v*) node.leaf(num);

      std::cout << "Leaf {" << std::endl;
      for (size_t i=0; i<num; i++) {
        for (size_t j=0; j<tri[i].size(); j++) {
          for (size_t k=0; k<depth; k++) std::cout << "  ";
          std::cout << "  Triangle { v0 = (" << tri[i].v0.x[j] << ", " << tri[i].v0.y[j] << ", " << tri[i].v0.z[j] << "),  "
            "v1 = (" << tri[i].v1.x[j] << ", " << tri[i].v1.y[j] << ", " << tri[i].v1.z[j] << "), "
            "v2 = (" << tri[i].v2.x[j] << ", " << tri[i].v2.y[j] << ", " << tri[i].v2.z[j] << "), "
            "geomID = " << tri[i].geomID(j) << ", primID = " << tri[i].primID(j) << " }" << std::endl;
        }
      }
      for (size_t k=0; k<depth; k++) std::cout << "  ";
      std::cout << "}" << std::endl;
    }
  }

  /* prints the triangle BVH of a scene */
  void print_bvh(RTCScene scene)
  {
    /* if the scene contains only triangles, the BVH4 acceleration structure can be obtained this way */
    AccelData* accel = ((Accel*)scene)->intersectors.ptr;
    BVH4* bvh4 = dynamic_cast<BVH4*>(accel);

    /* if there are also other geometry types, one has to iterate over the toplevel AccelN structure */
    if (bvh4 == nullptr) 
    {
      AccelN* accelN = dynamic_cast<AccelN*>(accel);
      if (accelN == nullptr)
        throw std::runtime_error("cannot access BVH4 acceleration structure"); // will not happen if you use this Embree version
      
      for (size_t i=0; i<accelN->accels.size() && bvh4 == nullptr; i++)
        bvh4 = dynamic_cast<BVH4*>(accelN->accels[i]->intersectors.ptr);
    }
    if (bvh4 == nullptr)
      throw std::runtime_error("cannot access BVH4 acceleration structure"); // will not happen if you use this Embree version
      
    /* now lets print the entire hierarchy */
    print_bvh4_triangle4v(bvh4->root,0);
  }

  /* main function in embree namespace */
  int main(int argc, char** argv) 
  {
    /* for best performance set FTZ and DAZ flags in MXCSR control and status register */
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    /* initialize ray tracing core and force bvh4.triangle4v hierarchy for triangles */
    rtcInit("tri_accel=bvh4.triangle4v");
    
    /* set error handler */
    rtcSetErrorFunction(error_handler);
    
    /* create scene */
    g_scene = rtcNewScene(RTC_SCENE_STATIC,RTC_INTERSECT1);
    addCube(g_scene,Vec3fa(-1,0,0));
    addCube(g_scene,Vec3fa(1,0,0));
    addCube(g_scene,Vec3fa(0,0,-1));
    addCube(g_scene,Vec3fa(0,0,1));
    addHair(g_scene);
    addGroundPlane(g_scene);
    rtcCommit (g_scene);

    /* print triangle BVH */
    print_bvh(g_scene);

    /* cleanup */
    rtcDeleteScene (g_scene);
    rtcExit();
    return 0;
  }
}

int main(int argc, char** argv)
{
  try {
    return embree::main(argc, argv);
  }
  catch (const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cout << "Error: unknown exception caught." << std::endl;
    return 1;
  }
}