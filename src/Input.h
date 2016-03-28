//
//  Input.h
//  Fluid3D
//
//  Created by etienne cella on 2016-03-26.
//
//

#ifndef Input_h
#define Input_h

#include "cinder/gl/gl.h"
#include "cinder/Rand.h"


using namespace ci;

class Input
{

    
public:
    Input();
    void setupParams(params::InterfaceGlRef mParams, const string& prefix);
    void update();
    void draw();
    vec3 getPosition() const noexcept;
    vec3 getVelocity() const noexcept;
private:
    void computeTarget();
    vec3  mPosition{.0f, .0f, .0f}, mVelocity{.0f, .0f, .0f}, mTarget{.0f, .0f, 1.0f};
    float mFriction{.05f}, mAcceleration{.005f};

};

Input::Input()
{
    computeTarget();
}

vec3 Input::getPosition() const noexcept { return mPosition; }

vec3 Input::getVelocity() const noexcept { return mVelocity; }

void Input::setupParams(params::InterfaceGlRef mParams, const string& prefix)
{
    mParams->addParam(prefix + "friction", &mFriction ).min( 0.0f ).max(.1f);
    mParams->addParam(prefix + "acceleration", &mAcceleration ).min( 0.0f );
}

void Input::update()
{
    if (mTarget == mPosition) return;
    const auto d = mTarget - mPosition;
    
    mVelocity += normalize(d) * mAcceleration;
    mPosition += mVelocity;
    mVelocity *= 1.0f - mFriction;
    
    if (length(d) < .3f) computeTarget();
}

void Input::draw()
{
    gl::ScopedColor c{1.0f, .0f, .0f};
    gl::drawSphere(mPosition, .02f);
}

void Input::computeTarget()
{
    const auto rot = quat((vec3(Rand::randFloat(), Rand::randFloat(), Rand::randFloat()) + vec3(.5f)) * (float)M_PI);
    mTarget = (rot * normalize(mTarget)) * Rand::randFloat(.2f, 1.4f);
}

#endif /* Input_h */
