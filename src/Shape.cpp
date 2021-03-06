#include "Shape.h"
#include <iostream>

#include "GLSL.h"
#include "Program.h"
#include "GameObject.h"
#include "TextureManager.h"
#include "GameManager.h"

#include <glm/gtx/rotate_vector.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

Shape::Shape() :
	vaoID(0),
	min(glm::vec3(0,0,0)),
	max(glm::vec3(0, 0, 0))
{
}

Shape::~Shape()
{
}

void Shape::loadMesh(const string &meshName, std::vector<std::string> manualTextures) {
	// Load geometry
	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;

    string mtlBase = "../resources/";

	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str(), mtlBase.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
        for(unsigned int i = 0; i < shapes.size(); i++) {
            posBuf.push_back(shapes[i].mesh.positions);
            texBuf.push_back(shapes[i].mesh.texcoords);
            eleBuf.push_back(shapes[i].mesh.indices);
            norBuf.push_back(shapes[i].mesh.normals);

            if (norBuf[i].size() == 0) {
                calculateNormals(i);
            }

            //TODO(nurgan) check if that assumption hold
            // theoreteically each face can have its own material.
            // as it always seems to be the case anyways, we assume one material is used for a whole SHAPE.
            int shapeMaterialID = shapes[i].mesh.material_ids[0];

            if(shapeMaterialID < 0 || shapeMaterialID > objMaterials.size() - 1) {
                // no material
                materialPresent.push_back(false);
                textureNames.push_back("");
                materials.push_back(nullptr);
            } else {

                materialPresent.push_back(true);

                std::shared_ptr<Material> material = std::make_shared<Material>();
                *material = {
                        objMaterials[shapeMaterialID].ambient[0], objMaterials[shapeMaterialID].ambient[1], objMaterials[shapeMaterialID].ambient[2],
                        objMaterials[shapeMaterialID].diffuse[0], objMaterials[shapeMaterialID].diffuse[1], objMaterials[shapeMaterialID].diffuse[2],
                        objMaterials[shapeMaterialID].specular[0], objMaterials[shapeMaterialID].specular[1], objMaterials[shapeMaterialID].specular[2],
                        objMaterials[i].shininess
                };

                materials.push_back(material);

                string shapeTextureName = objMaterials[shapeMaterialID].diffuse_texname;

                textureNames.push_back(shapeTextureName);

                //first check if there is a texture name
                if(shapeTextureName != "") {

                    if (textures.count(shapeTextureName) == 0) {
                        Texture *texture = new Texture();
                        texture->loadTexture("../resources/" + shapeTextureName, shapeTextureName);
                        textures[shapeTextureName] = texture;
                    }
                }
            }

            if(manualTextures.size() > 0) {
                manualTexturePresent = true;
                manualTextureNames = manualTextures;
            }

        }

		findAndSetMinAndMax();
	}
}

/**
 * Produces accurate normals from the vertices of the provided shape mesh
 *
 * Taken and modified from provided "lighting slides" pdf on PolyLearn along
 * with tips given on the PolyLearn forms by Prof. Wood
 */

void Shape::calculateNormals(int i) {
	float v1[3], v2[3], nor[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;
	float length;

	// Set all vertex normals to zero so we can add the adjacent face normals to each
    norBuf[i].clear();
    norBuf[i].resize(posBuf[i].size());

    for (size_t v = 0; v < norBuf[i].size(); ++v) {
        norBuf[i][v] = 0.0;
    }

    // Calculate the normal for each face; add to adjacent vertices
    for (size_t v = 0; v < eleBuf[i].size() / 3; ++v) {
        int idx1 = eleBuf[i][3 * v + 0];
        int idx2 = eleBuf[i][3 * v + 1];
        int idx3 = eleBuf[i][3 * v + 2];

        // Calculate two vectors from the three points
        v1[x] = posBuf[i][3 * idx1 + x] - posBuf[i][3 * idx2 + x];
        v1[y] = posBuf[i][3 * idx1 + y] - posBuf[i][3 * idx2 + y];
        v1[z] = posBuf[i][3 * idx1 + z] - posBuf[i][3 * idx2 + z];

        v2[x] = posBuf[i][3 * idx2 + x] - posBuf[i][3 * idx3 + x];
        v2[y] = posBuf[i][3 * idx2 + y] - posBuf[i][3 * idx3 + y];
        v2[z] = posBuf[i][3 * idx2 + z] - posBuf[i][3 * idx3 + z];

        // Take the cross product of the two vectors to get
        // the normal vector which will be stored in out
        nor[x] = v1[y] * v2[z] - v1[z] * v2[y];
        nor[y] = v1[z] * v2[x] - v1[x] * v2[z];
        nor[z] = v1[x] * v2[y] - v1[y] * v2[x];

        // Normalize the vector
        length = sqrt(nor[x] * nor[x] + nor[y] * nor[y] + nor[z] * nor[z]);

        nor[x] /= length;
        nor[y] /= length;
        nor[z] /= length;

        // Set the normal into the shape's normal buffer
        norBuf[i][3 * idx1 + x] += nor[x];
        norBuf[i][3 * idx1 + y] += nor[y];
        norBuf[i][3 * idx1 + z] += nor[z];

        norBuf[i][3 * idx2 + x] += nor[x];
        norBuf[i][3 * idx2 + y] += nor[y];
        norBuf[i][3 * idx2 + z] += nor[z];

        norBuf[i][3 * idx3 + x] += nor[x];
        norBuf[i][3 * idx3 + y] += nor[y];
        norBuf[i][3 * idx3 + z] += nor[z];
    }

    // Normalize each vector's normal, effectively giving us a weighted average based on
    // the area of each face adjacent to each vertex
    for (size_t v = 0; v < norBuf[i].size() / 3; ++v) {
        nor[x] = norBuf[i][(v * 3) + x];
        nor[y] = norBuf[i][(v * 3) + y];
        nor[z] = norBuf[i][(v * 3) + z];

        length = sqrt(nor[x] * nor[x] + nor[y] * nor[y] + nor[z] * nor[z]);

        nor[x] /= length;
        nor[y] /= length;
        nor[z] /= length;

        norBuf[i][(v * 3) + x] = nor[x];
        norBuf[i][(v * 3) + y] = nor[y];
        norBuf[i][(v * 3) + z] = nor[z];
    }


}

void Shape::resize() {
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
   float epsilon = 0.001;

	// From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
    maxExtent = 0.0f;
	xExtent = max.x - min.x;
	yExtent = max.y - min.y;
	zExtent = max.z - min.z;

	if (xExtent >= yExtent && xExtent >= zExtent) {
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent) {
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent) {
		maxExtent = zExtent;
	}

	scaleX = 2.0 / maxExtent;
	shiftX = min.x + (xExtent / 2.0);
	scaleY = 2.0 / maxExtent;
	shiftY = min.y + (yExtent / 2.0);
	scaleZ = 2.0 / maxExtent;
	shiftZ = min.z + (zExtent / 2.0);

    int bufNum = posBuf.size();
	// Go through all verticies shift and scale them
    for(int i = 0; i < bufNum; i++) {
        for (size_t v = 0; v < posBuf[i].size() / 3; v++) {
            posBuf[i][3 * v + 0] = (posBuf[i][3 * v + 0] - shiftX) * scaleX;
            assert(posBuf[i][3 * v + 0] >= -1.0 - epsilon);
            assert(posBuf[i][3 * v + 0] <= 1.0 + epsilon);

            posBuf[i][3 * v + 1] = (posBuf[i][3 * v + 1] - shiftY) * scaleY;
            assert(posBuf[i][3 * v + 1] >= -1.0 - epsilon);
            assert(posBuf[i][3 * v + 1] <= 1.0 + epsilon);

            posBuf[i][3 * v + 2] = (posBuf[i][3 * v + 2] - shiftZ) * scaleZ;
            assert(posBuf[i][3 * v + 2] >= -1.0 - epsilon);
            assert(posBuf[i][3 * v + 2] <= 1.0 + epsilon);
        }
    }

	// Shift and scale min and max values
	float minX = (min.x - shiftX) * scaleX;
	float minY = (min.y - shiftY) * scaleY;
	float minZ = (min.z - shiftZ) * scaleZ;
	min = glm::vec3(minX, minY, minZ);

	float maxX = (max.x - shiftX) * scaleX;
	float maxY = (max.y - shiftY) * scaleY;
	float maxZ = (max.z - shiftZ) * scaleZ;
	max = glm::vec3(maxX, maxY, maxZ);

}

void Shape::init() {

    int bufNum = posBuf.size();


    for(int i = 0; i < bufNum; i++) {

        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);

        // Send the position array to the GPU
        unsigned posBufIDRef;
        glGenBuffers(1, &posBufIDRef);
        posBufID.push_back(posBufIDRef);
        glBindBuffer(GL_ARRAY_BUFFER, posBufID[i]);
        glBufferData(GL_ARRAY_BUFFER, posBuf[i].size() * sizeof(float), &(posBuf[i][0]), GL_STATIC_DRAW);

        // Send the normal array to the GPU
        if (norBuf[i].empty()) {
            norBufID.push_back(0);
        } else {
            unsigned norBufIDRef;
            glGenBuffers(1, &norBufIDRef);
            norBufID.push_back(norBufIDRef);
            glBindBuffer(GL_ARRAY_BUFFER, norBufID[i]);
            glBufferData(GL_ARRAY_BUFFER, norBuf[i].size() * sizeof(float), &(norBuf[i][0]), GL_STATIC_DRAW);
        }

        // Send the texture array to the GPU
        if (texBuf[i].empty()) {
            texBufID.push_back(0);
        } else {
            unsigned texBufIDRef;
            glGenBuffers(1, &texBufIDRef);
            texBufID.push_back(texBufIDRef);
            glBindBuffer(GL_ARRAY_BUFFER, texBufID[i]);
            glBufferData(GL_ARRAY_BUFFER, texBuf[i].size() * sizeof(float), &(texBuf[i][0]), GL_STATIC_DRAW);
        }

        // Send the element array to the GPU
        unsigned eleBufIDRef;
        glGenBuffers(1, &eleBufIDRef);
        eleBufID.push_back(eleBufIDRef);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf[i].size() * sizeof(unsigned int), &(eleBuf[i][0]), GL_STATIC_DRAW);

        // Unbind the arrays
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        assert(glGetError() == GL_NO_ERROR);

    }


}

void Shape::draw(const shared_ptr<Program> prog, std::shared_ptr<Material> defaultMtl) {
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

    glBindVertexArray(vaoID);

    TextureManager textureManager = TextureManager::instance();
    int t = textureManager.getTextureToggle();
    if(manualTexturePresent) {

        if(manualTextureNames.size() == 1) {
            t = 0;
        } else {
            if(GameManager::instance().isInHellMode()) {
                t = 1;
            } else {
                t = 0;
            }
            //t = t % (manualTextureNames.size()) ;
        }

        TextureManager::instance().getTexture(manualTextureNames[t])->bind(0,prog);
        glUniform1i(prog->getUniform("textureActive"), 1);
    }

    int bufNum = posBuf.size();
    // Send the position array to the GPU
    for(int i = 0; i < bufNum; i++) {

        // Bind position buffer
        h_pos = prog->getAttribute("vertPos");
        GLSL::enableVertexAttribArray(h_pos);
        glBindBuffer(GL_ARRAY_BUFFER, posBufID[i]);
        glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);

        // Bind normal buffer
        h_nor = prog->getAttribute("vertNor");
        if (h_nor != -1 && norBufID[i] != 0) {
            GLSL::enableVertexAttribArray(h_nor);
            glBindBuffer(GL_ARRAY_BUFFER, norBufID[i]);
            glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
        }

        if (texBufID[i] != 0) {
            // Bind texcoords buffer
            h_tex = prog->getAttribute("vertTex");
            if (h_tex != -1 && texBufID[i] != 0) {
                GLSL::enableVertexAttribArray(h_tex);
                glBindBuffer(GL_ARRAY_BUFFER, texBufID[i]);
                glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
            }
        }

        // Bind element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID[i]);

        if(materialPresent[i]) {
           bindMtl(prog, materials[i]);
        } else {
           bindMtl(prog, defaultMtl);
        }

        // check if texture for shape
        if(!manualTexturePresent) {
            if (textureNames[i] != "") {
                textures[textureNames[i]]->bind(0, prog);
                glUniform1i(prog->getUniform("textureActive"), 1);
            } else {
                glUniform1i(prog->getUniform("textureActive"), 0);
            }
        }
        // Draw
        glDrawElements(GL_TRIANGLES, (int) eleBuf[i].size(), GL_UNSIGNED_INT, (const void *) 0);

        if(!manualTexturePresent) {
            if (textureNames[i] != "") {
                textures[textureNames[i]]->unbind();
            }
        }
    }
    // Disable and unbind
    if(manualTexturePresent) {
        TextureManager::instance().getTexture(manualTextureNames[t])->unbind();
    }

    if (h_tex != -1) {
        GLSL::disableVertexAttribArray(h_tex);
    }
    if (h_nor != -1) {
        GLSL::disableVertexAttribArray(h_nor);
    }

    GLSL::disableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

float Shape::randFloat(float a, float b) {
   float randNum = ((float)rand()) / (float)RAND_MAX;
   float diff = b - a;
   float off = randNum * diff;

   return a + off;
}

std::shared_ptr<std::vector<glm::vec3>> Shape::calcFragmentDir(glm::vec3 direction) {
   std::shared_ptr<std::vector<glm::vec3>> eleDir = std::make_shared<std::vector<glm::vec3>>();

   // Calculate the axis to randomize the direction of the objects about.
   glm::vec3 vertRotAxis = glm::vec3(0.0, 1.0, 0.0);
   glm::vec3 horRotAxis = glm::cross(direction, vertRotAxis);

   int bufNum = posBuf.size();
   for (int i = 0; i < bufNum; i++) {
      // Calculate a random direction to translate the point on.
      glm::vec3 dirHor = glm::rotate(direction, randFloat(-M_PI / 4.0f, M_PI / 4.0f), vertRotAxis);
      glm::vec3 dirVert = glm::rotate(direction, randFloat(-M_PI / 12.0f, M_PI / 6.0f), horRotAxis);
      glm::vec3 dir = glm::normalize((dirHor + dirVert) * 0.5f);

      eleDir->push_back(dir);
   }

   return eleDir;
}

void Shape::fracture(const std::shared_ptr<Program> prog, std::shared_ptr<Material> defaultMtl,
   std::shared_ptr<MatrixStack> M, std::shared_ptr<GameObject> obj) {
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);

    TextureManager textureManager = TextureManager::instance();
    int t = textureManager.getTextureToggle();
    if(manualTexturePresent) {

        if(manualTextureNames.size() == 1) {
            t = 0;
        } else {
            t = t % (manualTextureNames.size()) ;
        }

        TextureManager::instance().getTexture(manualTextureNames[t])->bind(0,prog);
        glUniform1i(prog->getUniform("textureActive"), 1);
    }

   int bufNum = posBuf.size();
   // Send the position array to the GPU
   for (int i = 0; i < bufNum; i++) {
      M->pushMatrix();
      M->loadIdentity();

      /* Load in a new model transform per shape within the larger Shape obj.
       * For fracturing we are just going to translate the shape. */ 
      M->translate(obj->getFragmentPos()->at(i));
      M->scale(obj->getScale());
      glm::mat4 rotation = obj->transform.getRotate();
      M->rotateMat4(rotation);
      glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, glm::value_ptr(M->topMatrix()));
      glm::mat4 tiM = glm::transpose(glm::inverse(M->topMatrix()));
      glUniformMatrix4fv(prog->getUniform("tiM"), 1, GL_FALSE, glm::value_ptr(tiM));

       // Bind position buffer
      h_pos = prog->getAttribute("vertPos");
      GLSL::enableVertexAttribArray(h_pos);
      glBindBuffer(GL_ARRAY_BUFFER, posBufID[i]);
      glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);

      // Bind normal buffer
      h_nor = prog->getAttribute("vertNor");
      if (h_nor != -1 && norBufID[i] != 0) {
         GLSL::enableVertexAttribArray(h_nor);
         glBindBuffer(GL_ARRAY_BUFFER, norBufID[i]);
         glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
      }

      if (texBufID[i] != 0) {
         // Bind texcoords buffer
         h_tex = prog->getAttribute("vertTex");
         if (h_tex != -1 && texBufID[i] != 0) {
            GLSL::enableVertexAttribArray(h_tex);
            glBindBuffer(GL_ARRAY_BUFFER, texBufID[i]);
            glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
         }
      }

      // Bind element buffer
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID[i]);

      if(materialPresent[i]) {
         bindMtl(prog, materials[i]);
      } else {
         bindMtl(prog, defaultMtl);
      }

       // check if texture for shape
       if(!manualTexturePresent) {
           if (textureNames[i] != "") {
               textures[textureNames[i]]->bind(0, prog);
               glUniform1i(prog->getUniform("textureActive"), 1);
           } else {
               glUniform1i(prog->getUniform("textureActive"), 0);
           }
       }

      // Draw
      glDrawElements(GL_TRIANGLES, (int) eleBuf[i].size(), GL_UNSIGNED_INT, (const void *) 0);


       if(!manualTexturePresent) {
           if (textureNames[i] != "") {
               textures[textureNames[i]]->unbind();
           }
       }

      M->popMatrix();
   }
    // Disable and unbind
    if(manualTexturePresent) {
        TextureManager::instance().getTexture(manualTextureNames[t])->unbind();
    }

   if (h_tex != -1) {
      GLSL::disableVertexAttribArray(h_tex);
   }
   if (h_nor != -1) {
      GLSL::disableVertexAttribArray(h_nor);
   }

   GLSL::disableVertexAttribArray(h_pos);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Shape::bindMtl(const std::shared_ptr<Program> prog, std::shared_ptr<Material> material) const {
    if(material != nullptr) {
        glUniform3f(prog->getUniform("MatAmb"), material->rAmb, material->gAmb, material->bAmb);
        glUniform3f(prog->getUniform("MatDif"), material->rDif, material->gDif, material->bDif);
        glUniform3f(prog->getUniform("MatSpc"), material->rSpc, material->gSpc, material->bSpc);
        glUniform1f(prog->getUniform("MatShiny"), material->shininess);
        glUniform1f(prog->getUniform("MatAlpha"), material->alpha);
    }

}

glm::vec3& Shape::getMin() {
	return this->min;
}

glm::vec3& Shape::getMax() {
	return this->max;
}

void Shape::findAndSetMinAndMax(glm::mat4 orientTransform) {
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = FLT_MAX;
	maxX = maxY = maxZ = -FLT_MAX;

    int bufNum = posBuf.size();
	// Go through all vertices to determine min and max of each dimension
    for(int i = 0; i < bufNum; i++) {
        for (size_t v = 0; v < posBuf[i].size() / 3; v++) {
            glm::vec3 curVertex(posBuf[i][3 * v + 0], posBuf[i][3 * v + 1], posBuf[i][3 * v + 2]);
            curVertex = orientTransform * glm::vec4(curVertex, 1.0f);

            if (curVertex.x < minX) minX = curVertex.x;
            if (curVertex.x > maxX) maxX = curVertex.x;

            if (curVertex.y < minY) minY = curVertex.y;
            if (curVertex.y > maxY) maxY = curVertex.y;

            if (curVertex.z < minZ) minZ = curVertex.z;
            if (curVertex.z > maxZ) maxZ = curVertex.z;
        }
    }
	min = glm::vec3(minX, minY, minZ);
	max = glm::vec3(maxX, maxY, maxZ);
}
