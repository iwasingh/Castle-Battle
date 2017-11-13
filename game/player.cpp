#include "player.h"
#include <iostream>
#include <math.h>
using namespace irr;
using namespace KEYBOARD;
void Player::shoot(f32 power){

    if(this->btBall) return;

    /*Calculate vector */
    core::vector3df position = this->cannon->getPosition() + this->barrel->getBoundingBox().MaxEdge;
    position.X += this->barrel->getBoundingBox().MinEdge.X; // adjust offset
    core::vector3df edges[8];
    this->barrel->getBoundingBox().getEdges(edges);
    position.Y = edges[1].Y - 0.3f;

    this->btBall = new Ball(this->smgr,this->driver,this->physics,position);
    this->btBall->irrBall->setPosition(position);
    core::vector3df shoot = core::vector3df(0,
        power * MAX_RANGE_Y,
        power * MAX_RANGE_X);
    this->btBall->btBall->setLinearVelocity(toBulletVector(shoot));
    this->btBall->btBall->applyCentralForce(toBulletVector(core::vector3df(0, 50.f, 0)));
    this->btBall->setCamera(this->camera->getCamera());

}
/*
* Init : ..
*/
Player::Player(IrrlichtDevice* device, scene::ISceneManager* smgr, video::IVideoDriver* driver, core::vector3df position, Physics* physics, PLAYER_TYPE type){
    this->cannon = smgr->addAnimatedMeshSceneNode(
    smgr->getMesh("media/cannon/cannon.obj"),
    0,
    type,
    position
    );
    this->cannon->setMaterialFlag(video::EMF_LIGHTING, false);
    this->cannon->setMaterialTexture(0,driver->getTexture("media/cannon/cannon_tex.png"));
    this->cannon->setMaterialTexture(1,driver->getTexture("media/cannon/cannonwagon_tex.png"));
    if(type == HUMAN){
        core::vector3df offset =  core::vector3df(
                0,
                this->cannon->getBoundingBox().MaxEdge.Y+0.6f,
                -2.f);

        core::vector3df rotation = core::vector3df(0,0,0);
        this->camera = new Camera(offset,rotation,smgr,cannon);


    }
    this->smgr = smgr;
    this->initKeyboard(device);
    this->barrel = this->cannon->getMesh()->getMeshBuffer(0);
    this->wagon = this->cannon->getMesh()->getMeshBuffer(1);
    this->cannon->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)scene::EDS_BBOX_BUFFERS);
//    this->cannon->setDebugDataVisible(scene::EDS_MESH_WIRE_OVERLAY);

    this->driver = driver;
    this->angle = this->refreshAngle();
    this->btBall = 0;
    this->physics = physics;
    this->rotation = core::vector3df(0,this->cannon->getBoundingBox().getCenter().Y,1); //@deprecated
}
scene::IAnimatedMeshSceneNode* Player::getNode() {
    return this->cannon;
}
void Player::initKeyboard(IrrlichtDevice* device){
    device->setEventReceiver(&this->keyboard);
}
void Player::loop(HUD::HUD* hud){
    Key* key = this->keyboard.IsKeyDown();
    ACTION_KEYBOARD action = key == 0 ? ACTION_NULL : key->action ;
    this->inclinate(action);
    switch(action){
        case SHOOT:
                hud->animatePower();
        break;
        default:
            if(this->keyboard.getLastKey()->action == SHOOT){
                this->shoot(hud->getPower());
            }
            if(this->btBall)
                this->btBall->moveCamera();
        break;
    };
}
f32 Player::refreshAngle(){

    core::aabbox3df box2 = this->barrel->getBoundingBox();
    core::aabbox3df lower = this->wagon->getBoundingBox();
//    core::vector3df vec1 = box2.MaxEdge.normalize();
    core::line3df cannonline = core::line3df(this->barrel->getPosition(1),this->barrel->getPosition(this->barrel->getVertexCount()-1));
    core::vector3df vec1 = cannonline.getVector().normalize();
    core::vector3df vec2 = lower.MaxEdge.normalize();
    f32 length = vec1.getLengthSQ() * vec2.getLengthSQ();
    return acos(vec2.dotProduct(vec1) * core::reciprocal_squareroot(length)) * core::RADTODEG64;


}
core::matrix4 Player::getInclinateValues(ACTION_KEYBOARD key){

    irr::core::matrix4 m;
    f32 velocity = 1;
    switch(key){
     case INCLINATE_UP:
                if(this->refreshAngle() > MAX_ANGLE_TOP)
                    velocity = 0;
                else {
                    this->angle = this->refreshAngle();
                    velocity = 1;
                }

                m.setRotationDegrees(core::vector3df(-INCLINATE_FACTOR * velocity,0,0));
                m.rotateVect(this->rotation);

            return m;
        break;
     case INCLINATE_DOWN:
            if(this->refreshAngle() < MAX_ANGLE_BOTTOM)
                    velocity = 0;
            else {
                    this->angle = this->refreshAngle();
                    velocity = 1;
                }

            m.setRotationDegrees(core::vector3df(INCLINATE_FACTOR * velocity,0,0));
            m.rotateVect(this->rotation);
            return m;
        break;
    }

}
void Player::inclinate(ACTION_KEYBOARD action){

// for optimization i will some instruction inside the switch that could go outside.In the switch it will not make useless calls
    switch(action){

            case INCLINATE_UP:
                  this->smgr->getMeshManipulator()->transform(this->barrel, this->getInclinateValues(INCLINATE_UP));

            break;

            case INCLINATE_DOWN:
                  this->smgr->getMeshManipulator()->transform(this->barrel, this->getInclinateValues(INCLINATE_DOWN));
            break;
    }

    this->barrel->recalculateBoundingBox();


}



//                std::cout
//                <<" "<<this->barrel->getPosition(0).getHorizontalAngle().X
//                <<" "<<this->barrel->getPosition(0).getHorizontalAngle().Y
//                <<" "<<this->barrel->getPosition(0).getHorizontalAngle().Z
