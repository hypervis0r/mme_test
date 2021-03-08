#include <windows.h>
#include <Mmreg.h>
#include <stdio.h>

#pragma comment(lib, "Winmm.lib")

//Handle used for synchronization. Main thread waits for event to be signalled to clean up
HANDLE recordMicEvent = NULL;

//All these default values should be overwritten


//Callback saves data
//void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance,
//	DWORD_PTR dwParam1, DWORD_PTR dwParam2)
//{
//	if (uMsg == WIM_DATA) {
//		memcpy(dataBuffer, recordBuffer, buffersize);
//		SetEvent(recordMicEvent);
//	}
//}

int main(void)
{
	HANDLE hDone = NULL;
	HWAVEIN hWavIn = NULL;
	HWAVEOUT hWavOut = NULL;
	WAVEFORMATEX wFormat = { 0 };
	WAVEHDR wh = { 0 };
	UINT secondsToRecord = 0;
	MMRESULT mResult = 0;
	UINT buffersize = 0;
	PBYTE recordBuffer = NULL;
	char cErrorBuffer[1024] = { 0 };

	wFormat.wFormatTag = WAVE_FORMAT_PCM;
	wFormat.nChannels = 1;
	wFormat.nSamplesPerSec = 44100L;
	wFormat.nBlockAlign = 1;
	wFormat.nAvgBytesPerSec = wFormat.nSamplesPerSec * wFormat.nBlockAlign;
	wFormat.wBitsPerSample = 8;
	wFormat.cbSize = 0;

	secondsToRecord = 5;

	buffersize = secondsToRecord * wFormat.nSamplesPerSec;
	recordBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buffersize);

	mResult = waveInOpen(&hWavIn, WAVE_MAPPER, &wFormat, (DWORD_PTR)NULL, (DWORD_PTR)NULL, WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE);
	if (mResult != MMSYSERR_NOERROR)
	{
		waveInGetErrorTextA(mResult, cErrorBuffer, sizeof(cErrorBuffer));
		printf("[-] waveInOpen failed: %s\n", cErrorBuffer);
		return 0;
	}

	wh.lpData = (LPSTR)recordBuffer;
	wh.dwBufferLength = buffersize;
	wh.dwFlags = 0;
	waveInPrepareHeader(hWavIn, &wh, sizeof(wh));
	waveInAddBuffer(hWavIn, &wh, sizeof(wh));

	puts("[+] Recording...\n");
	mResult = waveInStart(hWavIn);
	if (mResult != MMSYSERR_NOERROR)
	{
		waveInGetErrorTextA(mResult, cErrorBuffer, sizeof(cErrorBuffer));
		printf("[-] waveInStart failed: %s\n", cErrorBuffer);
		return 0;
	}

	Sleep(secondsToRecord * 1000 + 1000);
	
	mResult = waveInStop(hWavIn);
	if (mResult != MMSYSERR_NOERROR)
	{
		waveInGetErrorTextA(mResult, cErrorBuffer, sizeof(cErrorBuffer));
		printf("[-] waveInStop failed: %s\n", cErrorBuffer);
		return 0;
	}
	
	puts("[+] Finished recording\n");
	waveInClose(hWavIn);

	mResult = waveOutOpen(&hWavOut, WAVE_MAPPER, &wFormat, (DWORD_PTR)NULL, (DWORD_PTR)NULL, WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE);
	if (mResult != MMSYSERR_NOERROR)
	{
		printf("[-] waveOutOpen failed\n");
		return 0;
	}

	mResult = waveOutWrite(hWavOut, &wh, sizeof(wh));
	if (mResult != MMSYSERR_NOERROR)
	{
		printf("[-] waveOutWrite failed\n");
		return 0;
	}

	// ** Wait until sound finishes playing
	Sleep(secondsToRecord * 1000 + 1000);

	waveOutClose(hWavOut);
	HeapFree(GetProcessHeap(), 0, recordBuffer);
}