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
#include <unordered_map>

struct Texture {
    GLuint ID;
    std::string type;
    std::string path;
};

// TEXTURE IS USELESS FOR NOW BUT ILL DO LATER

class Model {
    public:
    std::vector<Object> Objects;
    ComputeObjectArray objectHandler;
    std::vector<Texture> textures_loaded;

    GLuint TextureArrayID = 0;

    std::unordered_map<std::string, Texture> textureCache;
    
        Model(const char *path) : objectHandler(Objects) {
            std::cerr << "Loading model...\n";
            loadModel(path);
            BuildTextureArray();
            std::cout << "Texture array ID: " << TextureArrayID << std::endl;
        }
        inline std::vector<Object> &GetObjectVector() {
            return Objects;
        }
        inline GLuint GetTextureArrayID() {
            return TextureArrayID;
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
            float Scale = 0.7f;
            //cerr << "EBO creation\n";

            // indices

            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                Triangle tri;
                aiFace face = mesh->mFaces[i];
                unsigned int i0 = face.mIndices[0];
                unsigned int i1 = face.mIndices[1];
                unsigned int i2 = face.mIndices[2];

                tri.v0.Position = glm::vec3(mesh->mVertices[i0].x, mesh->mVertices[i0].y, mesh->mVertices[i0].z) * Scale;
                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i0].x; 
                    vec.y = mesh->mTextureCoords[0][i0].y;
                    tri.v0.TexCoords = vec;
                } else {
                    tri.v0.TexCoords = glm::vec2(0.0f);
                }
                if (mesh->HasNormals()) { //normal add
                    tri.v0.Normal.x = mesh->mNormals[i0].x;
                    tri.v0.Normal.y = mesh->mNormals[i0].y;
                    tri.v0.Normal.z = mesh->mNormals[i0].z; 
                }

                tri.v1.Position = glm::vec3(mesh->mVertices[i1].x, mesh->mVertices[i1].y, mesh->mVertices[i1].z) * Scale;
                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i1].x; 
                    vec.y = mesh->mTextureCoords[0][i1].y;
                    tri.v1.TexCoords = vec;
                } else {
                    tri.v1.TexCoords = glm::vec2(0.0f);
                }
                if (mesh->HasNormals()) { //normal add
                    tri.v1.Normal.x = mesh->mNormals[i1].x;
                    tri.v1.Normal.y = mesh->mNormals[i1].y;
                    tri.v1.Normal.z = mesh->mNormals[i1].z; 
                }

                tri.v2.Position = glm::vec3(mesh->mVertices[i2].x, mesh->mVertices[i2].y, mesh->mVertices[i2].z) * Scale;
                if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates?
                    glm::vec2 vec;
                    vec.x = mesh->mTextureCoords[0][i2].x; 
                    vec.y = mesh->mTextureCoords[0][i2].y;
                    tri.v2.TexCoords = vec;
                } else {
                    tri.v2.TexCoords = glm::vec2(0.0f);
                }   
                if (mesh->HasNormals()) { //normal add
                    tri.v2.Normal.x = mesh->mNormals[i2].x;
                    tri.v2.Normal.y = mesh->mNormals[i2].y;
                    tri.v2.Normal.z = mesh->mNormals[i2].z; 
                }

                aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
                aiColor3D EmissiveColor(0.0f, 0.0f, 0.0f);
                float EmissiveIntensity = 1.0f;

                if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, EmissiveColor) == AI_SUCCESS && (EmissiveColor.r > 0.0 || EmissiveColor.g > 0.0 || EmissiveColor.b > 0.0)) {
                    mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, EmissiveIntensity);
                    objectHandler.addTriangleEmission(tri.v0, tri.v1, tri.v2, glm::vec3(EmissiveColor.r, EmissiveColor.g, EmissiveColor.b) * EmissiveIntensity);
                } else {
                    objectHandler.addTriangle(tri.v0, tri.v1, tri.v2, glm::vec3(0.0), 0.0, 1, 0); //temp colour
                }
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

                std::string fullPath = directory + '/' + str.C_Str();

                std::cout << "Looking up: " << fullPath << "... ";

                auto it = textureCache.find(fullPath);
                if (it != textureCache.end()) { //if already loaded
                    textures.push_back(it->second);
                    std::cout << "already loaded." << std::endl;
                    continue;
                }
                std::cout << "new texture." << std::endl;

                Texture texture;
                texture.ID = TextureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = fullPath;
                textures.push_back(texture);
                textures_loaded.push_back(texture); // add to loaded textures

                textureCache[fullPath] = texture;
            }

            std::cout << "Textures all loaded. Returning...\n";
            return textures;
        } 

        void BuildTextureArray() { //GL_TEXTURE_2D_ARRAY
            if (textures_loaded.empty()) return; //most obvious check ever?

            glm::ivec2 MaxSize(0); //maximum size of a texture

            for (auto &texIt : textureCache) {
                Texture &Tex = texIt.second;
                glm::ivec2 Size;
                stbi_info(Tex.path.c_str(), &Size.x, &Size.y, nullptr); //idc about channel #
                MaxSize = glm::max(MaxSize, Size);
            }

            glGenTextures(1, &TextureArrayID);
            glBindTexture(GL_TEXTURE_2D_ARRAY, TextureArrayID);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MaxSize.x, MaxSize.y, textures_loaded.size()); //depth = size of textures_loaded

            int index = 0;
            for (auto &texIt : textureCache) {
                Texture &Tex = texIt.second;

                glm::ivec2 size;
                unsigned char *data = stbi_load(Tex.path.c_str(), &size.x, &size.y, nullptr, 4); //req_comp is requested components (components = colour components = channels)

                if (data) { //does data exist?? if so then write it to the 2d texture array
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, size.x, size.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

                    //free it now since it exists it needs to be unloaded
                    stbi_image_free(data);
                }

                index++;
            }

            //final stuff
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            //reset
        }

        unsigned int TextureFromFile(const char *path, const std::string &directory) {
            std::string filename = std::string(path);
            filename = directory + '/' + filename;
            std::cout << "Loading texture: " << filename << std::endl;

            unsigned int textureID;
            glGenTextures(1, &textureID);

            stbi_set_flip_vertically_on_load(true);

            int width, height, nrComponents;
            unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

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