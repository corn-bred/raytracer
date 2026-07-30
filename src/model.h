#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shaders.h"
//#include "texturebuffer.h"
#include "objecthandler.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

struct Texture {
    GLuint ID;
    std::string type;
    std::string path;
};

// TEXTURE IS USELESS FOR NOW BUT ILL DO LATER

class Model {
    std::vector<Object> Objects;
    ObjectArray objectHandler;
    std::vector<Texture> textures_loaded;

    public:
        Model(const char *path) : objectHandler(Objects) {
            std::cerr << "Loading model...\n";
            loadModel(path);
        }
        inline std::vector<Object> &GetObjectVector() {
            return Objects;
        }
    private:
        // model data
        std::string directory;

        void loadModel(std::string path) {
            Assimp::Importer import;
            std::cerr << "Importing scene...\n";
            const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);	
	
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
            {
                std::cout << "ASSIMP ERROR:" << import.GetErrorString() << std::endl;
                return;
            }
            directory = path.substr(0, path.find_last_of('/'));
            std::cerr << "Scene imported. Processing nodes and data...\n";
            processNode(scene->mRootNode, scene);
            std::cerr << "Data and nodes processed.\n";
        }

        void processNode(aiNode *node, const aiScene *scene) {
            //cerr << "Processing meshes in scene...\n";
            for (unsigned int i = 0; i < node->mNumMeshes; i++) {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
                processMesh(mesh, scene);
                //cerr << "Mesh returned successfully.\n";
            }
            //cerr << "Meshes processed. Processing the nodes...\n";
            
            for (unsigned int i = 0; i < node->mNumChildren; i++) {
                processNode(node->mChildren[i], scene);
            }
        }  

        void processMesh(aiMesh *mesh, const aiScene *scene) {
            //cerr << "EBO creation\n";
            // indices
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                Triangle tri;
                aiFace face = mesh->mFaces[i];
                unsigned int i0 = face.mIndices[0];
                unsigned int i1 = face.mIndices[1];
                unsigned int i2 = face.mIndices[2];

                tri.v0.Position = glm::vec3(mesh->mVertices[i0].x, mesh->mVertices[i0].y, mesh->mVertices[i0].z);
                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i0].x; 
                    vec.y = mesh->mTextureCoords[0][i0].y;
                    tri.v0.TexCoords = vec;
                } else {
                    tri.v0.TexCoords = glm::vec2(0.0f);
                }

                tri.v1.Position = glm::vec3(mesh->mVertices[i1].x, mesh->mVertices[i1].y, mesh->mVertices[i1].z);
                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i1].x; 
                    vec.y = mesh->mTextureCoords[0][i1].y;
                    tri.v1.TexCoords = vec;
                } else {
                    tri.v1.TexCoords = glm::vec2(0.0f);
                }

                tri.v2.Position = glm::vec3(mesh->mVertices[i2].x, mesh->mVertices[i2].y, mesh->mVertices[i2].z);
                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i2].x; 
                    vec.y = mesh->mTextureCoords[0][i2].y;
                    tri.v2.TexCoords = vec;
                } else {
                    tri.v2.TexCoords = glm::vec2(0.0f);
                }   
                objectHandler.addTriangle(tri.v0, tri.v1, tri.v2, glm::vec3(1.0, 0.6, 0.0)); //temp colour
            }  

            //cerr << "Texture creation\n";
            //// process material
            //cerr << "Material processing...\n";
            if(mesh->mMaterialIndex >= 0) {
                //cerr << "Mesh has material index.\n";
                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
                //cerr << "Inserting diffuse textures...\n";
                textures_loaded.insert(textures_loaded.end(), diffuseMaps.begin(), diffuseMaps.end());
                //cerr << "Diffuse textures loaded.";
                std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
                //cerr << "Inserting specular textures...\n";
                textures_loaded.insert(textures_loaded.end(), specularMaps.begin(), specularMaps.end());
                //cerr << "Specular textures loaded.\n";
            }
            //cerr << "Mesh processing complete. Returning...\n";
            std::cout << "Mesh has " << mesh->mNumVertices << " vertices and " << mesh->mNumFaces << " faces." << std::endl;
        }  

        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
            //cerr << "Loading textures...\n";
            std::vector<Texture> textures;
            //cerr << "Material count: " << mat->GetTextureCount(type) << "\n";
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
                //cerr << "Material #" << i << "\n";
                aiString str;
                mat->GetTexture(type, i, &str);
                bool skip = false;
                for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                    if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) { //are the textures the same?
                        textures.push_back(textures_loaded[j]); //add the texture then
                        skip = true; 
                        //cerr << "Texture added\n";
                        break;
                    }
                } if(!skip) {   // if texture hasn't been loaded already, load it
                    //cerr << "New texture found\n";
                    Texture texture;
                    texture.ID = TextureFromFile(str.C_Str(), directory);
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture); // add to loaded textures
                    //cerr << "New texture loaded\n";
                }
            }
            std::cerr << "Textures all loaded. Returning...\n";
            return textures;
        } 

        unsigned int TextureFromFile(const char *path, const std::string &directory) {
            std::string filename = std::string(path);
            filename = directory + '/' + filename;

            unsigned int textureID;
            glGenTextures(1, &textureID);

            int width, height, nrComponents;
            unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

            stbi_set_flip_vertically_on_load(true);
            if (data) {
                GLenum format;
                if (nrComponents == 1)
                    format = GL_RED;
                else if (nrComponents == 3)
                    format = GL_RGB;
                else if (nrComponents == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
            }
            else {
                std::cout << "Texture failed to load at path: " << path << std::endl;
                stbi_image_free(data);
            }

        return textureID;
    }
};