#pragma once
#include <Strings/String.h>
#include <Time/GameTime.h>
#include <Physics/Physics.h>
#include <Logging/Log.h>
#include <Constants/AllocTypes.h>
#include <Memory/Handle.h>
#include <Config/Config.h>
#include <DebugUtils/Assertions.h>
#include <Input/Input.h>
#include <Constants/ControllerConstants.h>
#include <Libraries/Assimp/Importer.hpp>
#include <Libraries/Assimp/scene.h>
#include <Libraries/Assimp/postprocess.h>
#include <ResourceManagement/ResourceManager.h>
#include <ResourceManagement/ResourceLoaders.h>
#include <FileManagement/ArchiveTypes.h>
#include <ResourceManagement/ResourceArchive.h>
#include <Strings/StringUtils.h>
#include <Rendering/Model.h>
#include <FileManagement/ModelFile.h>
#include <Rendering/Camera/Camera.h>
#include <Rendering/Font.h>
#include <Rendering/Text.h>
#include <DataStructures/OcTree.h>
#include <Rendering/Culling/OcTreeCuller.h>
#include <Rendering/Culling/DummyCuller.h>
#include <Rendering/Renderer.h>
#include <Random/Random.h>
#include <EngineLogic/SceneGraph/ModelNode.h>
#include <Hashing/HashTable.h>
#include <Scripting/ScriptIntegration.h>
#include "../TestBase.h"
#include "../TestObjects.h"
#include <MultiThreading/StdThreading.h>
#include <Audio/XAudio2Audio.h>
#include <Audio/SoundLoaders.h>
#include <Platforms/IPlatform.h>

namespace LeEK
{
	namespace Tests
	{
		//a test that does nothing
		//...not really sure why I did this?
		//can use it as boilerplate I guess
		class NullTest : public TestBase
		{
		public:
			bool Startup(Game* game) { return false; }
			void Shutdown(Game* game) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		/**
		Reusable testing framework
		which includes a keyboard and mouse,
		a camera, and keybindings that move the camera.
		*/
		class TestBaseWithResources : public TestBase
		{
		protected:
			TypedHandle<Keyboard> kbd;
			TypedHandle<Mouse> mouse;
			IPlatform* plat;
			F32 camMoveVel;
			F32 camRotVel;
			F32 mouseSensitivity;
			bool enableMouseLook;

			//key bindings for camera
			U8 camFwdKey, camBackKey, camLeftKey, camRightKey;
			U8 camUpKey, camDownKey;

			//global key bindings
			U8 mouseLookToggleKey;

			//The camera that displays the world.
			CameraBase* viewCam;

		private:			
			void initCamera(CameraBase* cam)
			{
				cam->SetAspectRatio(gfx->ScreenAspect());
				cam->SetFOV(Math::PI_OVER_2);
				cam->SetNearDist(.25f);
				cam->SetFarDist(100.0f);
			}
			void updateGlobalCommands(const GameTime& time)
			{
				//toggle mouselook
				if(kbd->KeyPressed(mouseLookToggleKey))
				{
					enableMouseLook = !enableMouseLook;
					plat->SetShouldLockMouse(enableMouseLook);
				}
			}
			void updateCamPos(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 transVec = Vector3::Zero;
				if(kbd->KeyDown(camFwdKey))
				{
					transVec += viewCam->Forward() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(camBackKey))
				{
					transVec -= viewCam->Forward() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(camLeftKey))
				{
					transVec += viewCam->Right() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(camRightKey))
				{
					transVec -= viewCam->Right() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(camUpKey))
				{
					transVec += viewCam->Up() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(camDownKey))
				{
					transVec -= viewCam->Up() * camMoveVel * elapsedSec;
				}
				viewCam->Translate(transVec);
			}
			void updateCamRot(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 rotVec = Vector3::Zero;
				
				//also update mouse tilt and pan, if desired
				if(enableMouseLook)
				{
					Vector2 mouseDelta = mouse->GetMousePos() * (camRotVel * elapsedSec * mouseSensitivity);
					//remember that the mouse delta is linear and rotvec is rotational!
					rotVec.SetX(rotVec.X() + mouseDelta.Y());
					rotVec.SetY(rotVec.Y() - mouseDelta.X());
				}

				viewCam->RotateEuler(rotVec.X(), rotVec.Y(), rotVec.Z());
				viewCam->SetUp(Vector3::Up);
			}
		public:
			TestBaseWithResources()
			{
				
			}
			virtual bool Startup(Game* game)
			{
				return true;
			}
			virtual bool PreStartup(Game* game)
			{
				if(!TestBase::PreStartup(game))
				{
					return false;
				}
				//init the input
				camMoveVel = 1;
				camRotVel = 1;
				mouseSensitivity = 1;
				enableMouseLook = false;
				plat = game->Platform();
				if(!plat)
				{
					Log::E("Couldn't find platform subsystem in game!");
					return false;
				}
				plat->SetShouldLockMouse(enableMouseLook);
				kbd = HandleMgr::RegisterPtr(&game->Input()->GetKeyboard(0));
				mouse = HandleMgr::RegisterPtr(&game->Input()->GetMouse(0));

				//set keys to defaults
				camFwdKey = KEY_W;
				camBackKey = KEY_S;
				camLeftKey = KEY_A;
				camRightKey = KEY_D;
				camUpKey = KEY_E;
				camDownKey = KEY_C;

				mouseLookToggleKey = KEY_TAB;

				//cameras
				viewCam = LNew(FreeLookCamera, AllocType::TEST_ALLOC, "TestAlloc")();
				initCamera(viewCam);

				return true;
			}
			virtual void Shutdown(Game* game) { TestBase::Shutdown(game); }
			virtual void Update(Game* game, const GameTime& time)
			{
				updateGlobalCommands(time);
				updateCamPos(time);
				updateCamRot(time);
			}
			//Draws to the screen.
			//You can assume that the screen will be cleared by this call.
			virtual void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);
				gfx->SetWorldViewProjection(Matrix4x4::Identity,
											viewCam->GetViewMatrix(),
											viewCam->GetProjMatrix());
			}
		};

		//basically is TestBulletPhysics(),
		//but also includes
		class FrameworkTest : public TestBase
		{
		private:
			PhysicsWorld physics;
			DebugDrawer* debugDrawer;
			Matrix4x4 world, view, projection;
			F32 worldArray[16];
			F32 viewArray[16];
			F32 projArray[16];

			//physics members
			BoxCollShape* boxCollider;
			BoxCollShape* floorCollider;
			CompoundCollShape* colliderHolder;
			CompoundCollShape* floorColliderHolder;
			RigidBody* boxBody;
			RigidBody* floorBody;

			InputManager input;
			TypedHandle<Keyboard> kbd;
			bool paused;
			void initInput(Game* game)
			{
				input.Startup(game->Platform());
				TypedHandle<InputManager> inputHnd = HandleMgr::FindHandle(&input);
				if(!inputHnd.GetHandle())
				{
					inputHnd = HandleMgr::RegisterPtr(&input);
				}
				//link input manager w/ platform
				IPlatform* plat = game->Platform();
				plat->InitInput(game->Input());
				//and tell platform we're ready for input
				kbd = HandleMgr::RegisterPtr(&input.GetKeyboard(0));
			}
			void initPhysics(Game* game)
			{
				Log::D("In FrameworkTest::Startup()");
				physics.Startup();
				Log::D("Physics world initialized!");

				//then setup debug physics renderer
				debugDrawer = CustomNew<DebugDrawer>(TEST_ALLOC, "TestAlloc", game->GfxWrapper());
				debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawAabb);
				physics.AttachDebugDrawer(debugDrawer);
				Log::D("Attached debug drawer to physics world");

				//then spawn a box
				boxCollider = CustomNew<BoxCollShape>(PHYS_ALLOC, "RBAlloc", 1.0f, 0.5f, 1.0f);
				colliderHolder = CustomNew<CompoundCollShape>(PHYS_ALLOC, "RBAlloc");
				colliderHolder->AddShape(boxCollider, Vector3::Zero, Quaternion::Identity);
				boxBody = CustomNew<RigidBody>(PHYS_ALLOC, "RBAlloc", 1.0f, Vector3::Zero, Quaternion::Identity, *colliderHolder);
				//now attach the box to the physics world!
				physics.AttachRigidBody(boxBody);
				Log::D("Attached box to physics world");

				//setup gravity
				Vector3 grav = Vector3(0.0f, -9.8f, 0.0f);
				physics.SetGravityVector(grav);

				//spawn a floor, too
				floorCollider = CustomNew<BoxCollShape>(PHYS_ALLOC, "RBAlloc", 50.0f, 20.0f, 50.0f);
				floorColliderHolder = CustomNew<CompoundCollShape>(PHYS_ALLOC, "RBAlloc");
				floorColliderHolder->AddShape(floorCollider, Vector3::Zero, Quaternion::Identity);
				floorBody = CustomNew<RigidBody>(PHYS_ALLOC, "RBAlloc", 0.0f, Vector3(0, -50, 0), Quaternion::Identity, *floorColliderHolder);
				physics.AttachRigidBody(floorBody);
			}
		public:
			FrameworkTest(void) 
			{
				paused = true;
			}
			~FrameworkTest(void) {}
			bool Startup(Game* game)
			{
				//setup input
				initInput(game);
				initPhysics(game);

				world = Matrix4x4::Identity;//Matrix4x4::BuildTranslation(boxPosition);
				float aspect = game->GfxWrapper()->ScreenAspect();
				Log::D(String("Aspect Ratio: ") + aspect );
				projection = Matrix4x4::BuildPerspectiveRH(game->GfxWrapper()->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
				//we don't have a camera class yet; ordinarily it would handle this
				view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, -3.0f),//place it 3 units forward
														boxBody->Position(),//looking at the box
														Vector3::Up); //and use default up direction for now

				world.CopyToBuffer(worldArray);
				view.CopyToBuffer(viewArray);
				projection.CopyToBuffer(projArray);
				return true;
			}
			void Shutdown(Game* game)
			{
				Log::D("In FrameworkTest::Shutdown()");
			}
			void Update(Game* game, const GameTime& time) 
			{
				if(kbd->KeyPressed(KEY_SPACE))
				{
					paused = !paused;
				}
				if(!paused)
				{
					{
							PROFILE("Physics");
							physics.Update(time);
					}
					{
						PROFILE("GetRendererState");
						//print any errors
						//game->GfxWrapper()->PrintShaderStatus();
					}
					{
						PROFILE("BuildViewMat");
						//update the camera's view to look at the box
						view = Matrix4x4::BuildViewLookAtRH(Vector3(0.0f, 0.0f, -3.0f),
															boxBody->Position(),
															Vector3::Up);
					}
				}
			}
			void Draw(Game* game, const GameTime& time)
			{
				game->GfxWrapper()->Clear(Colors::Gray20Pct);
				if(!game->GfxWrapper()->SetWorldViewProjection(world, view, projection))
				{
					Log::D("Couldn't set WVP matrices!");
				}
				{
					PROFILE("Debug Render");
					physics.DebugDraw();
				}
			}
		};

		class HandlesTest : public TestBase
		{
		public:
			HandlesTest(void) {}
			~HandlesTest(void) {}
			bool Startup(Game* game)
			{
				char* text1 = "Here's the first text!";
				char* text2 = "And here's the second text!";
				U32 x = 8492;
				TestParent* dynObj = CustomNew<TestParent>(TEST_ALLOC, "TestObjAlloc");
				//now generate handles
				Handle txt1Hnd = HandleMgr::RegisterPtr((void*)text1);
				Handle txt2Hnd = HandleMgr::RegisterPtr((void*)text2);
				Handle intHnd = HandleMgr::RegisterPtr((void*)&x);
				Handle dynObjHnd = HandleMgr::RegisterPtr((void*)dynObj);
				Log::D(String("Handle to text1: ") + txt1Hnd);
				Log::D(String("Value pointed by handle: ") + HandleMgr::GetPointer<char>(txt1Hnd));
				Log::D(String("Handle to text2: ") + txt2Hnd);
				Log::D(String("Value pointed by handle: ") + HandleMgr::GetPointer<char>(txt2Hnd));
				Log::D(String("Handle to x: ") + intHnd);
				Log::D(String("Value pointed by handle: ") + *HandleMgr::GetPointer<U32>(intHnd));
				Log::D(String("Handle to dynObj: ") + dynObjHnd);
				Log::D("Calling function of dynObj.");
				HandleMgr::GetPointer<TestParent>(dynObjHnd)->TestFunc();

				Log::D("Moving handle to text1 to point to text2.");
				HandleMgr::MoveHandle(txt1Hnd, text2);
				Log::D(String("Handle to text1: ") + txt1Hnd);
				Log::D(String("Value pointed by handle: ") + HandleMgr::GetPointer<char>(txt1Hnd));

				Log::D("Deleting handle to dynObj.");
				HandleMgr::DeleteHandle<TestParent>(dynObjHnd);
				Log::D(String("Handle to dynObj: ") + dynObjHnd);
				Log::D(String("Pointer of handle: ") + (U64)HandleMgr::GetPointer(dynObjHnd));

				//now test the type-wrapped handle
				TestChild* typedObj = CustomNew<TestChild>(TEST_ALLOC, "TestObjAlloc");
				TypedHandle<TestChild> typedHnd = HandleMgr::RegisterPtr(typedObj);
				Log::D(String("Typed handle to typedObj: ") + typedHnd.GetHandle());
				Log::D("Calling function of typedObj.");
				typedHnd->TestFunc();
				Log::D("Deleting handle to typedObj.");
				HandleMgr::DeleteHandle(typedHnd);
				Log::D(String("Typed handle to typedObj: ") + typedHnd.GetHandle());
				Log::D(String("Pointer of handle: ") + (U64)typedHnd.Ptr());

				TestParent* testArray = CustomArrayNew<TestParent>(4, TEST_ALLOC, "TestObjAlloc");
				CustomArrayDelete(testArray);
				return false;
			}
		};

		class DbgDrawTest : public TestBase
		{
		private:
			Matrix4x4 world, view, projection;
		public:
			DbgDrawTest(void) {}
			~DbgDrawTest(void) {}
			bool Startup(Game* game)
			{
				if(!TestBase::Startup(game))
				{
					return false;
				}
				world = Matrix4x4::Identity;
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());
				projection = Matrix4x4::BuildPerspectiveRH(gfx->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
				//we don't have a camera class yet; ordinarily it would handle this
				view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, -3.0f),//Vector3(1.0f, 1.0f, -3.0f), //place it 3 units forward
														Vector3(0.0f, 0.0f, 0.0f), //by default, look at the origin
														Vector3::Up); //and use default up direction for now
				//now try assigning the matrices as uniforms
				//world.CopyToBuffer(worldArray);
				//view.CopyToBuffer(viewArray);
				//projection.CopyToBuffer(projArray);
				Log::SetVerbosity(Log::VERB);
				return true;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time)
			{
				//nothing to update, this is all drawing all the time apparently
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);
				gfx->SetWorldViewProjection(world, view, projection);
				//draw a coordinate system!
				Vector3 coordStart = Vector3(2.0f, 2.0f, 0.0f);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(1.0f, 0.0f, 0.0f), Colors::Red);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0.0f, 1.0f, 0.0f), Colors::Green);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0.0f, 0.0f, 1.0f), Colors::Blue);
				//then draw a box
				F32 totSecs = time.TotalGameTime().ToSeconds();
				//and make it spin, too
				gfx->DebugDrawBox(	Vector3(-2.0f, 0.0f, 0.0f), 
											Quaternion::FromEulerAngles(totSecs - 1.0f, totSecs, totSecs + 1.0f),
											0.5f,
											0.5f,
											0.5f,
											Colors::Cyan);
				//then draw a moving sphere!
				F32 pos = Math::Cos(totSecs);
				gfx->DebugDrawSphere(Vector3(pos, pos, 0.0f), 1.0f, Colors::Yellow);
			}
		};

		class CfgTest : public TestBase
		{
		public:
			CfgTest(void) {}
			~CfgTest(void) {}
			bool Startup(Game* game)
			{
				Log::SetVerbosity(Log::VERB);
				//if the test output already exists, delete it
				Path out1Path(String(Filesystem::GetProgDir()) + "/cfgTestOut1.xml");
				if(Filesystem::Exists(out1Path))
				{
					Filesystem::RemoveFile(out1Path);
				}
				Path out2Path(String(Filesystem::GetProgDir()) + "/cfgTestOut2.xml");
				if(Filesystem::Exists(out2Path))
				{
					Filesystem::RemoveFile(out2Path);
				}
				//first test setting values
				const char* intName = "Int";
				const char* boolName = "Bool";
				const char* floatName = "Float";
				const char* stringName = "String";
				Log::D("Setting settings");
				Config::SetIntSetting(intName, 42);
				Config::SetBoolSetting(boolName, true);
				Config::SetFloatSetting(floatName, 8.492f);
				Config::SetStrSetting(stringName, "A string");
				Log::D(String("The value of ") + stringName + " is " + Config::GetStrSetting(stringName));
				Log::D(String("The value of ") + floatName + " is " + Config::GetFloatSetting(floatName));
				Log::D(String("The value of ") + boolName + " is " + Config::GetBoolSetting(boolName));
				Log::D(String("The value of ") + intName + " is " + Config::GetIntSetting(intName));
				Log::D("Saving settings...");
				//then test saving values
				//save to two files!
				if(Config::SaveCfgFile(out1Path) && Config::SaveCfgFile(out2Path))
				{
					Log::D("Save complete!");
				}
				else
				{
					Log::D("Save failed!");
					return false;
				}
				//then test loading values
				//Log::D("Loading settings from cfgTestIn.xml...");
				if(Config::LoadCfgFile(String(Filesystem::GetProgDir()) + "/TestContent/cfgTestInAlt.xml"))
				{
					Log::D("Load complete!");
				}
				else
				{
					Log::D("Load failed!");
					return false;
				}
				const char* intInName = "InInt";
				const char* boolInName = "InBool";
				const char* floatInName = "InFloat";
				const char* stringInName = "InString";
				Log::D(String("The value of ") + stringInName + " is " + Config::GetStrSetting(stringInName));
				Log::D(String("The value of ") + floatInName + " is " + Config::GetFloatSetting(floatInName));
				Log::D(String("The value of ") + boolInName + " is " + Config::GetBoolSetting(boolInName));
				Log::D(String("The value of ") + intInName + " is " + Config::GetIntSetting(intInName));
				//set some more values, and save to the second file
				Config::SetStrSetting(stringName, "The string has changed");
				Log::D(String("The value of ") + stringName + " is " + Config::GetStrSetting(stringName));
				if(Config::SaveCfgFile(out2Path))
				{
					Log::D("Save complete!");
				}
				else
				{
					Log::D("Save failed!");
					return false;
				}
				//attempt to reload the settings
				F32 elapsedTimeMs = 0;
				game->Time().Tick();
				Config::ClearSettings();
				game->Time().Tick();
				elapsedTimeMs = game->Time().ElapsedGameTime().ToMilliseconds();
				Log::D(String("Cleared settings in ") + elapsedTimeMs + "ms");
				game->Time().Tick();
				Config::LoadCfgFile(out2Path);
				game->Time().Tick();
				elapsedTimeMs = game->Time().ElapsedGameTime().ToMilliseconds();
				Log::D(String("Loaded settings in ") + elapsedTimeMs + "ms");
				//now attempt to load an invalid file
				Config::ClearSettings();
				Config::LoadCfgFile(String(Filesystem::GetProgDir()) + "/TestContent/cfgTestIn.xml");
				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		//strange bug; an alloc somewhere in here
		//causes a tag in the allocator's taglist to be corrupted
		class InputTest : public TestBase
		{
		private:
			Matrix4x4 world, view, projection;
			Vector3 box1Pos, box1Euler;
			Vector3 box2Pos, box2Euler;
			F32 moveAccel, rotAccel;
			F32 moveVel, rotVel;
			F32 box1VelFactor, box2VelFactor;
			void updateTerm(Game* game, const GameTime& time)
			{
				//do tests HERE; afterwards input will be reset to nulls
				//print state of controller 0 every half second
				timerMs += time.ElapsedGameTime().ToMilliseconds();
				if(timerMs > updateRateMs)
				{
					if(Log::BufferEnabled())
					{
						Log::SetBufferPaused(true);
					}
					game->Platform()->ClearTerm();
					/*
					Controller& ctrlr = input->GetController(0);
					//list axis
					
					Log::D("Controller 0:");
					for(U32 i = 0; i < ctrlr.Info.NumAxis; ++i)
					{
						Log::D(String("\tAxis ") + i + ":\t" + ctrlr.GetAxis(i));
					}
					Log::D("\tButtons Pressed: ");
					//list buttons
					for(U32 i = 0; i < ctrlr.Info.NumBtns; ++i)
					{
						if(ctrlr.ButtonDown(i))
						{
							Log::D(String("\t\t") + i);
						}
					}*/
					for(U32 i = 0; i < input->NumHID(); ++i)
					{
						Controller& ctrlr = input->GetController(i);
						for(U32 j = 0; j < ctrlr.Info.NumBtns; ++j)
						{
							if(ctrlr.ButtonDown(j))
							{
								Log::D(String("Controller ") + i + ", button down: " + j);
							}
						}
					}
					//iterate over keyboards
					for(U32 i = 0; i < input->NumKbd(); ++i)
					{
						Keyboard& kbd = input->GetKeyboard(i);
						for(U32 j = 0; j < 255; ++j)
						{
							if(kbd.KeyDown(j))
							{
								Log::D(String("Keyboard ") + i + ", key down: " + GetKeyName(j));
							}
						}
					}
					//and over mice
					for(U32 i = 0; i < input->NumMouse(); ++i)
					{
						Mouse& mouse = input->GetMouse(i);
						for(U8 j = 0; j < mouse.MaxNumBtns(); ++j)
						{
							if(mouse.ButtonDown(j))
							{
								Log::D(String("Mouse ") + i + ", button down: " + j);
							}
						}
						//also check if mouse wheel's moved
						I16 wheelDelta = mouse.GetWheelDelta();
						if(wheelDelta != 0)
						{
							Log::D(String("Mouse ") + i + ", wheel moved: " + wheelDelta);
						}
					}
					//game->Platform()->ClearTerm();
					Mouse& mouse = input->GetMouse(0);
					Log::D(String("Mouse Position: ") + input->GetGUIMousePos().ToString());
					if(Log::BufferEnabled())
					{
						Log::SetBufferPaused(false);
					}
					//also display mouse position
					timerMs -= updateRateMs;
				}
			}
			void updateKbd(Game* game, const GameTime& time)
			{
				Keyboard& kbd = input->GetKeyboard(0);
				//fine tuning button!
				if(kbd.KeyDown(KEY_LSHIFT))
				{
					box1VelFactor = 0.5f;
				}
				F32 b1MoveVel = box1VelFactor * moveVel;
				F32 b1RotVel = box1VelFactor * rotVel;
				//position
				if(kbd.KeyDown(KEY_W))
				{
					box1Pos.SetZ(box1Pos.Z() - b1MoveVel);
				}
				if(kbd.KeyDown(KEY_S))
				{
					box1Pos.SetZ(box1Pos.Z() + b1MoveVel);
				}
				if(kbd.KeyDown(KEY_A))
				{
					box1Pos.SetX(box1Pos.X() - b1MoveVel);
				}
				if(kbd.KeyDown(KEY_D))
				{
					box1Pos.SetX(box1Pos.X() + b1MoveVel);
				}
				if(kbd.KeyDown(KEY_E))
				{
					box1Pos.SetY(box1Pos.Y() + b1MoveVel);
				}
				if(kbd.KeyDown(KEY_C))
				{
					box1Pos.SetY(box1Pos.Y() - b1MoveVel);
				}
				//rotation
				if(kbd.KeyDown(KEY_I))
				{
					box1Euler.SetX(box1Euler.X() + b1RotVel);
				}
				if(kbd.KeyDown(KEY_K))
				{
					box1Euler.SetX(box1Euler.X() - b1RotVel);
				}
				if(kbd.KeyDown(KEY_J))
				{
					box1Euler.SetY(box1Euler.Y() + b1RotVel);
				}
				if(kbd.KeyDown(KEY_L))
				{
					box1Euler.SetY(box1Euler.Y() - b1RotVel);
				}
				if(kbd.KeyDown(KEY_U))
				{
					box1Euler.SetZ(box1Euler.Z() + b1RotVel);
				}
				if(kbd.KeyDown(KEY_M))
				{
					box1Euler.SetZ(box1Euler.Z() - b1RotVel);
				}
			}
			void updateCtrlr(Game* game, const GameTime& time)
			{
				if(input->NumHID() < 1)
				{
					return;
				}
				Controller& ctrlr = input->GetController(0);
				//fine tuning button!
				if(ctrlr.ButtonDown(0))
				{
					box2VelFactor = 0.5f;
				}
				F32 b2MoveVel = box2VelFactor * moveVel;
				F32 b2RotVel = box2VelFactor * rotVel;
				//note; we want some inverted control options in here eventually
				Vector2 box2VelVec = ctrlr.GetAxisPairFiltered(1,0, ZoneType::RADIUS) * b2MoveVel;
				box2VelVec.SetY(-box2VelVec.Y());
				box2Pos += Vector3(	box2VelVec.X(),
									box2VelVec.Y(),
									0.0f);
				if(ctrlr.ButtonDown(1))
				{
					box2Pos.SetZ(box2Pos.Z() + b2MoveVel);
				}
				if(ctrlr.ButtonDown(2))
				{
					box2Pos.SetZ(box2Pos.Z() - b2MoveVel);
				}
				Vector2 box2RotVec = ctrlr.GetAxisPairFiltered(2,3, ZoneType::RADIUS) * b2RotVel;
				box2RotVec.SetX(-box2RotVec.X());
				box2Euler += Vector3(	-box2RotVec.X(),
										box2RotVec.Y(),
										0.0f);
				if(ctrlr.ButtonDown(4))
				{
					box2Euler.SetZ(box2Euler.Z() + b2RotVel);
				}
				if(ctrlr.ButtonDown(5))
				{
					box2Euler.SetZ(box2Euler.Z() - b2RotVel);
				}
			}
			void updateMouse(Game* game, const GameTime& time)
			{
			}
		public:
			InputTest()
			{ 
				showWnd = true;
			}
			~InputTest() {}
			TypedHandle<InputManager> input;
			F32 timerMs;
			F32 updateRateMs;
			bool Startup(Game* game)
			{
				world = Matrix4x4::Identity;
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());
				projection = Matrix4x4::BuildPerspectiveRH(gfx->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
				//we don't have a camera class yet; ordinarily it would handle this
				view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, 3.0f), //place it 3 units BACKWARD
														Vector3(0.0f, 0.0f, 0.0f), //by default, look at the origin
														Vector3::Up); //and use default up direction for now

				//init box1 state
				box1Pos = Vector3::Zero;
				box1Euler = Vector3::Zero;
				box2Pos = Vector3::Zero;
				box2Euler = Vector3::Zero;
				box1VelFactor = 1.0f;
				box2VelFactor = 1.0f;
				moveAccel = 1.5f;
				rotAccel = 1.5f;
				moveVel = 0;
				moveVel = 0;

				//first thing we want to do is enumerate the devices.
				//display names, and display controls on each device:
				//	* buttons
				//	* axii
				L_ASSERT(game->Platform() && "No platform found!");
				timerMs = 0;
				updateRateMs = 100;
				IPlatform* plat = game->Platform();
				game->Stats().SetShouldPrint(false);
				return true;
			}
			void Shutdown(Game* game) 
			{
			}
			void PreOS(Game* game)
			{
				
			}
			void Update(Game* game, const GameTime& time)
			{
				//updateTerm(game, time);
				//next, run the sim.
				//WASD move the box1 around, EC move it up and down.
				//IK rotate the box1 around X, JL around Y, UM around Z.
				//If a controller's plugged in,
				//Axii 1,2,3 move box1 around XYZ, and buttons 1-6 rotate about XYZ.
				//Maybe I'll figure out something for the mouse...
				Keyboard& kbd = input->GetKeyboard(0);
				//also, handle quitting
				if(kbd.KeyDown(KEY_ESCAPE))
				{
					game->Quit();
				}
				//update rates
				moveVel = moveAccel * time.ElapsedGameTime().ToSeconds();
				rotVel = rotAccel * time.ElapsedGameTime().ToSeconds();
				box1VelFactor = 1.0f;
				box2VelFactor = 1.0f;

				updateKbd(game, time);
				updateCtrlr(game, time);

				//clamp the position now
				box1Pos.SetX(Math::Clamp(box1Pos.X(), -2.5f, 2.5f));
				box1Pos.SetY(Math::Clamp(box1Pos.Y(), -2.5f, 2.5f));
				box1Pos.SetZ(Math::Clamp(box1Pos.Z(), -4.5f, 0.5f));

				box2Pos.SetX(Math::Clamp(box2Pos.X(), -2.5f, 2.5f));
				box2Pos.SetY(Math::Clamp(box2Pos.Y(), -2.5f, 2.5f));
				box2Pos.SetZ(Math::Clamp(box2Pos.Z(), -4.5f, 0.5f));

				updateTerm(game, time);
			}
			void Draw(Game* game, const GameTime& time) 
			{
				gfx->Clear(Colors::Gray20Pct);
				gfx->SetWorldViewProjection(world, view, projection);
				//draw a coordinate system!
				Vector3 coordStart = Vector3(2.0f, 2.0f, 0.0f);
				/*
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(1,0,0), Colors::Red);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0,1,0), Colors::Green);
				//remember +Z points towards viewer in RHS!
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0,0,1), Colors::Blue);
				*/
				//then draw a box
				F32 totSecs = time.TotalGameTime().ToSeconds();
				//and make it spin, too
				gfx->DebugDrawBox(	box1Pos, 
					Quaternion::FromEulerAngles(box1Euler.X(), box1Euler.Y(), box1Euler.Z()),
											0.5f,
											0.5f,
											0.5f,
											Colors::Cyan);
				gfx->DebugDrawBox(	box2Pos, 
					Quaternion::FromEulerAngles(box2Euler.X(), box2Euler.Y(), box2Euler.Z()),
											0.5f,
											0.5f,
											0.5f,
											Colors::Magenta);
				//gfx->DebugDrawSphere(	box1Pos,	0.5f, Colors::Yellow);
			}
		};

		/**
		Tests model loading, writing(if necessary),
		and model rendering with point lights.
		*/
		class ModelTest : public TestBase
		{
		protected:
			Matrix4x4 world, view, projection;
			Geometry* geomList;
			ResPtr texPtr;
			ResPtr defaultTexPtr;
			ResPtr modelPtr;
			String modelName;
			U32 numMeshes;
			TypedHandle<InputManager> input;
			ResourceManager resMgr;
			TypedHandle<Keyboard> kbd;
			Vector3 camPos;
			Vector3 camLookAt;
			Vector3 camUp;
			Vector3 lightPos;
			Vector3 lightColor;
			Color AABBBndColor;
			Color SphereBndColor;
			bool shouldDrawBounds;

			Model model;
			void loadMeshes(const aiScene* scene, Geometry* geomList)
			{
				F32 totalLoadTimeSecs = 0;
				//super fun - we need to convert the mesh's vector data to our vector version,
				//then pass that converted data to the geometry.
				//since we're here, might as well dynamically allocate the temporary buffers, too.
				const U32 MAX_ELEMENTS = 1 << 19;
				Color* convertedColor = CustomArrayNew<Color>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				Vector3* convertedPos = CustomArrayNew<Vector3>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				Vector3* convertedNorms = CustomArrayNew<Vector3>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				Vector2* convertedUVs = CustomArrayNew<Vector2>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				U32* indices = CustomArrayNew<U32>(MAX_ELEMENTS * 3, MESH_ALLOC, "MeshTempBufAlloc");
				for(U32 i = 0; i < scene->mNumMeshes; ++i)
				{
					time->Tick();
					aiMesh* mesh = scene->mMeshes[i];
					U32 numVerts = mesh->mNumVertices;
					U32 numFaces = mesh->mNumFaces;
					//we can assume this, 
					//since the importer triangulates everything
					U32 numIndices = numFaces*3;
					L_ASSERT(	numVerts < MAX_ELEMENTS && 
								numFaces < MAX_ELEMENTS && 
								"Mesh is too big!");
					//need to rotate the vertices, too.
					//Blender uses +Z as up, +Y forward;
					//we use +Y up, -Z forward
					Matrix4x4 convertUpAxis = Matrix4x4::FromEulerAngles(
											-Math::PI_OVER_2, 0, Math::PI);
					#pragma region Temp Buffer Load
					const bool hasColor = mesh->GetNumColorChannels() > 0;
					const bool hasNormals = mesh->HasNormals();
					const bool hasUVs = mesh->HasTextureCoords(0);
					for(U32 j = 0; j < numVerts; ++j)
					{
						aiVector3D& pos = mesh->mVertices[j];
						Vector3 newPos = Vector3(pos.x, pos.y, pos.z);
						convertedPos[j] = convertUpAxis.MultiplyPoint(newPos);
						if(hasColor)
						{
							aiColor4D& color = mesh->mColors[0][j];
							convertedColor[j] = Color(color.r, color.g, color.b, color.a);
						}
						else
						{
							convertedColor[j] = Colors::White;
						}
						if(hasNormals)
						{
							aiVector3D& norm = mesh->mNormals[j];
							convertedNorms[j] = convertUpAxis.MultiplyVector(Vector3(norm.x, norm.y, norm.z));
						}
						else
						{
							//can't just say it has NO normals...
							convertedNorms[j] = Vector3::Up;
						}
						if(hasUVs)
						{
							aiVector3D& uv = mesh->mTextureCoords[0][j];
							Vector3 rotated = Vector3(uv.x, uv.y, uv.z);
							//rotated = convertUpAxis.MultiplyVector(rotated);
							convertedUVs[j] = Vector2(rotated.X(), rotated.Y());
							//Log::V(String("UV ") + j + ":\t" + convertedUVs[j].ToString());
						}
						else
						{
							convertedUVs[j] = Vector2::Zero;
						}
					}
					//index is a little easier to iterate over.
					for(U32 j = 0; j < numFaces; ++j)
					{
						aiFace& face = mesh->mFaces[j];
						for(U32 k = 0; k < 3; ++k)
						{
							indices[(j*3) + k] = face.mIndices[k];
						}
					}
					#pragma endregion
					//now FINALLY init the geometry.
					Log::D(	String("Loading mesh ") + i +
							". Vertices: " + numVerts + 
							", Indices: " + numIndices);
					TypedArrayHandle<Vertex> vertHnd = GeomHelpers::BuildVertArray(convertedPos, 
						convertedNorms, 
						convertedColor, 
						convertedUVs, 
						numVerts);
					if(!vertHnd.GetHandle())
					{
						Log::W(String("Failed to load mesh ") + i + "!");
					}
					U32* inds = CustomArrayNew<U32>(numIndices * 3,
													MESH_ALLOC, 
													"MeshIndexAlloc");
					memcpy(inds, indices, numIndices * sizeof(U32));
					geomList[i].Initialize(vertHnd,
										HandleMgr::RegisterPtr((void*)inds),
										numVerts, 
										numIndices, 
										1);
					
					//and load it into the renderer's buffer!
					if(!gfx->InitGeometry(geomList[i]))
					{
						Log::W(String(	"Failed to load mesh ") + i	+
										" into renderer!");
					}
					time->Tick();
					F32 loadTimeSecs = time->ElapsedGameTime().ToSeconds();
					Log::D(String("Mesh loaded in ") + loadTimeSecs + "s");
					totalLoadTimeSecs += loadTimeSecs;
				}
				//now get rid of the buffers!
				CustomArrayDelete(convertedColor);
				CustomArrayDelete(convertedPos);
				CustomArrayDelete(convertedNorms);
				CustomArrayDelete(indices);
				Log::D(String("All meshes loaded in ") + totalLoadTimeSecs +					"s");
			}
			void listMeshDetails(const aiScene* scene)
			{
				Log::D("Meshes:");
				for(U32 i = 0; i < scene->mNumMeshes; ++i)
				{
					time->Tick();
					Log::D(String("\tMesh ") + i);
					aiMesh* mesh = scene->mMeshes[i];
					Log::D(String("\tName: ") + mesh->mName.C_Str());
					Log::D(String("\tVertices: ") + mesh->mNumVertices);
					Log::D(String("\tFaces: ") + mesh->mNumFaces);
					//and list the indices.
					//we triangulate on import, so we can assume there's
					//3*(num faces) indices.
					Log::D(String("\tIndices: ") + mesh->mNumFaces*3);
					Log::D(String("\tBones: ") + mesh->mNumBones);
					Log::D(String("\tUV Channels: ") + mesh->GetNumUVChannels());
					Log::D(String("\tColor Channels: ") + mesh->GetNumColorChannels());
					Log::D(String("\tHas Normals: ") + mesh->HasNormals());
					Log::D(String("\tHas Tangent Data: ") + mesh->HasTangentsAndBitangents());
					time->Tick();
					F32 readTimeSecs = time->ElapsedGameTime().ToSeconds();
					Log::D(String("Mesh LISTED in ") + readTimeSecs + "s");
				}
			}
			void listMaterialDetails(const aiScene* scene)
			{
				aiString name;
				aiColor3D color(0,0,0);
				U32 intVal = 0;
				F32 floatVal = 0;
				Log::D("Materials:");
				for(U32 i = 0; i < scene->mNumMaterials; ++i)
				{
					time->Tick();
					Log::D(String("\tMaterial ") + i);
					aiMaterial* mat = scene->mMaterials[i];
					mat->Get(AI_MATKEY_NAME, name);
					Log::D(String("\tName: ") + name.C_Str());
					mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
					Log::D(String("\tDiffuse: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
					//clear the color
					color = aiColor3D(0,0,0);
					mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
					Log::D(String("\tSpecular: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
					color = aiColor3D(0,0,0);
					mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
					Log::D(String("\tAmbient: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
					mat->Get(AI_MATKEY_ENABLE_WIREFRAME, intVal);
					Log::D(String("\tWireframe: ") + ((bool)intVal));
					intVal = 0;
					mat->Get(AI_MATKEY_TWOSIDED, intVal);
					Log::D(String("\tTwo Sided: ") + ((bool)intVal));
					mat->Get(AI_MATKEY_OPACITY, floatVal);
					Log::D(String("\tOpacity: ") + floatVal);
					floatVal = 0;
					mat->Get(AI_MATKEY_SHININESS, floatVal);
					Log::D(String("\tShininess: ") + floatVal);
					time->Tick();
					F32 readTimeSecs = time->ElapsedGameTime().ToSeconds();
					Log::D(String("Material LISTED in ") + readTimeSecs + "s");
				}
			}
			void initInput(Game* game)
			{
				input = game->Input();
				kbd = HandleMgr::RegisterPtr(&input->GetKeyboard(0));
			}
			void initMatrices(Game* game)
			{
				world = Matrix4x4::Identity;
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());
				projection = Matrix4x4::BuildPerspectiveRH(gfx->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
				//we don't have a camera class yet; ordinarily it would handle this
				camPos = Vector3(3.0f, 3.0f, 3.0f);
				camUp = Vector3::Up;
				lightPos = Vector3(1.0f, 1.0f, 1.0f);
				lightColor = Vector3(1.0f, 1.0f, 1.0f);
				camLookAt = Vector3(0.0f, 0.0f, 0.0f);
				//view = Matrix4x4::BuildViewRH(camPos, Vector3::Forward, Vector3::Up);
				view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, camUp);
			}
			bool initShaders()
			{
				Vector< ShaderFilePair > pathsColor, pathsDiffuse, pathsDiffuseTextured;
				pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.vp").c_str())) );
				pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.fp").c_str())) );

				pathsDiffuse.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.vp").c_str())) );
				pathsDiffuse.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.fp").c_str())) );

				pathsDiffuseTextured.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.vp").c_str())) );
				pathsDiffuseTextured.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.fp").c_str())) );

				return	gfx->MakeShader("Color", 2, pathsColor) &&
						gfx->MakeShader("Diffuse", 2, pathsDiffuse) &&
						gfx->MakeShader("DiffuseTextured", 2, pathsDiffuseTextured);
			}
			bool initModelImport(Game* game, const char* archName)
			{
				//get the resource for the test mesh
				ResGUID modelGUID(archName, "Models/" + modelName + ".dae");
				Log::D(String("Opening ") + modelGUID.Name);
				//modelName = modelGUID.BaseName();
				ResPtr modelRes = resMgr.GetResource(modelGUID);
				if(!modelRes)
				{
					Log::E("Couldn't get model!");
					return false;
				}

				//load texture
				Log::SetVerbosity(Log::VERB);
				ResGUID pngGUID(archName, "Textures/testimg1024.png");

				time->Tick();
				texPtr = resMgr.GetResource(pngGUID);
				time->Tick();

				if(!texPtr)
				{
					Log::E("Couldn't load texture!");
					return false;
				}
				Log::D(String("Loaded texture in ") + time->ElapsedGameTime().ToSeconds() + "s");

				Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
				Log::D(String("Indicated size of tex (kB): ") + texRef.Width * texRef.Height * (U32)texRef.BitDepth / 1024);

				time->Tick();
				bool texLoaded = gfx->InitTexture(texRef, TextureMeta::DIFFUSE);
				time->Tick();
				if(!texLoaded)
				{
					Log::E("Couldn't init texture into graphics wrapper!");
					return false;
				}
				Log::D(String("Loaded texture into graphics wrapper in ") + time->ElapsedGameTime().ToSeconds() + "s");

				Log::SetVerbosity(Log::DEBUG);
				Assimp::Importer importer;
				importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);//1);
				U32 flags = aiProcess_Triangulate |		
							aiProcess_CalcTangentSpace |
							aiProcess_GenUVCoords |
							aiProcess_GenNormals |
							aiProcess_SortByPType |
							aiProcess_OptimizeMeshes |
							aiProcess_ImproveCacheLocality |
							aiProcess_TransformUVCoords |
							aiProcess_FindDegenerates |
							aiProcess_JoinIdenticalVertices |
							aiProcess_RemoveRedundantMaterials |
							aiProcess_LimitBoneWeights |
							aiProcess_SplitLargeMeshes |
							aiProcess_FindInvalidData |
							0;

				//for now, we just want to try navigating the mesh.
				time->Tick();
				 const aiScene* scene = importer.ReadFileFromMemory(modelRes->Buffer(), modelRes->Size(), flags);
				//const aiScene* scene = importer.ReadFile((String(game->Platform()->GetProgDir()) + "/TestContent/Models/SpaceShip.dae").c_str(),
				//	flags | aiProcess_TransformUVCoords | aiProcess_Triangulate);
				time->Tick();
				F32 loadTimeSecs = time->ElapsedGameTime().ToSeconds();
				if(!scene)
				{
					//load failed
					Log::E("Load failed! Error:");
					Log::E(importer.GetErrorString());
					return false;
				}
				Log::D(String("Imported model in ") + loadTimeSecs + "s!");
				//begin noting information
				numMeshes = scene->mNumMeshes;
				Log::D(String("Number of meshes: ") + numMeshes);
				listMeshDetails(scene);
				Log::D(String("Number of materials: ") + scene->mNumMaterials);
				listMaterialDetails(scene);

				//now load meshes!
				geomList = CustomArrayNew<Geometry>(scene->mNumMeshes, MESH_ALLOC, "MeshAlloc");
				memset(geomList, 0, scene->mNumMeshes*sizeof(Geometry));
				Log::SetVerbosity(Log::VERB);
				loadMeshes(scene, geomList);
				Log::SetVerbosity(Log::DEBUG);

				return true;
			}
			bool buildModel(Game* game, const char* archName)
			{
				//we don't import material data from assimp yet,
				//use a default
				Material defMat;
				defMat.DiffuseColor = Colors::White;
				defMat.SpecularColor = Colors::Black;
				defMat.EmissiveColor = Colors::Black;
				defMat.DiffuseTexGUID = ResGUID(archName, "Textures/testimg1024.png");

				//iterate through geometry
				for(U32 i = 0; i < numMeshes; ++i)
				{
					//we don't import material data from assimp yet,
					//use a default
					Mesh mesh(defMat, geomList[i]);
					//and insert the new mesh
					model.AddMesh(mesh);
				}
				//bounds are definitely invalid, recalc
				model.RecalcBounds();
				Log::D("Built model from imported data!");

				return true;
			}
			bool exportModel(Game* game, const char* archName)
			{
				//Bounds might not be valid,
				//make a quick check before writing.
				if(!model.HasBounds())
				{
					model.RecalcBounds();
				}
				//now write the model!
				Path filePath = String(Filesystem::GetProgDir()) + "/Output/" + modelName + ".lmdl";
				Log::D(String("Writing model file to ") + filePath.ToString());
				//make the output directory if it doesn't exist
				if(!Filesystem::Exists(filePath.GetDirectory()))
				{
					if(!Filesystem::MakeDirectory(filePath.GetDirectory()))
					{
						Log::W("Couldn't make model output directory!");
						return false;
					}
				}
				//overwrite if necessary
				if(Filesystem::Exists(filePath))
				{
					Filesystem::RemoveFile(filePath);
				}
				DataStream* modelFile = Filesystem::OpenFile(filePath);
				if(!modelFile)
				{
					Log::W("Couldn't open model file for writing!");
					return false;
				}

				time->Tick();
				bool fileSaved = ModelMgr::WriteModelFile(model, modelFile);
				time->Tick();
				if(!fileSaved)
				{
					Log::W("Couldn't write model file!");
					return false;
				}
				Log::D(String("Wrote model file in ") + time->ElapsedGameTime().ToSeconds() + "s");

				Filesystem::CloseFile(modelFile);

				return true;
			}
			bool loadModelRes(Game* game, const char* archName)
			{
				//remember to add the importer, stupid
				//resMgr.RegisterLoader(GetSharedPtr(CustomNew<ModelLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				ResGUID lmdlGUID(archName, "Models/" + modelName + ".lmdl");
				Log::D(String("Loading .lmdl file ") + lmdlGUID.Name);
				time->Tick();
				modelPtr = resMgr.GetResource(lmdlGUID);
				time->Tick();
				if(!modelPtr)
				{
					Log::E(String("Failed to load ") + lmdlGUID.Name + "!");
					return false;
				}
				Log::D(String("Loaded .lmdl in ") + time->ElapsedGameTime().ToSeconds() + "s");
				return true;
			}
			bool initModelIntoRenderer(Model& model)
			{
				//Steps after this will delete geometry data;
				//recalculate bounds if needed before that data is lost.
				//if(!model.HasBounds())
				//{
					model.RecalcBounds();
				//}
				for(U32 i = 0; i < model.MeshCount(); ++i)
				{
					if(!gfx->InitGeometry(model.GetMesh(i)->GetGeometry()))
					{
						Log::E("Couldn't load model into OpenGL!");
						return false;
					}
					//we can get rid of geometry data now
					const Geometry& geom = model.GetMesh(i)->GetGeometry();
					//HandleMgr::DeleteHandle(geom.VertexHandle());
					//HandleMgr::DeleteHandle(geom.IndexHandle());

					ResPtr diffTex = resMgr.GetResource(model.GetMesh(i)->GetMaterial().DiffuseTexGUID);
					Log::D(String("Opening texture resource ") + model.GetMesh(i)->GetMaterial().DiffuseTexGUID.Name);
					if(!diffTex)
					{
						Log::E("Couldn't get mesh texture resource!");
						return false;
					} 
					Texture2D& diffTexRef = *(Texture2D*)diffTex->Buffer();
					if(!diffTexRef.TextureBufferHandle)
					{
						if(!gfx->InitTexture(diffTexRef, TextureMeta::DIFFUSE))
						{
							Log::E("Couldn't init mesh texture!");
							return false;
						}
					}
				}
				return true;
			}
			void updateCamPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				if(kbd->KeyDown(KEY_W))
				{
					camPos += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					camPos -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					camPos -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					camPos += Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_E))
				{
					camPos += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_C))
				{
					camPos -= Vector3::Up * elapsedSec;
				}
			}
			void updateLightPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				if(kbd->KeyDown(KEY_I))
				{
					lightPos += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_K))
				{
					lightPos -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_J))
				{
					lightPos -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_L))
				{
					lightPos += Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_U))
				{
					lightPos += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_M))
				{
					lightPos -= Vector3::Up * elapsedSec;
				}
			}
			void drawNormals(const Geometry& geom, float normLen, const Color& color)
			{
				const Vertex* vertArray = geom.Vertices();
				for(U32 i = 0; i < geom.VertexCount(); ++i)
				{
					const Vertex& vert = vertArray[i];
					gfx->DebugDrawLine(vert.Position, vert.Position + (normLen* vert.Normal), color);
				}
			}
			void drawModel(const Model& modelToDraw)
			{
				Texture2D& defTexRef = *(Texture2D*)defaultTexPtr->Buffer();

				for(U32 i = 0; i < modelToDraw.MeshCount(); ++i)
				{
					const Mesh& mesh = *modelToDraw.GetMesh(i);
					const Material& mat = mesh.GetMaterial();
					const Geometry& geom = mesh.GetGeometry();
					//need to setup mesh uniforms;
					//since system doesn't use materials yet,
					//that's just the texture
					//use a default material if there's no texture specified
					ResPtr texPtr = resMgr.GetResource(mat.DiffuseTexGUID);
					const Texture2D& texRef = *(Texture2D*)texPtr->Buffer();
					gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
					/*if(!texPtr)
					{
						gfx->SetTexture(defTexRef, TextureMeta::DIFFUSE);
					}
					else
					{
						const Texture2D& texRef = *(Texture2D*)texPtr->Buffer();
						gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
					}*/

					gfx->Draw(geom);
				}
			}
			void drawBounds(const Model& modelToUse)
			{
				//Bounds center is in model space,
				//transform by world matrix
				Vector3 modelPos = world.MultiplyPoint(modelToUse.BoundsCenter());
				gfx->DebugDrawAABB(	modelPos,
									modelToUse.AABBHalfBounds(),
									AABBBndColor);

				gfx->DebugDrawSphere(	modelPos,
										modelToUse.BoundingRadius(),
										SphereBndColor);
			}
		public:
			ModelTest(void) 
			{
				AABBBndColor = Colors::Green;
				SphereBndColor = Colors::LtGreen;
			}
			~ModelTest(void) {}
			
			bool Startup(Game* game)
			{
				shouldDrawBounds = true;
				geomList = 0;
				if(!TestBase::Startup(game))
				{
					return false;
				}
				//game->Stats().SetShouldPrint(false);

				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				//register texture importers...
				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";
				modelName = "taurus";

				//load up default texture
				ResGUID defTexGUID(ARCHIVE_NAME, "Textures/defaultTex.png"); 
				defaultTexPtr = resMgr.GetResource(defTexGUID);
				if(!defaultTexPtr)
				{
					Log::E("Couldn't load default texture!");
					return false;
				}
				if(!gfx->InitTexture(*(Texture2D*)defaultTexPtr->Buffer(), TextureMeta::DIFFUSE))
				{
					Log::E("Couldn't load default texture into OpenGL!");
					return false;
				}

				//setup input system
				initInput(game);
				initMatrices(game);

				//setup shaders
				if(!initShaders())
				{
					Log::E("Failed to build shaders!");
					return false;
				}

				
				//now try setting up the importer
				/*if(!initModelImport(game, ARCHIVE_NAME))
				{
					return false;
				}
				if(!buildModel(game, ARCHIVE_NAME))
				{
					return false;
				}
				Log::SetVerbosity(Log::VERB);
				//and write out a .lmdl
				if(!exportModel(game, ARCHIVE_NAME))
				{
					return false;
				}*/
				
				//try loading that model now
				//and try loading a model resource
				Log::SetVerbosity(Log::VERB);
				if(!loadModelRes(game, ARCHIVE_NAME))
				{
					return false;
				}
				Model& resModel = *(Model*)modelPtr->Buffer();
				if(!initModelIntoRenderer(resModel))
				{
					return false;
				}
				
				Log::SetVerbosity(Log::DEBUG);

				return true;
			}
			void Shutdown(Game* game)
			{
				if(geomList)
				{
					for(U32 i = 0; i < numMeshes; ++i)
					{
						gfx->ShutdownMesh(geomList[i]);
						//geometry doesn't handle its own memory,
						//so delete its buffers
						HandleMgr::DeleteArrayHandle(geomList[i].VertexHandle());
						HandleMgr::DeleteArrayHandle(geomList[i].IndexHandle());
					}
					CustomArrayDelete(geomList);
				}
				resMgr.Shutdown();
			}
			void Update(Game* game, const GameTime& time)
			{
				updateCamPos(time);
				updateLightPos(time);
				if(kbd->KeyPressed(KEY_B))
				{
					shouldDrawBounds = !shouldDrawBounds;
				}
				view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, camUp);
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);

				gfx->SetWorldViewProjection(world, view, projection);
				//draw a coordinate system!
				Vector3 coordStart = Vector3(2.0f, 2.0f, 0.0f);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(1.0f, 0.0f, 0.0f), Colors::Red);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0.0f, 1.0f, 0.0f), Colors::Green);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0.0f, 0.0f, 1.0f), Colors::Blue);
				//next, draw the light's position.
				gfx->DebugDrawSphere(lightPos, 0.5f, Color(lightColor));
				//also draw normals
				/*for(U32 i = 0; i < numMeshes; ++i)
				{
					//drawNormals(geomList[i], 0.5f, Colors::Red);
				}*/
				//retrieve the model from the loaded resource
				Model& resModel = *(Model*)modelPtr->Buffer();
				//draw the model's bounds.
				if(shouldDrawBounds)
				{
					drawBounds(resModel);
				}
				//finally, setup for the actual model...
				gfx->SetShader("DiffuseTextured");
				gfx->SetVec3Uniform("lightDiffuse", lightColor);
				gfx->SetVec3Uniform("lightPos", lightPos);
				//Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
				//gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
				gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
				//and draw the actual model.
				drawModel(resModel);
			}
		};

		class ConvertModelTest : public TestBase
		{
		protected:
			//Geometry* geomList;
			ResPtr modelPtr;
			String modelName;
			U32 numMeshes;
			EditorResManager resMgr;

			Model model;

			//conversion fields
			Color* convertedColor;
			Vector3* convertedPos;
			Vector3* convertedNorms;
			Vector2* convertedUVs;
			U32* indices;

			static const U32 MAX_ELEMENTS = 1 << 19;

			void loadMeshes(const aiScene* scene, const char* archName)
			{
				F32 totalLoadTimeSecs = 0;
				//super fun - we need to convert the mesh's vector data to our vector version,
				//then pass that converted data to the geometry.
				
				for(U32 i = 0; i < scene->mNumMeshes; ++i)
				{
					time->Tick();
					aiMesh* mesh = scene->mMeshes[i];
					U32 numVerts = mesh->mNumVertices;
					U32 numFaces = mesh->mNumFaces;
					//we can assume this, since the importer triangulates everything
					U32 numIndices = numFaces*3;
					L_ASSERT(numVerts < MAX_ELEMENTS && numFaces < MAX_ELEMENTS && "Mesh is too big!");
					//need to rotate the vertices, too.
					//Blender uses +Z as up, +Y forward;
					//we use +Y up, -Z forward
					Matrix4x4 convertUpAxis = Matrix4x4::FromEulerAngles(-Math::PI_OVER_2, 0, Math::PI);
					#pragma region Temp Buffer Load
					const bool hasColor = mesh->GetNumColorChannels() > 0;
					const bool hasNormals = mesh->HasNormals();
					const bool hasUVs = mesh->HasTextureCoords(0);
					for(U32 j = 0; j < numVerts; ++j)
					{
						aiVector3D& pos = mesh->mVertices[j];
						Vector3 newPos = Vector3(pos.x, pos.y, pos.z);
						convertedPos[j] = convertUpAxis.MultiplyPoint(newPos);
						if(hasColor)
						{
							aiColor4D& color = mesh->mColors[0][j];
							convertedColor[j] = Color(color.r, color.g, color.b, color.a);
						}
						else
						{
							convertedColor[j] = Colors::White;
						}
						if(hasNormals)
						{
							aiVector3D& norm = mesh->mNormals[j];
							convertedNorms[j] = convertUpAxis.MultiplyVector(Vector3(norm.x, norm.y, norm.z));
						}
						else
						{
							//can't just say it has NO normals...
							convertedNorms[j] = Vector3::Up;
						}
						if(hasUVs)
						{
							aiVector3D& uv = mesh->mTextureCoords[0][j];
							Vector3 rotated = Vector3(uv.x, uv.y, uv.z);
							//rotated = convertUpAxis.MultiplyVector(rotated);
							convertedUVs[j] = Vector2(rotated.X(), rotated.Y());
							//Log::V(String("UV ") + j + ":\t" + convertedUVs[j].ToString());
						}
						else
						{
							convertedUVs[j] = Vector2::Zero;
						}
					}
					//index is a little easier to iterate over.
					for(U32 j = 0; j < numFaces; ++j)
					{
						aiFace& face = mesh->mFaces[j];
						for(U32 k = 0; k < 3; ++k)
						{
							indices[(j*3) + k] = face.mIndices[k];
						}
					}
					#pragma endregion
					//now FINALLY init the geometry.
					//Log::D(String("Loading mesh ") + i + ". Vertices: " + numVerts + ", Indices: " + numIndices);
					TypedArrayHandle<Vertex> vertHnd = GeomHelpers::BuildVertArray(convertedPos, convertedNorms, convertedColor, convertedUVs, numVerts);
					if(!vertHnd.Ptr())
					{
						Log::W(String("Failed to load mesh ") + i + "!");
					}
					U32* inds = CustomArrayNew<U32>(numIndices * 3, MESH_ALLOC, "MeshIndexAlloc");
					L_ASSERT(inds);
					memcpy(inds, indices, numIndices * sizeof(U32));
					Geometry geom = Geometry();
					geom.Initialize(vertHnd, HandleMgr::RegisterPtr((void*)inds),
											numVerts, numIndices, 1);
					time->Tick();

					F32 loadTimeSecs = time->ElapsedGameTime().ToSeconds();
					//Log::D(String("Mesh loaded in ") + loadTimeSecs + "s");
					totalLoadTimeSecs += loadTimeSecs;

					//we don't import material data from assimp yet,
					//use a default
					Material defMat;
					defMat.DiffuseColor = Colors::White;
					defMat.SpecularColor = Colors::Black;
					defMat.EmissiveColor = Colors::Black;
					defMat.DiffuseTexGUID = ResGUID(archName, "Textures/testimg1024.png");

					//we don't import material data from assimp yet,
					//use a default
					Mesh modUnit(defMat, geom);
					//and insert the new mesh
					model.AddMesh(modUnit);
				}

				Log::D(String("All meshes in model loaded in ") + totalLoadTimeSecs + "s");
			}
			void listMeshDetails(const aiScene* scene)
			{
				Log::D("Meshes:");
				for(U32 i = 0; i < scene->mNumMeshes; ++i)
				{
					time->Tick();
					Log::D(String("\tMesh ") + i);
					aiMesh* mesh = scene->mMeshes[i];
					Log::D(String("\tName: ") + mesh->mName.C_Str());
					Log::D(String("\tVertices: ") + mesh->mNumVertices);
					Log::D(String("\tFaces: ") + mesh->mNumFaces);
					//and list the indices.
					//we triangulate on import, so we can assume there's
					//3*(num faces) indices.
					Log::D(String("\tIndices: ") + mesh->mNumFaces*3);
					Log::D(String("\tBones: ") + mesh->mNumBones);
					Log::D(String("\tUV Channels: ") + mesh->GetNumUVChannels());
					Log::D(String("\tColor Channels: ") + mesh->GetNumColorChannels());
					Log::D(String("\tHas Normals: ") + mesh->HasNormals());
					Log::D(String("\tHas Tangent Data: ") + mesh->HasTangentsAndBitangents());
					time->Tick();
					F32 readTimeSecs = time->ElapsedGameTime().ToSeconds();
					Log::D(String("Mesh LISTED in ") + readTimeSecs + "s");
				}
			}
			void listMaterialDetails(const aiScene* scene)
			{
				aiString name;
				aiColor3D color(0,0,0);
				U32 intVal = 0;
				F32 floatVal = 0;
				Log::D("Materials:");
				for(U32 i = 0; i < scene->mNumMaterials; ++i)
				{
					time->Tick();
					Log::D(String("\tMaterial ") + i);
					aiMaterial* mat = scene->mMaterials[i];
					mat->Get(AI_MATKEY_NAME, name);
					Log::D(String("\tName: ") + name.C_Str());
					mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
					Log::D(String("\tDiffuse: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
					//clear the color
					color = aiColor3D(0,0,0);
					mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
					Log::D(String("\tSpecular: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
					color = aiColor3D(0,0,0);
					mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
					Log::D(String("\tAmbient: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
					mat->Get(AI_MATKEY_ENABLE_WIREFRAME, intVal);
					Log::D(String("\tWireframe: ") + ((bool)intVal));
					intVal = 0;
					mat->Get(AI_MATKEY_TWOSIDED, intVal);
					Log::D(String("\tTwo Sided: ") + ((bool)intVal));
					mat->Get(AI_MATKEY_OPACITY, floatVal);
					Log::D(String("\tOpacity: ") + floatVal);
					floatVal = 0;
					mat->Get(AI_MATKEY_SHININESS, floatVal);
					Log::D(String("\tShininess: ") + floatVal);
					time->Tick();
					F32 readTimeSecs = time->ElapsedGameTime().ToSeconds();
					Log::D(String("Material LISTED in ") + readTimeSecs + "s");
				}
			}
			bool initModelImport(Game* game, const ResGUID& modelGUID, const char* archName)
			{
				//modelName = modelGUID.BaseName();
				ResPtr modelRes = resMgr.GetResource(modelGUID);
				if(!modelRes)
				{
					Log::E("Couldn't get model!");
					return false;
				}

				Log::SetVerbosity(Log::DEBUG);
				Assimp::Importer importer;
				importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);//1);
				U32 flags = aiProcess_Triangulate |		
							aiProcess_CalcTangentSpace |
							aiProcess_GenUVCoords |
							aiProcess_GenNormals |
							aiProcess_SortByPType |
							aiProcess_OptimizeMeshes |
							aiProcess_ImproveCacheLocality |
							aiProcess_TransformUVCoords |
							aiProcess_FindDegenerates |
							aiProcess_JoinIdenticalVertices |
							aiProcess_RemoveRedundantMaterials |
							aiProcess_LimitBoneWeights |
							aiProcess_SplitLargeMeshes |
							aiProcess_FindInvalidData |
							0;

				//for now, we just want to try navigating the mesh.
				time->Tick();
				 const aiScene* scene = importer.ReadFileFromMemory(modelRes->Buffer(), modelRes->Size(), flags);
				//const aiScene* scene = importer.ReadFile((String(game->Platform()->GetProgDir()) + "/TestContent/Models/SpaceShip.dae").c_str(),
				//	flags | aiProcess_TransformUVCoords | aiProcess_Triangulate);
				time->Tick();
				F32 loadTimeSecs = time->ElapsedGameTime().ToSeconds();
				if(!scene)
				{
					//load failed
					Log::E("Load failed! Error:");
					Log::E(importer.GetErrorString());
					return false;
				}
				Log::D(String("Imported model in ") + loadTimeSecs + "s!");
				//begin noting information
				numMeshes = scene->mNumMeshes;
				Log::D(String("Number of meshes: ") + numMeshes);
				//listMeshDetails(scene);
				Log::D(String("Number of materials: ") + scene->mNumMaterials);
				//listMaterialDetails(scene);

				//now load meshes!
				/*geomList = CustomArrayNew<Geometry>(scene->mNumMeshes, MESH_ALLOC, "MeshAlloc");
				memset(geomList, 0, scene->mNumMeshes*sizeof(Geometry));*/
				loadMeshes(scene, archName);//geomList);

				return true;
			}
			/*bool buildModel(Game* game, const char* archName)
			{
				//we don't import material data from assimp yet,
				//use a default
				Material defMat;
				defMat.DiffuseColor = Colors::White;
				defMat.SpecularColor = Colors::Black;
				defMat.EmissiveColor = Colors::Black;
				defMat.DiffuseTexGUID = ResGUID(archName, "Textures/testimg1024.png");

				//iterate through geometry
				for(U32 i = 0; i < numMeshes; ++i)
				{
					//we don't import material data from assimp yet,
					//use a default
					Mesh mesh(defMat, geomList[i]);
					//and insert the new mesh
					model.AddMesh(mesh);
				}
				Log::D("Built model from imported data!");

				return true;
			}*/
			bool exportModel(Game* game, const ResGUID& modelGUID)
			{
				//now write the model!
				Path filePath = String(Filesystem::GetProgDir()) + "/Output/" + modelGUID.BaseName() + ".lmdl";
				Log::D(String("Writing model file to ") + filePath.ToString());
				//make the output directory if it doesn't exist
				if(!Filesystem::Exists(filePath.GetDirectory()))
				{
					if(!Filesystem::MakeDirectory(filePath.GetDirectory()))
					{
						Log::W("Couldn't make model output directory!");
						return false;
					}
				}
				//overwrite if necessary
				if(Filesystem::Exists(filePath))
				{
					Filesystem::RemoveFile(filePath);
				}
				DataStream* modelFile = Filesystem::OpenFile(filePath);
				if(!modelFile)
				{
					Log::W("Couldn't open model file for writing!");
					return false;
				}

				time->Tick();
				bool fileSaved = ModelMgr::WriteModelFile(model, modelFile);
				time->Tick();
				if(!fileSaved)
				{
					Log::W("Couldn't write model file!");
					return false;
				}
				Log::D(String("Wrote model file in ") + time->ElapsedGameTime().ToSeconds() + "s");

				Filesystem::CloseFile(modelFile);

				return true;
			}
			void deleteGeomList()
			{
				for(U32 i = 0; i < model.MeshCount(); ++i)
				{
					//geometry doesn't handle its own memory,
					//so delete its buffers
					Geometry& geom = model.GetMesh(i)->GetGeometry();
					HandleMgr::DeleteArrayHandle(geom.VertexHandle());
					HandleMgr::DeleteArrayHandle(geom.IndexHandle());
				}
				model.Clear();
			}
		public:
			ConvertModelTest(void)
			{
				//geomList = NULL;
			}
			~ConvertModelTest(void) {}
			
			bool Startup(Game* game)
			{
				if(!TestBase::Startup(game))
				{
					return false;
				}
				game->Stats().SetShouldPrint(false);

				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";
				ResGUID archGUID = ResGUID(ARCHIVE_NAME, "");
				//try to iterate through the archive for .dae files
				U32 numFiles = resMgr.GetNumResourcesInArchive(archGUID);
				if(!numFiles)
				{
					Log::E("No files!");
					return false;
				}
				Log::D(String("Found ") + numFiles + " files");

				//init the conversion buffers
				convertedColor = CustomArrayNew<Color>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				convertedPos = CustomArrayNew<Vector3>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				convertedNorms = CustomArrayNew<Vector3>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				convertedUVs = CustomArrayNew<Vector2>(MAX_ELEMENTS, MESH_ALLOC, "MeshTempBufAlloc");
				indices = CustomArrayNew<U32>(MAX_ELEMENTS * 3, MESH_ALLOC, "MeshTempBufAlloc");

				for(U32 i = 0; i < numFiles; ++i)
				{
					ResGUID resGUID = resMgr.GetResGUID(archGUID, i);
					if(StringUtils::WildcardMatch(resGUID.ResName(), "*.dae"))
					{
						Log::D(String("Importing model ") + resGUID.Filename());
						//now try setting up the importer
						if(!initModelImport(game, resGUID, ARCHIVE_NAME))
						{
							Log::D("Import failed!");
							return false;
						}
						/*if(!buildModel(game, ARCHIVE_NAME))
						{
							Log::D("Failed to build model!");
							return false;
						}*/
						//and write out a .lmdl
						if(!exportModel(game, resGUID))
						{
							Log::D("Failed to export model!");
							return false;
						}
						deleteGeomList();
						//geomList = NULL;
					}
				}

				//now get rid of the buffers!
				CustomArrayDelete(convertedColor);
				CustomArrayDelete(convertedPos);
				CustomArrayDelete(convertedNorms);
				//CustomArrayDelete(convertedUVs);
				CustomArrayDelete(indices);

				Log::D("Export complete.");
				return false;
			}
			void Shutdown(Game* game)
			{
				resMgr.Shutdown();
			}
			void Update(Game* game, const GameTime& time)
			{
			}
			void Draw(Game* game, const GameTime& time)
			{
			}
		};

		//Tests out features of the Camera classes.
		//WASD, EC move camera, RF adjusts FOV, TG adjust near distance, YH adjust far distance.
		//JL pan, IK tilt, UM rolls.
		//End resets the camera.
		//PgUp and PgDn change camera types.
		class CameraTest : public TestBase
		{
		private:
			enum CameraType { CAMTYPE_FREE_LOOK, CAMTYPE_LOOK_AT, CAMTYPE_ORBIT, CAMTYPE_LEN } camType;

			Matrix4x4 world;
			Geometry tri;
			TypedHandle<Keyboard> kbd;
			TypedHandle<Mouse> mouse;
			CameraBase* camera;
			ResourceManager resMgr;

			String modelName;
			ResPtr modelPtr;

			bool enableMouseLook;

			F32 moveVel;
			F32 fovAdjRate;
			F32 planeAdjRate;
			F32 rotVel;
			F32 mouseSensitivity;

			bool initResMgr(Game* game)
			{
				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";
				modelName = "taurus";

				//load up default texture
				ResGUID defTexGUID(ARCHIVE_NAME, "Textures/defaultTex.png"); 
				ResPtr defaultTexPtr = resMgr.GetResource(defTexGUID);
				if(!defaultTexPtr)
				{
					Log::E("Couldn't load default texture!");
					return false;
				}
				if(!gfx->InitTexture(*(Texture2D*)defaultTexPtr->Buffer(), TextureMeta::DIFFUSE))
				{
					Log::E("Couldn't load default texture into OpenGL!");
					return false;
				}
				return true;
			}
			void initInput(Game* game)
			{
				kbd = HandleMgr::RegisterPtr(&game->Input()->GetKeyboard(0));
				mouse = HandleMgr::RegisterPtr(&game->Input()->GetMouse(0));
			}
			void resetCamera(CameraBase* cam)
			{
				cam->SetAspectRatio(gfx->ScreenAspect());
				cam->SetFOV(Math::PI_OVER_2);
				cam->SetNearDist(0.25f);
				cam->SetFarDist(50.0f);
				cam->SetPosition(-3.0*Vector3::Forward);
				switch(camType)
				{
				case CAMTYPE_FREE_LOOK:
					{
						LogD("Switching to free look camera.");
						FreeLookCamera* castCam = (FreeLookCamera*)cam;
						castCam->SetUp(Vector3::Up);
						castCam->SetForward(Vector3::Forward);
						break;
					}
				case CAMTYPE_LOOK_AT:
					{
						LogD("Switching to look-at camera.");
						((LookAtCamera*)cam)->SetLookAtPos(Vector3(0.0f, 0.0f, 0.0f));
						break;
					}
				case CAMTYPE_ORBIT:
					{
						LogD("Switching to orbit camera.");
						OrbitCamera* castCam = (OrbitCamera*)cam;
						castCam->SetCenter(Vector3(0.0f, 0.0f, 0.0f));
						castCam->SetRadius(3.0f);
						castCam->SetOrientation(Quaternion::Identity);
						break;
					}
				default:
					{
						L_ASSERT("Invalid camera type!");
						break;
					}
				}
			}
			void initCamera()
			{
				CameraBase* newCamera = 0;
				switch(camType)
				{
				case CAMTYPE_FREE_LOOK:
					{
						Log::D("Switching to free look camera");
						newCamera = CustomNew<FreeLookCamera>(TEST_ALLOC, "TestAlloc");
						break;
					}
				case CAMTYPE_LOOK_AT:
					{
						Log::D("Switching to look-at camera");
						newCamera = CustomNew<LookAtCamera>(TEST_ALLOC, "TestAlloc");
						break;
					}
				case CAMTYPE_ORBIT:
					{
						Log::D("Switching to orbit camera");
						newCamera = CustomNew<OrbitCamera>(TEST_ALLOC, "TestAlloc");
						break;
					}
				default:
					{
						//how'd we even get here!?
						Log::W("Invalid camera type!");
						//reset to a lookat camera
						camType = CAMTYPE_LOOK_AT;
						newCamera = CustomNew<LookAtCamera>(TEST_ALLOC, "TestAlloc");
						break;
					}
				}
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());
				resetCamera(newCamera);
				if(camera)
				{
					CustomDelete(camera);
					camera = 0;
				}
				camera = newCamera;
			}
			bool initModel(Game* game)
			{
				//try to create a triangle
				Vector3 triPos[] = {	Vector3(-1.0f, -1.0f, 0.0f),  //bottom left
										Vector3(0.0f, 1.0f, 0.0f),  //top middle
										Vector3(1.0f, -1.0f, 0.0f)}; //bottom right
				Color triColor[] = {	Color(1.0f, 0.0f, 0.0f),  //bottom left
										Color(0.0f, 1.0f, 0.0f),  //top middle
										Color(0.0f, 0.0f, 1.0f)}; //bottom right
				U32 indices[] = {	2,  //bottom right
									1,  //top middle
									0}; //bottom left
				U32 verts = 3;
				U32 inds = 3;
				U32 uvs = 0;

				bool result = GeomHelpers::BuildGeometry(tri, triPos, 0, triColor, 0, indices, verts, inds, uvs);
				if(!result)
				{
					Log::E("Failed to build model!");
					return false;
				}
				else
				{
					Log::D("Built model!");
				}
				//and load the model
				Log::D("Loading model...");
				if(!gfx->InitGeometry(tri))
				{
					Log::E("Failed to load model!");
					return false;
				}
				return true;
			}
			bool initShaders()
			{
				Vector< std::pair< ShaderType,Path > > pathsColor, pathsDiffuse, pathsDiffuseTextured;
				pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.vp").c_str())) );
				pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.fp").c_str())) );

				pathsDiffuse.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.vp").c_str())) );
				pathsDiffuse.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.fp").c_str())) );

				pathsDiffuseTextured.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.vp").c_str())) );
				pathsDiffuseTextured.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.fp").c_str())) );

				return	gfx->MakeShader("Color", 2, pathsColor) &&
						gfx->MakeShader("Diffuse", 2, pathsDiffuse) &&
						gfx->MakeShader("DiffuseTextured", 2, pathsDiffuse);
			}
			bool loadModelRes(Game* game, const char* archName)
			{
				//remember to add the importer, stupid
				resMgr.RegisterLoader(GetSharedPtr(CustomNew<ModelLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				ResGUID lmdlGUID(archName, "Models/" + modelName + ".lmdl");
				Log::D(String("Loading .lmdl file ") + lmdlGUID.Name);
				time->Tick();
				modelPtr = resMgr.GetResource(lmdlGUID);
				time->Tick();
				if(!modelPtr)
				{
					Log::E(String("Failed to load ") + lmdlGUID.Name + "!");
					return false;
				}
				Log::D(String("Loaded .lmdl in ") + time->ElapsedGameTime().ToSeconds() + "s");
				return true;
			}
			bool initModelIntoRenderer(Model& model)
			{
				//Model& model = *(Model*)modelPtr->Buffer();
				for(U32 i = 0; i < model.MeshCount(); ++i)
				{
					if(!gfx->InitGeometry(model.GetMesh(i)->GetGeometry()))
					{
						Log::E("Couldn't load model into OpenGL!");
						return false;
					}
					ResPtr diffTex = resMgr.GetResource(model.GetMesh(i)->GetMaterial().DiffuseTexGUID);
					if(!diffTex)
					{
						Log::E("Couldn't get mesh texture resource!");
						return false;
					} 
					Texture2D& diffTexRef = *(Texture2D*)diffTex->Buffer();
					if(!diffTexRef.TextureBufferHandle)
					{
						if(!gfx->InitTexture(diffTexRef, TextureMeta::DIFFUSE))
						{
							Log::E("Couldn't init mesh texture!");
							return false;
						}
					}
				}
				return true;
			}
			void updateGlobalCommands(const GameTime& time)
			{
				if(kbd->KeyPressed(KEY_END))
				{
					resetCamera(camera);
				}
				//camera type change
				CameraType newCType = camType;
				if(kbd->KeyPressed(KEY_PGUP))
				{
					I32 nextTypeIndex = camType + 1 % CAMTYPE_LEN;
					newCType = (CameraType)nextTypeIndex;
				}
				if(kbd->KeyPressed(KEY_PGDN))
				{
					I32 nextTypeIndex = newCType == 0 ? CAMTYPE_LEN-1 : camType - 1;
					newCType = (CameraType)nextTypeIndex;
				}
				if(newCType != camType)
				{
					camType = newCType;
					initCamera();
				}
				//toggle mouselook
				if(kbd->KeyPressed(KEY_MINUS))
				{
					enableMouseLook = !enableMouseLook;
				}
			}
			void updateCamPos(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 transVec = Vector3::Zero;
				if(kbd->KeyDown(KEY_W))
				{
					transVec += camera->Forward() * moveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					transVec -= camera->Forward() * moveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					transVec += camera->Right() * moveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					transVec -= camera->Right() * moveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_E))
				{
					transVec += camera->Up() * moveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_C))
				{
					transVec -= camera->Up() * moveVel * elapsedSec;
				}
				camera->Translate(transVec);
			}
			void updateCamRot(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 rotVec = Vector3::Zero;
				//tilt
				if(kbd->KeyDown(KEY_I))
				{
					rotVec.SetX(rotVec.X() + (rotVel * elapsedSec));
				}
				if(kbd->KeyDown(KEY_K))
				{
					rotVec.SetX(rotVec.X() - (rotVel * elapsedSec));
				}
				//pan
				if(kbd->KeyDown(KEY_J))
				{
					rotVec.SetY(rotVec.Y() - (rotVel * elapsedSec));
				}
				if(kbd->KeyDown(KEY_L))
				{
					rotVec.SetY(rotVec.Y() + (rotVel * elapsedSec));
				}
				//roll
				if(kbd->KeyDown(KEY_U))
				{
					rotVec.SetZ(rotVec.Z() + (rotVel * elapsedSec));
				}
				if(kbd->KeyDown(KEY_M))
				{
					rotVec.SetZ(rotVec.Z() - (rotVel * elapsedSec));
				}
				//also update mouse tilt and pan, if desired
				if(enableMouseLook)
				{
					Vector2 mouseDelta = mouse->GetMousePos() * (rotVel * elapsedSec * mouseSensitivity);
					//remember that the mouse delta is linear and rotvec is rotational!
					rotVec.SetX(rotVec.X() + mouseDelta.Y());
					rotVec.SetY(rotVec.Y() - mouseDelta.X());
				}

				camera->RotateEuler(rotVec.X(), rotVec.Y(), rotVec.Z());
				camera->SetUp(Vector3::Up);
			}
			void updateCamProj(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				//Update the FOV,
				F32 newFOV = camera->FOV();
				if(kbd->KeyDown(KEY_R))
				{
					newFOV += fovAdjRate * elapsedSec;
				}
				if(kbd->KeyDown(KEY_F))
				{
					newFOV -= fovAdjRate * elapsedSec;
				}
				camera->SetFOV(newFOV);

				//and the near/far planes.
				F32 newNPlane = camera->NearDist();
				if(kbd->KeyDown(KEY_T))
				{
					newNPlane += planeAdjRate * elapsedSec;
				}
				if(kbd->KeyDown(KEY_G))
				{
					newNPlane -= planeAdjRate * elapsedSec;
				}
				camera->SetNearDist(newNPlane);

				F32 newFPlane = camera->FarDist();
				if(kbd->KeyDown(KEY_Y))
				{
					newFPlane += planeAdjRate * elapsedSec;
				}
				if(kbd->KeyDown(KEY_H))
				{
					newFPlane -= planeAdjRate * elapsedSec;
				}
				camera->SetFarDist(newFPlane);
			}
		public:
			CameraTest(void)
			{
				moveVel = 1.0f;
				fovAdjRate = 1.0f;
				planeAdjRate = 1.0f;
				rotVel = 1.0f;
				mouseSensitivity = 1.0f;

				camType = CAMTYPE_LOOK_AT;
				camera = 0;
				enableMouseLook = true;

				//only matrix that needs initializing now!
				world = Matrix4x4::Identity;
			}
			~CameraTest(void) {}
			bool Startup(Game* game)
			{
				game->Platform()->SetShouldLockMouse(true);
				//game->Stats().SetShouldPrint(false);
				if(!TestBase::Startup(game))
				{
					return false;
				}

				if(!initResMgr(game))
				{
					return false;
				}

				//setup input system
				initInput(game);

				initCamera();

				if(!initModel(game))
				{
					return false;
				}

				//setup shaders
				if(!initShaders())
				{
					Log::E("Failed to build shaders!");
					return false;
				}
				gfx->SetShader("Color");

				/*if(!loadModelRes(game, ARCHIVE_NAME))
				{
					return false;
				}
				Model& resModel = *(Model*)modelPtr->Buffer();
				if(!initModelIntoRenderer(resModel))
				{
					return false;
				}*/

				return true;
			}
			void Shutdown(Game* game)
			{
				gfx->ShutdownMesh(tri);
			}
			void Update(Game* game, const GameTime& time)
			{
				updateGlobalCommands(time);
				updateCamPos(time);
				updateCamRot(time);
				updateCamProj(time);
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);
				gfx->SetWorldViewProjection(world, camera->GetViewMatrix(), camera->GetProjMatrix());
				gfx->Draw(tri);
			}
		};

		class ResFileTest : public TestBase
		{
		public:
			ResFileTest(void) {}
			~ResFileTest(void) {}
			bool Startup(Game* game)
			{
				Log::SetVerbosity(Log::VERB);
				//ZipFile file;
				Path filePath(String(Filesystem::GetProgDir()) + "/TestContent/Archives/WithFolders.zip");
				/*game->Time().Tick();
				bool fileLoaded = file.Init(filePath);
				game->Time().Tick();
				if(!fileLoaded)
				{
					Log::E("Couldn't open file archive!");
					return false;
				}
				else
				{
					Log::D(String("Loaded archive in ") + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
				}
				//otherwise, start listing data
				I32 numFiles = file.GetNumFiles();
				Log::D(String("Found ") + numFiles + " files in archive");
				for(I32 i = 0; i < numFiles; ++i)
				{
					Log::D(String("File ") + i + ":");
					Log::D(String("Name: ") + file.GetFilename(i));
					Log::D(String("Size: ") + file.GetFileLen(i));
				}

				//and try to load the files
				char** dataBuffers = CustomArrayNew<char*>(numFiles, TEST_ALLOC, "TestTempBufAlloc");
				
				//also set up a buffer to preview the first few bytes of data
				//only really good for text data
				*/const U32 dispLimit = 128;
				char dispBuf[dispLimit];
				memset(dispBuf, 0, dispLimit);
				F32 totalLoadMs = 0;/*
				for(I32 i = 0; i < numFiles; ++i)
				{
					//decompress the file!
					dataBuffers[i] = CustomArrayNew<char>(file.GetFileLen(i), TEST_ALLOC, "TestTempBufAlloc");
					if(!dataBuffers[i])
					{
						Log::W(String("Couldn't allocate buffer for file ") + i + "!");
						continue;
					}
					game->Time().Tick();
					fileLoaded = file.ReadFile(i, dataBuffers[i]);
					game->Time().Tick();
					if(!fileLoaded)
					{
						Log::W(String("Couldn't read file ") + i + "!");
						continue;
					}

					Log::D(String("Read file ") + i + " in " + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
					totalLoadMs += game->Time().ElapsedGameTime().ToMilliseconds();

					//and build the preview
					strncpy_s(dispBuf, dataBuffers[i], dispLimit);
					dispBuf[dispLimit - 1] = 0;
					Log::D(String("File preview: ") + dispBuf);
				}

				Log::D(String("Loaded all files in ") + totalLoadMs + " ms");

				for(I32 i = 0; i < numFiles; ++i)
				{
					CustomArrayDelete(dataBuffers[i]);
				}
				CustomArrayDelete(dataBuffers);

				//no need to close the file, it'll do that on destruction

				//anyway, test out the GUID system.
				Log::D("Building sample GUID.");
				ResGUID exampleGUID("archive.zip:Folder1/folDer2\\FILE.ext");
				Log::D(String("GUID: ") + exampleGUID.Name);
				Log::D(String("GUID file name: ") + exampleGUID.ResName());
				Log::D(String("GUID extension: ") + exampleGUID.Extension());
				Log::D(String("GUID archive name: ") + exampleGUID.ResArchiveName());

				Log::D("Building BROKEN GUID!");
				ResGUID brokenGUID("archive.zip/Folder1/folDer2/FILE.ext");
				Log::D(String("Broken GUID: ") + brokenGUID.Name);*/

				//now test out the resource archive interface.
				Log::D("Testing resource archive system...");
				IResourceArchive* resArch = CustomNew<ZipResArchive>(TEST_ALLOC, "ArchiveAlloc", filePath);
				if(!resArch || !resArch->Open())
				{
					Log::E("Failed to open archive!");
					return false;
				}
				Log::D(String("Opened archive ") + resArch->GetArchiveName());
				Log::D(String("Archive contains ") + resArch->GetNumResources() + " resources.");
				//make a GUID from the first nonzero resource.
				U32 resourceToLoad = 0;
				String archString = String("TestContent/Archives/") + resArch->GetArchiveName();
				for(U32 i = 0; i < resArch->GetNumResources(); ++i)
				{
					ResGUID tempGUID(archString, resArch->GetResourceName(i));
					if(resArch->GetRawSize(tempGUID) > 0)
					{
						resourceToLoad = i;
						break;
					}
				}
				ResGUID archGUID(archString, resArch->GetResourceName(resourceToLoad));
				Log::D(String("Archive GUID for resource: ") + archGUID.Name);
				//and try to load from it.
				U32 bufSize = resArch->GetRawSize(archGUID);
				Log::D(String("Indicated resource size: ") + bufSize);
				char* tempBuf = CustomArrayNew<char>(bufSize, TEST_ALLOC, "TestTempBufAlloc");
				if(resArch->GetRawResource(archGUID, tempBuf) > 0)
				{
					strncpy_s(dispBuf, tempBuf, dispLimit);
					dispBuf[dispLimit - 1] = 0;
					Log::D("Loaded data.");
					Log::D(String("Preview: ") + dispBuf);
				}

				CustomArrayDelete(tempBuf);
				CustomDelete(resArch);

				//now test the resource manager!
				Log::D("Testing resource manager...");
				ResourceManager resMgr;
				if(!resMgr.Init(16))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				//first load

				Log::D("Loading resource via resource manager...");
				game->Time().Tick();
				std::shared_ptr<Resource> resPtrCacheMiss = resMgr.GetResource(archGUID);
				game->Time().Tick();
				Log::D(String("Loaded resource in ") + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
				Log::D("Loading resource again...");
				game->Time().Tick();
				std::shared_ptr<Resource> resPtrNoMiss = resMgr.GetResource(archGUID);
				game->Time().Tick();


				if(*resPtrCacheMiss != *resPtrNoMiss)
				{
					Log::E("Loaded data doesn't match!");
					return false;
				}

				Log::D(String("Fetched resource pointer in ") + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
				Log::D(String("Name: ") + resPtrNoMiss->GUID().ResName());
				strncpy_s(dispBuf, resPtrNoMiss->Buffer(), dispLimit);
				dispBuf[dispLimit - 1] = 0;
				Log::D(String("Preview: ") + dispBuf);

				//and test resource loading
				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				ResGUID pngGUID("/TestContent/Archives/TestContent.zip",
								"Textures/testImg1024.png");

				game->Time().Tick();
				std::shared_ptr<Resource> pngPtr = resMgr.GetResource(pngGUID);
				game->Time().Tick();

				if(!pngPtr)
				{
					Log::E("Couldn't load PNG!");
					return false;
				}

				Texture2D* texHeader = (Texture2D*)(void*)pngPtr->Buffer();
				Log::D(String("Loaded PNG in ") + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
				Log::D("Texture info follows: ");
				Log::D(String("Width:\t") + texHeader->Width);
				Log::D(String("Height:\t") + texHeader->Height);
				Log::D(String("Bit Depth:\t") + texHeader->BitDepth);
				Log::D(String("Pixel Type:\t") + texHeader->PixType);
				size_t dataSize = (texHeader->Width * texHeader->Height * texHeader->BitDepth) / 8;
				Log::D(String("Data size should be ") + dataSize + " bytes");

				Log::D("Shutting down resource manager...");
				resMgr.Shutdown();

				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		class WildcardTest : public TestBase
		{
		public:
			WildcardTest(void) {}
			~WildcardTest(void) {}
			bool Startup(Game* game)
			{
				const I32 MAX_STR = 256;
				char inStr[MAX_STR] = {0};
				char inWld[MAX_STR] = {0};
				F32 matchTimeMs = 0;
				while(true)
				{
					Log::D("Enter input string, or enter * to quit:");
					std::cin.getline(inStr, MAX_STR);
					Log::D(inStr);
					//String inStrObj(inStr);
					if(strcmp(inStr, "*") == 0)
					{
						break;
					}
					Log::D("Enter wildcard string:");
					std::cin.getline(inWld, MAX_STR);
					Log::D(inWld);
					game->Time().Tick();
					bool matched = StringUtils::WildcardMatch(inStr, inWld);
					game->Time().Tick();
					matchTimeMs = game->Time().ElapsedGameTime().ToMilliseconds();
					if(matched)
					{
						Log::D("String matched!");
					}
					else
					{
						Log::D("No match.");
					}
					Log::D(String("Result found in ") + matchTimeMs + "ms");
				}
				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		class TextureTest : public TestBase
		{
		private:
			Matrix4x4 world, view, projection;
			ResourceManager resMgr;
			ResPtr texPtr;
			Geometry quad;
			InputManager input;
			TypedHandle<Keyboard> kbd;
			Vector3 camPos;
			Vector3 camLookAt;
		public:
			TextureTest(void) {}
			~TextureTest(void) {}
			bool Startup(Game* game)
			{
				if(!TestBase::Startup(game))
				{
					return false;
				}

				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";

				//setup input system
				input.Startup(game->Platform());
				TypedHandle<InputManager> inputHnd = HandleMgr::FindHandle(&input);
				if(!inputHnd.GetHandle())
				{
					inputHnd = HandleMgr::RegisterPtr(&input);
				}
				//link input manager w/ platform
				IPlatform* plat = game->Platform();
				plat->InitInput(game->Input());
				//and tell platform we're ready for input
				kbd = HandleMgr::RegisterPtr(&input.GetKeyboard(0));

				world = Matrix4x4::Identity;
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());
				projection = Matrix4x4::BuildPerspectiveRH(gfx->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
				//we don't have a camera class yet; ordinarily it would handle this
				camPos = -3.0*Vector3::Forward;
				camLookAt = Vector3(0.0f, 0.0f, 0.0f);
				//view = Matrix4x4::BuildViewRH(camPos, Vector3::Forward, Vector3::Up);
				view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, Vector3::Up);

				//try to create a triangle
				Vector3 pos[] = {	Vector3(-1.0f, -1.0f, 0.0f),	//bottom left
									Vector3(1.0f, -1.0f, 0.0f),		//bottom right
									Vector3(-1.0f, 1.0f, 0.0f),		//top left
									Vector3(1.0f, 1.0f, 0.0f) };	//top right

				Vector3 norm[] = {	Vector3(0.0f, 0.0f, 1.0f),  //bottom left
									Vector3(0.0f, 0.0f, 1.0f),  //top middle
									Vector3(0.0f, 0.0f, 1.0f),
									Vector3(0.0f, 0.0f, 1.0f)};

				Vector2 uv[] = {	Vector2(0.0f, 0.0f),	//bottom left
									Vector2(1.0f, 0.0f),	//bottom right
									Vector2(0.0f, 1.0f),	//top left
									Vector2(1.0f, 1.0f)};	//top right

				Color color[] = {	Color(1.0f, 0.0f, 0.0f),  //bottom left
									Color(0.0f, 1.0f, 0.0f),  //top middle
									Color(0.0f, 0.0f, 1.0f), //bottom right
									Color(1.0f, 0.0f, 1.0f)}; //bottom right

				U32 indices[] = {	0,
									1,  //bottom right
									2,  //top middle
									3,
									2,
									1}; //bottom left
				U32 verts = 4;
				U32 inds = 6;
				U8 uvs = 4;

				bool result = GeomHelpers::BuildGeometry(quad, pos, norm, color, uv, indices, verts, inds, uvs);
				if(!result)
				{
					Log::E("Failed to build model!");
					return false;
				}
				else
				{
					Log::D("Built model!");
				}
				Log::D(String("Model has ") + quad.VertexCount() + " vertices");
				//setup shaders

				Vector< ShaderFilePair > pathsColor, TextureUnshaded;
				pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.vp").c_str())) );
				pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.fp").c_str())) );

				TextureUnshaded.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/TextureUnshaded.vp").c_str())) );
				TextureUnshaded.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./haders/Basic/TextureUnshaded.fp").c_str())) );


				if(	!gfx->MakeShader("Color", 2, pathsColor) ||
					!gfx->MakeShader("TextureUnshaded", 2, TextureUnshaded))
				{
					Log::E("Failed to build shaders!");
					return false;
				}

				if(!gfx->SetShader("Color"))
				{
					Log::E("Couldn't assign shader!");
					return false;
				}
				//and load the model
				Log::D("Loading model...");
				if(!gfx->InitGeometry(quad))
				{
					Log::E("Failed to load model!");
					return false;
				}

				//load up the texture
				ResGUID pngGUID(ARCHIVE_NAME, "Textures/testimg1024.png");
				texPtr = resMgr.GetResource(pngGUID);

				if(!texPtr)
				{
					Log::E("Couldn't load texture!");
					return false;
				}

				Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
				Log::D(String("Indicated size of tex (kB): ") + texRef.Width * texRef.Height * (U32)texRef.BitDepth / 1024);

				if(!gfx->InitTexture(texRef, TextureMeta::DIFFUSE))
				{
					Log::E("Couldn't init texture into OpenGL!");
					return false;
				}

				return true;
			}
			void Shutdown(Game* game)
			{
				game->Platform()->InitInput(game->Input());
				input.Shutdown();
				gfx->ShutdownMesh(quad);
				resMgr.Shutdown();
			}
			void updateCamPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				if(kbd->KeyDown(KEY_W))
				{
					camPos += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					camPos -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					camPos -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					camPos += Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_E))
				{
					camPos += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_C))
				{
					camPos -= Vector3::Up * elapsedSec;
				}
			}
			void Update(Game* game, const GameTime& time)
			{
				updateCamPos(time);
				view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, Vector3::Up);
				//view = Matrix4x4::BuildViewRH(camPos, Vector3::Forward, Vector3::Up);
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);
				gfx->SetWorldViewProjection(world, view, projection);
				gfx->SetShader("TextureUnshaded");
				Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
				gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
				gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
				gfx->Draw(quad);
			}
		};

		class FontTest : public TestBase
		{
		private:
			Matrix4x4 world, view, projection;
			ResourceManager resMgr;
			ResPtr fontPtr;
			Geometry quad;
			//InputManager input;
			TypedHandle<Keyboard> kbd;
			TranslateCamera* cam;
			Font font;
			Text text;
		public:
			FontTest(void) : font() {}
			~FontTest(void) {}
			bool Startup(Game* game)
			{
				if(!TestBase::Startup(game))
				{
					return false;
				}

				//game->Stats().SetShouldPrint(false);
				game->Stats().SetUpdateRate(1.0f / 2);

				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";

				//setup input system
				kbd = HandleMgr::RegisterPtr(&game->Input()->GetKeyboard(0));

				world = Matrix4x4::Identity;
				//setup camera
				cam = CustomNew<FreeLookCamera>(TEST_ALLOC, "CameraAlloc");
				cam->SetProjType(ProjType::PT_ORTHOGRAPHIC);
				cam->SetPosition(Vector3(0.5f, 0.5f, 0.5f));
				cam->SetUp(Vector3::Up);
				cam->SetForward(Vector3::Forward);
				//((LookAtCamera*)cam)->SetLookAtPos(Vector3::Zero);


				//try to create a triangle
				Vector3 pos[] = {	Vector3(0.0f, 0.0f, 0.0f),	//bottom left
									Vector3(1.0f, 0.0f, 0.0f),		//bottom right
									Vector3(0.0f, 1.0f, 0.0f),		//top left
									Vector3(1.0f, 1.0f, 0.0f) };	//top right

				Vector3 norm[] = {	Vector3(0.0f, 0.0f, 1.0f),  //bottom left
									Vector3(0.0f, 0.0f, 1.0f),  //top middle
									Vector3(0.0f, 0.0f, 1.0f),
									Vector3(0.0f, 0.0f, 1.0f)};

				Vector2 uv[] = {	Vector2(0.0f, 0.0f),	//bottom left
									Vector2(1.0f, 0.0f),	//bottom right
									Vector2(0.0f, 1.0f),	//top left
									Vector2(1.0f, 1.0f)};	//top right

				Color color[] = {	Color(1.0f, 0.0f, 0.0f),  //bottom left
									Color(0.0f, 1.0f, 0.0f),  //top middle
									Color(0.0f, 0.0f, 1.0f), //bottom right
									Color(1.0f, 0.0f, 1.0f)}; //bottom right

				U32 indices[] = {	0,
									1,  //bottom right
									2,  //top middle
									3,
									2,
									1}; //bottom left
				U32 verts = 4;
				U32 inds = 6;
				U8 uvs = 4;

				bool result = GeomHelpers::BuildGeometry(quad, pos, norm, color, uv, indices, verts, inds, uvs);
				if(!result)
				{
					Log::E("Failed to build model!");
					return false;
				}
				else
				{
					Log::D("Built model!");
				}
				Log::D(String("Model has ") + quad.VertexCount() + " vertices");
				//setup shaders

				Vector< ShaderFilePair > Basic, TextureUnshaded;
				Basic.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Text.vp").c_str())) );
				Basic.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Text.fp").c_str())) );

				if(	!gfx->MakeShader("Text", 2, Basic) )
				{
					Log::E("Failed to build shaders!");
					return false;
				}

				if(!gfx->SetShader("Text"))
				{
					Log::E("Couldn't assign shader!");
					return false;
				}
				//and load the model
				Log::D("Loading model...");
				if(!gfx->InitGeometry(quad))
				{
					Log::E("Failed to load model!");
					return false;
				}

				//generate the texture
				String fontName = "ubuntu-r.ttf";//"LiberationSans-Regular.ttf";
				ResGUID fontGUID(ARCHIVE_NAME, "Fonts/" + fontName);
				fontPtr = resMgr.GetResource(fontGUID);

				if(!fontPtr)
				{
					Log::E("Couldn't load texture!");
					return false;
				}

				result = font.GenerateFromResource(fontPtr, gfx, 32, 1024);
				if(!result)
				{
					Log::E("Couldn't build glyph atlas!");
					return false;
				}
				Log::D("Built glyph atlas");

				String longString = "Infernal noise! war seemed a civil game\nTo this uproar; horrid confusion heaped\nUpon confusion rose. And now all Heaven\nHad gone to wrack, with ruin overspread,\nHad not the Almighty Father, where he sits\nShrined in his sanctuary of Heaven secure,\nConsulting on the sum of things, foreseen\nThis tumult, and permitted all, advised,\nThat his great purpose he might so fulfil,\nTo honour his Anointed Son, avenged\nUpon his enemies, and to declare\nAll power on him transferred.";

				text.SetFont(font);
				text.SetString(longString);

				return true;
			}
			void Shutdown(Game* game)
			{
				game->Platform()->InitInput(game->Input());
				gfx->ShutdownMesh(quad);
				resMgr.Shutdown();
			}
			void updateCamPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 camVel = Vector3::Zero;
				if(kbd->KeyDown(KEY_W))
				{
					camVel += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					camVel -= Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					camVel -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					camVel += Vector3::Right * elapsedSec;
				}
				cam->Translate(camVel);
			}
			void updateText(Game* game)
			{
				text.SetString(String("FPS: ") + (I32)(game->Stats().GetFPSCounter().FPS()));
			}
			void Update(Game* game, const GameTime& time)
			{
				updateCamPos(time);
				updateText(game);
				//view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, Vector3::Up);
				//view = Matrix4x4::BuildViewRH(camPos, Vector3::Forward, Vector3::Up);
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);
				gfx->SetWorldViewProjection(world, cam->GetViewMatrix(), cam->GetProjMatrix());
				/*const GlyphInfo& inf = font.GetGlyphInfo(text.GetString()[0]);
				const Rectangle& tC = inf.TexCoords;
				const Vector2 tCCenter = tC.Center();
				Vector3 glyphCenter = Vector3(	tCCenter.X(),
												tCCenter.Y(),
												0.0f);
				gfx->DebugDrawBox(	glyphCenter, Quaternion::Identity, 
					(tC.Height() / 2.0f) * 1.0f, (tC.Width() / 2.0f) * 1.0f, 2.0f, Colors::Green);
					*/
				gfx->SetShader("Text");
				gfx->SetTexture(font.GetTextureHandle(), TextureMeta::DIFFUSE);
				gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
				//draw a debug box in the bounds of the character's texture box.
				//gfx->Draw(quad);
				gfx->Draw(text);
			}
		};

		class MemTest : public TestBase
		{
		public:
			MemTest(void) {}
			~MemTest(void) {}
			bool Startup(Game* game) { return true; }
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time)
			{
				char* bigAlloc1 = CustomArrayNew<char>(32768, TEST_ALLOC, "TestAlloc");
				char* bigAlloc2 = CustomArrayNew<char>(40000, TEST_ALLOC, "TestAlloc");
				CustomArrayDelete(bigAlloc1);
				CustomArrayDelete(bigAlloc2);
			}
		};

		class DemoTest : public TestBase
		{
		private:
			PhysicsWorld physics;
			DebugDrawer* debugDrawer;
			Matrix4x4 world;
			F32 worldArray[16];

			//physics members
			BoxCollShape* boxCollider;
			BoxCollShape* floorCollider;
			SphereCollShape* ballCollider;
			CompoundCollShape* colliderHolder;
			CompoundCollShape* floorColliderHolder;
			CompoundCollShape* ballColliderHolder;
			RigidBody* playerBody;
			RigidBody* floorBody;
			RigidBody* ballBody;
			BoxCollShape* bulletCollider;

			//player properties
			F32 maxVel;
			F32 moveForce;
			F32 rotForce;
			F32 playerMass;

			F32 gPadSensitivity;

			//bullet properties
			F32 bulletForce;
			F32 bulletMass;

			//camera members
			FreeLookCamera* camera;

			//InputManager input;
			TypedHandle<Keyboard> kbd;
			TypedHandle<Controller> ctrlr;

			bool paused;
			void initInput(Game* game)
			{
				/*
				input.Startup(game->Platform());
				TypedHandle<InputManager> inputHnd = HandleMgr::FindHandle(&input);
				if(!inputHnd.GetHandle())
				{
					inputHnd = HandleMgr::RegisterPtr(&input);
				}
				//link input manager w/ platform
				IPlatform* plat = game->Platform();
				plat->InitInput(game->Input());
				//and tell platform we're ready for input
				*/
				kbd = HandleMgr::RegisterPtr(&game->Input()->GetKeyboard(0));
				ctrlr = HandleMgr::RegisterPtr(&game->Input()->GetController(0));
			}
			void initPhysics(Game* game)
			{
				physics.Startup();
				Log::D("Physics world initialized!");

				//then setup debug physics renderer
				debugDrawer = CustomNew<DebugDrawer>(TEST_ALLOC, "TestAlloc", game->GfxWrapper());
				debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
				physics.AttachDebugDrawer(debugDrawer);
				Log::D("Attached debug drawer to physics world");

				//then spawn a box
				boxCollider = CustomNew<BoxCollShape>(PHYS_ALLOC, "RBAlloc", 1.0f, 0.5f, 1.0f);
				colliderHolder = CustomNew<CompoundCollShape>(PHYS_ALLOC, "RBAlloc");
				colliderHolder->AddShape(boxCollider, Vector3::Zero, Quaternion::Identity);
				playerBody = CustomNew<RigidBody>(PHYS_ALLOC, "RBAlloc", playerMass, Vector3::Zero, Quaternion::Identity, *colliderHolder);
				//now attach the box to the physics world!
				physics.AttachRigidBody(playerBody);
				Log::D("Attached box to physics world");

				//setup gravity
				Vector3 grav = Vector3(0.0f, 0.0f, 0.0f);
				physics.SetGravityVector(grav);

				//spawn a floor, too
				floorCollider = CustomNew<BoxCollShape>(PHYS_ALLOC, "RBAlloc", 50.0f, 20.0f, 50.0f);
				floorColliderHolder = CustomNew<CompoundCollShape>(PHYS_ALLOC, "RBAlloc");
				floorColliderHolder->AddShape(floorCollider, Vector3::Zero, Quaternion::Identity);
				floorBody = CustomNew<RigidBody>(PHYS_ALLOC, "RBAlloc", 0.0f, Vector3(0, -50, 0), Quaternion::Identity, *floorColliderHolder);
				physics.AttachRigidBody(floorBody);

				//also make a ball
				ballCollider = CustomNew<SphereCollShape>(PHYS_ALLOC, "RBAlloc", 2.0f);
				ballColliderHolder = CustomNew<CompoundCollShape>(PHYS_ALLOC, "RBAlloc");
				ballColliderHolder->AddShape(ballCollider, Vector3::Zero, Quaternion::Identity);
				ballBody = CustomNew<RigidBody>(PHYS_ALLOC, "RBAlloc", 0.0f, Vector3(10, 0, 0), Quaternion::Identity, *ballColliderHolder);
				physics.AttachRigidBody(ballBody);

				//and the general bullet collision body
				bulletCollider = CustomNew<BoxCollShape>(PHYS_ALLOC, "RBAlloc", 0.25f, 0.25f, 0.25f);
			}
			void initPlayerActor()
			{
				maxVel = 100.0f;
				moveForce = 1000.0f;
				rotForce = 200.0f;
				playerMass = 1.0f;

				gPadSensitivity = 1.0f;

				bulletForce = 2000.0f;
				bulletMass = 0.05f;
			}
			void initCamera()
			{
				camera = CustomNew<FreeLookCamera>(TEST_ALLOC, "TestAlloc");
				camera->SetAspectRatio(gfx->ScreenAspect());
				camera->SetFOV(Math::PI_OVER_2);
				camera->SetNearDist(0.25f);
				camera->SetFarDist(2000.0f);
				camera->SetPosition(Vector3(0.0f, 0.0f, -3.0f));
				//keep the camera on the player
				updateCamera();
			}
			void updatePlayerPos(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				F32 impulse = moveForce * elapsedSec;
				Vector3 force = Vector3::Zero;
				if(kbd->KeyDown(KEY_W))
				{
					force += playerBody->Forward() * impulse;
				}
				if(kbd->KeyDown(KEY_S))
				{
					force -= playerBody->Forward() * impulse;
				}
				if(kbd->KeyDown(KEY_A))
				{
					force -= playerBody->Right() * impulse;
				}
				if(kbd->KeyDown(KEY_D))
				{
					force += playerBody->Right() * impulse;
				}
				if(kbd->KeyDown(KEY_E))
				{
					force += playerBody->Up() * impulse;
				}
				if(kbd->KeyDown(KEY_C))
				{
					force -= playerBody->Up() * impulse;
				}
				Vector2 ctrlrStrafe = ctrlr->GetAxisPairFiltered(1, 0, ZoneType::RADIUS) * impulse * gPadSensitivity;
				F32 ctrlrThrust = ctrlr->GetAxis(4) * impulse * gPadSensitivity;
				//force += playerBody->Right() * 
				force +=	(playerBody->Right() * ctrlrStrafe.X()) +
							(playerBody->Up() * -ctrlrStrafe.Y()) +
							(playerBody->Forward() * -ctrlrThrust);
				//force = impulse * force.GetNormalized();
				playerBody->ApplyForce(force);
			}
			void updatePlayerRot(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				F32 impulse = rotForce * elapsedSec;
				Vector3 rotVec = Vector3::Zero;
				//tilt
				if(kbd->KeyDown(KEY_I))
				{
					rotVec.SetX(rotVec.X() + (impulse));
				}
				if(kbd->KeyDown(KEY_K))
				{
					rotVec.SetX(rotVec.X() - (impulse));
				}
				//pan
				if(kbd->KeyDown(KEY_J))
				{
					rotVec.SetY(rotVec.Y() + (impulse));
				}
				if(kbd->KeyDown(KEY_L))
				{
					rotVec.SetY(rotVec.Y() - (impulse));
				}
				//roll
				if(kbd->KeyDown(KEY_U))
				{
					rotVec.SetZ(rotVec.Z() + (impulse));
				}
				if(kbd->KeyDown(KEY_M))
				{
					rotVec.SetZ(rotVec.Z() - (impulse));
				}
				Vector2 ctrlrVec = ctrlr->GetAxisPairFiltered(3, 2, ZoneType::RADIUS);
				rotVec += Vector3(	ctrlrVec.Y() * impulse * gPadSensitivity,
									-ctrlrVec.X() * impulse * gPadSensitivity,
									0.0f);
				if(ctrlr->ButtonDown(4))
				{
					rotVec.SetZ(rotVec.Z() + impulse * 0.5f);
				}
				if(ctrlr->ButtonDown(5))
				{
					rotVec.SetZ(rotVec.Z() - impulse * 0.5f);
				}
				//also update mouse tilt and pan, if desired
				/*
				if(enableMouseLook)
				{
					Vector2 mouseDelta = mouse->GetMousePos() * (rotForce * elapsedSec * mouseSensitivity);
					//remember that the mouse delta is linear and rotvec is rotational!
					rotVec.SetX(rotVec.X() + mouseDelta.Y());
					rotVec.SetY(rotVec.Y() + mouseDelta.X());
				}
				*/
				//rotVec = impulse * rotVec.GetNormalized();
				playerBody->ApplyTorque(rotVec);
				//camera->RotateEuler(rotVec.X(), rotVec.Y(), rotVec.Z());
			}
			void updatePlayerInput(const GameTime& time)
			{
				updatePlayerPos(time);
				updatePlayerRot(time);
				//player's moved, keep camera following
				updateCamera();
				//also handle shooting
				if(kbd->KeyDown(KEY_SPACE) || ctrlr->ButtonPressed(0))
				{
					//make a rigidbody for the new bullet
					RigidBody* nextBullet = CustomNew<RigidBody>(PHYS_ALLOC, "RBAlloc", 
						bulletMass, playerBody->Position() + (3.0f * playerBody->Forward()), playerBody->Rotation(), *bulletCollider);
					//insert it into the world
					physics.AttachRigidBody(nextBullet);
					//and apply the desired impulse
					nextBullet->ApplyForce(bulletForce * playerBody->Forward());
				}
			}
			void updateCamera()
			{
				//camera should follow player
				
				camera->SetPosition(playerBody->Position() - 3.0f * playerBody->Forward() + 0.5f * playerBody->Up());
				camera->SetUp(playerBody->Up());
				camera->SetForward(playerBody->Forward());
				
				//camera->SetLookAtPos(playerBody->Position());
			}
		public:
			DemoTest(void)
			{
				paused = false;
			}
			~DemoTest(void) {}
			bool Startup(Game* game)
			{
				game->Stats().SetShouldPrint(false);
				
				TestBase::Startup(game);
				//setup input
				initInput(game);
				//setup player properties
				initPlayerActor();
				initPhysics(game);
				initCamera();

				world = Matrix4x4::Identity;//Matrix4x4::BuildTranslation(boxPosition);

				world.CopyToBuffer(worldArray);
				return true;
			}
			void Shutdown(Game* game)
			{
			}
			void Update(Game* game, const GameTime& time) 
			{
				if(kbd->KeyPressed(KEY_SPACE))
				{
					//paused = !paused;
				}
				if(!paused)
				{
					updatePlayerInput(time);
					{
							PROFILE("Physics");
							physics.Update(time);
					}
				}
			}
			void Draw(Game* game, const GameTime& time)
			{
				game->GfxWrapper()->Clear(Colors::Gray30Pct);
				if(!game->GfxWrapper()->SetWorldViewProjection(world, camera->GetViewMatrix(), camera->GetProjMatrix()))
				{
					Log::D("Couldn't set WVP matrices!");
				}
				{
					PROFILE("Debug Render");
					physics.DebugDraw();
				}
			}
		};

		class TransformTest : public TestBase
		{
			//General idea - have a bunch of models
			//hiearchially attached, at least 2 models.

			/*
			WASD/EC move camera.
			TFGH/RY move parent, VBN rotate parent along X/Y/Z axis.
			IJKL/UO move child, M,comma,period rotate child along X/Y/Z axis.
			*/

		private:
			//parent model has no parent,
			//so its local transform is its world transform too
			Transform parentT;
			Transform childLocT;
			Transform childWorldT;
			
			String modelName;
			ResourceManager resMgr;
			ResPtr defaultTexPtr;
			ResPtr modelPtr;

			LookAtCamera camera;

			InputManager input;
			TypedHandle<Keyboard> kbd;

			Vector3 lightPos, lightColor;

			void initCamera()
			{
				camera.SetPosition(Vector3(0, 2, 5));
				camera.SetLookAtPos(Vector3::Zero);
			}
			void initInput(Game* game)
			{
				input.Startup(game->Platform());
				TypedHandle<InputManager> inputHnd = HandleMgr::FindHandle(&input);
				if(!inputHnd.GetHandle())
				{
					inputHnd = HandleMgr::RegisterPtr(&input);
				}
				//link input manager w/ platform
				IPlatform* plat = game->Platform();
				plat->InitInput(game->Input());
				//and tell platform we're ready for input
				kbd = HandleMgr::RegisterPtr(&input.GetKeyboard(0));
			}
			void initMatrices(Game* game)
			{
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());

				lightPos = Vector3(-10.0f, 10.0f, 10.0f);
				lightColor = Vector3(1.0f, 1.0f, 1.0f);
			}
			bool initShaders()
			{
				Vector< ShaderFilePair > pathsColor, pathsDiffuse, pathsDiffuseTextured;
				pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.vp").c_str())) );
				pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.fp").c_str())) );

				pathsDiffuse.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.vp").c_str())) );
				pathsDiffuse.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.fp").c_str())) );

				pathsDiffuseTextured.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.vp").c_str())) );
				pathsDiffuseTextured.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.fp").c_str())) );

				return	gfx->MakeShader("Color", 2, pathsColor) &&
						gfx->MakeShader("Diffuse", 2, pathsDiffuse) &&
						gfx->MakeShader("DiffuseTextured", 2, pathsDiffuseTextured);
			}
			bool loadModelRes(Game* game, const char* archName)
			{
				//remember to add the importer, stupid
				//resMgr.RegisterLoader(GetSharedPtr(CustomNew<ModelLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				ResGUID lmdlGUID(archName, "Models/" + modelName + ".lmdl");
				Log::D(String("Loading .lmdl file ") + lmdlGUID.Name);
				time->Tick();
				modelPtr = resMgr.GetResource(lmdlGUID);
				time->Tick();
				if(!modelPtr)
				{
					Log::E(String("Failed to load ") + lmdlGUID.Name + "!");
					return false;
				}
				Log::D(String("Loaded .lmdl in ") + time->ElapsedGameTime().ToSeconds() + "s");
				return true;
			}
			bool initModelIntoRenderer(Model& model)
			{
				//Steps after this will delete geometry data;
				//recalculate bounds if needed before that data is lost.
				//if(!model.HasBounds())
				//{
					model.RecalcBounds();
				//}
				for(U32 i = 0; i < model.MeshCount(); ++i)
				{
					if(!gfx->InitGeometry(model.GetMesh(i)->GetGeometry()))
					{
						Log::E("Couldn't load model into OpenGL!");
						return false;
					}
					//we can get rid of geometry data now
					const Geometry& geom = model.GetMesh(i)->GetGeometry();
					//HandleMgr::DeleteHandle(geom.VertexHandle());
					//HandleMgr::DeleteHandle(geom.IndexHandle());

					ResPtr diffTex = resMgr.GetResource(model.GetMesh(i)->GetMaterial().DiffuseTexGUID);
					Log::D(String("Opening texture resource ") + model.GetMesh(i)->GetMaterial().DiffuseTexGUID.Name);
					if(!diffTex)
					{
						Log::E("Couldn't get mesh texture resource!");
						return false;
					} 
					Texture2D& diffTexRef = *(Texture2D*)diffTex->Buffer();
					if(!diffTexRef.TextureBufferHandle)
					{
						if(!gfx->InitTexture(diffTexRef, TextureMeta::DIFFUSE))
						{
							Log::E("Couldn't init mesh texture!");
							return false;
						}
					}
				}
				return true;
			}
			void initTransforms()
			{
				parentT = Transform::Identity;
				childLocT = Transform::Identity;
				childLocT.SetPosition(Vector3(1.5f,1.5f,1.5f));
				childWorldT = Transform::Identity;
			}
			void drawModel(const Model& modelToDraw)
			{
				Texture2D& defTexRef = *(Texture2D*)defaultTexPtr->Buffer();

				for(U32 i = 0; i < modelToDraw.MeshCount(); ++i)
				{
					const Mesh& mesh = *modelToDraw.GetMesh(i);
					const Material& mat = mesh.GetMaterial();
					const Geometry& geom = mesh.GetGeometry();
					//need to setup mesh uniforms;
					//since system doesn't use materials yet,
					//that's just the texture
					//use a default material if there's no texture specified
					ResPtr texPtr = resMgr.GetResource(mat.DiffuseTexGUID);
					const Texture2D& texRef = *(Texture2D*)texPtr->Buffer();
					gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
					/*if(!texPtr)
					{
						gfx->SetTexture(defTexRef, TextureMeta::DIFFUSE);
					}
					else
					{
						const Texture2D& texRef = *(Texture2D*)texPtr->Buffer();
						gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
					}*/

					gfx->Draw(geom);
				}
			}
			void updateCamPos(const GameTime& time)
			{
				Vector3 camPos = Vector3::Zero;
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				if(kbd->KeyDown(KEY_W))
				{
					camPos += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					camPos -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					camPos -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					camPos += Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_Q))
				{
					camPos += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_E))
				{
					camPos -= Vector3::Up * elapsedSec;
				}
				camera.Translate(camPos);
			}
			void updateParentTrans(const GameTime& time)
			{
				Vector3 trans = Vector3::Zero;
				Vector3 rotation = Vector3::Zero;
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				//Horizontal movement...
				if(kbd->KeyDown(KEY_T))
				{
					trans += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_G))
				{
					trans -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_F))
				{
					trans -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_H))
				{
					trans += Vector3::Right * elapsedSec;
				}

				//Vertical
				if(kbd->KeyDown(KEY_R))
				{
					trans += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_Y))
				{
					trans -= Vector3::Up * elapsedSec;
				}

				//Rotation
				if(kbd->KeyDown(KEY_V))
				{
					rotation += Vector3(1,0,0) * elapsedSec;
				}
				if(kbd->KeyDown(KEY_B))
				{
					rotation += Vector3(0,1,0) * elapsedSec;
				}
				if(kbd->KeyDown(KEY_N))
				{
					rotation += Vector3(0,0,1) * elapsedSec;
				}
				parentT.Translate(trans);
				parentT.Rotate(Quaternion::FromEulerAngles(rotation));
			}
			void updateChildTrans(const GameTime& time)
			{
				Vector3 trans = Vector3::Zero;
				Vector3 rotation = Vector3::Zero;
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				//Horizontal movement...
				if(kbd->KeyDown(KEY_I))
				{
					trans += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_K))
				{
					trans -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_J))
				{
					trans -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_L))
				{
					trans += Vector3::Right * elapsedSec;
				}

				//Vertical
				if(kbd->KeyDown(KEY_U))
				{
					trans += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_O))
				{
					trans -= Vector3::Up * elapsedSec;
				}

				//Rotation
				if(kbd->KeyDown(KEY_M))
				{
					rotation += Vector3(1,0,0) * elapsedSec;
				}
				if(kbd->KeyDown(KEY_COMMA))
				{
					rotation += Vector3(0,1,0) * elapsedSec;
				}
				if(kbd->KeyDown(KEY_PERIOD))
				{
					rotation += Vector3(0,0,1) * elapsedSec;
				}
				childLocT.Translate(trans);
				childLocT.Rotate(Quaternion::FromEulerAngles(rotation));
			}
			void updateTransforms()
			{
				//update parent input
				//update child input

				//and recalculate child world trans
				childWorldT = childLocT * parentT;
			}
		public:
			TransformTest(void)
			{
				defaultTexPtr = 0;
				modelPtr = 0;
				camera = LookAtCamera();
				initTransforms();
			}
			~TransformTest(void) {}
			bool Startup(Game* game)
			{
				if(!TestBase::Startup(game))
				{
					return false;
				}
				//game->Stats().SetShouldPrint(false);

				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				//register texture importers...
				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";
				modelName = "spaceship";

				//load up default texture
				ResGUID defTexGUID(ARCHIVE_NAME, "Textures/defaultTex.png"); 
				defaultTexPtr = resMgr.GetResource(defTexGUID);
				if(!defaultTexPtr)
				{
					Log::E("Couldn't load default texture!");
					return false;
				}
				if(!gfx->InitTexture(*(Texture2D*)defaultTexPtr->Buffer(), TextureMeta::DIFFUSE))
				{
					Log::E("Couldn't load default texture into OpenGL!");
					return false;
				}

				//setup input system
				initInput(game);
				initMatrices(game);

				//setup shaders
				if(!initShaders())
				{
					Log::E("Failed to build shaders!");
					return false;
				}

				initCamera();

				//try loading that model now
				//and try loading a model resource
				Log::SetVerbosity(Log::VERB);
				if(!loadModelRes(game, ARCHIVE_NAME))
				{
					return false;
				}
				Model& resModel = *(Model*)modelPtr->Buffer();
				if(!initModelIntoRenderer(resModel))
				{
					return false;
				}
				
				Log::SetVerbosity(Log::DEBUG);

				return true;
			}
			void Shutdown(Game* game)
			{
				game->Platform()->InitInput(game->Input());
				input.Shutdown();
				resMgr.Shutdown();
			}
			void Update(Game* game, const GameTime& time)
			{
				updateCamPos(time);
				updateParentTrans(time);
				updateChildTrans(time);
				updateTransforms();
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);

				gfx->SetWorldViewProjection(Matrix4x4::Identity, camera.GetViewMatrix(), camera.GetProjMatrix());
				//next, draw the light's position.
				gfx->DebugDrawSphere(lightPos, 0.5f, Color(lightColor));

				//retrieve the model from the loaded resource
				Model& resModel = *(Model*)modelPtr->Buffer();
				//draw the model's bounds.
				//finally, setup for the actual model...
				gfx->SetShader("DiffuseTextured");
				gfx->SetVec3Uniform("lightDiffuse", lightColor);
				gfx->SetVec3Uniform("lightPos", lightPos);
				//Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
				//gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
				gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
				//and draw the parent and child.
				gfx->SetWorld(parentT.ToMatrix());
				drawModel(resModel);

				gfx->SetWorld(childWorldT.ToMatrix());
				drawModel(resModel);
			}
		};

		class QuaternionTest : public TestBase
		{
		private:

		public:
			bool Startup(Game* game)
			{
				game->Stats().SetShouldPrint(false);

				using namespace LeEK;
				//test length and distance w/ unit vectors
				LogD("Testing quaternion construction:\n---");
				Vector3 v = Vector3::Right;
				Matrix4x4 mRot45X = Matrix4x4::BuildXRotation(45.0f*Math::DEG_TO_RAD);
				Matrix4x4 mRot45Y = Matrix4x4::BuildYRotation(45.0f*Math::DEG_TO_RAD);
				Matrix4x4 mRot45Z = Matrix4x4::BuildZRotation(45.0f*Math::DEG_TO_RAD);

				Matrix4x4 mRot45XYZ = Matrix4x4::FromEulerAngles(45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD);

				Quaternion qRot45X = Quaternion::BuildXRotation(45.0f*Math::DEG_TO_RAD);
				Quaternion qRot45Y = Quaternion::BuildYRotation(45.0f*Math::DEG_TO_RAD);
				Quaternion qRot45Z = Quaternion::BuildZRotation(45.0f*Math::DEG_TO_RAD);
				using namespace std;
				LogD("qRot45X = " + qRot45X.ToString());
				LogD("qRot45Y = " + qRot45Y.ToString());
				LogD("qRot45Z = " + qRot45Z.ToString());

				LogD("building XYZ quaternion");
				Quaternion qRot45XYZ = Quaternion::FromEulerAngles(45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD);//qRot45X * qRot45Y * qRot45Z;
				LogD("qRot45X = " + qRot45X.ToString());
				LogD("qRot45Y = " + qRot45Y.ToString());
				LogD("qRot45Z = " + qRot45Z.ToString());
				LogD("building ZYX quaternion");
				Quaternion qRot45ZYX = qRot45Z * qRot45Y * qRot45X;
				LogD("qRot45X = " + qRot45X.ToString());
				LogD("qRot45Y = " + qRot45Y.ToString());
				LogD("qRot45Z = " + qRot45Z.ToString());
				Quaternion qRot45XZY = qRot45X * qRot45Z * qRot45Y;
				Quaternion qRot45YXZ = qRot45Y * qRot45X * qRot45Z;
				Quaternion qRot45YZX = qRot45Y * qRot45Z * qRot45X;
				Quaternion qRot45ZXY = qRot45Z * qRot45X * qRot45Y;

				LogD("\nTesting identity and inverse quaternion:\n---");
				Quaternion qRot45XYZInverse = qRot45XYZ.GetInverse();
				LogD("qRot45XYZ = " + qRot45XYZ.ToString());
				LogD("inverse(qRot45XYZ) = " + qRot45XYZInverse.ToString());
				LogD("qRot45XYZ * identity = " + (qRot45XYZ * Quaternion::Identity).ToString());
				LogD("qRot45XYZ * inverse = " + (qRot45XYZ * qRot45XYZInverse).ToString());

				LogD("\nComparing matrix and vector rotation:\n---");
				LogD("v = " + v.ToString());
				LogD("v*mRot45X = " + mRot45X.MultiplyPoint(v).ToString());
				LogD("v*qRot45X = " + qRot45X.RotateVector(v).ToString());
				LogD("v*mRot45Y = " + mRot45Y.MultiplyPoint(v).ToString());
				LogD("v*qRot45Y = " + qRot45Y.RotateVector(v).ToString());
				LogD("v*mRot45Z = " + mRot45Z.MultiplyPoint(v).ToString());
				LogD("v*qRot45X = " + qRot45Z.RotateVector(v).ToString());
				LogD("v*mRot45XYZ = " + mRot45XYZ.MultiplyPoint(v).ToString());
				LogD("v*mFromQuat(qRot45XYZ) = " + qRot45XYZ.ToMatrix().MultiplyPoint(v).ToString());
				LogD("v*qRot45XYZ = " + qRot45XYZ.RotateVector(v).ToString());
				LogD("v*qRot45ZYX = " + qRot45ZYX.RotateVector(v).ToString());
				LogD("v*qRot45X*qRot45Y*qRot45Z = " + qRot45Z.RotateVector(qRot45Y.RotateVector(qRot45X.RotateVector(v))).ToString());
				LogD("v*qFromMat(mRot45XYZ) = " + Quaternion::FromMatrix(mRot45XYZ).RotateVector(v).ToString()); //Doesn't work! TODO: Fix Quaternion::FromMatrix()

				LogD("v = " + v.ToString());

				LogD("mRot45Y = " + mRot45Y.ToString());
				LogD("mat(qRot45Y) = " + qRot45Y.ToMatrix().ToString());
				LogD("mRot45XYZ = " + mRot45XYZ.ToString());
				LogD("mat(qRot45XYZ) = " + qRot45XYZ.ToMatrix().ToString());
				LogD("mat(qRot45ZYX) = " + qRot45ZYX.ToMatrix().ToString());

				//test half angles
				LogD("\nTesting half angles:\n---");
				F32 qCos = 2.0f * pow(Math::Cos(45.0f*Math::DEG_TO_RAD / 2.0f), 2) - 1.0f;
				LogD(String("qCos = ") + qCos);
				F32 mCos = Math::Cos(45.0f*Math::DEG_TO_RAD);
				LogD(String("mCos = ") + mCos);
				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time)	{}
		};

		class ScriptTest : public TestBase
		{
		public:
			ScriptTest(void) {}
			~ScriptTest(void) {}
			static void test()
			{
				Log::D("Called from Lua!");
			}
			bool Startup(Game* game)
			{ 
				LuaInterface lua;
				using namespace luabridge;
				getGlobalNamespace(lua.LuaState())
					.addFunction("test", test);
				//LuaRegisterFunction(lua, "test", test);
				String scriptPath = String(game->Platform()->FindFullProgPath()) + "/TestContent/Scripts/test.lua";
				Log::D("Opening " + scriptPath);
				lua.ExecuteFile(scriptPath.c_str());
				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		class ModelToolFileTest : public TestBase
		{
		protected:
			Matrix4x4 world, view, projection;
			char* geomDataBuf;
			ResPtr texPtr;
			ResPtr defaultTexPtr;
			String modelName;
			U32 numMeshes;
			InputManager input;
			ResourceManager resMgr;
			TypedHandle<Keyboard> kbd;
			Vector3 camPos;
			Vector3 camLookAt;
			Vector3 camUp;
			Vector3 lightPos;
			Vector3 lightColor;
			Color AABBBndColor;
			Color SphereBndColor;

			Model model;
			void initInput(Game* game)
			{
				input.Startup(game->Platform());
				TypedHandle<InputManager> inputHnd = HandleMgr::FindHandle(&input);
				if(!inputHnd.GetHandle())
				{
					inputHnd = HandleMgr::RegisterPtr(&input);
				}
				//link input manager w/ platform
				IPlatform* plat = game->Platform();
				plat->InitInput(inputHnd);
				//and tell platform we're ready for input
				kbd = HandleMgr::RegisterPtr(&input.GetKeyboard(0));
			}
			void initMatrices(Game* game)
			{
				world = Matrix4x4::Identity;
				//projection is the basic defaults
				Log::D(String("Aspect Ratio: ") + gfx->ScreenAspect());
				projection = Matrix4x4::BuildPerspectiveRH(gfx->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
				//we don't have a camera class yet; ordinarily it would handle this
				camPos = Vector3(3.0f, 3.0f, 3.0f);
				camUp = Vector3::Up;
				lightPos = Vector3(1.0f, 1.0f, 1.0f);
				lightColor = Vector3(1.0f, 1.0f, 1.0f);
				camLookAt = Vector3(0.0f, 0.0f, 0.0f);
				//view = Matrix4x4::BuildViewRH(camPos, Vector3::Forward, Vector3::Up);
				view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, camUp);
			}
			bool initShaders()
			{
				Vector< ShaderFilePair > pathsColor, pathsDiffuse, pathsDiffuseTextured;
				pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.vp").c_str())) );
				pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.fp").c_str())) );

				pathsDiffuse.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.vp").c_str())) );
				pathsDiffuse.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.fp").c_str())) );

				pathsDiffuseTextured.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.vp").c_str())) );
				pathsDiffuseTextured.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.fp").c_str())) );

				return	gfx->MakeShader("Color", 2, pathsColor) &&
						gfx->MakeShader("Diffuse", 2, pathsDiffuse) &&
						gfx->MakeShader("DiffuseTextured", 2, pathsDiffuseTextured);
			}
			bool loadModelFile(String fileName)
			{
				String filePath = String(Filesystem::GetProgDir()) + fileName;
				DataStream* file = Filesystem::OpenFile(filePath);
				//try to read the stream in
				char* mdlData = file->ReadAll();
				size_t geomDataSize = ModelMgr::FindFileGeomSize(mdlData);
				if(geomDataSize < 1)
				{
					return false;
				}
				geomDataBuf = CustomArrayNew<char>(geomDataSize, AllocType::MESH_ALLOC, "MeshAlloc");
				bool result = ModelMgr::ReadModelMemory(model, mdlData, file->FileSize(), geomDataBuf, geomDataSize);
				CustomArrayDelete(mdlData);
				Filesystem::CloseFile(file);
				return result;
			}
			bool initModelIntoRenderer(Model& model)
			{
				//Steps after this will delete geometry data;
				//recalculate bounds if needed before that data is lost.
				//if(!model.HasBounds())
				//{
					//model.RecalcBounds();
				//}
				for(U32 i = 0; i < model.MeshCount(); ++i)
				{
					if(!gfx->InitGeometry(model.GetMesh(i)->GetGeometry()))
					{
						Log::E("Couldn't load model into OpenGL!");
						return false;
					}
					//we can get rid of geometry data now
					const Geometry& geom = model.GetMesh(i)->GetGeometry();
					//HandleMgr::DeleteHandle(geom.VertexHandle());
					//HandleMgr::DeleteHandle(geom.IndexHandle());

					//the tool doesn't write GUIDs,
					//so replace w/ default textures.
					//Default's already initiaized - just set it.
					Material& mat = model.GetMesh(i)->GetMaterial();
					mat.DiffuseTexGUID = defaultTexPtr->GUID();
				}
				return true;
			}
			void updateCamPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				if(kbd->KeyDown(KEY_W))
				{
					camPos += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					camPos -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					camPos -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					camPos += Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_E))
				{
					camPos += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_C))
				{
					camPos -= Vector3::Up * elapsedSec;
				}
			}
			void updateLightPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				if(kbd->KeyDown(KEY_I))
				{
					lightPos += Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_K))
				{
					lightPos -= Vector3::Forward * elapsedSec;
				}
				if(kbd->KeyDown(KEY_J))
				{
					lightPos -= Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_L))
				{
					lightPos += Vector3::Right * elapsedSec;
				}
				if(kbd->KeyDown(KEY_U))
				{
					lightPos += Vector3::Up * elapsedSec;
				}
				if(kbd->KeyDown(KEY_M))
				{
					lightPos -= Vector3::Up * elapsedSec;
				}
			}
			void drawNormals(const Geometry& geom, float normLen, const Color& color)
			{
				const Vertex* vertArray = geom.Vertices();
				for(U32 i = 0; i < geom.VertexCount(); ++i)
				{
					const Vertex& vert = vertArray[i];
					gfx->DebugDrawLine(vert.Position, vert.Position + (normLen* vert.Normal), color);
				}
			}
			void drawModel(const Model& modelToDraw)
			{
				Texture2D& defTexRef = *(Texture2D*)defaultTexPtr->Buffer();

				for(U32 i = 0; i < modelToDraw.MeshCount(); ++i)
				{
					const Mesh& mesh = *modelToDraw.GetMesh(i);
					const Material& mat = mesh.GetMaterial();
					const Geometry& geom = mesh.GetGeometry();
					//need to setup mesh uniforms;
					//since system doesn't use materials yet,
					//that's just the texture
					//use a default material if there's no texture specified
					ResPtr texPtr = resMgr.GetResource(mat.DiffuseTexGUID);
					const Texture2D& texRef = *(Texture2D*)texPtr->Buffer();
					gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
					/*if(!texPtr)
					{
						gfx->SetTexture(defTexRef, TextureMeta::DIFFUSE);
					}
					else
					{
						const Texture2D& texRef = *(Texture2D*)texPtr->Buffer();
						gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
					}*/

					gfx->Draw(geom);
				}
			}
			void drawBounds(const Model& modelToUse)
			{
				//Bounds center is in model space,
				//transform by world matrix
				Vector3 modelPos = world.MultiplyPoint(modelToUse.BoundsCenter());
				gfx->DebugDrawAABB(	modelPos,
									modelToUse.AABBHalfBounds(),
									AABBBndColor);

				gfx->DebugDrawSphere(	modelPos,
										modelToUse.BoundingRadius(),
										SphereBndColor);
			}
			void shutdownModel()
			{
				CustomArrayDelete(geomDataBuf);
			}
		public:
			ModelToolFileTest(void) 
			{
				AABBBndColor = Colors::Green;
				SphereBndColor = Colors::LtGreen;
			}
			~ModelToolFileTest(void) {}
			
			bool Startup(Game* game)
			{
				if(!TestBase::Startup(game))
				{
					return false;
				}
				//game->Stats().SetShouldPrint(false);

				//init resource manager
				if(!resMgr.Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				//register texture importers...
				resMgr.RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				const char* ARCHIVE_NAME = "/TestContent/Archives/TestContent.zip";

				//load up default texture
				ResGUID defTexGUID(ARCHIVE_NAME, "Textures/defaultTex.png"); 
				defaultTexPtr = resMgr.GetResource(defTexGUID);
				if(!defaultTexPtr)
				{
					Log::E("Couldn't load default texture!");
					return false;
				}
				if(!gfx->InitTexture(*(Texture2D*)defaultTexPtr->Buffer(), TextureMeta::DIFFUSE))
				{
					Log::E("Couldn't load default texture into OpenGL!");
					return false;
				}

				//setup input system
				initInput(game);
				initMatrices(game);

				//setup shaders
				if(!initShaders())
				{
					Log::E("Failed to build shaders!");
					return false;
				}
				
				//try loading that model now
				modelName = "SpaceShip.lmdl";
				Log::SetVerbosity(Log::VERB);
				if(!loadModelFile(modelName))
				{
					Log::E("Couldn't load model!");
					return false;
				}
				if(!initModelIntoRenderer(model))
				{
					return false;
				}
				
				Log::SetVerbosity(Log::DEBUG);

				return true;
			}
			void Shutdown(Game* game)
			{
				game->Platform()->InitInput(game->Input());
				input.Shutdown();
				resMgr.Shutdown();
			}
			void Update(Game* game, const GameTime& time)
			{
				updateCamPos(time);
				updateLightPos(time);
				view = Matrix4x4::BuildViewLookAtRH(camPos, camLookAt, camUp);
			}
			void Draw(Game* game, const GameTime& time)
			{
				gfx->Clear(Colors::Gray20Pct);

				gfx->SetWorldViewProjection(world, view, projection);
				//draw a coordinate system!
				Vector3 coordStart = Vector3(2.0f, 2.0f, 0.0f);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(1.0f, 0.0f, 0.0f), Colors::Red);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0.0f, 1.0f, 0.0f), Colors::Green);
				gfx->DebugDrawLine(coordStart, coordStart + Vector3(0.0f, 0.0f, 1.0f), Colors::Blue);
				//next, draw the light's position.
				gfx->DebugDrawSphere(lightPos, 0.5f, Color(lightColor));
				//draw the model's bounds.
				drawBounds(model);
				//finally, setup for the actual model...
				gfx->SetShader("DiffuseTextured");
				gfx->SetVec3Uniform("lightDiffuse", lightColor);
				gfx->SetVec3Uniform("lightPos", lightPos);
				//Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
				//gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
				gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
				//and draw the actual model.
				drawModel(model);
			}
		};

		/**
		Tests that cameras have their frustums properly placed,
		and that frustums can properly traverse octrees.
		*/
		class OcTreeTest : public TestBaseWithResources
		{
		private:
			IPlatform* plat;

			//The camera that will be traversing the octree.
			CameraBase* traversingCam;

			Vector<AABBBounds> boxes;
			Vector<SphereBounds> spheres;
			Vector<AABBBounds*> dynBoxes;
			Plane plane;
			Vector3 planeOrigin;
			Color visibleColor, culledColor;
			Color frontPlaneColor, behindPlaneColor;
			F32 ocTreeSize;

			//The octree containing the bounding objects.
			DebugBoundsOcTree* ocTree;
			SphereBounds movingSphere;
			DebugBoundsOcTree::Node* sphereNode;

			void initCamera(CameraBase* cam)
			{
				cam->SetAspectRatio(gfx->ScreenAspect());
				cam->SetFOV(Math::PI_OVER_2);
				cam->SetNearDist(.25f);
				cam->SetFarDist(1000.0f);
			}
			void initTree()
			{
				ocTree = LNew(DebugBoundsOcTree, AllocType::TEST_ALLOC, "TestAlloc")(ocTreeSize);
				//just try inserting the objects into the octree
				for(int i = 0; i < boxes.size(); ++i)
				{
					ocTree->Insert(&(boxes[i]));
					//boxes[i].second = culledColor;
				}
				for(int i = 0; i < spheres.size(); ++i)
				{
					ocTree->Insert(&(spheres[i]));
					//spheres[i].second = culledColor;
				}
			}
			void updateGlobalCommands(const GameTime& time)
			{
				//toggle mouselook
				if(kbd->KeyPressed(KEY_COMMA))
				{
					enableMouseLook = !enableMouseLook;
				}
			}
			void updateCamPos(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 transVec = Vector3::Zero;
				if(kbd->KeyDown(KEY_W))
				{
					transVec += viewCam->Forward() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_S))
				{
					transVec -= viewCam->Forward() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_A))
				{
					transVec += viewCam->Right() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_D))
				{
					transVec -= viewCam->Right() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_E))
				{
					transVec += viewCam->Up() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_C))
				{
					transVec -= viewCam->Up() * camMoveVel * elapsedSec;
				}
				viewCam->Translate(transVec);
			}
			void updateCamRot(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 rotVec = Vector3::Zero;
				
				//also update mouse tilt and pan, if desired
				if(enableMouseLook)
				{
					Vector2 mouseDelta = mouse->GetMousePos() * (camRotVel * elapsedSec * mouseSensitivity);
					//remember that the mouse delta is linear and rotvec is rotational!
					rotVec.SetX(rotVec.X() + mouseDelta.Y());
					rotVec.SetY(rotVec.Y() - mouseDelta.X());
				}

				viewCam->RotateEuler(rotVec.X(), rotVec.Y(), rotVec.Z());
				viewCam->SetUp(Vector3::Up);
			}
			
			void updateTravCamPos(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 transVec = Vector3::Zero;
				if(kbd->KeyDown(KEY_UPARR))
				{
					transVec += traversingCam->Forward() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_DNARR))
				{
					transVec -= traversingCam->Forward() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_LARR))
				{
					transVec += traversingCam->Right() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_RARR))
				{
					transVec -= traversingCam->Right() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_PGUP))
				{
					transVec += traversingCam->Up() * camMoveVel * elapsedSec;
				}
				if(kbd->KeyDown(KEY_PGDN))
				{
					transVec -= traversingCam->Up() * camMoveVel * elapsedSec;
				}
				//planeOrigin += transVec;
				//plane.SetOrigin(planeOrigin);
				traversingCam->Translate(transVec);
			}
			
			void updateTravCamRot(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 rotVec = Vector3::Zero;
				//tilt
				if(kbd->KeyDown(KEY_I))
				{
					rotVec.SetX(rotVec.X() + (camRotVel * elapsedSec));
				}
				if(kbd->KeyDown(KEY_K))
				{
					rotVec.SetX(rotVec.X() - (camRotVel * elapsedSec));
				}
				//pan
				if(kbd->KeyDown(KEY_J))
				{
					rotVec.SetY(rotVec.Y() - (camRotVel * elapsedSec));
				}
				if(kbd->KeyDown(KEY_L))
				{
					rotVec.SetY(rotVec.Y() + (camRotVel * elapsedSec));
				}
				//roll
				if(kbd->KeyDown(KEY_U))
				{
					rotVec.SetZ(rotVec.Z() + (camRotVel * elapsedSec));
				}
				if(kbd->KeyDown(KEY_M))
				{
					rotVec.SetZ(rotVec.Z() - (camRotVel * elapsedSec));
				}
				//plane.SetNormal(Quaternion::FromEulerAngles(rotVec) * plane.GetNormal());
				traversingCam->RotateEuler(rotVec.X(), rotVec.Y(), rotVec.Z());
			}
			void updateTravCamProj(const GameTime& time)
			{
				//handle near and far planes
				//running out of keys... 1-2 for near,
				//3-4 for far.
				//Clamp near to [0, far),
				//far to (near, <max range>].

				//then FOV (clamp to [1,179] degrees)
			}
			void updateSpherePos(const GameTime& time)
			{
				const F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 transVec = Vector3::Zero;
				if(kbd->KeyDown(KEY_T))
				{
					transVec += Vector3::Forward;
				}
				if(kbd->KeyDown(KEY_G))
				{
					transVec -= Vector3::Forward;
				}
				if(kbd->KeyDown(KEY_H))
				{
					transVec += Vector3::Right;
				}
				if(kbd->KeyDown(KEY_F))
				{
					transVec -= Vector3::Right;
				}
				if(kbd->KeyDown(KEY_Y))
				{
					transVec += Vector3::Up;
				}
				if(kbd->KeyDown(KEY_N))
				{
					transVec -= Vector3::Up;
				}
				transVec *= camMoveVel * elapsedSec;
				if(transVec.LengthSquared() != 0.0f)
				{
					movingSphere.SetCenter(movingSphere.Center() + transVec);
					sphereNode = ocTree->UpdateNode(sphereNode);
				}
			}
			void updateDynScene(const GameTime& time)
			{
				if(kbd->KeyPressed(KEY_R))
				{
					AABBBounds* newBox = LNew(AABBBounds, AllocType::TEST_ALLOC, "TestAlloc")(Random::InCube(ocTreeSize), Random::InRange(0.125f, 5.0f));
					dynBoxes.push_back(newBox);
					ocTree->Insert(dynBoxes[dynBoxes.size()-1]);
				}
				if(kbd->KeyPressed(KEY_V) && dynBoxes.size() > 0)
				{
					AABBBounds* toRemove = dynBoxes[dynBoxes.size()-1];
					ocTree->Remove(toRemove);
					dynBoxes.pop_back();
					LDelete(toRemove);
				}
			}
			void drawTravCamFrustPlanes()
			{
				//There's 6 planes on the frustum;
				//colors are limited.
				//Use red and green for pairs of frusta
				//on an axis; green indicates
				//the plane on the positive side of the axis,
				//red the plane on the negative.
				//individual tones indicate the axis.
				const Frustum& f = traversingCam->GetWorldFrustum();
				const CameraBase* cam = traversingCam;
				F32 halfFOV = cam->FOV() / 2;
				F32 halfDist = cam->FarDist() / 2;
				F32 halfHeight = Math::Tan(halfFOV)*halfDist;
				F32 halfWidth = halfHeight * cam->AspectRatio();
				Vector3 camPos = traversingCam->Position();
				Vector3 halfPt = camPos + (halfDist * cam->Forward());
				//near and far
				gfx->DebugDrawPlane(camPos + (cam->NearDist() * cam->Forward()),
									f.Near().GetNormal(),
									Color(0.5f,	0.0f, 0.0f));
				gfx->DebugDrawPlane(camPos + (cam->FarDist() * cam->Forward()),
									f.Far().GetNormal(),
									Color(1.0f,	0.0f, 0.0f));
				//left and right
				gfx->DebugDrawPlane(halfPt + (halfWidth * cam->Right()),
									f.Left().GetNormal(),
									Color(0.0f,	0.5f, 0.0f));
				gfx->DebugDrawPlane(halfPt + (-halfWidth * cam->Right()),
									f.Right().GetNormal(),
									Color(0.0f,	1.0f, 0.0f));
				//top and bottom
				gfx->DebugDrawPlane(halfPt + (-halfHeight * cam->Up()),
									f.Bottom().GetNormal(),
									Color(0.0f,	0.0f, 0.5f));
				gfx->DebugDrawPlane(halfPt + (halfHeight * cam->Up()),
									f.Top().GetNormal(),
									Color(0.0f,	0.0f, 1.0f));
			}
			void drawTraversingCam()
			{
				//draw a really small sphere to mark the camera's position.
				gfx->DebugDrawSphere(traversingCam->Position(), 0.125f, Colors::LtBlue);
				//update the traversing frustum as needed
				//drawTravCamFrustPlanes();
				//then draw the frustum.
				gfx->DebugDrawFrustum(traversingCam, Colors::LtBlue);
			}
			void drawOcTreeNode(const OcTreeNodeBase* node, const Vector3& nodeCenter, F32 nodeRegionSize)
			{
				Frustum& camFrus = traversingCam->GetWorldFrustum();
				//draw each of this node's bounds
				for(int i = 0; i < OcTreeNodeBase::LOCATION_COUNT; ++i)
				{
					Vector3 subSecCenter = ocTree->GetNodeCenter(	nodeCenter,
																	nodeRegionSize,
																	(OcTreeNodeBase::ChildLocation)i);
					F32 childSecSize = nodeRegionSize / 2;
					auto child = node->Children[i];
					Color nodeColor = (child != NULL && child->IsLeaf()) ? Colors::LtGreen : Color(0.5f,.6f,0.5f);
					if(child != NULL)
					{
						//draw the child
						//gfx->DebugDrawAABB(subSecCenter, childSecSize/2, nodeColor);
						//really should not do this in production code,
						//but test against the subsector
						AABBBounds subSecBnd = ocTree->GetSubSectorAABB((OcTreeNode<Bounds*>*)node, nodeCenter, nodeRegionSize, (OcTreeNodeBase::ChildLocation)i);
						if(camFrus.Test(subSecBnd) && child->IsLeaf())
						{
							nodeColor = Colors::LtYellow;
						}
						//if the child has children, recurse
						if(!child->IsLeaf())
						{
							drawOcTreeNode(child, subSecCenter, childSecSize);
						}
						else
						{
							gfx->DebugDrawAABB(subSecBnd, nodeColor);
						}
					}
				}
			}
			void drawOcTree()
			{
				const OcTreeNode<Bounds*>* treeRoot = ocTree->Root();
				if(treeRoot)
				{
					drawOcTreeNode(treeRoot, Vector3::Zero, ocTree->RegionSize());
				}
			}
			void drawScene()
			{
				//draw a big-ol box surrounding the scene
				//gfx->DebugDrawAABB(Vector3::Zero, 500.0f * Vector3::One, Colors::Gray30Pct);

				//gfx->DebugDrawPlane(planeOrigin, plane.GetNormal(), Colors::Orange);
				//Cull the octree
				Frustum& camFrust = traversingCam->GetWorldFrustum();
				Vector<Bounds*> bndList = ocTree->FindAllInBounds(camFrust);
				gfx->DebugDrawSphere(movingSphere, Colors::White);
				//now draw all objects in frustum
				for(auto i = bndList.cbegin(); i != bndList.cend(); ++i)
				{
					Bounds* bnd = (*i);
					if(bnd != NULL && camFrust.Test(*bnd))
					{
						if(bnd->GetType() == Bounds::BND_AABB)
						{
							gfx->DebugDrawAABB(*((AABBBounds*)bnd), Colors::Yellow);
						}
						else
						{
							gfx->DebugDrawSphere(bnd->Center(), bnd->Radius(), Colors::Yellow);
						}
					}
					//if(bnd->GetType() == Bounds::BND_SPHERE)
					//{
					//}
				}

				drawOcTree();
			}
		public:
			OcTreeTest()
			{
				
			}
			bool Startup(Game* game)
			{
				if(!TestBaseWithResources::Startup(game))
				{
					return false;
				}
				//init the input
				camMoveVel = 5;
				camRotVel = 1;
				mouseSensitivity = 3;

				//cameras
				viewCam->SetFarDist(1000.0f);
				viewCam->SetPosition(Vector3(4, 3, 4));
				traversingCam = LNew(FreeLookCamera, AllocType::TEST_ALLOC, "TestAlloc")();
				initCamera(traversingCam);
				traversingCam->SetFarDist(10);
				//put the traversing camera in front of the view camera
				traversingCam->SetPosition(Vector3(0, -1, -1));

				visibleColor = Colors::Yellow;
				culledColor = Colors::Blue;

				frontPlaneColor = Colors::LtGreen;
				behindPlaneColor = Colors::LtRed;

				//no octree yet; build the bounding volumes
				ocTreeSize = 500.0f;
				boxes = Vector<AABBBounds>();//LArrayNew(boxPair, boxes.size(), AllocType::TEST_ALLOC, "TestAlloc");
				boxes.reserve(boxes.size());
				spheres = Vector<SphereBounds>();//LArrayNew(spherePair, spheres.size(), AllocType::TEST_ALLOC, "TestAlloc");
				spheres.reserve(spheres.size());

				boxes.push_back(AABBBounds(Vector3(-.25, 0, -3), 1, .5f, .25f));
				boxes.push_back(AABBBounds(Vector3(0, 0, -50), 1));
				boxes.push_back(AABBBounds(Vector3(7, 0, -3), 2));

				spheres.push_back(SphereBounds(Vector3(.33, 9, -4), .5f));
				spheres.push_back(SphereBounds(Vector3(0, -20, -30), 4.0f));
				spheres.push_back(SphereBounds(Vector3(0, .10f, -2), 1.0f));

				//make a really simple plane -
				//origin at 0, normal up.
				planeOrigin = Vector3::Zero;
				plane = Plane(planeOrigin, Vector3::Up);

				//prep the octree.
				initTree();
				
				//Init the player-mobile bound.
				movingSphere = SphereBounds(Vector3(-5, 3, 0), 1);
				sphereNode = ocTree->Insert(&movingSphere);
				return true;
			}
			void Shutdown(Game* game) { TestBaseWithResources::Shutdown(game); }
			void Update(Game* game, const GameTime& time)
			{
				TestBaseWithResources::Update(game, time);
				updateTravCamPos(time);
				updateSpherePos(time);
				updateTravCamRot(time);
				updateDynScene(time);
			}
			void Draw(Game* game, const GameTime& time)
			{
				TestBaseWithResources::Draw(game, time);
				drawTraversingCam();
				drawScene();
			}
		};

		class BoundsTest : public TestBaseWithResources
		{
		private:
			AABBBounds movingBox, staticBox;
			SphereBounds staticSphere;

			F32 moveVel;

			Key boxFwd, boxBack, boxLeft, boxRight, boxUp, boxDown;
			Color movingColor, staticColor, collisionColor;
			Color movBoxCurrColor, statBoxCurrColor;

			void updateMoveBox(const GameTime& time)
			{
				Vector3 moveVec = Vector3::Zero;
				F32 frameTime = time.ElapsedGameTime().ToSeconds();

				if(kbd->KeyDown(boxFwd))
				{
					moveVec += Vector3::Forward * moveVel * frameTime;
				}
				if(kbd->KeyDown(boxBack))
				{
					moveVec -= Vector3::Forward * moveVel * frameTime;
				}
				if(kbd->KeyDown(boxLeft))
				{
					moveVec -= Vector3::Right * moveVel * frameTime;
				}
				if(kbd->KeyDown(boxRight))
				{
					moveVec += Vector3::Right * moveVel * frameTime;
				}
				if(kbd->KeyDown(boxUp))
				{
					moveVec += Vector3::Up * moveVel * frameTime;
				}
				if(kbd->KeyDown(boxDown))
				{
					moveVec -= Vector3::Up * moveVel * frameTime;
				}
				movingBox.SetCenter(movingBox.Center() + moveVec);
			}
			void updateCollisions(const GameTime& time)
			{
				if(movingBox.Test(staticBox) || movingBox.Test(staticSphere))
				{
					movBoxCurrColor = collisionColor;
				}
				else
				{
					movBoxCurrColor = movingColor;
				}
			}
			void drawBoxes(const GameTime& time)
			{
				gfx->DebugDrawAABB(movingBox, movBoxCurrColor);
				gfx->DebugDrawAABB(staticBox, statBoxCurrColor);
				gfx->DebugDrawSphere(staticSphere, staticColor);
			}
		public:
			bool Startup(Game* game)
			{
				if(!TestBaseWithResources::Startup(game))
				{
					return false;
				}

				boxFwd = KEY_T;
				boxBack = KEY_G;
				boxLeft = KEY_F;
				boxRight = KEY_H;
				boxUp = KEY_Y;
				boxDown = KEY_N;

				movingColor = Colors::Green;
				staticColor = Colors::White;
				collisionColor = Colors::Red;

				//initialize the boxes
				movingBox = AABBBounds(Vector3(1,2,-3), 1);
				staticBox = AABBBounds(Vector3(-1,0,-1), 1);
				staticSphere = SphereBounds(Vector3(0,0,-1), 0.5f);

				moveVel = 1.0f;

				movBoxCurrColor = movingColor;
				statBoxCurrColor = staticColor;

				return true;
			}
			void Shutdown(Game* game) { TestBaseWithResources::Shutdown(game); }
			void Update(Game* game, const GameTime& time) 
			{
				TestBaseWithResources::Update(game, time);
				updateMoveBox(time);
				updateCollisions(time);
			}
			void Draw(Game* game, const GameTime& time) 
			{
				TestBaseWithResources::Draw(game, time);
				drawBoxes(time);
			}
		};

		/**
		Tests functionality of the scene graph system,
		and parts of the renderer system. In particular,
		movement of objects in the scene graph and octree
		are major parts of this test.

		Render state and blending are NOT tested.
		*/
		class SceneTest : public TestBaseWithResources
		{
		private:
			TypedHandle<Renderer> renderer;
			TypedHandle<Culler> culler;
			GfxWrapperHandle gfx;
			TypedHandle<Model> mdl;
			String mdlName;
			ResPtr mdlPtr;
			TypedHandle<ResourceManager> resMgr;
			TypedHandle<CameraBase> camera;
			String archiveName;
			TypedHandle<Shader> defShader, textShader;
			static const U32 MAX_NUM_MODELS = 1000;
			F32 maxDistributionRange;
			F32 camSpeed;
			F32 farPlane;

			bool initResources()
			{
				resMgr = HandleMgr::RegisterPtr(LNew(ResourceManager, AllocType::TEST_ALLOC, "TestAlloc"));
				//init resource manager
				if(!resMgr->Init(128))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				//register texture importers...
				resMgr->RegisterLoader(GetSharedPtr(CustomNew<PNGLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				//modelName = "taurus";
				return true;
			}
			bool initShaders()
			{
				Vector< ShaderFilePair > pathsColor, pathsDiffuse, pathsDiffuseTextured;
				pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.vp").c_str())) );
				pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Color.fp").c_str())) );

				pathsDiffuse.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.vp").c_str())) );
				pathsDiffuse.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Diffuse.fp").c_str())) );

				pathsDiffuseTextured.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.vp").c_str())) );
				pathsDiffuseTextured.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/DiffuseTextured.fp").c_str())) );
				
				Vector< ShaderFilePair > text;
				text.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Text.vp").c_str())) );
				text.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "./Shaders/Basic/Text.fp").c_str())) );

				defShader = gfx->MakeShader("DiffuseTextured", 2, pathsDiffuseTextured);
				textShader = gfx->MakeShader("Text", 2, text);
				return	gfx->MakeShader("Color", 2, pathsColor) &&
						gfx->MakeShader("Diffuse", 2, pathsDiffuse) &&
						defShader && textShader;
			}
			bool initCamera()
			{
				camera = HandleMgr::RegisterPtr(viewCam).GetHandle();
				//base class' initialization function handles the loading for us.
				//Set the camera speed and sensitivity.
				camMoveVel = camSpeed;
				camera->SetFarDist(farPlane);
				return true;
			}
			bool loadModelRes(Game* game, const String& archName)
			{
				//remember to add the importer, stupid
				//resMgr.RegisterLoader(GetSharedPtr(CustomNew<ModelLoader>(RESLOADER_ALLOC, "ResLoaderAlloc")));
				ResGUID lmdlGUID(archName, "Models/" + mdlName + ".lmdl");
				Log::D(String("Loading .lmdl file ") + lmdlGUID.Name);
				time->Tick();
				mdlPtr = resMgr->GetResource(lmdlGUID);
				time->Tick();
				if(!mdlPtr)
				{
					Log::E(String("Failed to load ") + lmdlGUID.Name + "!");
					return false;
				}
				Log::D(String("Loaded .lmdl in ") + time->ElapsedGameTime().ToSeconds() + "s");
				return true;
			}
			bool initModelIntoRenderer(Model& model)
			{
				//Steps after this will delete geometry data;
				//recalculate bounds if needed before that data is lost.
				//if(!model.HasBounds())
				//{
					model.RecalcBounds();
				//}
				for(U32 i = 0; i < model.MeshCount(); ++i)
				{
					if(!gfx->InitGeometry(model.GetMesh(i)->GetGeometry()))
					{
						Log::E("Couldn't load model into OpenGL!");
						return false;
					}
					//we can get rid of geometry data now
					const Geometry& geom = model.GetMesh(i)->GetGeometry();
					//HandleMgr::DeleteHandle(geom.VertexHandle());
					//HandleMgr::DeleteHandle(geom.IndexHandle());

					ResPtr diffTex = resMgr->GetResource(model.GetMesh(i)->GetMaterial().DiffuseTexGUID);
					Log::D(String("Opening texture resource ") + model.GetMesh(i)->GetMaterial().DiffuseTexGUID.Name);
					if(!diffTex)
					{
						Log::E("Couldn't get mesh texture resource!");
						return false;
					} 
					Texture2D& diffTexRef = *(Texture2D*)diffTex->Buffer();
					if(!diffTexRef.TextureBufferHandle)
					{
						if(!gfx->InitTexture(diffTexRef, TextureMeta::DIFFUSE))
						{
							Log::E("Couldn't init mesh texture!");
							return false;
						}
					}
				}
				return true;
			}
			bool loadModel(Game* game)
			{

				//try loading that model now
				//and try loading a model resource
				Log::SetVerbosity(Log::VERB);
				if(!loadModelRes(game, archiveName))
				{
					return false;
				}
				mdl = HandleMgr::RegisterPtr((Model*)mdlPtr->Buffer());
				if(!initModelIntoRenderer(*mdl))
				{
					return false;
				}

				return true;
			}
			bool initCuller()
			{
				culler = HandleMgr::RegisterPtr(	LNew(OcTreeCuller, AllocType::TEST_ALLOC, "TestAlloc")
													()
												).GetHandle();
				culler->SetCamera(camera);

				return true;
			}
			bool initRenderer()
			{

				//load up default texture
				ResGUID defTexGUID(archiveName, "Textures/defaultTex.png");

				renderer = HandleMgr::RegisterPtr(LNew	(Renderer, AllocType::RENDERER_ALLOC, "RendererAlloc")
														(gfx, camera, culler, resMgr, defTexGUID));
				renderer->Init();

				return true;
			}
			bool initScene()
			{
				//For now, just make a few objects randomly placed in space,
				//select one as the object we move.
				for(U32 i = 0; i < MAX_NUM_MODELS; ++i)
				{
					Vector3 mdlPos = Random::InCube(	-maxDistributionRange / 2,
														maxDistributionRange / 2);
					Quaternion mdlHdg = Random::InCubicEulerRange();

					TypedHandle<ModelNode> mdlNode = HandleMgr::RegisterPtr(LNew(ModelNode, TEST_ALLOC, "TestAlloc")());
					mdlNode->SetGeometry(mdl);
					mdlNode->LocalTransform().SetOrientation(mdlHdg);
					mdlNode->LocalTransform().SetPosition(mdlPos);
					//don't forget to add the shader instance!
					if(defShader)
					{
						mdlNode->AttachLocalShader(defShader);
					}
					renderer->InsertNodeAt(mdlNode.GetHandle());
				}

				//After initial test, insert a bunch of objects.
				//Randomly put a random number of children on some of them.
				//Then randomly put a random number of children on some of those children.
				//The hierarchy system is a little strange - you put a grouping node on,
				//the parent's geometry is a child of that node,
				//then all children are under a second grouping node.

				//Pray it doesn't crash!
				return true;
			}
			/*
			void updateCamPos(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector3 camPos = Vector3::Zero;
				if(kbd->KeyDown(KEY_W))
				{
					camPos += Vector3::Forward;
				}
				if(kbd->KeyDown(KEY_S))
				{
					camPos -= Vector3::Forward;
				}
				if(kbd->KeyDown(KEY_A))
				{
					camPos -= Vector3::Right;
				}
				if(kbd->KeyDown(KEY_D))
				{
					camPos += Vector3::Right;
				}
				if(kbd->KeyDown(KEY_E))
				{
					camPos += Vector3::Up;
				}
				if(kbd->KeyDown(KEY_C))
				{
					camPos -= Vector3::Up;
				}
				camPos.Normalize();

				camera->Translate(camPos * elapsedSec * camSpeed);
			}
			void updateCamRot(const GameTime& time)
			{
				F32 elapsedSec = time.ElapsedGameTime().ToSeconds();
				Vector2 camRot = mouse->GetMouseDelta() * elapsedSec;

				camera->RotateEuler(camRot.Y(), camRot.X(), 0);
			}
			void updateCamera(const GameTime& time)
			{
				updateCamPos(time);
				updateCamRot(time);
			}
			*/
			void drawDebugUI(Game* game, const GameTime& time)
			{
			}
		public:
			bool Startup(Game* game) 
			{
				
				//Acquire and initialize components.
				renderer = 0;
				culler = 0;
				gfx = game->GfxWrapper();
				camera = 0;
				mdl = 0;
				kbd = 0;
				mouse = 0;

				mdlName = "taurus";
				archiveName  = "TestContent/Archives/TestContent.zip";

				defShader = 0;
				textShader = 0;

				maxDistributionRange = 500.0f;
				camSpeed = maxDistributionRange / 5.0f;
				farPlane = maxDistributionRange / 4.0f;	

				kbd = HandleMgr::RegisterPtr(&game->Input()->GetKeyboard(0));
				mouse = HandleMgr::RegisterPtr(&game->Input()->GetMouse(0));

				if(!initResources())
				{
					return false;
				}

				if(!loadModel(game))
				{
					return false;
				}

				//init the camera
				if(!initCamera())
				{
					return false;
				}

				//setup shaders
				if(!initShaders())
				{
					Log::E("Failed to build shaders!");
					return false;
				}

				//setup culler
				if(!initCuller())
				{
					return false;
				}

				//then renderer
				if(!initRenderer())
				{
					return false;
				}

				//setup scene graph
				if(!initScene())
				{
					return false;
				}

				//Now we should be ready!
				return true;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time)
			{
				TestBaseWithResources::Update(game, time);
				//updateCamera(time);
				//updateShip(time);
			}
			void Draw(Game* game, const GameTime& time)
			{
				TestBaseWithResources::Draw(game, time);
				gfx->DebugDrawBox(Vector3::Zero, Quaternion::Identity, Vector3::One * (maxDistributionRange / 2), Colors::LtBlue);
				renderer->DrawScene();
				//gfx->DebugDrawFrustum(camera.Ptr(), Colors::White);
			}
		};

		class HashTableTest : public TestBase
		{
			typedef Vector3 keyT;
			typedef U32 valT;
			typedef HashTable<keyT, valT> tableT;
			tableT table;
		public:
			HashTableTest()
			{
			}
			bool Startup(Game* game)
			{
				int numElem = 1000;
				table = tableT();
				LogD(	String("Built hash table. Table has capacity ") +
						table.capacity() + " and max load factor " +
						table.max_load_factor());
				game->Time().Tick();
				for(int i = 0; i < numElem; ++i)
				{
					//U32 key = i + 8493;
					keyT key = keyT(i + 8493, i + 908, i + 114);
					U32 val = Random::InRange(1, 989);
					table[key] = val;
				}
				game->Time().Tick();
				LogD(	String("Added ") + numElem + " elements to the table in " +
						game->Time().ElapsedGameTime().ToSeconds() + " seconds; table has capacity for "
						+ table.capacity() + " elements.");
				LogD(String("There were ") + table.num_collisions() + " collisions.");
				float runningAvg = 0;
				for(int i = 0; i < numElem; ++i)
				{
					//U32 key = i + 8493;
					keyT key = keyT(i + 8493, i + 908, i + 114);
					game->Time().Tick();
					U32 val = table[key];
					game->Time().Tick();
					runningAvg += game->Time().ElapsedGameTime().ToSeconds();
				}
				runningAvg /= numElem;
				LogD(String("Average access time is ") + runningAvg + " seconds.");
				LogD("Erasing elements...");
				game->Time().Tick();
				for(int i = 0; i < numElem; ++i)
				{
					//U32 key = i + 8493;
					keyT key = keyT(i + 8493, i + 908, i + 114);
					auto beforeErase = table.size();
					auto afterErase = table.erase(key);
					if(afterErase == beforeErase)
					{
						LogE(String("Failed to erase key ") + key.ToString() + "!");
						return false;
					}
				}
				game->Time().Tick();
				LogD(	String("Erased elements in ") +
						game->Time().ElapsedGameTime().ToSeconds() + " seconds.");
				LogD("Clearing table...");
				table.clear();
				LogD(	String("Table cleared; table has capacity for ")
						+ table.capacity() + " elements.");
				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		class ThreadTest : public TestBase
		{
		public:
			ThreadTest()
			{
			}
			bool Startup(Game* game)
			{
				//simple test - make two threads increment a var 50 times
				//first w/o mutex, then with mutex
				F32 x = 0;
				class noMutex : public IThreadClient
				{
				private:
					F32* x;
				public:
					noMutex(F32* var) { x = var; }
					void Run()
					{
						const F32 delta = 0.001f;
						for(float i = 0.0f; i < Math::PI_OVER_2; i += delta)
						{
							(*x) += Math::Sin(i) * delta;
						}
					}
				};
				Log::D(String("x = ") + x);
				//run the threads here
				noMutex testCodeA(&x);
				noMutex testCodeB(&x);
				Thread threadA(&testCodeA);
				Thread threadB(&testCodeB);
				Log::D("Attempting to run threads...");
				threadA.Start();
				threadB.Start();
				threadA.Join();
				threadB.Join();
				Log::D(String("Threads are now finished. x = ") + x);
				//reset values
				x = 0;
				Log::D(String("Let's try again. x = ") + x);
				Thread threadA2(&testCodeA);
				Thread threadB2(&testCodeB);
				Log::D("Attempting to run threads...");
				threadA2.Start();
				threadB2.Start();
				threadA2.Join();
				threadB2.Join();
				Log::D(String("Threads are now finished. x = ") + x);
				//now try with mutex.
				//reset values
				x = 0;
				IMutex* mut = &Mutex();
				//put mutex here!
				class withMutex : public IThreadClient
				{
				private:
					F32* x;
					IMutex* m;
				public:
					withMutex(F32* var, IMutex* mutex)
					{
						x = var; 
						m = mutex;
					}
					void Run()
					{
						//put lock here!
						Log::D("Attempting to lock mutex...");
						//ILock lock = m->GetLock();
						Log::D("Locked mutex!");
						const F32 delta = 0.001f;
						for(float i = 0.0f; i < Math::PI_OVER_2; i += delta)
						{
							(*x) += Math::Sin(i) * delta;
						}
						Log::D("Unlocking mutex.");
					}
				};
				Log::D(String("Let's try with mutexes. x = ") + x);
				withMutex testCodeC(&x, mut);
				withMutex testCodeD(&x, mut);
				Thread threadC(&testCodeC);
				Thread threadD(&testCodeD);
				Log::D("Attempting to run threads...");
				threadC.Start();
				threadD.Start();
				threadC.Join();
				threadD.Join();
				Log::D(String("Threads are now finished. x = ") + x);
				//reset values
				x = 0;
				Log::D(String("Let's try again. x = ") + x);
				Thread threadC2(&testCodeC);
				Thread threadD2(&testCodeD);
				Log::D("Attempting to run threads...");
				threadC2.Start();
				threadD2.Start();
				threadC2.Join();
				threadD2.Join();
				Log::D(String("Threads are now finished. x = ") + x);
				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		class DbgResMgrTest : public TestBase
		{
		public:
			DbgResMgrTest(void) {}
			~DbgResMgrTest(void) {}
			bool Startup(Game* game)
			{
				//ZipFile file;
				Path filePath("/TestContent/cfgTestIn.xml");

				//also set up a buffer to preview the first few bytes of data
				//only really good for text data
				const U32 dispLimit = 128;
				char* dispBuf = LArrayNew(char, dispLimit, AllocType::TEST_ALLOC, "TestAlloc");
				memset(dispBuf, 0, dispLimit);

				//now test the resource manager!
				Log::D("Testing resource manager...");
				DebugResManager resMgr;
				if(!resMgr.Init(16))
				{
					Log::E("Couldn't init resource manager!");
					return false;
				}

				//first load

				Log::D("Loading filesystem resource via resource manager...");
				game->Time().Tick();
				std::shared_ptr<Resource> resPtrCacheMiss = resMgr.OpenResFromFile(filePath);
				game->Time().Tick();
				Log::D(String("Loaded resource in ") + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
				Log::D("Loading resource again...");
				game->Time().Tick();
				std::shared_ptr<Resource> resPtrNoMiss = resMgr.OpenResFromFile(filePath);
				game->Time().Tick();


				if(*resPtrCacheMiss != *resPtrNoMiss)
				{
					Log::E("Loaded data doesn't match!");
					return false;
				}

				Log::D(String("Fetched resource pointer in ") + game->Time().ElapsedGameTime().ToMilliseconds() + " ms");
				Log::D(String("Name: ") + resPtrNoMiss->GUID().ResName());
				Log::D(String("Indicated file size: ") + Filesystem::GetFileSize(Filesystem::GetProgDir() + filePath.ToString()));
				strncpy_s(dispBuf, dispLimit, resPtrNoMiss->Buffer(), dispLimit-1);
				//dispBuf[dispLimit - 1] = 0;
				Log::D(String("Preview: ") + dispBuf);

				Log::D("Loading archive resource via resource manager...");
				Path archPath(String(Filesystem::GetProgDir()) + "/TestContent/Archives/WithFolders.zip");
				Log::D("Testing resource archive system...");
				IResourceArchive* resArch = CustomNew<ZipResArchive>(TEST_ALLOC, "ArchiveAlloc", archPath);
				if(!resArch || !resArch->Open())
				{
					Log::E("Failed to open archive!");
					return false;
				}
				String archString = String("TestContent/Archives/") + resArch->GetArchiveName();
				U32 resourceToLoad = 0;
				for(U32 i = 0; i < resArch->GetNumResources(); ++i)
				{
					ResGUID tempGUID(archString, resArch->GetResourceName(i));
					if(resArch->GetRawSize(tempGUID) > 0)
					{
						resourceToLoad = i;
						break;
					}
				}
				ResGUID archGUID(archString, resArch->GetResourceName(resourceToLoad));
				Log::D(String("Archive GUID for resource: ") + archGUID.Name);
				auto archRes = resMgr.GetResource(archGUID);
				Log::D("Opened archive resource");
				Log::D(String("Name: ") + archRes->GUID().ResName());
				LArrayDelete(dispBuf);
				Log::D("Shutting down resource manager...");
				resMgr.Shutdown();

				return false;
			}
			void Shutdown(Game* game) {}
			void Update(Game* game, const GameTime& time) {}
			void Draw(Game* game, const GameTime& time) {}
		};

		class AudioTest : public TestBase
		{
		private:
			IAudioWrapper* audioMgr;
			DebugResManager resMgr;
			ISoundInstance* wavSound;
			ISoundInstance* oggSound;
			TypedHandle<Keyboard> kbd;
		public:
			bool Startup(Game* game)
			{
				audioMgr = NULL;
				wavSound = NULL;
				oggSound = NULL;
				kbd = 0;

				//Prep the resource manager.
				if(!resMgr.Init(64))
				{
					Log::E("Failed to initialize resource manager!");
					return false;
				}
				//Attach handlers...
				resMgr.RegisterLoader(GetSharedPtr(LNew(WAVLoader, RESLOADER_ALLOC, "ResLoaderAlloc")()));
				resMgr.RegisterLoader(GetSharedPtr(LNew(OGGLoader, RESLOADER_ALLOC, "ResLoaderAlloc")()));

				//Load up an audio file.
				String wavPath = "TestContent/Sounds/ding.wav";
				auto wavRes = resMgr.OpenResFromFile(wavPath);
				String oggPath = "TestContent/Sounds/hi.ogg";
				auto oggRes = resMgr.OpenResFromFile(oggPath);
				if(!wavRes || !oggRes)
				{
					Log::E("Failed to load audio file!");
					return false;
				}
				SoundExtraData* sndExtra = (SoundExtraData*)wavRes->Extra();
				if(!sndExtra)
				{
					Log::E("Failed to get metadata from audio file " + wavPath + "!");
					return false;
				}
				audioMgr = LNew(XAudioAudioWrapper, AllocType::SOUND_ALLOC, "SoundAlloc")(sndExtra->GetFormat());
				if(!audioMgr->Initialize())
				{
					Log::E("Failed to initialize audio wrapper!");
					return false;
				}
				wavSound = audioMgr->InitInstance(wavRes);
				if(!wavSound)
				{
					Log::E("Failed to create a playable WAV sound!");
					return false;
				}
				oggSound = audioMgr->InitInstance(oggRes);
				if(!oggSound)
				{
					Log::E("Failed to create a playable OGG sound!");
					return false;
				}
				//Try to get input...
				kbd = HandleMgr::RegisterPtr(&game->Input()->GetKeyboard(0));
				if(!kbd)
				{
					Log::E("Failed to get keyboard!");
					return false;
				}

				Log::D("Press Q to play a WAV, W to play an OGG.");
				return true;
			}
			void Shutdown(Game* game)
			{
				audioMgr->Shutdown();
				resMgr.Shutdown();
			}
			void Update(Game* game, const GameTime& time)
			{
				if(kbd->KeyPressed(KEY_Q))
				{
					wavSound->Play();
				}
				if(kbd->KeyPressed(KEY_W))
				{
					oggSound->Play();
				}
			}
			void Draw(Game* game, const GameTime& time) {}
		};
	}
}
