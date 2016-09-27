#ifdef L_ENABLE_OLD_TESTS
#include "StdAfx.h"
#include "FunctionalityTests.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
#include "Math/Quaternion.h"
#include "Time/GameTime.h"
#include "Platforms/Win32Platform.h"
//commenting out so it can compile on school computer
//#include <Boost_1_51_0\boost\date_time\posix_time\posix_time.hpp>
//#include "Libraries\Boost_1_51_0\boost\date_time\posix_time\posix_time.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <FileManagement/Filesystem.h>
#include <FileManagement/DataStream.h>
#include <Rendering/Geometry.h>
#include <MultiThreading/StdThreading.h>
#include <MultiThreading/IThreadClient.h>
#include <Hashing/HashMap.h>
#include <Hashing/HashedString.h>
#include <Physics/Physics.h>
#include <Logging/Log.h>
#include <Memory/Allocator.h>
#include <Random/Random.h>
#include <DebugUtils/Assertions.h>
#include <Stats/FPSCounter.h>
#include <Stats/StatMonitor.h>
#include <Structures/Handle.h>
#include <Stats/Profiling.h>
#include "TestObjects.h"
//#include <Strings/LString.h>
//#include <boost\date_time\posix_time\posix_time.hpp>
//#include "Math\MathFunctions.h"
//using namespace LeEK;

using namespace LeEK;

void FunctionalityTests::TestVectors()
{
	using namespace LeEK;
	Vector3 x = Vector3(0.0f, 1.0f, 2.0f);
	Vector3 one = Vector3::One;

	using namespace std;
	Log::D(String("x = ") + x.ToString());
	Log::D(String("one = ") + one.ToString());
	Log::D(String("SqrLen(x) = ") + x.LengthSquared());
	Log::D(String("x . x = ") + x.Dot(x));
	Log::D(String("Up . Right = ") + Vector3::Dot(Vector3::Up, Vector3::Right));
	Log::D(String("x.Cross(x) = ") + x.Cross(x).ToString());
	Log::D(String("Up + Right = ") + (Vector3::Up + Vector3::Right).ToString());
	Log::D(String("Up.Cross(Right) = ") + Vector3::Cross(Vector3::Up, Vector3::Right).ToString());
	Log::D(String("Right.Cross(Up) = ") + Vector3::Cross(Vector3::Right, Vector3::Up).ToString());
	Vector3 sum = x + one;
	Log::D(String("x + one = ") + sum.ToString());
	Log::D(String("x = ") + x.ToString());
	//start testing matrices
	//build two translation matrices
	Matrix4x4 translateMat1 = Matrix4x4(1.0f,	0.0f,	0.0f,	0.0f,
										0.0f,	1.0f,	0.0f,	0.0f,
										0.0f,	0.0f,	1.0f,	0.0f,
										1.0f,	2.0f,	3.0f,	1.0f);
	Matrix4x4 translateMat2 = Matrix4x4(1.0f,	0.0f,	0.0f,	0.0f,
										0.0f,	1.0f,	0.0f,	0.0f,
										0.0f,	0.0f,	1.0f,	0.0f,
										4.0f,	5.0f,	6.0f,	1.0f);
	Matrix4x4 identity = Matrix4x4(	1.0f,	0.0f,	0.0f,	0.0f,
									0.0f,	1.0f,	0.0f,	0.0f,
									0.0f,	0.0f,	1.0f,	0.0f,
									0.0f,	0.0f,	0.0f,	1.0f);
	Log::D(String("T1 = ") + translateMat1.ToString());
	Log::D(String("T2 = ") + translateMat2.ToString());
	Matrix4x4 combination1x2 = translateMat1 * translateMat2;
	Log::D(String("T1 x T2 = ") + combination1x2.ToString());
	Log::D(String("T1 = ") + translateMat1.ToString());
	Log::D(String("T2 = ") + translateMat2.ToString());
	Matrix4x4 combination2x1 = translateMat2 * translateMat1;
	Log::D(String("T2 x T1 = ") + combination2x1.ToString());
	Log::D(String("T1 = ") + translateMat1.ToString());
	Log::D(String("T2 = ") + translateMat2.ToString());
	Log::D(String("T1 x I") + (translateMat1 * identity).ToString());
	Log::D(String("I x T1") + (identity * translateMat1).ToString());
	Log::D(String("Transpose(T1) = ") + translateMat1.GetTranspose().ToString());
	Log::D(translateMat1.MultiplyPoint(x).ToString());
	Log::D(String("x = ") + x.ToString());
	Log::D(translateMat1.MultiplyVector(x).ToString());
	Log::D(String("x = ") + x.ToString());
	Log::D(combination1x2.MultiplyPoint(x).ToString());

	//test length and distance w/ unit vectors
	Vector3 p1 = Vector3(1.0f, 0.0f, 0.0f);
	Vector3 p2 = Vector3(2.0f, 0.0f, 0.0f);
	Vector3 p3 = Vector3(Math::Cos(30.0f*Math::DEG_TO_RAD), Math::Sin(30.0f*Math::DEG_TO_RAD), 0.0f);

	Log::D(String("p1 = ") + p1.ToString());
	Log::D(String("p2 = ") + p2.ToString());
	Log::D(String("p3 = ") + p3.ToString());
	Log::D(String("normalize(p3) = ") + p3.GetNormalized().ToString());
	Log::D(String("p3 = ") + p3.ToString());

	Log::D(String("|p1| = ") + p1.Length());
	Log::D(String("|p2| = ") + p2.Length());
	Log::D(String("Dist(p2, p1) = ") + Vector3::Distance(p1, p2));
	Log::D(String("|p2 - p1| = ") + (p2 - p1).Length());
	Log::D(String("|p2 + p1| = ") + (p2 + p1).Length());
	Log::D(String("|p3|^2 = ") + p3.LengthSquared());
	Log::D(String("|p3| = ") + p3.Length());

	Log::D("normalizing p3");
	p3 = p3.GetNormalized();
	Log::D(String("p3 = ") + p3.ToString());
	Log::D(String("|p3| = ") + p3.Length());

	Log::D("building p3 from rotation matrix");
	p3 = Matrix4x4::BuildZRotation(30.0f).MultiplyPoint(Vector3(1.0f, 0.0f, 0.0f));
	Log::D(String("p3 = ") + p3.ToString());
	Log::D(String("|p3|^2 = ") + p3.LengthSquared());
	Log::D(String("|p3| = ") + p3.Length());

	Log::D("normalizing p3");
	p3 = p3.GetNormalized();
	Log::D(String("p3 = ") + p3.ToString());
	Log::D(String("|p3| = ") + p3.Length());
}

void FunctionalityTests::TestMatrixOps()
{
	using namespace LeEK;
	//test matrix integrity with invertible matrices
	Matrix4x4 A = Matrix4x4(	1.0f, 0.0f, 0.0f, 1.0f,
								0.0f, 2.0f, 1.0f, 2.0f,
								2.0f, 1.0f, 0.0f, 1.0f,
								2.0f, 0.0f, 1.0f, 4.0f);

	Matrix3x3 Q = Matrix3x3(	-1, 3, -3,
								0, -6, 5,
								-5, -3, 1);
	using namespace std;
	cout << "A = " << A.ToString() << endl;
	//print submatrices of A(0,0-3)
	for(int i = 0; i < Matrix4x4::MATRIX_SIZE; i++)
	{
		Matrix3x3 sub = A.GetSubmatrix(0, i);
		cout << "sub(A)(0," << i << ") = S = " << sub.ToString() << endl;
		cout << "det(S) = " << sub.Determinant() << endl;
	}
	//test determinant functions
	cout << "det(A) = " << A.Determinant() << endl;
	cout << "Q = " << Q.ToString() << endl;
	cout << "det(Q) = " << Q.Determinant() << endl;
	//test inversion
	Matrix4x4 AInverse = A.FindInverse();
	//and confirm inversion and matrix mutiplication
	cout << "A^-1 = " << AInverse.ToString() << endl;
	cout << "A * A^-1 = " << (A * AInverse).ToString() << endl;
	cout << "A^-1 * A = " << (AInverse * A).ToString() << endl;

	//test vector-matrix multiplication by rotating a unit x matrix 90, then 45 degrees
	Vector3 v = Vector3::Right;
	Matrix4x4 rot45X = Matrix4x4::BuildXRotation(45.0f*Math::DEG_TO_RAD);
	Matrix4x4 rot90Y = Matrix4x4::BuildYRotation(90.0f*Math::DEG_TO_RAD);
	Matrix4x4 rot45Y = Matrix4x4::BuildYRotation(45.0f*Math::DEG_TO_RAD);
	Matrix4x4 rot90Z = Matrix4x4::BuildZRotation(90.0f*Math::DEG_TO_RAD);
	Matrix4x4 rot45Z = Matrix4x4::BuildZRotation(45.0f*Math::DEG_TO_RAD);

	Matrix4x4 rot90XYZ = Matrix4x4::BuildRotation(90.0f*Math::DEG_TO_RAD, 90.0f*Math::DEG_TO_RAD, 90.0f*Math::DEG_TO_RAD);
	Matrix4x4 rot45XYZ = Matrix4x4::BuildRotation(45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD);

	Matrix4x4 translate = Matrix4x4::BuildTranslation(2.0f, 4.0f, 6.0f);
	//also test matrix combination, by combining the 45 deg rotation matrix with itself and seeing if it gives the same
	//transform as a 90 deg rotation
	cout << "v = " << v.ToString() << endl;
	cout << "v*T= " << translate.MultiplyPoint(v).ToString() << endl;
	cout << "v*T*T = " << (translate * translate).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot90Y = " << rot90Y.MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Y = " << rot45Y.MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Y*Rot45Y = " << (rot45Y * rot45Y).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot90Z = " << rot90Z.MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Z = " << rot45Z.MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45X*Rot45Y*Rot45Z = " << (rot45X * rot45Y * rot45Z).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45X*Rot45Z*Rot45Y = " << (rot45X * rot45Z * rot45Y).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Y*Rot45X*Rot45Z = " << (rot45Y * rot45X * rot45Z).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Y*Rot45Z*Rot45X = " << (rot45Y * rot45Z * rot45X).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Z*Rot45X*Rot45Y = " << (rot45Z * rot45X * rot45Y).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45Z*Rot45Y*Rot45X = " << (rot45Z * rot45Y * rot45X).MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot45XYZ = " << rot45XYZ.MultiplyPoint(v).ToString() << endl;

	//finally, test out of range rotations - rotate by 405 degrees and see if it has the same result as a 45 deg rotation
	Matrix4x4 rot405Y = Matrix4x4::BuildYRotation(405.0f*Math::DEG_TO_RAD);
	cout << "v*Rot45Y = " << rot45Y.MultiplyPoint(v).ToString() << endl;
	cout << "v*Rot405Y = " << rot405Y.MultiplyPoint(v).ToString() << endl;
}

void FunctionalityTests::TestTrig()
{
	using namespace LeEK;
	//we'll test the new trig functions by taking a running average of their absolute error
	F32 sinError = 0.0f;
	F32 cosError = 0.0f;
	F32 leekCallVal = 0.0f;
	F32 cmathCallVal = 0.0f;
	GameTime time;
	F64 leekCalcTime = 0.0;
	F64 cmathCalcTime = 0.0;
	using namespace std;
	Log::D("Testing Math::Sin().");
	time.Tick();
	for(F32 t = -Math::PI; t <= Math::PI; t += (Math::PI / 100.0f))
	{
		time.Tick();
		leekCallVal = Math::Sin(t);
		time.Tick();
		leekCalcTime += time.ElapsedGameTime().ToMilliseconds();
		time.Tick();
		cmathCallVal = sin(t);
		time.Tick();
		cmathCalcTime += time.ElapsedGameTime().ToMilliseconds();
		using namespace std;
		Log::V(String("Math::Sin( ") + t + ")\t= " + leekCallVal);
		Log::V(String("cmath::sin( ") + t + ")\t= " + cmathCallVal);
		Log::V(String("Error\t= ") + (cmathCallVal - leekCallVal));
		//take average of absolute value of absolute error
		sinError += abs(cmathCallVal - leekCallVal);//(sinError + abs(cmathCallVal - leekCallVal)) / 2.0f;
	}
	leekCalcTime /= 100;
	cmathCalcTime /= 100;
	Log::D(String("Average calc time for Math::Sin(): ") + leekCalcTime);
	Log::D(String("Average calc time for cmath::Sin(): ") + cmathCalcTime);
	leekCalcTime = 0.0;
	cmathCalcTime = 0.0;
	Log::D("Testing Math::Cos().");

	for(F32 t = -Math::PI; t <= Math::PI; t += (Math::PI / 100.0f))
	{
		time.Tick();
		leekCallVal = Math::Cos(t);
		time.Tick();
		leekCalcTime += time.ElapsedGameTime().ToMilliseconds();
		time.Tick();
		cmathCallVal = cos(t);
		time.Tick();
		cmathCalcTime += time.ElapsedGameTime().ToMilliseconds();
		using namespace std;
		Log::V(String("Math::Cos( ") + t + ")\t= " + leekCallVal);
		Log::V(String("cmath::cos( ") + t + ")\t= " + cmathCallVal);
		Log::V(String("Error\t= ") + (cmathCallVal - leekCallVal));
		cosError += abs(cmathCallVal - leekCallVal);//(cosError + abs(cmathCallVal - leekCallVal)) / 2.0f;
	}
	leekCalcTime /= 100;
	cmathCalcTime /= 100;
	Log::D(String("Average calc time for Math::Cos(): ") + leekCalcTime);
	Log::D(String("Average calc time for cmath::Cos(): ") + cmathCalcTime);
	leekCalcTime = 0.0;
	cmathCalcTime = 0.0;
	Log::D(String("Average Absolute Error of Sin() = ") + sinError / 100);
	Log::D(String("Average Absolute Error of Cos() = ") + cosError / 100);
}

void FunctionalityTests::TestSqrt()
{
	using namespace LeEK;
	F64 leekDoubleCallVal;
	F32 leekFloatCallVal;
	F64 cmathCallVal;
	F32 sseCallVal;

	F64 doubleError = 0.0;
	F64 floatError = 0.0;
	F64 sseError = 0.0;

	F64 stlTime = 0.0f;
	F64 leekDoubleTime = 0.0f;
	F64 leekFloatTime = 0.0f;
	F64 sseTime = 0.0;

	GameTime time;
	time.Tick();

	using namespace std;
	for(F64 t = 0.0; t <= 1.0; t += 0.01)
	{
		time.Tick();
		leekDoubleCallVal = Math::Sqrt(t);
		time.Tick();
		leekDoubleTime += time.ElapsedGameTime().ToMilliseconds();
		time.Tick();
		leekFloatCallVal = Math::Sqrt((float)t);
		time.Tick();
		leekFloatTime += time.ElapsedGameTime().ToMilliseconds();
		time.Tick();
		cmathCallVal = sqrt(t);
		time.Tick();
		stlTime += time.ElapsedGameTime().ToMilliseconds();
		time.Tick();
		sseCallVal = Math::sseSqrt(t);
		//Log::D(StrFromVal(sseCallVal));
		time.Tick();
		sseTime += time.ElapsedGameTime().ToMilliseconds();
		using namespace std;
#ifdef VERBOSE
		cout << "Math::(double)Sqrt( " << t << ")\t= " << leekDoubleCallVal << endl;
		cout << "Math::(float)Sqrt( " << t << ")\t= " << leekFloatCallVal << endl;
		cout << "cmath::sqrt( " << t << ")\t= " << cmathCallVal << endl;
		cout << "Double Error\t= " << cmathCallVal - leekDoubleCallVal << endl;
		cout << "Float Error\t= " << cmathCallVal - leekFloatCallVal << endl;
#endif
		//take average of absolute value of absolute error
		doubleError += abs(cmathCallVal - leekDoubleCallVal);//(doubleError + abs(cmathCallVal - leekDoubleCallVal)) / 2.0;
		floatError += abs(cmathCallVal - leekFloatCallVal);//(floatError + abs(cmathCallVal - leekFloatCallVal)) / 2.0;
		sseError += abs(cmathCallVal - sseCallVal);
	}

	Log::D(String("Average Double Error\t= ") + (doubleError / 100));
	Log::D(String("Average Float Error\t= ") + (floatError / 100));
	Log::D(String("Average SSE Error\t= ") + (sseError / 100));
	Log::D(String("Average STL Time =\t") + (stlTime / 100));
	Log::D(String("Average LeEK Double Time =\t") + (leekDoubleTime / 100));
	Log::D(String("Average LeEK Float Time =\t") + (leekFloatTime / 100));
	Log::D(String("Average SSE Time =\t") + (sseTime / 100));
}

void FunctionalityTests::TestQuaternions()
{
	using namespace LeEK;
	//test length and distance w/ unit vectors
	Vector3 v = Vector3::Right;
	Matrix4x4 mRot45X = Matrix4x4::BuildXRotation(45.0f*Math::DEG_TO_RAD);
	Matrix4x4 mRot45Y = Matrix4x4::BuildYRotation(45.0f*Math::DEG_TO_RAD);
	Matrix4x4 mRot45Z = Matrix4x4::BuildZRotation(45.0f*Math::DEG_TO_RAD);

	Matrix4x4 mRot45XYZ = Matrix4x4::BuildRotation(45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD);

	Quaternion qRot45X = Quaternion::BuildXRotation(45.0f*Math::DEG_TO_RAD);
	Quaternion qRot45Y = Quaternion::BuildYRotation(45.0f*Math::DEG_TO_RAD);
	Quaternion qRot45Z = Quaternion::BuildZRotation(45.0f*Math::DEG_TO_RAD);
	using namespace std;
	LogD("qRot45X = " + qRot45X.ToString());
	LogD("qRot45Y = " + qRot45Y.ToString());
	LogD("qRot45Z = " + qRot45Z.ToString());

	LogD("building XYZ quaternion");
	Quaternion qRot45XYZ = Quaternion::BuildRotation(45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD, 45.0f*Math::DEG_TO_RAD);//qRot45X * qRot45Y * qRot45Z;
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
	//LogD("v*qRot45ZYX = " + qRot45ZYX.RotateVector(v).ToString());
	LogD("v*qRot45X*qRot45Y*qRot45Z = " + qRot45Z.RotateVector(qRot45Y.RotateVector(qRot45X.RotateVector(v))).ToString());
	LogD("v*qFromMat(mRot45XYZ) = " + Quaternion::FromMatrix(mRot45XYZ).RotateVector(v).ToString()); //Doesn't work! TODO: Fix Quaternion::FromMatrix()

	LogD("v = " + v.ToString());

	LogD("mRot45Y = " + mRot45Y.ToString());
	LogD("mat(qRot45Y) = " + qRot45Y.ToMatrix().ToString());
	LogD("mRot45XYZ = " + mRot45XYZ.ToString());
	LogD("mat(qRot45XYZ) = " + qRot45XYZ.ToMatrix().ToString());
	LogD("mat(qRot45ZYX) = " + qRot45ZYX.ToMatrix().ToString());

	//test half angles
	F32 qCos = 2.0f * pow(Math::Cos(45.0f*Math::DEG_TO_RAD / 2.0f), 2) - 1.0f;
	LogD(String("qCos = ") + qCos);
	F32 mCos = Math::Cos(45.0f*Math::DEG_TO_RAD);
	LogD(String("mCos = ") + mCos);
}

void FunctionalityTests::Test2DFunctions()
{
	using namespace LeEK;
	//test length, operations, and arithmetic
	Vector2 r = Vector2::Right;
	Vector2 u = Vector2::Up;
	using namespace std;
	cout << "r = " << r.ToString() << endl;
	cout << "u = " << u.ToString() << endl;
	cout << "r + u = " << (r + u).ToString() << endl;
	cout << "|r| = " << r.Length() << endl;
	cout << "|u| = " << u.Length() << endl;
	cout << "|r + u| = " << (r + u).Length() << endl;
	cout << "r . u = " << r.Dot(u) << endl;
	//test matrix operations and transforms
	Matrix3x3 T = Matrix3x3::BuildTranslation(0, 1);
	Matrix3x3 R = Matrix3x3::BuildRotation2D(45.0f*Math::DEG_TO_RAD);
	Matrix3x3 S = Matrix3x3::BuildUniformScale2D(2.0f);
	Matrix3x3 I = Matrix3x3::Identity;
	Vector2 res = T.MultiplyPoint(r);
	cout << "det(I) = " << I.Determinant() << endl;
	cout << "T(0,1)*r = " << res.ToString() << endl;
	res = R.MultiplyPoint(r);
	cout << "R(45)*r = " << res.ToString() << endl;
	res = S.MultiplyPoint(r);
	cout << "S(2)*r = " << res.ToString() << endl;
	res = (T*R*S).MultiplyPoint(Vector2::Zero);
	cout << "Build point from T(0,1)R(45)S(2) = " << res.ToString() << endl;
	res = (S*R*T).MultiplyPoint(Vector2::Zero);
	cout << "Build point from S(2)R(45)T(0,1) = " << res.ToString() << endl;
}

void FunctionalityTests::TestGameTime()
{
	using namespace LeEK;
	GameTime gTime = GameTime();
	gTime.Tick();
	boost::posix_time::ptime testStart = boost::posix_time::second_clock::local_time();
	F64 elapsedTime = 0.0; //elapsed time in seconds
	while(elapsedTime < 1000.0f)
	{
		//Sleep(1000);
		gTime.Tick();
		//std::cout << gTime.ElapsedGameTime().ToSeconds() << std::endl;
		elapsedTime += gTime.ElapsedGameTime().ToMilliseconds();
	}
	//once one second has elapsed, get the current time according to boost
	//std::cout << gTime.ElapsedGameTime().ToMilliseconds() << std::endl;
	boost::posix_time::ptime testEnd = boost::posix_time::second_clock::local_time();
	Log::D(String("Elapsed time according to GameTime:\t") + elapsedTime);
	Log::D(String("Total time according to GameTime:\t") + gTime.TotalGameTime().ToMilliseconds());
	Log::D(String("Elapsed time according to Boost::posix_time:\t") + (F64)(testEnd - testStart).total_milliseconds());
	Log::D(String("Elapsed time according to Windows:\t") + 1000);
	//next, compare speed of retrieving a Duration and of retrieving a Boost ptime
	F64 gtRetrieveTime = 0.0;
	F64 boostRetrieveTime = 0.0;
	F64 tgtRetrieveTime = 0.0;
	F64 tickTime = 0;
	int n = 10000;
	for(int i = 0; i < n; i++)
	{
		gTime.Tick();
		Duration gtTime = gTime.ElapsedGameTime();
		gTime.Tick();
		gtRetrieveTime += gTime.ElapsedGameTime().ToMilliseconds();
		gTime.Tick();
		boost::posix_time::ptime boostTime = boost::posix_time::second_clock::local_time();
		gTime.Tick();
		boostRetrieveTime += gTime.ElapsedGameTime().ToMilliseconds();
		gTime.Tick();
		DWORD tgtTime = timeGetTime();
		gTime.Tick();
		tgtRetrieveTime += gTime.ElapsedGameTime().ToMilliseconds();
		gTime.Tick();
		gTime.Tick();
		//not sure this is even remotely valid
		tickTime = (tickTime+gTime.ElapsedGameTime().ToMilliseconds())/2;
	}
	Log::D(String("Average time to retrieve a Duration (ms):\t") + (gtRetrieveTime / n));
	Log::D(String("Average time to retrieve a Boost::posix_time::ptime (ms):\t") + (boostRetrieveTime / n));
	Log::D(String("Average time to retrieve time from Windows(timeGetTime) (ms):\t") + (tgtRetrieveTime / n));
	Log::D(String("Average time to tick GameTime maybe (ms):\t") + tickTime);
	Log::D("(Not sure that last one's really valid)");
	Log::D(String("Size of GameTime:\t") + sizeof(GameTime));
	Log::D(String("Size of Duration:\t") + sizeof(Duration));
	Log::D(String("Size of Boost::posix_time::ptime:\t") + sizeof(boost::posix_time::ptime));
	Log::D(String("Size of DWORD:\t") + sizeof(DWORD));
}

void FunctionalityTests::TestFileSystem()
{
	using namespace LeEK;
	/*
	FILE *fp;
	fp=fopen(".\\test.bin", "wb");
	char x[10]="ABCDEFGHI";
	fwrite(x, sizeof(x), 1, fp);
	fclose(fp);
	*/
	std::cout << Filesystem::GetCurrentPath() << "\n";
	//try making a directory tree
	if(!Filesystem::MakeDirectory(Path("./Test1/Test2")))
	{
		std::cout << String("Couldn't write directory ") + Filesystem::GetCurrentPath() + "/Test2" + " !" << "\n";
		return;
	}
	std::cout << "Created directory!" << "\n";
	//now try writing a (presumably nonexistent) file in that directory
	Path f1 = Path("./file1.txt");
	DataStream* f1Stream = Filesystem::OpenFile(f1);
	if(!f1Stream)
	{
		std::cout << "Couldn't create file " + f1.ToString() + " !" << "\n";
		return;
	}
	std::cout << "Opened file " + f1.ToString() << "\n";
	//now delete the file, in case a prior test left it here
	f1Stream->Close();
	if(Filesystem::RemoveFile(f1))
	{
		std::cout << "Deleted file " + f1.ToString() + "!" << "\n";
	}
	CustomDelete(f1Stream);
	f1Stream = Filesystem::OpenFile(f1);
	//now try to write data to the file
	String s = "The quick brown fox jumps over the lazy dog.";
	f1Stream->Write(s.c_str());
	f1Stream->Close();
	std::cout << "Wrote a line!" << "\n";
	//now read the file
	
	CustomDelete(f1Stream);
	f1Stream = Filesystem::OpenFile(f1);
	if(!f1Stream)
	{
		std::cout << "Couldn't open file \"./Test1/Test2/file1.txt\"!" << "\n";
		return;
	}
	//char* readString = new char[Filesystem::GetFileSize(f1) / sizeof(char)];
	//f2Stream->Read(readString, Filesystem::GetFileSize(f1) / sizeof(char));
	//std::cout << "Reading line..." << "\n";
	//std::cout << readString << "\n";

	//add a new line.
	f1Stream->Write("\n\tFew black taxis drive up major roads on quiet hazy nights.");
	f1Stream->Close();
	std::cout << "Wrote another line!" << "\n";
	CustomDelete(f1Stream);
	f1Stream = Filesystem::OpenFile(f1);
	if(!f1Stream)
	{
		std::cout << "Couldn't open file \"./Test1/Test2/file1.txt\"!" << "\n";
		return;
	}
	//delete [] readString;
	char* readString = f1Stream->ReadAll();
	std::cout << "Reading lines..." << "\n";
	//display the string
	std::cout << readString << "\n";
	//now try displaying it as a string object
	std::cout << "As LeEK::String: \n" + String(readString) << "\n";
	f1Stream->Close();
	//try reading an existing file now
	CustomDelete(readString); // delete [] readString;
	Path f2("./ASCII_Test.txt");
	DataStream* f2Stream = Filesystem::OpenFile(f2);
	if(!f2Stream)
	{
		std::cout << "Couldn't create file " + f2.ToString() + " !" << "\n";
		return;
	}
	readString = f2Stream->ReadAll(); //NOTE: buffers need to be null terminated, files won't have the null in them
	std::cout << "Reading lines..." << "\n";
	std::cout << readString << "\n";
	f2Stream->Close();
}

void FunctionalityTests::TestWindowBuild()
{
	using namespace LeEK;
	IPlatform* win32 = IPlatform::GetPlatformInstance();//Win32Platform::GetInstance();
	win32->Startup();
	win32->GetWindow(NULL, Win32Platform::WINDOW, 1024, 768);
	bool shouldQuit = false;
	while(!shouldQuit)
	{
		shouldQuit = win32->UpdateOS();
	}
	win32->Shutdown();
}

void FunctionalityTests::TestRenderer()
{
	using namespace LeEK;
	IPlatform* win32 = IPlatform::GetPlatformInstance();
	win32->Startup();
	OGLGrpWrapper* oglRenderer = (OGLGrpWrapper*)win32->BuildGraphicsWrapper(OPEN_GL);
	//if the window was built properly, enter a mock game loop
	if(win32->GetWindow(oglRenderer, Win32Platform::WINDOW, 1024, 768))
	{
		bool shouldQuit = false;
		while(!shouldQuit)
		{
			shouldQuit = win32->UpdateOS();
			win32->BeginRender(oglRenderer);
			//clear to a tealish color
			oglRenderer->Clear(Color(0.0f, 1.0f, 0.5f));
			win32->EndRender(oglRenderer);
		}
		win32->ShutdownGraphicsWrapper(oglRenderer);
		win32->Shutdown();
	}
	else
	{
		std::cout << "Failed to build window!\n";
		win32->Shutdown();
	}
}

void FunctionalityTests::TestModels()
{
	//try to create a triangle
	Vector3 triPos[] = {	Vector3(-1.0f, -1.0f, 0.0f),  //bottom left
							Vector3(0.0f, 1.0f, 0.0f),  //top middle
							Vector3(1.0f, -1.0f, 0.0f)}; //bottom right
	Color triColor[] = {	Color(1.0f, 0.0f, 0.0f),  //bottom left
							Color(0.0f, 1.0f, 0.0f),  //top middle
							Color(0.0f, 0.0f, 1.0f)}; //bottom right
	U32 indices[] = {	0,  //bottom left
						1,  //top middle
						2}; //bottom right
	Geometry tri;
	bool result = GeomHelpers::BuildGeometry(tri, triPos, NULL, triColor, NULL, indices, 3, 3, 0);
	if(!result)
	{
		std::cout << "Failed to build triangle!\n";
		return;
	}
	else
	{
		std::cout << "Built triangle!\n";
	}
	//IPlatform::InitInstance();
	IPlatform* win32 = IPlatform::GetPlatformInstance();
	win32->Startup();
	OGLGrpWrapper* oglRenderer = (OGLGrpWrapper*)win32->BuildGraphicsWrapper(OPEN_GL);

	//if the window was built properly, we can prep OpenGL data
	if(win32->GetWindow(oglRenderer, Win32Platform::WINDOW, 1024, 768))
	{
		bool shouldQuit = false;
		//now that we have a graphics wrapper, try to load shaders and the triangle
		std::cout << "Loading shaders...\n";
		
		//if(!oglRenderer->ProgramFromShaderPair(	"Color",
		//										Path("./Shaders/Basic/Color.vp"), 
		//										Path("./Shaders/Basic/Color.fp"), 
		//										0))//, 
												//POSITION, "inputPosition", 
												//COLOR, "inputColor"))
		if(!oglRenderer->MakeShader("Color", 2, 
									VERTEX, Path("./Shaders/Basic/Color.vp"), 
									FRAGMENT, Path("./Shaders/Basic/Color.fp")))
		{
			std::cout << "Failed to build shaders!\n";
			win32->ShutdownGraphicsWrapper(oglRenderer);
			oglRenderer->ShutdownMesh(tri);
			win32->Shutdown();
			return;
		}
		if(!oglRenderer->SetShader("Color"))
		{
			std::cout << "Couldn't assign shader!\n";
			win32->ShutdownGraphicsWrapper(oglRenderer);
			oglRenderer->ShutdownMesh(tri);
			win32->Shutdown();
			return;
		}
		std::cout << "Loading triangle...\n";
		if(!oglRenderer->InitGeometry(tri))
		{
			std::cout << "Failed to load triangle into OpenGL!\n";
			win32->ShutdownGraphicsWrapper(oglRenderer);
			oglRenderer->ShutdownMesh(tri);
			win32->Shutdown();
			return;
		}

		//now try to build the matrices,
		//since we don't have anything to store them in currently
		Matrix4x4 world, view, projection;
		//move the mesh in front of the screen
		Vector3 transVec(-1.0f, 0.0f, 0.0f);
		world = Matrix4x4::BuildTranslation(transVec);//0.0f, 0.0f, 0.0f);
		//projection is the basic defaults
		std::cout << "Aspect Ratio: " << oglRenderer->ScreenAspect() << "\n";
		projection = Matrix4x4::BuildPerspectiveRH(oglRenderer->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
		//we don't have a camera class yet; ordinarily it would handle this
		view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, -3.0f), //place it 3 units forward
												Vector3(0.0f, 0.0f, 0.0f), //by default, look at the origin
												Vector3::Up); //and use default up direction for now
		//now try assigning the matrices as uniforms
		F32 worldArray[16];
		F32 viewArray[16];
		F32 projArray[16];
		world.CopyToBuffer(worldArray);
		view.CopyToBuffer(viewArray);
		projection.CopyToBuffer(projArray);
		while(!shouldQuit)
		{
			shouldQuit = win32->UpdateOS();
			//print any errors
			oglRenderer->PrintShaderStatus();
			win32->BeginRender(oglRenderer);
			//clear to a dim color
			oglRenderer->Clear(Colors::Gray20Pct);
			//try drawing the mesh
			//oglRenderer->SetWorldViewProjection(worldArray, viewArray, projArray);
			if(!oglRenderer->SetWorldViewProjection(world, view, projection))
			{
				std::cout << "Couldn't set WVP matrices!\n";
			}
			oglRenderer->Draw(tri);
			win32->EndRender(oglRenderer);
		}
		win32->ShutdownGraphicsWrapper(oglRenderer);
		oglRenderer->ShutdownMesh(tri);
		win32->Shutdown();
	}
	else
	{
		std::cout << "Failed to build window!\n";
		win32->Shutdown();
		return;
	}
}

/*void FunctionalityTests::TestThreads()
{
	using namespace LeEK;
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
	std::cout << "x = " << x << "\n";
	//run the threads here
	noMutex testCodeA(&x);
	noMutex testCodeB(&x);
	Thread threadA(&testCodeA);
	Thread threadB(&testCodeB);
	std::cout << "Attempting to run threads...\n";
	threadA.Start();
	threadB.Start();
	threadA.Join();
	threadB.Join();
	std::cout << "Threads are now finished. x = " << x << "\n";
	//reset values
	x = 0;
	std::cout << "Let's try again. x = " << x << "\n";
	Thread threadA2(&testCodeA);
	Thread threadB2(&testCodeB);
	std::cout << "Attempting to run threads...\n";
	threadA2.Start();
	threadB2.Start();
	threadA2.Join();
	threadB2.Join();
	std::cout << "Threads are now finished. x = " << x << "\n";
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
			std::cout << "Attempting to lock mutex...\n";
			ILock lock = m->GetLock();
			std::cout << "Locked mutex!\n";
			const F32 delta = 0.001f;
			for(float i = 0.0f; i < Math::PI_OVER_2; i += delta)
			{
				(*x) += Math::Sin(i) * delta;
			}
			std::cout << "Unlocking mutex.\n";
		}
	};
	std::cout << "Let's try with mutexes. x = " << x << "\n";
	withMutex testCodeC(&x, mut);
	withMutex testCodeD(&x, mut);
	Thread threadC(&testCodeC);
	Thread threadD(&testCodeD);
	std::cout << "Attempting to run threads...\n";
	threadC.Start();
	threadD.Start();
	threadC.Join();
	threadD.Join();
	std::cout << "Threads are now finished. x = " << x << "\n";
	//reset values
	x = 0;
	std::cout << "Let's try again. x = " << x << "\n";
	Thread threadC2(&testCodeC);
	Thread threadD2(&testCodeD);
	std::cout << "Attempting to run threads...\n";
	threadC2.Start();
	threadD2.Start();
	threadC2.Join();
	threadD2.Join();
	std::cout << "Threads are now finished. x = " << x << "\n";
}*/

void FunctionalityTests::TestHashing()
{
	//show the keys, the hashed names, then hashed string versions, then practice storing the data
	//compare with stl map
	String keys[] = { "Rocket Man", "Down Under", "Thriller" };
	//String key1 = "Rocket Man";
	//String key2 = "Down Under";
	//String key3 = "Thriller";
	HashedString hKeys[] = { HashedString(keys[0]), HashedString(keys[1]), HashedString(keys[2]) };
	//HashedString hKey1(key1);
	//HashedString hKey2(key2);
	//HashedString hKey3(key3);
	for(int i = 0; i < 3; i++)
	{
		std::cout << "Key " << i << ": " << keys[i] << "; hash value: " << Hash(keys[i]) << "\n";
		std::cout << "Hashed Key " << i << ": " << hKeys[i].OriginalString() << "; hash value: " << hKeys[i].Value() << "\n";
	}
	GameTime time = GameTime();
	time.Tick();

	//now set the values
	std::map<String, String> map;
	std::cout << "Testing std::map...\n";
	time.Tick();
	map[keys[0]] = "Shadow";
	map[keys[1]] = "Silver";
	map[keys[2]] = "Sonic";
	time.Tick();
	F64 mapBuildTime = time.ElapsedGameTime().ToMilliseconds();
	std::cout << "std::map built in " << mapBuildTime << "ms\n";
	//time the access speed
	F64 elAccessTime = 0;
	F64 avgAccessTime = 0;
	U32 sampleCount = 1000;
	for(U32 i = 0; i < 3; i++)
	{
		for(U32 j = 0; j < sampleCount; j++)
		{
			time.Tick();
			String value = map[keys[i]];
			time.Tick();
			elAccessTime += time.ElapsedGameTime().ToMilliseconds();
		}
		String value = map[keys[i]];
		avgAccessTime = elAccessTime / sampleCount;
		std::cout << "map[" << keys[i] << "]: " << value << "; accessed in " << avgAccessTime << "ms\n";
	}
	//test hash map
	HashMap<String> hashMap;
	std::cout << "Testing HashMap...\n";
	time.Tick();
	hashMap[keys[0]] = "Shadow";
	hashMap[keys[1]] = "Silver";
	hashMap[keys[2]] = "Sonic";
	time.Tick();
	F64 hashMapBuildTime = time.ElapsedGameTime().ToMilliseconds();
	std::cout << "HashMap built in " << hashMapBuildTime << "ms\n";
	elAccessTime = 0;
	for(U32 i = 0; i < 3; i++)
	{
		for(U32 j = 0; j < sampleCount; j++)
		{
			time.Tick();
			String value = hashMap[keys[i]];
			time.Tick();
			elAccessTime += time.ElapsedGameTime().ToMilliseconds();
		}
		String value = hashMap[keys[i]];
		avgAccessTime = elAccessTime / sampleCount;
		std::cout << "hashMap[" << keys[i] << "]: " << value << "; accessed in " << avgAccessTime << "ms\n";
	}
	//test hash map with hashed strings
	HashMap<String> hashMapViaHStr;
	std::cout << "Testing HashMap with HashedStrings...\n";
	time.Tick();
	hashMapViaHStr[hKeys[0]] = "Shadow";
	hashMapViaHStr[hKeys[1]] = "Silver";
	hashMapViaHStr[hKeys[2]] = "Sonic";
	time.Tick();
	F64 hashMapViaHStrBuildTime = time.ElapsedGameTime().ToMilliseconds();
	std::cout << "hashMapViaHStr built in " << hashMapViaHStrBuildTime << "ms\n";
	elAccessTime = 0;
	for(U32 i = 0; i < 3; i++)
	{
		for(U32 j = 0; j < sampleCount; j++)
		{
			time.Tick();
			String value = hashMapViaHStr[hKeys[i].Value()];
			time.Tick();
			elAccessTime += time.ElapsedGameTime().ToMilliseconds();
		}
		String value = hashMapViaHStr[hKeys[i]];
		avgAccessTime = elAccessTime / sampleCount;
		std::cout << "hashMapViaHStr[" << hKeys[i].OriginalString() << "]: " << value << "; accessed in " << avgAccessTime << "ms\n";
	}
	Log::D(String("Size of HashMap: ") + sizeof(hashMap));
}

void FunctionalityTests::TestDebugDraw()
{
	IPlatform* win32 = IPlatform::GetPlatformInstance();
	win32->Startup();
	OGLGrpWrapper* oglRenderer = (OGLGrpWrapper*)win32->BuildGraphicsWrapper(OPEN_GL);

	//if the window was built properly, we can prep OpenGL data
	if(win32->GetWindow(oglRenderer, Win32Platform::WINDOW, 1024, 768))
	{
		bool shouldQuit = false;

		//now try to build the matrices,
		//since we don't have anything to store them in currently
		Matrix4x4 world, view, projection;
		//move the mesh in front of the screen
		world = Matrix4x4::Identity;//Matrix4x4::BuildTranslation(0.0f, 0.0f, 3.0f);
		//projection is the basic defaults
		std::cout << "Aspect Ratio: " << oglRenderer->ScreenAspect() << "\n";
		projection = Matrix4x4::BuildPerspectiveRH(oglRenderer->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
		//we don't have a camera class yet; ordinarily it would handle this
		view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, -3.0f),//Vector3(1.0f, 1.0f, -3.0f), //place it 3 units forward
												Vector3(0.0f, 0.0f, 0.0f), //by default, look at the origin
												Vector3::Up); //and use default up direction for now
		//now try assigning the matrices as uniforms
		F32 worldArray[16];
		F32 viewArray[16];
		F32 projArray[16];
		world.CopyToBuffer(worldArray);
		view.CopyToBuffer(viewArray);
		projection.CopyToBuffer(projArray);
		while(!shouldQuit)
		{
			shouldQuit = win32->UpdateOS();
			//print any errors
			oglRenderer->PrintShaderStatus();
			win32->BeginRender(oglRenderer);
			//clear to a dim color
			oglRenderer->Clear(Colors::Gray20Pct);
			//try drawing a line first
			//oglRenderer->SetWorldViewProjection(worldArray, viewArray, projArray);
			if(!oglRenderer->SetWorldViewProjection(world, view, projection))
			{
				std::cout << "Couldn't set WVP matrices!\n";
			}
			//oglRenderer->DebugDrawLine(Vector3(-1.0f, -1.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f), Colors::Red);
			//then draw a box
			oglRenderer->DebugDrawBox(	Vector3(-1.0f, 0.0f, 0.0f), 
										Quaternion::BuildRotation(0.0f, Math::PI_OVER_4, 0.0f),
										0.5f,
										0.5f,
										0.5f,
										Colors::Blue);
			//oglRenderer->DebugDrawSphere(Vector3::Zero, 1.0f, Colors::Yellow);
			//then draw a sphere!
			win32->EndRender(oglRenderer);
		}
		win32->ShutdownGraphicsWrapper(oglRenderer);
		win32->Shutdown();
	}
	else
	{
		std::cout << "Failed to build window!\n";
		win32->Shutdown();
		return;
	}
}

void FunctionalityTests::TestBulletPhysics()
{
	//setup windows, then wrapper, then physics
	IPlatform* win32 = IPlatform::GetPlatformInstance();
	win32->Startup();
	OGLGrpWrapper* oglRenderer = (OGLGrpWrapper*)win32->BuildGraphicsWrapper(OPEN_GL);

	PhysicsWorld physics;
	physics.Startup();
	Log::D("Physics world initialized!");

	//then setup debug physics renderer
	DebugDrawer debugDrawer(oglRenderer);
	debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawAabb);
	physics.AttachDebugDrawer(&debugDrawer);
	Log::D("Attached debug drawer to physics world");

	//then spawn a box
	BoxCollShape boxCollider(1.0f, 0.5f, 1.0f);
	CompoundCollShape colliderHolder;
	colliderHolder.AddShape(&boxCollider, Vector3::Zero, Quaternion::Identity);
	RigidBody boxBody(1.0f, Vector3::Zero, colliderHolder);

	//now attach the box to the physics world!
	physics.AttachRigidBody(&boxBody);
	Log::D("Attached box to physics world");
	Vector3 grav = Vector3(0.0f, -9.8f, 0.0f);
	physics.SetGravityVector(grav);

	//spawn a floor, too
	BoxCollShape floorCollider(50, 20, 50);
	CompoundCollShape floorColliderHolder;
	floorColliderHolder.AddShape(&floorCollider, Vector3::Zero, Quaternion::Identity);
	RigidBody floorBody(0.0f, Vector3(0, -50, 0), floorColliderHolder);
	physics.AttachRigidBody(&floorBody);
	//we'll also need a timer to pass deltas to physics

	GameTime gameTime;
	gameTime.Tick();

	if(win32->GetWindow(oglRenderer, Win32Platform::WINDOW, 1024, 768))
	{
		bool shouldQuit = false;

		Matrix4x4 world, view, projection;
		world = Matrix4x4::Identity;//Matrix4x4::BuildTranslation(boxPosition);
		Log::D(String("Aspect Ratio: ") + oglRenderer->ScreenAspect());
		projection = Matrix4x4::BuildPerspectiveRH(oglRenderer->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
		//we don't have a camera class yet; ordinarily it would handle this
		view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, -3.0f),//place it 3 units forward
												boxBody.Position(),//looking at the box
												Vector3::Up); //and use default up direction for now

		//now try assigning the matrices as uniforms
		F32 worldArray[16];
		F32 viewArray[16];
		F32 projArray[16];
		world.CopyToBuffer(worldArray);
		view.CopyToBuffer(viewArray);
		projection.CopyToBuffer(projArray);

		//enter the game loop
		while(!shouldQuit)
		{
			shouldQuit = win32->UpdateOS();
			gameTime.Tick();
			physics.Update(gameTime);
			//print any errors
			oglRenderer->PrintShaderStatus();
			//update the camera's view to look at the box
			view = Matrix4x4::BuildViewLookAtRH(Vector3(0.0f, 0.0f, -3.0f),
												boxBody.Position(),
												Vector3::Up);
			win32->BeginRender(oglRenderer);

			//clear to a dim color
			oglRenderer->Clear(Colors::Gray20Pct);
			if(!oglRenderer->SetWorldViewProjection(world, view, projection))
			{
				Log::D("Couldn't set WVP matrices!");
			}
			physics.DebugDraw();

			win32->EndRender(oglRenderer);
		}
		win32->ShutdownGraphicsWrapper(oglRenderer);
		win32->Shutdown();
	}
	else
	{
		Log::E("Failed to build window!");
		win32->Shutdown();
		return;
	}
}

F32 testLoggingValueFromFunction()
{
	return 47.0f;
}

void FunctionalityTests::TestLogging()
{
	//Since Log doesn't dump to files yet, 
	//we can't test how a log file would look yet.
	Log::SetVerbosity(Log::Verbosity::INFO);
	//Log::ClearTagFilter();
	//display current priority, should be Info (4)
	std::cout << "Current verbose level is: " << Log::GetVerbosity() << "\n";
	Log::E("Error message!");
	Log::W("Warning message!");
	Log::I("Info!");
	//these shouldn't display!
	Log::D("Debug!");
	Log::V("Verbose message!");

	//now raise to full verbosity
	Log::SetVerbosity(Log::Verbosity::VERB);
	std::cout << "Current verbose level is now: " << Log::GetVerbosity() << "\n";
	//now these should all display
	Log::E("Another error!");
	Log::W("More Warnings!");
	Log::I("Nag line!");
	Log::D("More data!");
	Log::V("Verbose again!");

	//concatenation won't work like with std::cout, since this is taking
	//one string, rather than a series of stream writes
	//have to build the string before writing it
	Log::I("Let's try concatenation:");
	Log::I(String("A = ") + 1);
	//Log::I(String("Float = ") + 1.0f);
	Log::I(String("Word 1, ") + String("Word 2"));
	Log::I(String("Value from function: ") + testLoggingValueFromFunction() + ", also here's a string literal!");
}

void FunctionalityTests::TestAllocator()
{
	U32* x = CustomNew<U32>(0, "Test");
	CustomDelete(x);
	//Delete(x);

	//try templated versions
	Log::D("Attempting allocation via templated overrides...");
	U64* y = CustomNew<U64>(0, "TemplatedTest", 3); //NICE!
	CustomDelete(y);
	//U64 z = CustomNew<U64[4]>(0, "TemplatedArrayTest");
	U64* z = CustomArrayNew<U64>(4, 0, "ArrayTest");//CustomArrayNew<U64>(4, 0, "Test");//CustomNew<U64[]>(0, "TemplatedArrayTest");
	CustomDelete(z);

	TestParent* to1 = CustomNew<TestParent>(0, "ObjAllocTest");
	CustomDelete(to1);

	TestChild* tc1 = CustomNew<TestChild>(0, "InheritAllocTest");
	CustomDelete(tc1);

	TestParent* tc2 = CustomNew<TestChild>(0, "PolyAllocTest");
	CustomDelete(tc2);
}

void FunctionalityTests::TestStrings()
{
	Log::SetVerbosity(Log::VERB);
	//Log::ClearTagFilter();
	String strA("this is a String");
	std::cout << strA << "\n";
	String strB = "this is another String";
	std::cout << strB << "\n";
	String strC = "this ISN'T cast from a std::string ";
	std::cout << strC << "\n";
	std::string stdStr = "and this is a std::string! ";
	std::cout << stdStr << "\n";
	strA = "This is ";
	std::cout << strA << "\n";
	strB = "two concatenated strings";
	std::cout << strB << "\n";
	std::cout << strA+strB << "\n";
	std::cout << strC+stdStr << "\n";
	std::cout << stdStr+strC << "\n";
	std::cout << "Let's access the String's c_str: \n";
	std::cout << strC.c_str() << "\n";
	std::cout << "Let's get stupid!\n";
	strC = "This is coming from a String!";
	Log::D(strC);
	Log::D(String("This is a native concat: ") + 8492);
	Log::D(8492 + String(" ...And a lhs concat!"));
	Log::D(String("A float: ") + 1.1618f + ", and an int: " + 8675309);
}

void FunctionalityTests::TestRNG()
{
	const U32 NUM_TESTS = 8492;//INT_MAX - 1;
	//test the int system
	I32 intMin = -65536;
	I32 intMax = 65536;
	bool passed = true;
	F64 avgTime = 0;
	GameTime time;
	//get a value to prime the running average
	time.Tick();
	I32 intPrimeVal = Random::InRange(intMin, intMax);
	time.Tick();
	avgTime = time.ElapsedGameTime().ToMilliseconds();
	//thank god this is < NUM_TESTS, else this could be an infloop
	for(U32 i = 0; i < NUM_TESTS; i++)
	{
		time.Tick();
		I32 rngVal = Random::InRange(intMin, intMax);
		time.Tick();
		avgTime = (avgTime+time.ElapsedGameTime().ToMilliseconds())/2;
		//Log::V(StrFromVal(rngVal));
		if(rngVal >= intMax)
		{
			Log::E("RNG failed for I32! Value too large!");
			passed = false;
			break;
		}
		if(rngVal < intMin)
		{
			Log::E("RNG failed for I32! Value too small!");
			passed = false;
			break;
		}
	}
	if(passed)
	{
		Log::D(String("RNG passed for I32. Average time to get value (ms): ") + avgTime);
	}
	//test the float system
	//remember floats can be in [min, max]
	F32 fltMin = -65536.0f;
	F32 fltMax = 65536.0f;
	passed = true;
	avgTime = 0;
	//prime the running average again
	time.Tick();
	F32 fltPrimeVal = Random::InRange(fltMin, fltMax);
	time.Tick();
	avgTime = time.ElapsedGameTime().ToMilliseconds();
	for(U32 i = 0; i < NUM_TESTS; i++)
	{
		time.Tick();
		F32 rngVal = Random::InRange(fltMin, fltMax);
		time.Tick();
		avgTime = (avgTime+time.ElapsedGameTime().ToMilliseconds())/2;
		if(rngVal > fltMax)
		{
			Log::E("RNG failed for F32! Value too large!");
			passed = false;
			break;
		}
		if(rngVal < fltMin)
		{
			Log::E("RNG failed for F32! Value too small!");
			passed = false;
			break;
		}
	}
	if(passed)
	{
		Log::D(String("RNG passed for F32. Average time to get value (ms): ") + avgTime);
	}
}

void FunctionalityTests::TestAsserts()
{
	Log::D("Let's try to crash the hell out of this!");
	L_ASSERT(false);
}

void FunctionalityTests::TestTerminalOps()
{
	//display some things, then clear some things.
	IPlatform* plat = IPlatform::GetPlatformInstance();
	Log::D(Allocator::FindAllocSummary());
	Log::D("Press any key to clear screen.");
	getchar();
	plat->ClearTerm();
	Log::D("Screen cleared!");
}

void FunctionalityTests::TestStatMonitoring()
{
	//run the physics test, and inside the loop, report the memory state.
	//clear the screen, so the report looks like it's real-time.

	//setup windows, then wrapper, then physics
	IPlatform* plat = IPlatform::GetPlatformInstance();
	plat->Startup();
	IGraphicsWrapper* oglRenderer = plat->BuildGraphicsWrapper(OPEN_GL);

	PhysicsWorld physics;
	physics.Startup();
	Log::D("Physics world initialized!");

	//then setup debug physics renderer
	DebugDrawer debugDrawer(oglRenderer);
	debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawAabb);
	physics.AttachDebugDrawer(&debugDrawer);
	Log::D("Attached debug drawer to physics world");

	if(plat->GetWindow(oglRenderer, IPlatform::WINDOW, 1024, 768))
	{
		//then spawn a box
		BoxCollShape boxCollider(1.0f, 0.5f, 1.0f);
		CompoundCollShape colliderHolder;
		colliderHolder.AddShape(&boxCollider, Vector3::Zero, Quaternion::Identity);
		RigidBody boxBody(1.0f, Vector3::Zero, colliderHolder);
		//now attach the box to the physics world!
		physics.AttachRigidBody(&boxBody);
		Log::D("Attached box to physics world");

		//setup gravity
		Vector3 grav = Vector3(0.0f, -9.8f, 0.0f);
		physics.SetGravityVector(grav);

		//spawn a floor, too
		BoxCollShape floorCollider(50, 20, 50);
		CompoundCollShape floorColliderHolder;
		floorColliderHolder.AddShape(&floorCollider, Vector3::Zero, Quaternion::Identity);
		RigidBody floorBody(0.0f, Vector3(0, -50, 0), floorColliderHolder);
		physics.AttachRigidBody(&floorBody);

		//we'll also need a timer to pass deltas to physics
		GameTime gameTime;
		gameTime.Tick();

		//and the monitor for the test
		StatMonitor stats;
		stats.Startup();
		Log::D("Initialized stat monitor!");

		Matrix4x4 world, view, projection;
		world = Matrix4x4::Identity;//Matrix4x4::BuildTranslation(boxPosition);
		Log::D(String("Aspect Ratio: ") + oglRenderer->ScreenAspect());
		projection = Matrix4x4::BuildPerspectiveRH(oglRenderer->ScreenAspect(), Math::PI_OVER_2, 0.25, 50);
		//we don't have a camera class yet; ordinarily it would handle this
		view = Matrix4x4::BuildViewLookAtRH(	Vector3(0.0f, 0.0f, -3.0f),//place it 3 units forward
												boxBody.Position(),//looking at the box
												Vector3::Up); //and use default up direction for now

		//now try assigning the matrices as uniforms
		F32 worldArray[16];
		F32 viewArray[16];
		F32 projArray[16];
		world.CopyToBuffer(worldArray);
		view.CopyToBuffer(viewArray);
		projection.CopyToBuffer(projArray);

		F32 secSinceStatUpdate = 0.0f;
		F32 secUpdateRate = 0.5f;
		//enter the game loop
		bool shouldQuit = false;
		while(!shouldQuit)
		{
			{
				PROFILE("Main Loop");
				{
					PROFILE("OS Update");
					shouldQuit = plat->UpdateOS();
				}
				gameTime.Tick();
				{
					PROFILE("Physics");
					physics.Update(gameTime);
				}
				{
					PROFILE("GetRendererState");
					//print any errors
					oglRenderer->PrintShaderStatus();
				}
				{
					PROFILE("Update");
					//update the camera's view to look at the box
					view = Matrix4x4::BuildViewLookAtRH(Vector3(0.0f, 0.0f, -3.0f),
														boxBody.Position(),
														Vector3::Up);
				}
				{
					PROFILE("Rendering");
					plat->BeginRender(oglRenderer);
					//clear to a dim color
					oglRenderer->Clear(Colors::Gray20Pct);
					if(!oglRenderer->SetWorldViewProjection(world, view, projection))
					{
						Log::D("Couldn't set WVP matrices!");
					}
					{
						PROFILE("Debug Render");
						physics.DebugDraw();
					}

					plat->EndRender(oglRenderer);
				}
			}
			//update the stat monitor
			stats.Update(gameTime);
		}
		stats.PrintStats();
		plat->ShutdownGraphicsWrapper(oglRenderer);
		plat->Shutdown();
	}
	else
	{
		Log::E("Failed to build window!");
		plat->Shutdown();
		return;
	}
}

void FunctionalityTests::TestHandles()
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
}
#endif //L_ENABLE_OLD_TESTS
