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

#include "MomoLogic.h"
#include "SceneLogic.h"

namespace
{
	template<typename T, gkBoolStatement stmt>
	gkIfNode<T, stmt>* gkCreateIfNode(gkLogicTree* tree, gkILogicSocket *a, gkILogicSocket *b)
	{
		typedef gkIfNode<T, stmt> NODE_TYPE;

		NODE_TYPE* ifNode = tree->createNode<NODE_TYPE >();
		ifNode->getA()->link(a);
		ifNode->getB()->link(b);

		return ifNode;
	}

	template<typename T, gkBoolStatement stmt>
	gkIfNode<T, stmt>* gkCreateIfNode(gkLogicTree* tree, gkILogicSocket *a, T b)
	{
		typedef gkIfNode<T, stmt> NODE_TYPE;

		NODE_TYPE* ifNode = tree->createNode<NODE_TYPE >();
		ifNode->getA()->link(a);
		ifNode->getB()->setValue(b);

		return ifNode;
	}

	template<typename T, gkBoolStatement stmt>
	gkIfNode<T, stmt>* gkCreateIfNode(gkLogicTree* tree, T a, gkILogicSocket *b)
	{
		typedef gkIfNode<T, stmt> NODE_TYPE;

		NODE_TYPE* ifNode = tree->createNode<NODE_TYPE >();
		ifNode->getA()->setValue(a);
		ifNode->getB()->link(b);

		return ifNode;
	}

	#define BOOL_AND_NODE( a, b ) gkCreateIfNode<bool, CMP_AND>(m_scene->m_tree, a, b)
	#define BOOL_OR_NODE( a, b ) gkCreateIfNode<bool, CMP_OR>(m_scene->m_tree, a, b)
	#define STRING_EQUAL_NODE( a, b ) gkCreateIfNode<gkString, CMP_EQUALS>(m_scene->m_tree, a, b)
	#define REAL_GREATER_NODE( a, b ) gkCreateIfNode<gkScalar, CMP_GREATER>(m_scene->m_tree, a, b)

	#define IS_TRUE( a ) a->getIS_TRUE()

	typedef gkIfNode<bool, CMP_AND> BOOL_AND_NODE_TYPE;
	typedef gkIfNode<bool, CMP_OR> BOOL_OR_NODE_TYPE;

	enum STATES
	{
		CARRY,
		CATCH,
		CATCH_ENDED,
		THROW_WITH,
		WALK,
		RUN,
		RUN_FASTER,
		WALK_BACK,
		IDLE_NASTY,
		IDLE_CAPOEIRA,
		FALL_UP,
		KICK
	};

	namespace velocity
	{
		gkScalar WALK = 1;
		gkScalar WALK_BACK = -1;
		gkScalar RUN = 2.5f;
		gkScalar RUN_FASTER = 4;
		gkScalar NONE = 0;
	}

	namespace animation
	{
		gkString CARRY = "Momo_Carry";
		gkString CATCH = "Momo_Catch";
		gkString DEATH = "Momo_Death";
		gkString DIE_LAVA = "Momo_DieLava";
		gkString DJ = "Momo_dj";
		gkString DROWNING = "Momo_Drowning";
		gkString EDGE_CLIMB = "Momo_EdgeClimb";
		gkString EDGE_IDLE = "Momo_EdgeIdle";
		gkString FALL = "Momo_Fall";
		gkString FALL_UP = "Momo_FallUp";
		gkString GLIDE = "Momo_Glide";
		gkString HIT_LIGHTLY = "Momo_Hit_Lightly";
		gkString HIT_CARRY = "Momo_HitCarry";
		gkString IDLE1 = "Momo_Idle1";
		gkString IDLE_CAPOEIRA = "Momo_IdleCapoeira";
		gkString IDLE_NASTY = "Momo_IdleNasty";
		gkString JUMP = "Momo_Jump";
		gkString KICK = "Momo_Kick";
		gkString LAND = "Momo_Land";
		gkString PICK = "Momo_Pick";
		gkString REVIVE = "Momo_Revive";
		gkString RUN = "Momo_Run";
		gkString RUN_FASTER = "Momo_RunFaster";
		gkString SHIMMY_L = "Momo_ShimmyL";
		gkString SHIMMY_R = "Momo_ShimmyR";
		gkString TAIL_WHIP = "Momo_TailWhip";
		gkString THROW_1 = "Momo_Throw1";
		gkString THROW_SHEEP = "Momo_ThrowSheep";
		gkString THROW_WITH = "Momo_ThrowWith";
		gkString THROW_WITHOUT = "Momo_ThrowWithout";
		gkString TURN_L = "Momo_Turn.L";
		gkString TURN_R = "Momo_Turn.R";
		gkString WALK = "Momo_Walk";
		gkString WALK_BACK = "Momo_WalkBack";
		gkString WALK_FAST = "Momo_WalkFast";
		gkString WALK_HAND = "Momo_WalkHand";
		gkString WALK_SLOW = "Momo_WalkSlow";
		gkString WALL_FLIP = "Momo_WallFlip";
	}
}

namespace particle
{
	gkString DUST_RUN = "DustRun";
	gkString DUST_WALK = "DustWalk";
}

MomoLogic::MomoLogic(const gkString& name, SceneLogic* scene)
: m_name(name),
m_scene(scene),
m_animNode(0),
m_pathFindingNode(0),
m_stateMachineNode(0),
m_followPathNode(0),
m_playerNode(0),
m_momoGrab(0),
m_kickTestNode(0),
m_momoCameraArcBall(0)
{
	CreateNodes();
	CreatePlayer();
	CreatePathfinding();
	CreateKick();
	CreateGrab();
	CreateMove();
	CreateDustTrail();
	CreateLoadUnload();
	CreateAnimation();
	CreateCameraArcBall();
	CreateStateMachine();
}

MomoLogic::~MomoLogic()
{
}

void MomoLogic::CreateNodes()
{
	m_animNode = m_scene->m_tree->createNode<gkAnimationNode>();
	m_pathFindingNode = m_scene->m_tree->createNode<gkFindPathNode>();
	m_stateMachineNode = m_scene->m_tree->createNode<gkStateMachineNode>();
	m_followPathNode = m_scene->m_tree->createNode<gkFollowPathNode>();
	m_playerNode = m_scene->m_tree->createNode<gkObjNode>();
	m_momoGrab = m_scene->m_tree->createNode<gkGrabNode>();
	m_kickTestNode = m_scene->m_tree->createNode<gkRayTestNode>();
	m_momoCameraArcBall = m_scene->m_tree->createNode<gkArcBallNode>();
}

void MomoLogic::CreatePlayer()
{
	m_playerNode->getUPDATE_OBJ()->link(m_scene->m_pulseNode->getOUTPUT());
	m_playerNode->getOBJ_NAME()->setValue(m_name);
}

void MomoLogic::CreatePathfinding()
{
	gkKeyNode* fKeyNode = m_scene->m_tree->createNode<gkKeyNode>();
	fKeyNode->setKey(KC_ZKEY);

	BOOL_AND_NODE_TYPE* ifNode = BOOL_AND_NODE(m_scene->m_ctrlKeyNode->getIS_DOWN(), fKeyNode->getPRESS());

	gkObjNode* targetNode = m_scene->m_tree->createNode<gkObjNode>();
	targetNode->setType(gkObjNode::SCREEN_XY);
	targetNode->getUPDATE_OBJ()->link(ifNode->getIS_TRUE());
	targetNode->getX()->link(m_scene->m_mouseNode->getABS_X());
	targetNode->getY()->link(m_scene->m_mouseNode->getABS_Y());

	m_pathFindingNode->getUPDATE()->link(
		IS_TRUE(BOOL_AND_NODE(ifNode->getIS_TRUE(), targetNode->getHAS_OBJ())));

	m_pathFindingNode->getSTART_POS()->link(m_playerNode->getPOSITION());
	m_pathFindingNode->getEND_POS()->link(targetNode->getHIT_POINT());

	m_followPathNode = m_scene->m_tree->createNode<gkFollowPathNode>();
	m_followPathNode->getUPDATE()->link(m_pathFindingNode->getPATH_FOUND());
	m_followPathNode->getTARGET()->link(m_playerNode->getOBJ());
	m_followPathNode->getPATH()->link(m_pathFindingNode->getPATH());
	m_followPathNode->setWalkVelocity(velocity::WALK);
	m_followPathNode->setRunVelocity(velocity::RUN);
	m_followPathNode->getSHOW_PATH_OFFSET()->setValue(gkVector3(0, 0, 0.3f));
}

void MomoLogic::CreateKick()
{
	m_kickTestNode->getENABLE()->link(m_stateMachineNode->isCurrentStatus(KICK)->getIS_TRUE());
	m_kickTestNode->getTARGET()->link(m_playerNode->getOBJ());
	m_kickTestNode->getRAY_ORIGIN_OFFSET()->setValue(gkVector3(0, 0, -0.1f));
	m_kickTestNode->getRAY_DIRECTION()->setValue(gkVector3(0, 0.5f, 0));
}

void MomoLogic::CreateGrab()
{
	m_momoGrab->getUPDATE()->setValue(true);
	m_momoGrab->getTARGET()->link(m_playerNode->getOBJ());
	m_momoGrab->getGRAB_DIRECTION()->setValue(gkVector3(0, 0.6f, 0));
	m_momoGrab->getTHROW_VEL()->setValue(gkVector3(0, 10, 0));
	m_momoGrab->getRELATED_OFFSET_POSITION()->setValue(gkVector3(0, -0.6f, 0.5f));

	m_momoGrab->getCREATE_PICK()->link(
		IS_TRUE(BOOL_AND_NODE(m_scene->m_rightMouseNode->getPRESS(), m_scene->m_ctrlKeyNode->getNOT_IS_DOWN()))
	);

	m_momoGrab->getRELEASE_PICK()->link(
		IS_TRUE(BOOL_AND_NODE(m_scene->m_rightMouseNode->getRELEASE(), m_scene->m_ctrlKeyNode->getNOT_IS_DOWN()))
	);

	m_momoGrab->getTHROW_OBJECT()->link(
		IS_TRUE(
			BOOL_AND_NODE(
				m_stateMachineNode->isCurrentStatus(THROW_WITH)->getIS_TRUE(), 
				IS_TRUE(REAL_GREATER_NODE(m_animNode->getTIME_POSITION(), 20.13f))
			)
		)
	);
}

void MomoLogic::CreateMove()
{
	typedef gkMapNode<int, gkScalar> MAP_NODE;
	
	MAP_NODE* mapNode = m_scene->m_tree->createNode< MAP_NODE >();
	mapNode->getINPUT()->link(m_stateMachineNode->getCURRENT_STATE());
	
	MAP_NODE::MAP mapping;
	mapping[WALK] = velocity::WALK;
	mapping[RUN] = velocity::RUN;
	mapping[RUN_FASTER] = velocity::RUN_FASTER;
	mapping[WALK_BACK] = velocity::WALK_BACK;
	mapping[CARRY] = velocity::NONE;
	mapping[IDLE_NASTY] = velocity::NONE;
	mapping[IDLE_CAPOEIRA] = velocity::NONE;
	mapping[FALL_UP] = velocity::NONE;

	mapNode->getMAPPING()->setValue(mapping);

	BOOL_AND_NODE_TYPE* ifNode = BOOL_AND_NODE(m_followPathNode->getHAS_REACHED_END(), mapNode->getHAS_OUTPUT());

	m_playerNode->getSET_ROTATION()->link(ifNode->getIS_TRUE());
	m_playerNode->getSET_ROTATION_VALUE()->link(m_momoCameraArcBall->getCURRENT_ROLL());

	m_playerNode->getSET_LINEAR_VEL()->link(ifNode->getIS_TRUE());
	m_playerNode->getSET_LINEAR_VEL_VALUE_Y()->link(mapNode->getOUTPUT());
}

void MomoLogic::CreateDustTrail()
{
	gkParticleNode* particle = m_scene->m_tree->createNode<gkParticleNode>();
	particle->getPARTICLE_SYSTEM_NAME()->setValue(particle::DUST_RUN);
	particle->getCREATE()->link(IS_TRUE(STRING_EQUAL_NODE(m_animNode->getCURRENT_ANIM_NAME(), animation::RUN_FASTER)));
	particle->getPOSITION()->link(m_playerNode->getPOSITION());
	particle->getORIENTATION()->link(m_playerNode->getROTATION());
}

void MomoLogic::CreateLoadUnload()
{
	// reload

	gkKeyNode* rKeyNode = m_scene->m_tree->createNode<gkKeyNode>();
	rKeyNode->setKey(KC_RKEY);

	m_playerNode->getRELOAD()->link(rKeyNode->getPRESS());

	// unload

	gkKeyNode* uKeyNode = m_scene->m_tree->createNode<gkKeyNode>();
	uKeyNode->setKey(KC_UKEY);

	m_playerNode->getUNLOAD()->link(uKeyNode->getPRESS());

	// load

	gkKeyNode* lKeyNode = m_scene->m_tree->createNode<gkKeyNode>();
	lKeyNode->setKey(KC_LKEY);

	m_playerNode->getLOAD()->link(lKeyNode->getPRESS());
}

void MomoLogic::CreateAnimation()
{
	typedef gkMapNode<int, gkString> MAP_NODE;
	
	MAP_NODE* mapNode = m_scene->m_tree->createNode< MAP_NODE >();
	mapNode->getINPUT()->link(m_stateMachineNode->getCURRENT_STATE());
	
	MAP_NODE::MAP mapping;
	mapping[CARRY] = animation::CARRY;
	mapping[CATCH] = animation::CATCH;
	mapping[CATCH_ENDED] = animation::CATCH;
	mapping[THROW_WITH] = animation::THROW_WITH;
	mapping[WALK] = animation::WALK;
	mapping[RUN] = animation::RUN;
	mapping[RUN_FASTER] = animation::RUN_FASTER;
	mapping[WALK_BACK] = animation::WALK_BACK;
	mapping[IDLE_NASTY] = animation::IDLE_NASTY;
	mapping[IDLE_CAPOEIRA] = animation::IDLE_CAPOEIRA;
	mapping[FALL_UP] = animation::FALL_UP;
	mapping[KICK] = animation::KICK;

	mapNode->getMAPPING()->setValue(mapping);

	m_animNode->getTARGET()->link(m_playerNode->getMESH_OBJ());
	m_animNode->getANIM_NAME()->link(mapNode->getOUTPUT());
}


void MomoLogic::CreateCameraArcBall()
{
	m_momoCameraArcBall->getCENTER_OBJ()->link(m_playerNode->getOBJ());

	m_momoCameraArcBall->getAVOID_BLOCKING()->link(
		IS_TRUE(
			BOOL_AND_NODE(
				IS_TRUE(
					BOOL_AND_NODE(
						m_stateMachineNode->isCurrentStatus(CATCH_ENDED)->getIS_FALSE(),
						m_stateMachineNode->isCurrentStatus(THROW_WITH)->getIS_FALSE()
					)
				),
				IS_TRUE(
					BOOL_AND_NODE(
						m_stateMachineNode->isCurrentStatus(CARRY)->getIS_FALSE(), 
						m_stateMachineNode->isCurrentStatus(CATCH)->getIS_FALSE()
					)
				)
			)
		)
	);

	m_momoCameraArcBall->getCENTER_POSITION()->link(m_playerNode->getPOSITION());
	m_momoCameraArcBall->getINITIAL_PITCH()->setValue(45);
	m_momoCameraArcBall->getTARGET()->link(m_scene->m_cameraPlayer->getOBJ());

	m_momoCameraArcBall->getREL_X()->link(m_scene->m_mouseNode->getREL_X());
	m_momoCameraArcBall->getREL_Y()->link(m_scene->m_mouseNode->getREL_Y());
	m_momoCameraArcBall->getREL_Z()->link(m_scene->m_mouseNode->getWHEEL());

	m_momoCameraArcBall->getMIN_PITCH()->setValue(0);
	m_momoCameraArcBall->getMAX_PITCH()->setValue(80);

	m_momoCameraArcBall->getMIN_ROLL()->setValue(-180);
	m_momoCameraArcBall->getMAX_ROLL()->setValue(180);

	m_momoCameraArcBall->getMIN_Z()->setValue(0.5f);
	m_momoCameraArcBall->getMAX_Z()->setValue(10);

	m_momoCameraArcBall->getKEEP_DISTANCE()->setValue(true);

	m_momoCameraArcBall->getUPDATE()->link(m_scene->m_ctrlKeyNode->getNOT_IS_DOWN());
}

void MomoLogic::CreateStateMachine()
{
	BOOL_OR_NODE_TYPE* walkConditionNode = BOOL_OR_NODE(m_scene->m_wKeyNode->getIS_DOWN(), m_followPathNode->getWALK());

	// Initial state
	m_stateMachineNode->getCURRENT_STATE()->setValue(IDLE_NASTY); 

	// Transitions

	// IDLE_NASTY TRANSITIONS
	m_stateMachineNode->addTransition(CATCH, IDLE_NASTY);
	m_stateMachineNode->addTransition(CATCH_ENDED, IDLE_NASTY);
	m_stateMachineNode->addTransition(CARRY, IDLE_NASTY);
	m_stateMachineNode->addTransition(THROW_WITH, IDLE_NASTY);
	m_stateMachineNode->addTransition(WALK, IDLE_NASTY);
	m_stateMachineNode->addTransition(WALK_BACK, IDLE_NASTY);
	m_stateMachineNode->addTransition(KICK, IDLE_NASTY);
	m_stateMachineNode->addTransition(FALL_UP, IDLE_NASTY);
	m_stateMachineNode->addTransition(IDLE_CAPOEIRA, IDLE_NASTY, 11000);

	// IDLE_CAPOEIRA TRANSITIONS
	m_stateMachineNode->addTransition(IDLE_NASTY, IDLE_CAPOEIRA, 70000);

	gkFallTestNode* fallTest = m_scene->m_tree->createNode<gkFallTestNode>();
	fallTest->getENABLE()->setValue(true);
	fallTest->getTARGET()->link(m_playerNode->getOBJ());

	//WALK TRANSITIONS
	walkConditionNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(IDLE_NASTY, WALK));
	walkConditionNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(IDLE_CAPOEIRA, WALK));
	walkConditionNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(WALK, WALK));
	walkConditionNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(KICK, WALK));
	m_followPathNode->getWALK()->link(m_stateMachineNode->addTransition(RUN, WALK));
	m_stateMachineNode->addTransition(RUN, WALK);


	// WALK_BACK TRANSITIONS
	m_scene->m_sKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(WALK, WALK_BACK));
	m_scene->m_sKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(IDLE_NASTY, WALK_BACK));
	m_scene->m_sKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(IDLE_CAPOEIRA, WALK_BACK));
	m_scene->m_sKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(WALK_BACK, WALK_BACK));
	m_scene->m_sKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(KICK, WALK_BACK));

	// KICK TRANSITIONS
	m_scene->m_spcKeyNode->getPRESS()->link(m_stateMachineNode->addTransition(WALK, KICK));
	m_scene->m_spcKeyNode->getPRESS()->link(m_stateMachineNode->addTransition(WALK_BACK, KICK));
	m_scene->m_spcKeyNode->getPRESS()->link(m_stateMachineNode->addTransition(IDLE_NASTY, KICK));
	m_scene->m_spcKeyNode->getPRESS()->link(m_stateMachineNode->addTransition(IDLE_CAPOEIRA, KICK));
	m_animNode->getNOT_HAS_REACHED_END()->link(m_stateMachineNode->addTransition(KICK, KICK));

	// FALL_UP TRANSITIONS
	fallTest->getFALLING()->link(m_stateMachineNode->addTransition(WALK, FALL_UP));
	fallTest->getFALLING()->link(m_stateMachineNode->addTransition(RUN, FALL_UP));
	fallTest->getFALLING()->link(m_stateMachineNode->addTransition(FALL_UP, FALL_UP));
	fallTest->getFALLING()->link(m_stateMachineNode->addTransition(RUN_FASTER, FALL_UP));
	fallTest->getFALLING()->link(m_stateMachineNode->addTransition(WALK_BACK, FALL_UP));
	fallTest->getFALLING()->link(m_stateMachineNode->addTransition(IDLE_NASTY, FALL_UP));

	// RUN TRANSITIONS
	m_scene->m_wKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(WALK, RUN, 1500));
	m_followPathNode->getRUN()->link(m_stateMachineNode->addTransition(WALK, RUN));
	m_followPathNode->getRUN()->link(m_stateMachineNode->addTransition(IDLE_NASTY, RUN));
	m_followPathNode->getRUN()->link(m_stateMachineNode->addTransition(IDLE_CAPOEIRA, RUN));

	m_scene->m_wKeyNode->getIS_DOWN()->link(m_stateMachineNode->addTransition(RUN, RUN));
	m_followPathNode->getRUN()->link(m_stateMachineNode->addTransition(RUN, RUN));

	m_stateMachineNode->addTransition(RUN_FASTER, RUN);


	// RUN_FASTER TRANSITIONS
	{
		BOOL_AND_NODE_TYPE* ifNode = BOOL_AND_NODE(m_scene->m_shiftKeyNode->getIS_DOWN(), m_scene->m_wKeyNode->getIS_DOWN());

		ifNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(RUN, RUN_FASTER));
		ifNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(RUN_FASTER, RUN_FASTER));
	}
	
	// CATCH TRANSITIONS

	{
		BOOL_AND_NODE_TYPE* ifNode = BOOL_AND_NODE(m_scene->m_ctrlKeyNode->getNOT_IS_DOWN(), m_scene->m_rightMouseNode->getPRESS());

		ifNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(IDLE_NASTY, CATCH));
		ifNode->getIS_TRUE()->link(m_stateMachineNode->addTransition(IDLE_CAPOEIRA, CATCH));
		m_animNode->getNOT_HAS_REACHED_END()->link(m_stateMachineNode->addTransition(CATCH, CATCH));
	}

	// CATCH_ENDED TRANSITIONS
	m_animNode->getHAS_REACHED_END()->link(m_stateMachineNode->addTransition(CATCH, CATCH_ENDED));

	// CARRY TRANSITIONS
	m_momoGrab->getCAUGHT_TRUE()->link(m_stateMachineNode->addTransition(CATCH_ENDED, CARRY));
	m_momoGrab->getCAUGHT_TRUE()->link(m_stateMachineNode->addTransition(CARRY, CARRY));

	// THROW_WITH TRANSITIONS
	IS_TRUE(BOOL_AND_NODE(m_scene->m_leftMouseNode->getPRESS(), m_scene->m_ctrlKeyNode->getNOT_IS_DOWN()))->link(
			m_stateMachineNode->addTransition(CARRY, THROW_WITH));

	m_animNode->getNOT_HAS_REACHED_END()->link(m_stateMachineNode->addTransition(THROW_WITH, THROW_WITH));
}
