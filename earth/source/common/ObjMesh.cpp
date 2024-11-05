#include "common.h"


bool Mesh::loadOBJ(const char * path){
  std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
  std::vector< vec3 > temp_vertices;
  std::vector< vec2 > temp_uvs;
  std::vector< vec3 > temp_normals;
  
  hasUV = true;
  
#ifdef _WIN32
  std::wstring wcfn;
  if (u8names_towc(path, wcfn) != 0)
    return NULL;
  FILE * file = _wfopen(wcfn.c_str(), L"r");
#else
  FILE * file = fopen(path, "r");
#endif //_WIN32

  if( file == NULL ){
    printf("Impossible to open the file !\n");
    return false;
  }
  
  char *line = new char[128];
  char *lineHeader = new char[128];
  
  while(true){
    memset(line, 0 , 128);
    memset(lineHeader, 0 , 128);
    
    if(fgets(line, 128, file) == NULL){ break; }
    sscanf(line, "%s ", lineHeader);
    
    if ( strcmp( lineHeader, "v" ) == 0 ){
      vec3 vertex;
      sscanf(&line[2], "%f %f %f", &vertex.x, &vertex.y, &vertex.z );
      temp_vertices.push_back(vertex);
      if(vertex.x < box_min.x){box_min.x = vertex.x; }
      if(vertex.y < box_min.y){box_min.y = vertex.y; }
      if(vertex.z < box_min.z){box_min.z = vertex.z; }
      if(vertex.x > box_max.x){box_max.x = vertex.x; }
      if(vertex.y > box_max.y){box_max.y = vertex.y; }
      if(vertex.z > box_max.z){box_max.z = vertex.z; }
    }else if ( strcmp( lineHeader, "vt" ) == 0 ){
      vec2 uv;
      sscanf(&line[3], "%f %f", &uv.x, &uv.y );
      temp_uvs.push_back(uv);
    }else if ( strcmp( lineHeader, "vn" ) == 0 ){
      vec3 normal;
      sscanf(&line[3], "%f %f %f", &normal.x, &normal.y, &normal.z );
      temp_normals.push_back(normal);
    }else if ( strcmp( lineHeader, "f" ) == 0 ){
      std::string vertex1, vertex2, vertex3;
      int vertexIndex[3], uvIndex[3], normalIndex[3];
      int matches = sscanf(&line[2], "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                           &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                           &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
      if (matches != 9){
        int matches = sscanf(&line[2], "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0],
                             &vertexIndex[1], &normalIndex[1],
                             &vertexIndex[2], &normalIndex[2]);
        if (matches == 6){
          hasUV = false;
        } else {
          printf("File can't be read by our simple parser : ( Try exporting with other options\n");
          return false;
        }
      }
      
      /* handle negative indices */
      /* (adjust for size during processing of each face, as per the old
       *  OBJ specification, instead of after the end of the file) */
      for (int negati = 0; negati < 3; negati++){
        if (vertexIndex[negati] < 0){
          vertexIndex[negati]+=temp_vertices.size();
          vertexIndex[negati]++; /* <- OBJ indices are one-based */
        }
        if (uvIndex[negati] < 0){
          uvIndex[negati]+=temp_uvs.size();
          uvIndex[negati]++;
        }
        if (normalIndex[negati] < 0){
          normalIndex[negati]+=temp_normals.size();
          normalIndex[negati]++;
        }
      }
      
      vertexIndices.push_back(vertexIndex[0]);
      vertexIndices.push_back(vertexIndex[1]);
      vertexIndices.push_back(vertexIndex[2]);
      if(hasUV){
        uvIndices    .push_back(uvIndex[0]);
        uvIndices    .push_back(uvIndex[1]);
        uvIndices    .push_back(uvIndex[2]);
      }
      normalIndices.push_back(normalIndex[0]);
      normalIndices.push_back(normalIndex[1]);
      normalIndices.push_back(normalIndex[2]);
    }
  }
  
  delete[] line;
  delete[] lineHeader;
  
  // For each vertex of each triangle
  for( unsigned int i=0; i<vertexIndices.size(); i++ ){
    unsigned int vertexIndex = vertexIndices[i];
    vec4 vertex = vec4(temp_vertices[ vertexIndex-1 ], 1.0);
    vertices.push_back(vertex);
  }
  
  if(hasUV){
    for( unsigned int i=0; i<uvIndices.size(); i++ ){
      unsigned int uvIndex = uvIndices[i];
      vec2 uv = temp_uvs[ uvIndex-1 ];
      uvs.push_back(uv);
    }
  }
  
  for( unsigned int i=0; i<normalIndices.size(); i++ ){
    unsigned int normalIndex = normalIndices[i];
    vec3 normal = temp_normals[ normalIndex-1 ];
    normals.push_back(normal);
  }
    
  center = box_min+(box_max-box_min)/2.0;
  scale = (std::max)(box_max.x - box_min.x, box_max.y-box_min.y);
  
  model_view = Scale(1.0/scale,           //Make the extents 0-1
                     1.0/scale,
                     1.0/scale)*
  Translate(-center);  //Orient Model About Center
  
  
  return true;
}

bool Mesh::makeSphere(int steps) {
  vertices.clear();
  normals.clear();
  uvs.clear();

  // Define the steps of theta (longitude) and phi (latitude)
  double step_theta = (2 * M_PI) / (steps - 1);
  double step_phi = M_PI / (steps - 1);

  // Creates the vertices, normals, and texture coordinates of the sphere
  for (unsigned int i = 0; i < steps; i++) {
    double phi = i * step_phi;

    for (unsigned int j = 0; j < steps; j++) {
      double theta = j * step_theta;

      float x = cos(theta) * sin(phi);
      float y = cos(phi);
      float z = sin(theta) * sin(phi);
      vec3 vertex = vec3(x, y, z);

      // Stores vertex positions, normals, and texture coordinates
      vertices.push_back(vec4(vertex, 1.0));
      normals.push_back(vertex);

      // Adjust texture coordinates to ensure correct mapping
      float u = 1.0f - (float)j / (steps - 1);
      float v = 1.0f - (float)i / (steps - 1);
      uvs.push_back(vec2(u, v));
    }
  }

  // Create a triangle index, cover the entire sphere
  std::vector<vec4> triangle_vertices;
  std::vector<vec3> triangle_normals;
  std::vector<vec2> triangle_uvs;

  for (unsigned int i = 0; i < steps - 1; i++) {
    for (unsigned int j = 0; j < steps - 1; j++) {

      int current = i * steps + j;
      int next = current + steps;

      triangle_vertices.push_back(vertices[current]);
      triangle_vertices.push_back(vertices[next]);
      triangle_vertices.push_back(vertices[current + 1]);
      triangle_normals.push_back(normals[current]);
      triangle_normals.push_back(normals[next]);
      triangle_normals.push_back(normals[current + 1]);
      triangle_uvs.push_back(uvs[current]);
      triangle_uvs.push_back(uvs[next]);
      triangle_uvs.push_back(uvs[current + 1]);

      triangle_vertices.push_back(vertices[current + 1]);
      triangle_vertices.push_back(vertices[next]);
      triangle_vertices.push_back(vertices[next + 1]);
      triangle_normals.push_back(normals[current + 1]);
      triangle_normals.push_back(normals[next]);
      triangle_normals.push_back(normals[next + 1]);
      triangle_uvs.push_back(uvs[current + 1]);
      triangle_uvs.push_back(uvs[next]);
      triangle_uvs.push_back(uvs[next + 1]);
    }
  }

  // Update the final vertex, normal, and texture coordinate data
  vertices = triangle_vertices;
  normals = triangle_normals;
  uvs = triangle_uvs;

  return true;
}
