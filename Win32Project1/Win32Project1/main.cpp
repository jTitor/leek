//by Jay Tennant 3/4/12
//A Brief Look at XAudio2: Playing a Sound
//win32developer.com
//this code provided free, as in public domain; score!

#include <windows.h>
#include "xaudio2.h"
#include "x3daudio.h"
#include "wave.h"

#define MAX 1

IXAudio2* g_engine;
IXAudio2SourceVoice* g_source[MAX];
IXAudio2MasteringVoice* g_master;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {

	//must call this for COM
	CoInitializeEx( NULL, COINIT_MULTITHREADED );

	//MessageBox( 0, TEXT("VIRUS = VERY YES"), TEXT("COMPUTER OVER"), MB_OK );

	//create the engine
	if( FAILED( XAudio2Create( &g_engine ) ) ) {
		CoUninitialize();
		return -1;
	}

	
	X3DAUDIO_HANDLE bob;
	X3DAudioInitialize(SPEAKER_STEREO, 7, bob);
	
	X3DAUDIO_LISTENER listener = {
		{0,0,0}, // Vector pointing forward
		{0,0,0}, // Vector pointing up
		{1,0,0}, // Position vector
		{0,0,0}, // Velocity vector
		NULL	 // Nobody likes you, cones
	};
	X3DAUDIO_EMITTER emitter = {
		NULL,	 // Cones everywhere
		{0,0,0}, // forward
		{0,0,0}, // up
		{10,0,0},// position
		{0,0,0}, // velocity
		0.01,	 // inner raduis
		0,		 // inner raduis angle
		1,		 // channel count
		0,		 // channel radius
		NULL,	 // pChannelAzimuths
		NULL,	 // pVolumeCurve
		NULL,	 // pLFECurve
		NULL,	 // pLPFDirectCurve
		NULL,	 // pLPFReverbCurve
		NULL,	 // pReverbCurve
		FLT_MIN,	 // CurveDistanceScaler
		1.0f,	 // DopplerScaler
	};

	XAUDIO2_DEVICE_DETAILS deviceDetails;
	g_engine->GetDeviceDetails(0,&deviceDetails);
	
	X3DAUDIO_DSP_SETTINGS DSPSettings = {0};
	FLOAT32 * matrix = new FLOAT32[1]; //deviceDetails.OutputFormat.Format.nChannels];
	DSPSettings.SrcChannelCount = 1;
	DSPSettings.DstChannelCount = 1; //deviceDetails.OutputFormat.Format.nChannels;
	DSPSettings.pMatrixCoefficients = matrix;

	X3DAudioCalculate(bob, &listener, &emitter,
    X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
    &DSPSettings );



	//create the mastering voice
	if( FAILED( g_engine->CreateMasteringVoice( &g_master ) ) ) {
		g_engine->Release();
		CoUninitialize();
		return -2;
	}
	
	//helper class to load wave files; trust me, this makes it MUCH easier
	Wave buffer;

	//load a wave file
	if( !buffer.load( "LASER.WAV" ) ) {
		g_engine->Release();
		CoUninitialize();
		return -3;
	}

	//create the source voice, based on loaded wave format
	for(int x=0;x<MAX;x++) {
		if( FAILED( g_engine->CreateSourceVoice( &g_source[x], buffer.wf() ) ) ) {
			g_engine->Release();
			CoUninitialize();
			return -4;
		}
	}
	
	//start consuming audio in the source voice
	for(int x=0;x<MAX;x++) {
		g_source[x]->Start();
	}

	//simple message loop
	//while( MessageBox( 0, TEXT("Do you want to play the sound?"), TEXT("ABLAX: PAS"), MB_YESNO ) == IDYES )
	
	for(int x=0;x<MAX;x++) {
		g_source[x]->SetOutputMatrix(g_master,1,deviceDetails.OutputFormat.Format.nChannels,DSPSettings.pMatrixCoefficients);
		g_source[x]->SetFrequencyRatio(DSPSettings.DopplerFactor);
		g_source[x]->SetOutputMatrix(g_master, 1, 1, &DSPSettings.ReverbLevel);

		//play the sound
		for(int i=0;i<10;i++) {
			listener.Position.x=i;
			X3DAudioCalculate(bob, &listener, &emitter,
				X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
				&DSPSettings );
			g_source[x]->SubmitSourceBuffer( buffer.xaBuffer() );
		}
	}

	MessageBox( 0, TEXT("VIRUS = VERY YES"), TEXT("COMPUTER OVER"), MB_OK );

	//release the engine, NOT the voices!
	g_engine->Release();

	//again, for COM
	CoUninitialize();

	return 0;
}