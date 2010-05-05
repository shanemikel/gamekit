/*
-------------------------------------------------------------------------------
    This file is part of OgreKit.
    http://gamekit.googlecode.com/

    Copyright (c) 2006-2010 Nestor Silveira.

    Contributor(s): none yet.
-------------------------------------------------------------------------------
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/

#include "SceneLogic.h"
#include "MomoLogic.h"
#include "RatLogic.h"

namespace object
{
	gkString PLAYER = "Player";
	gkString CAMERA = "View";
}

namespace material
{
	gkString ARROW_CURSOR = "ArrowCursor";
}

SceneLogic::SceneLogic(gkScene* pScene) 
: m_scene(pScene),
m_tree(gkNodeManager::getSingleton().create()),
m_momo(0)
{
	CreateInput();

	gkPulseNode* pulse = m_tree->createNode<gkPulseNode>();
	pulse->getUPDATE()->setValue(true);

	m_cameraPlayer = m_tree->createNode<gkObjNode>();
	m_cameraPlayer->getUPDATE_OBJ()->link(m_pulseNode->getOUTPUT());
	m_cameraPlayer->getOBJ_NAME()->setValue(object::CAMERA);

	CreateMomo();
	CreateRats();

	CreateExit();
	CreateCursor();
	CreatePick();
	CreateCursorCameraArcBall();
	CreateDebug();

	m_tree->solveOrder();

	gkGameObject* pCamera = pScene->getMainCamera();

	GK_ASSERT(pCamera);

	pCamera->attachLogic(m_tree);
}

SceneLogic::~SceneLogic()
{
}

void SceneLogic::CreateMomo()
{
	m_momo = PMOMO(new MomoLogic(object::PLAYER, this));
}

void SceneLogic::CreateRats()
{
	const gkGameObjectSet& objs = m_scene->getLoadedObjects();

	gkGameObjectSet::const_iterator it = objs.begin();

	while(it != objs.end())
	{
		gkGameObject* ob = *it;

		if(ob->getProperties().physicsState == GK_GHOST_CONTROLLER || ob->getProperties().physicsState == GK_DYNAMIC)
		{
			if(ob->getName().find("Rat") != gkString::npos)
			{
				m_rats.push_back(PRAT(new RatLogic(ob->getName(), this, m_momo)));

				//gkGameObject* obClone = static_cast<gkGameObject*>(ob->clone("RatCloned"));

				//obClone->load();

				//m_rats.push_back(PRAT(new RatLogic(obClone->getName(), this, m_momo)));
			}
		}

		++it;
	}
}

void SceneLogic::CreateInput()
{
	m_pulseNode = m_tree->createNode<gkPulseNode>();
	m_pulseNode->getUPDATE()->setValue(true);

	m_ctrlKeyNode = m_tree->createNode<gkKeyNode>();
	m_ctrlKeyNode->setKey(KC_LEFTCTRLKEY);

	m_shiftKeyNode = m_tree->createNode<gkKeyNode>();
	m_shiftKeyNode->setKey(KC_LEFTSHIFTKEY);

	m_wKeyNode = m_tree->createNode<gkKeyNode>();
	m_wKeyNode->setKey(KC_WKEY);

	m_sKeyNode = m_tree->createNode<gkKeyNode>();
	m_sKeyNode->setKey(KC_SKEY);

	m_cKeyNode = m_tree->createNode<gkKeyNode>();
	m_cKeyNode->setKey(KC_CKEY);

	m_spcKeyNode = m_tree->createNode<gkKeyNode>();
	m_spcKeyNode->setKey(KC_SPACEKEY);

	m_mouseNode = m_tree->createNode<gkMouseNode>();

	m_leftMouseNode = m_tree->createNode<gkMouseButtonNode>();

	m_rightMouseNode = m_tree->createNode<gkMouseButtonNode>();
	m_rightMouseNode->setButton(gkMouse::Right);
}


void SceneLogic::CreateExit()
{
	gkExitNode* exit = m_tree->createNode<gkExitNode>();

	gkKeyNode* key = m_tree->createNode<gkKeyNode>();

	key->setKey(KC_ESCKEY);

	exit->getEXIT()->link(key->getPRESS());
}

void SceneLogic::CreateCursor()
{
	gkCursorNode* cursor = m_tree->createNode<gkCursorNode>();

	cursor->getENABLE()->link(m_ctrlKeyNode->getIS_DOWN());
	cursor->getUPDATE()->link(m_mouseNode->getMOTION());
	cursor->getX()->link(m_mouseNode->getABS_X());
	cursor->getY()->link(m_mouseNode->getABS_Y());
	cursor->getMATERIAL_NAME()->setValue(material::ARROW_CURSOR);
}

void SceneLogic::CreatePick()
{
	gkPickNode* pick = m_tree->createNode<gkPickNode>();

	pick->getUPDATE()->link(m_ctrlKeyNode->getIS_DOWN());
	pick->getCREATE_PICK()->link(m_rightMouseNode->getPRESS());
	pick->getRELEASE_PICK()->link(m_rightMouseNode->getRELEASE());
	pick->getX()->link(m_mouseNode->getABS_X());
	pick->getY()->link(m_mouseNode->getABS_Y());
}

void SceneLogic::CreateCursorCameraArcBall()
{
	gkObjNode* centerObj = m_tree->createNode<gkObjNode>();
	centerObj->setType(gkObjNode::SCREEN_XY);
	centerObj->getX()->link(m_mouseNode->getABS_X());
	centerObj->getY()->link(m_mouseNode->getABS_Y());

	centerObj->getRESET()->link(m_ctrlKeyNode->getRELEASE());
	{
		gkIfNode<bool, CMP_AND>* ifNode = m_tree->createNode<gkIfNode<bool, CMP_AND> >();

		ifNode->getA()->link(m_leftMouseNode->getPRESS());
		ifNode->getB()->link(m_ctrlKeyNode->getIS_DOWN());

		centerObj->getUPDATE_OBJ()->link(ifNode->getIS_TRUE());
	}

	gkArcBallNode* arcBall = m_tree->createNode<gkArcBallNode>();
	arcBall->getCENTER_OBJ()->link(centerObj->getOBJ());
	arcBall->getCENTER_POSITION()->link(centerObj->getHIT_POINT());
	arcBall->getINITIAL_PITCH()->setValue(45);
	arcBall->getTARGET()->link(m_cameraPlayer->getOBJ());

	arcBall->getREL_X()->link(m_mouseNode->getREL_X());
	arcBall->getREL_Y()->link(m_mouseNode->getREL_Y());
	arcBall->getREL_Z()->link(m_mouseNode->getWHEEL());

	arcBall->getMIN_PITCH()->setValue(-90);
	arcBall->getMAX_PITCH()->setValue(90);

	arcBall->getMIN_ROLL()->setValue(-180);
	arcBall->getMAX_ROLL()->setValue(180);


	{
		gkIfNode<bool, CMP_AND>* ifANode = m_tree->createNode<gkIfNode<bool, CMP_AND> >();
		ifANode->getA()->link(m_ctrlKeyNode->getIS_DOWN());
		ifANode->getB()->link(m_leftMouseNode->getIS_DOWN());

		gkIfNode<bool, CMP_AND>* ifBNode = m_tree->createNode<gkIfNode<bool, CMP_AND> >();
		ifBNode->getA()->link(m_ctrlKeyNode->getIS_DOWN());
		ifBNode->getB()->link(m_mouseNode->getWHEEL_MOTION());

		gkIfNode<bool, CMP_OR>* ifNode = m_tree->createNode<gkIfNode<bool, CMP_OR> >();
		ifNode->getA()->link(ifANode->getIS_TRUE());
		ifNode->getB()->link(ifBNode->getIS_TRUE());

		arcBall->getUPDATE()->link(ifNode->getIS_TRUE());
	}
}

void SceneLogic::CreateDebug()
{
	gkShowPhysicsNode* showPhysics = m_tree->createNode<gkShowPhysicsNode>();
	showPhysics->getENABLE()->link(m_cKeyNode->getIS_DOWN());
}



