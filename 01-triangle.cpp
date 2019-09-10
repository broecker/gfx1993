#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>

#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform2.hpp>

#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "common/Color.h"
#include "common/Vertex.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"

unsigned int 	width = 640, height = 480;
std::string binaryName;

VertexList vertices;
IndexList indices;

std::shared_ptr<render::Framebuffer>	framebuffer;
std::shared_ptr<render::Depthbuffer>	depthbuffer;
std::unique_ptr<render::Rasterizer>		rasterizer;

std::shared_ptr<render::DefaultVertexTransform>	vertexShader;
std::shared_ptr<render::FragmentShader>	fragmentShader;

const static int MAX_FRAMES = 36;

// A single frame -- rotates and draws the triangle.
static void frame(int counter)
{
	// clear the buffers	
	Color clearColour(0, 0, 0.2f, 1.f);
	framebuffer->clear( clearColour );
	depthbuffer->clear();

	// reset the render matrices
	float rotation = (float)counter / MAX_FRAMES * glm::pi<float>() * 2;

    vertexShader->modelMatrix = glm::rotate(rotation, glm::vec3(0,1,0));
    vertexShader->viewMatrix = glm::lookAt(glm::vec3(0, 1, -10), glm::vec3(0,0,0), glm::vec3(0,1,0));
    vertexShader->projectionMatrix = glm::perspective(glm::radians(60.f), (float)width/height, 0.2f, 100.f);

	try
	{
	    rasterizer->drawTriangles(vertices, indices);
	}
	catch (const char* txt)
	{
		std::cerr << "Render error :\"" << txt << "\"\n";
	}
}

// Writes out a ppm image of the framebuffer.
void writeImage(int counter)
{
    char filename[128];
    sprintf(filename, "%s-%02d.ppm", binaryName.c_str(), counter);

    std::ofstream file(filename);
    file << "P3"  << std::endl;;
    file << width << " " << height << std::endl;
    file << 255 << std::endl;

    for (unsigned int y = 0; y < framebuffer->getHeight(); ++y)
    {
        for (unsigned int x = 0; x < framebuffer->getWidth(); ++x)
        {
            const Color& c = framebuffer->getPixels()[x + y*framebuffer->getWidth()];
            file << (int)(c.r * 255) << " " << (int)(c.g * 255) << " " << (int)(c.b * 255) << " ";
        }
        file << std::endl;
    }

    std::clog << "Wrote " << filename << std::endl;
    file.close();
}


int main(int argc, char** argv)
{
    binaryName = argv[0];

	srand( time(0) );

	// Setup the rasterizer and the shaders.
    rasterizer = std::make_unique<render::Rasterizer>();
    rasterizer->viewport = std::make_shared<render::Viewport>(0, 0, width, height);

	framebuffer = std::make_shared<render::Framebuffer>(width, height);
    rasterizer->framebuffer = framebuffer;

	depthbuffer = std::make_shared<render::Depthbuffer>(width, height);
    rasterizer->depthbuffer = depthbuffer;

    vertexShader = std::make_shared<render::DefaultVertexTransform>();
    rasterizer->vertexShader = vertexShader;

	fragmentShader = std::make_shared<render::InputColourShader>();
    rasterizer->fragmentShader = fragmentShader;

    // Create the triangle geometry
    vertices.push_back(Vertex(glm::vec4(-5, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec2(0,0)));
    vertices.push_back(Vertex(glm::vec4( 0, 5, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0,0)));
    vertices.push_back(Vertex(glm::vec4( 5, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(0, 1, 0, 1), glm::vec2(0,0)));

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(1);

    // Draw our frames.
    for (int i = 0; i < MAX_FRAMES; ++i)
    {
        frame(i);
        writeImage(i);
    }

	return 0;
}
