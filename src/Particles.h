//
//  Particles.h
//  Particles
//
//  Created by etienne cella on 2016-03-22.
//
//

#ifndef Particles_h
#define Particles_h

#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"
#include "cinder/CameraUi.h"

#include "cinder/gl/Vao.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/BufferTexture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

constexpr uint32_t P_PARTICLES_TOTAL = 2048 * 32;

constexpr uint32_t P_POSITION_INDEX = 0;
constexpr uint32_t P_VELOCITY_INDEX = 1;
constexpr uint32_t P_INDEX_INDEX = 2;

class Particles
{
public:
    
    Particles();
    void update(gl::BufferTextureRef fluidBufferTexture, int fluidFieldSize);
    void draw();
    void setupParams(params::InterfaceGlRef mParams, const string& prefix);    
private:
    
    void setupBuffers();
    void setupGlsl();
    int computeIndexOffset(int x, int y, int z);
    
    std::array<gl::BufferTextureRef, 2> mVelocityBufTexs;
    std::array<gl::BufferTextureRef, 2> mPositionBufTexs;
    std::array<gl::VaoRef, 2>           mVaos;
    std::array<gl::VboRef, 2>           mPositions, mVelocities, mIndices;
    gl::GlslProgRef                     mUpdateGlsl, mRenderGlsl;
    gl::Texture2dRef                    mParticleTex;
    float                               mFriction{.5f},
                                        mFluidVelocityFactor{.002f},
                                        mParticleSize{.032f},
                                        mVelocityToAlphaFactor{256.0f},
                                        mFogStart{.16f},
                                        mFogEnd{.32f};
    int                                 mToggleIndex{0};
};

Particles::Particles()
{
    
    auto img = loadImage(loadAsset("dot.png"));
    mParticleTex = gl::Texture2d::create(img);
    setupGlsl();
    setupBuffers();
}


void Particles::setupBuffers()
{
    std::array<vec3, P_PARTICLES_TOTAL> positions;
    std::array<vec3, P_PARTICLES_TOTAL> velocities;
    
    int i = 0;
    
    for ( i = 0; i < P_PARTICLES_TOTAL; i++)
    {
        //float fi = (float)i / (float)(P_FIELD_SIZE - 1);
        //positions[n] = vec3(fi - .5f, fj - .5f, fk - .5f);
        positions[i] = vec3(Rand::randFloat() - .5f,
                            Rand::randFloat() - .5f,
                            Rand::randFloat() - .5f);
        velocities[i] = vec3(0.0f, 0.0f, 0.0f);
    }

    for ( i = 0; i < 2; i++ )
    {
        mVaos[i] = gl::Vao::create();
        gl::ScopedVao scopeVao( mVaos[i] );
        {
            mPositions[i] = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), positions.data(), GL_STATIC_DRAW );
            {
                gl::ScopedBuffer sccopeBuffer( mPositions[i] );
                gl::vertexAttribPointer( P_POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 );
                gl::enableVertexAttribArray( P_POSITION_INDEX );
            }
            
            mVelocities[i] = gl::Vbo::create( GL_ARRAY_BUFFER, velocities.size() * sizeof(vec3), velocities.data(), GL_STATIC_DRAW );
            {
                gl::ScopedBuffer sccopeBuffer( mVelocities[i] );
                gl::vertexAttribPointer( P_VELOCITY_INDEX, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 );
                gl::enableVertexAttribArray( P_VELOCITY_INDEX );
            }
        }
    }

    mPositionBufTexs[0] = gl::BufferTexture::create( mPositions[0], GL_RGBA32F );
    mPositionBufTexs[1] = gl::BufferTexture::create( mPositions[1], GL_RGBA32F );
    
    mVelocityBufTexs[0] = gl::BufferTexture::create( mVelocities[0], GL_RGBA32F );
    mVelocityBufTexs[1] = gl::BufferTexture::create( mVelocities[1], GL_RGBA32F );
}

void Particles::setupGlsl()
{
    std::vector<std::string> feedbackVaryings({ "tf_position", "tf_velocity" });
    
    gl::GlslProg::Format updateFormat;
    updateFormat.vertex( loadAsset( "particles_update.vert" ) )
    .feedbackFormat( GL_SEPARATE_ATTRIBS )
    .feedbackVaryings( feedbackVaryings );
    
    mUpdateGlsl = gl::GlslProg::create( updateFormat );
    
    gl::GlslProg::Format renderFormat;
    renderFormat.vertex( loadAsset( "particles_render.vert" ) )
    .geometry( loadAsset( "particles_render.geom" ) )
    .fragment( loadAsset( "particles_render.frag" ) );
    
    mRenderGlsl = gl::GlslProg::create( renderFormat );
}

void Particles::update(gl::BufferTextureRef fluidBufferTexture, int fluidFieldSize)
{
    gl::ScopedGlslProg scopeGlsl( mUpdateGlsl );
    gl::ScopedState    scopeState( GL_RASTERIZER_DISCARD, true );
    
    mUpdateGlsl->uniform("friction", mFriction);
    mUpdateGlsl->uniform("fluid_field_size", fluidFieldSize);
    mUpdateGlsl->uniform("fluid_velocity_factor", mFluidVelocityFactor);
    
    mToggleIndex = (mToggleIndex + 1) % 2;
    const auto sourceIndex = mToggleIndex % 2;
    const auto targetIndex = (mToggleIndex + 1) % 2;
    
    gl::ScopedVao scopedVao( mVaos[sourceIndex] );
    
    gl::ScopedTextureBind scopePositionTex( mPositionBufTexs[sourceIndex]->getTarget(),
                                   /*mPositionBufTexs[sourceIndex]->getId(),*/ 0);
    
    gl::ScopedTextureBind scopeVelocityTex( mVelocityBufTexs[sourceIndex]->getTarget(),
                                           /*mVelocityBufTexs[sourceIndex]->getId(),*/ 1);
    
    gl::ScopedTextureBind scopeFluidTex( fluidBufferTexture->getTarget(),
                                           /*fluidBufferTexture->getId(),*/ 2);
    
    gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER,
                       P_POSITION_INDEX,
                       mPositions[targetIndex] );
    
    gl::bindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER,
                       P_VELOCITY_INDEX,
                       mVelocities[targetIndex] );
    
    gl::beginTransformFeedback( GL_POINTS );
    gl::drawArrays( GL_POINTS, 0, P_PARTICLES_TOTAL );
    gl::endTransformFeedback();
}

void Particles::draw()
{
    gl::ScopedVao scopeVao( mVaos[0] );
    
    CameraPersp mCam;
    // remember that we split the screen: getWindowAspectRatio() * .5f
    mCam.setPerspective(40, getWindowAspectRatio() * .5f, 1, 200);
    
    const auto angle = getElapsedSeconds() * .4f;
    constexpr auto radius = 2.0f;
    mCam.lookAt(vec3(radius * cos(angle), radius, radius * sin(angle)), vec3(.0f, .0f, 0.0f));
    gl::setMatrices( mCam );
    //gl::enableDepthRead();
    //gl::enableDepthWrite();
    //gl::enableAdditiveBlending();
    {
        gl::ScopedTextureBind tex( mParticleTex, 0 );
        gl::ScopedGlslProg scopeGlsl( mRenderGlsl );
        gl::setDefaultShaderVars();
        mRenderGlsl->uniform("tex_particle", 0);
        mRenderGlsl->uniform("ciProjection", mCam.getProjectionMatrix());
        mRenderGlsl->uniform("particle_size", mParticleSize);
        mRenderGlsl->uniform("velocity_to_alpha_factor", mVelocityToAlphaFactor);
        mRenderGlsl->uniform("fog_start", mFogStart );
        mRenderGlsl->uniform("fog_end", mFogEnd );
        gl::drawArrays( GL_POINTS, 0, P_PARTICLES_TOTAL );
    }
    gl::enableAlphaBlending();
    //gl::disableDepthRead();
    //gl::disableDepthWrite();
}

void Particles::setupParams(params::InterfaceGlRef mParams, const string& prefix)
{
    mParams->addParam(prefix + "particle_size", &mParticleSize ).min( 0.0f ).max(.2f);
    mParams->addParam(prefix + "velocity_to_alpha_factor", &mVelocityToAlphaFactor ).min( 0.0f ).max(512.0f);
    mParams->addParam(prefix + "friction", &mFriction ).min( 0.0f ).max(1.0f);
    mParams->addParam(prefix + "fluid_velocity_factor", &mFluidVelocityFactor ).min( 0.0f ).max(1.0f);
    mParams->addParam(prefix + "fog_start", &mFogStart );
    mParams->addParam(prefix + "fog_end", &mFogEnd );
}

#endif /* Particles_h */
