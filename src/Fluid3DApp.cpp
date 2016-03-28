#include <memory>
#include <algorithm>
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"

#include "Fluid3D.h"
#include "Particles.h"
#include "Input.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Fluid3DApp : public App
{
  public:
    
    Fluid3DApp();
    void update() override;
    void draw() override;

    Input                  mInput;
    Fluid3D                mFluid;
    Particles              mParticles;
    params::InterfaceGlRef mParams;
    float                  mFPS{.0f};
};

Fluid3DApp::Fluid3DApp()
{
    mParams = params::InterfaceGl::create( "Field3D", ivec2( 250, 250 ) );
    mParams->addParam("FPS", &mFPS);
    mParams->addSeparator();
    mParams->addText("input");
    mInput.setupParams(mParams, "i_");
    mParams->addSeparator();
    mParams->addText("fluid");
    mFluid.setupParams(mParams, "f_");
    mParams->addSeparator();
    mParams->addText("particles");
    mParticles.setupParams(mParams, "p_");
}

void Fluid3DApp::update()
{
    mInput.update();
    mFluid.update(mInput.getPosition(), mInput.getVelocity());
    mParticles.update(mFluid.getVelocityPressureBufferTexRef(), F_FIELD_SIZE);
    mFPS = getAverageFps();
}

void Fluid3DApp::draw()
{
    gl::clear( Color( 0, 0, 0 ) );
    {
        gl::ScopedViewport scopeViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
        mFluid.draw();
        mInput.draw();
    }
    {
        gl::ScopedViewport scopeViewport(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
        mParticles.draw();
    }
    mParams->draw();
}


CINDER_APP( Fluid3DApp, RendererGl(),
[&]( App::Settings *settings ) {
    settings->setWindowSize( 1280, 720 );
})
