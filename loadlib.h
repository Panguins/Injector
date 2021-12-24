#pragma once
#include <windows.h>
#include <iostream>
#include <io.h>

class LoadLib {
public:
	static bool Inject( int pID, std::string dll ) {
		printf( "\nGetting address from kernel32.dll.\n" );
		HANDLE loadLib = GetProcAddress( GetModuleHandle( TEXT( "kernel32.dll" ) ), "LoadLibraryA" );

		if ( _access_s( dll.c_str( ), 0 ) != 0 ) {
			std::cout << "Couldn't find library" << std::endl;
			return false;
		}

		printf( "Opening target process.\n" );
		HANDLE handle = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, 0, pID );
		if ( !handle ) {
			std::cout << "Couldn't open process" << std::endl;
			return false;
		}

		printf( "Allocating memory for location.\n" );
		const char* dllChars = dll.c_str( );
		LPVOID base = VirtualAllocEx( handle, 0, strlen( dllChars ) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

		printf( "Writing location\n" );
		WriteProcessMemory( handle, base, dllChars, strlen( dllChars ) + 1, 0 );

		printf( "Calling LoadLibraryA\n" );
		HANDLE thread = CreateRemoteThread( handle, 0, 0, (LPTHREAD_START_ROUTINE)loadLib, base, 0, 0 );
		printf( "Waiting for LoadLibraryA to finish\n" );
		WaitForSingleObject( thread, INFINITE );

		DWORD dwExit = 0;
		GetExitCodeThread( thread, &dwExit );

		printf( "Running cleanup\n" );
		VirtualFreeEx( handle, base, 0, MEM_RELEASE );
		CloseHandle( handle );

		if ( !dwExit ) {
			std::cout << "Exit code is null" << std::endl;
			return false;
		}

		if ( !thread ) {
			return false;
		}
		return true;
	}
};
