//
//  Fluid3D.h
//  Fluid3D
//
//  Created by etienne cella on 2016-03-22.
//
//

#ifndef Fluid3D_h
#define Fluid3D_h

#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"
#include "cinder/CameraUi.h"

#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/BufferTexture.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

constexpr uint32_t F_FIELD_SIZE  = 16;
constexpr uint32_t F_NODES_TOTAL = F_FIELD_SIZE * F_FIELD_SIZE * F_FIELD_SIZE;

constexpr uint32_t F_POSITION_INDEX           = 0;
constexpr uint32_t F_VELOCITY_PRESSURE_INDEX  = 1;
constexpr uint32_t F_CONNECTION_A_INDEX       = 2;
constexpr uint32_t F_CONNECTION_B_INDEX       = 3;

class Fluid3D
{
public:
    
    Fluid3D();
    void update(vec3 inputPosition, vec3 inputVelocity);
    void draw();
    void setupParams(params::InterfaceGlRef mParams, const string& prefix);
    gl::BufferTextureRef getVelocityPressureBufferTexRef();
    
private:
    
    void setupBuffers();
    void setupGlsl();
    int computeIndexOffset(int x, int y, int z);
    
    std::array<gl::VaoRef, 2>           mVaos;
    std::array<gl::VboRef, 2>           mPositions, mVelocitiesPressure, mConnectionsA, mConnectionsB;
    std::array<gl::BufferTextureRef, 2> mVelPressureBufTexs;
    gl::GlslProgRef                     mUpdateGlsl, mRenderGlsl;
    float                               mFriction{.002f}, mInputRadius{.36f}, mInputVelocityfactor{10.0f};
};

Fluid3D::Fluid3D()
{
    setupGlsl();
    setupBuffers();
}

gl::BufferTextureRef Fluid3D::getVelocityPressureBufferTexRef()
{
    return mVelPressureBufTexs[0];
}

int Fluid3D::computeIndexOffset(int x, int y, int z)
{
    return ((x + F_FIELD_SIZE) % F_FIELD_SIZE) +
            ((y + F_FIELD_SIZE) % F_FIELD_SIZE) * F_FIELD_SIZE +
            ((z + F_FIELD_SIZE) % F_FIELD_SIZE) * F_FIELD_SIZE * F_FIELD_SIZE;
}

void Fluid3D::setupBuffers()
{
    std::array<vec4, F_NODES_TOTAL> positions;
    std::array<vec4, F_NODES_TOTAL> velocityPressures;
    std::array<ivec4, F_NODES_TOTAL> connectionsA;
    std::array<ivec3, F_NODES_TOTAL> connectionsB;
    
    int i, j, k, n = 0;
    for ( k = 0; k < F_FIELD_SIZE; k++ )
    {
        float fk = (float)k / (float)(F_FIELD_SIZE - 1);
        for ( j = 0; j < F_FIELD_SIZE; j++ )
        {
            float fj = (float)j / (float)(F_FIELD_SIZE - 1);
            for ( i = 0; i < F_FIELD_SIZE; i++)
            {
                float fi = (float)i / (float)(F_FIELD_SIZE - 1);
                
                positions[n] = vec4(fi - .5f, fj - .5f, fk - .5f, 1.0f);
                
                velocityPressures[n] = vec4( 0.0f );
                
                connectionsA[n] = ivec4(computeIndexOffset(i + 1, j    , k    ),
                                        computeIndexOffset(i    , j + 1, k    ),
                                        computeIndexOffset(i    , j    , k + 1),
                                        // self offset, border: -1
                                        computeIndexOffset(i    , j,     k    ));
                
                connectionsB[n] = ivec3(computeIndexOffset(i - 1, j    , k    ),
                                        computeIndexOffset(i    , j - 1, k    ),
                                        computeIndexOffset(i    , j    , k - 1));
                ++n;
            }
        }
    }
    
    for ( i = 0; i < 2; i++ ) {
        mVaos[i] = gl::Vao::create();
        gl::ScopedVao scopeVao( mVaos[i] );
        {
            mPositions[i] = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec4), positions.data(), GL_STATIC_DRAW );
            {
                gl::ScopedBuffer sccopeBuffer( mPositions[i] );
                gl::vertexAttribPointer( F_POSITION_INDEX, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 );
                gl::enableVertexAttribArray( F_POSITION_INDEX );
            }
            
            mVelocitiesPressure[i] = gl::Vbo::create( GL_ARRAY_BUFFER, velocityPressures.size() * sizeof(vec4), velocityPressures.data(), GL_STATIC_DRAW );
            {
                gl::ScopedBuffer sccopeBuffer( mVelocitiesPressure[i] );
                gl::vertexAttribPointer( F_VELOCITY_PRESSURE_INDEX, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 );
                gl::enableVertexAttribArray( F_VELOCITY_PRESSURE_INDEX );
            }
            
            mConnectionsA[i] = gl::Vbo::create( GL_ARRAY_BUFFER, connectionsA.size() * sizeof(ivec4), connectionsA.data(), GL_STATIC_DRAW );
            {
                gl::ScopedBuffer scopeBuffer( mConnectionsA[i] );
                gl::vertexAttribIPointer( F_CONNECTION_A_INDEX, 4, GL_INT, 0, (const GLvoid*) 0 );
                gl::enableVertexAttribArray( F_CONNECTION_A_INDEX );
            }
            
            mConnectionsB[i] = gl::Vbo::create( GL_ARRAY_BUFFER, connectionsB.size() * sizeof(ivec4), connectionsB.data(), GL_STATIC_DRAW );
            {
                gl::ScopedBuffer scopeBuffer( mConnectionsB[i] );
                gl::vertexAttribIPointer( F_CONNECTION_B_INDEX, 3, GL_INT, 0, (const GLvoid*) 0 );
                gl::enableVertexAttribArray( F_CONNECTION_B_INDEX );
            }
        }
    }
    
    // buffer textures so we can access neighbors velocity and pressure
    mVelPressureBufTexs[0] = gl::BufferTexture::create( mVelocitiesPressure[0], GL_RGBA32F );
    mVelPressureBufTexs[1] = gl::BufferTexture::create( mVelocitiesPressure[1], GL_RGBA32F );
}

void Fluid3D::setupGlsl()
{
    std::vector<std::string> feedbackVaryings({ "tf_position", "tf_velocity_pressure" });
    
    gl::GlslProg::Format updateFormat;
    updateFormat.vertex( loadAsset( "fluid_update.vert" ) )
    .feedbackFormat( GL_SEPARATE_ATTRIBS )
    .feedbackVaryings( feedbackVaryings );
    
    mUpdateGlsl = gl::GlslProg::create( updateFormat );
    
    gl::GlslProg::Format renderFormat;
    renderFormat.vertex( loadAsset( "fluid_render.vert" ) )
    .fragment( loadAsset( "fluid_render.frag" ) );
    
    mRenderGlsl = gl::GlslProg::create( renderFormat );
}

void Fluid3D::update(vec3 inputPosition, vec3 inputVelocity)
{
    
    gl::ScopedGlslProg scopeGlsl( mUpdateGlsl );
    gl::ScopedState    scopeState( GL_RASTERIZER_DISCARD, true );
    
    mUpdateGlsl->uniform("friction", mFriction);
    mUpdateGlsl->uniform("inputRadius", mInputRadius);
    mUpdateGlsl->uniform("inputVelocity", inputVelocity * mInputVelocityfactor);
    mUpdateGlsl->uniform("inputPosition", inputPosition);
    
    // Using shader subroutines:
    // http://www.geeks3d.com/20140701/opengl-4-shader-subroutines-introduction-3d-programming-tutorial
    const auto shaderProgram = mUpdateGlsl->getHandle();
    GLuint indexFunc1 = glGetSubroutineIndex(shaderProgram, GL_VERTEX_SHADER, "compute_pressure");
    GLuint indexFunc2 = glGetSubroutineIndex(shaderProgram, GL_VERTEX_SHADER, "compute_velocity");
    
    for( auto i = 0; i < 2; ++i )
    {
        glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, i % 2 == 0 ? &indexFunc1 : &indexFunc2);
        
        const auto sourceIndex = i % 2;
        const auto targetIndex = (i + 1) % 2;
        
        gl::ScopedVao scopedVao( mVaos[sourceIndex] );
        
        gl::ScopedTextureBind scopeTex( mVelPressureBufTexs[sourceIndex]->getTarget(),
                                       /*mVelPressureBufTexs[sourceIndex]->getId()*/ 0);
        
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER,
                           F_POSITION_INDEX,
                           mPositions[0] ); // !! Fixed index !!
        
        gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER,
                           F_VELOCITY_PRESSURE_INDEX,
                           mVelocitiesPressure[targetIndex] );
        
        gl::beginTransformFeedback( GL_POINTS );
        gl::drawArrays( GL_POINTS, 0, F_NODES_TOTAL );
        gl::endTransformFeedback();
    }
}

void Fluid3D::draw()
{
    gl::ScopedVao scopeVao( mVaos[0] );

    CameraPersp mCam;
    // remember that we split the screen: getWindowAspectRatio() * .5f
    mCam.setPerspective(60, getWindowAspectRatio() * .5f, 1, 200);
    
    const auto angle = getElapsedSeconds() * .4f;
    constexpr auto radius = 2.0f;
    mCam.lookAt(vec3(radius * cos(angle), radius, radius * sin(angle)), vec3(.0f, .0f, 0.0f));
    gl::setMatrices( mCam );
    
    {
        gl::ScopedGlslProg scopeGlsl( mRenderGlsl );
        gl::setDefaultShaderVars();
        gl::pointSize( 1.5f);
        gl::drawArrays( GL_POINTS, 0, F_NODES_TOTAL );
    }
}

void Fluid3D::setupParams(params::InterfaceGlRef mParams, const string& prefix)
{
    mParams->addParam(prefix + "friction", &mFriction ).min( 0.0f ).max(1.0f);
    mParams->addParam(prefix + "input_radius", &mInputRadius);
    mParams->addParam(prefix + "input_velocity_factor", &mInputVelocityfactor);
}

#endif /* Fluid3D_h */
