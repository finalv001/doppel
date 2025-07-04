/*
 * Copyright 2023 Vienna University of Technology.
 * Institute of Computer Graphics and Algorithms.
 * This file is part of the GCG Lab Framework and must not be redistributed.
 */
#pragma once


#include "Utils.h"
#include <GL/glew.h>
#include <string>
#include <iostream>

/*!
 * 2D texture
 */
class Texture {
  protected:
    GLuint _handle;
    bool _init;

  public:
    /*!
     * Creates a texture from a file
     * @param file: path to the texture file (a DSS image)
     */
    Texture(std::string file);
    ~Texture();

    /*!
     * Activates the texture unit and binds this texture
     * @param unit: the texture unit
     */
    void bind(unsigned int unit);

    GLuint getID() const { return _handle; }
    bool isValid() const { return _init && _handle != 0; }
};

