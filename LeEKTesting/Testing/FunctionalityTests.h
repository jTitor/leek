#ifdef L_ENABLE_OLD_TESTS
#pragma once

namespace LeEK
{
	//Unit tests from WAY back,
	//when there was no framework for game logic.

	namespace FunctionalityTests
	{
		void TestVectors();
		void TestMatrixOps();
		void TestTrig();
		void TestSqrt();
		void TestQuaternions();
		void Test2DFunctions();
		void TestGameTime();
		void TestFileSystem();
		void TestWindowBuild();
		void TestRenderer();
		void TestModels();
		void TestShaders();
		void TestThreads();			//*implementation* incomplete as of 1/27
		void TestHashing();
		void TestDebugDraw();		//incomplete as of 8/12; cylinder doesn't draw right,
									//cone not implemented. I'd say there's no torus either, but that'll be a pain
									//to figure out anyway
		void TestBulletPhysics();
		void TestLogging();
		void TestAllocator();
		void TestStrings();
		void TestRNG();
		void TestAsserts();
		void TestTerminalOps();
		void TestStatMonitoring();
		void TestHandles();

		
	}
}
#endif // L_ENABLE_OLD_TESTS
