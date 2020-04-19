#ifndef SHADER_INCLUDED
#define SHADER_INCLUDED

#include "Pipeline.h"

namespace render
{

// Base class for vertex transformations. This is called on all given vertices for a draw operation.
    class VertexShader
    {
    public:
        virtual ~VertexShader() = default;

        virtual VertexOut &&transformSingle(const Vertex &in) = 0;

        // for STL algorithms
        VertexOut &&operator()(const Vertex &in)
        {
            return transformSingle(in);
        }
    };

// Simulates the OpenGL fixed function pipeline. It transforms vertices using a model, view and projection matrix.
    class DefaultVertexTransform : public VertexShader
    {
    public:
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;

        VertexOut &&transformSingle(const Vertex &in) override;
    };

// Base class for shading fragments. This shader is called once the fragment actually passes the depth test.
    class FragmentShader
    {
    public:
        virtual ~FragmentShader() = default;

        virtual Fragment &&shadeSingle(const ShadingGeometry &in) = 0;

    };

// Shades all fragments as the geometry's unlit color vertex attribute.
    class InputColorShader : public FragmentShader
    {
    public:
        Fragment &&shadeSingle(const ShadingGeometry &in) override;
    };

// Shades all fragments as the underlying normal in world coordinates.
    class NormalColorShader : public FragmentShader
    {
    public:
        Fragment &&shadeSingle(const ShadingGeometry &in) override;
    };

// Shades all fragments in a single, constant color.
    class SingleColorShader : public FragmentShader
    {
    public:
        SingleColorShader(const glm::vec4 initialColor) : color(initialColor) {}

        Fragment &&shadeSingle(const ShadingGeometry &in) override;

        inline void setColor(const glm::vec4 &color) { this->color = color; }

    private:
        glm::vec4 color;
    };

}

#endif