#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include "manual/manualmap.h"
#include "loadlib.h"
#include "target.h"
#include "target.h"

void SetupConsole( ) {
	SetConsoleTitle( "Kola mit Ice Injector" );
	system( "Color 0E" );
	std::cout << "\n  $$\\   $$\\          $$\\                               $$\\   $$\\           $$$$$$\\" << std::endl;
	std::cout << "  $$ | $$  |         $$ |                              \\__|  $$ |          \\_$$  _|" << std::endl;
	std::cout << "  $$ |$$  / $$$$$$\\  $$ | $$$$$$\\        $$$$$$\\$$$$\\  $$\\ $$$$$$\\           $$ |  $$$$$$$\\  $$$$$$\\" << std::endl;
	std::cout << "  $$$$$  / $$  __$$\\ $$ | \\____$$\\       $$  _$$  _$$\\ $$ |\\_$$  _|          $$ | $$  _____|$$  __$$\\" << std::endl;
	std::cout << "  $$  $$<  $$ /  $$ |$$ | $$$$$$$ |      $$ / $$ / $$ |$$ |  $$ |            $$ | $$ /      $$$$$$$$ |" << std::endl;
	std::cout << "  $$ |\\$$\\ $$ |  $$ |$$ |$$  __$$ |      $$ | $$ | $$ |$$ |  $$ |$$\\         $$ | $$ |      $$   ____|" << std::endl;
	std::cout << "  $$ | \\$$\\\\$$$$$$  |$$ |\\$$$$$$$ |      $$ | $$ | $$ |$$ |  \\$$$$  |      $$$$$$\\\\$$$$$$$\\ \\$$$$$$$\\" << std::endl;
	std::cout << "  \\__|  \\__|\\______/ \\__| \\_______|      \\__| \\__| \\__|\\__|   \\____/       \\______|\\_______| \\_______|" << std::endl;
	std::cout << std::endl << std::endl << std::endl;
	std::cout << "                                $$$$$$\\                                     $$\\" << std::endl;
	std::cout << "                                \\_$$  _|                                    $$ |" << std::endl;
	std::cout << "                                  $$ |  $$$$$$$\\  $$\\  $$$$$$\\   $$$$$$$\\ $$$$$$\\    $$$$$$\\   $$$$$$\\" << std::endl;
	std::cout << "                                  $$ |  $$  __$$\\ \\__|$$  __$$\\ $$  _____|\\_$$  _|  $$  __$$\\ $$  __$$\\" << std::endl;
	std::cout << "                                  $$ |  $$ |  $$ |$$\\ $$$$$$$$ |$$ /        $$ |    $$ /  $$ |$$ |  \\__|" << std::endl;
	std::cout << "                                  $$ |  $$ |  $$ |$$ |$$   ____|$$ |        $$ |$$\\ $$ |  $$ |$$ |" << std::endl;
	std::cout << "                                $$$$$$\\ $$ |  $$ |$$ |\\$$$$$$$\\ \\$$$$$$$\\   \\$$$$  |\\$$$$$$  |$$ |" << std::endl;
	std::cout << "                                \\______|\\__|  \\__|$$ | \\_______| \\_______|   \\____/  \\______/ \\__|" << std::endl;
	std::cout << "                                            $$\\   $$ |" << std::endl;
	std::cout << "                                            \\$$$$$$  |" << std::endl;
	std::cout << "                                             \\______/ " << std::endl << std::endl;
}

std::string GetDLLPath( std::string dllName ) {
	std::string dllPath;
	char tempPath[MAX_PATH];
	GetModuleFileName( GetModuleHandle( NULL ), tempPath, ( sizeof( tempPath ) ) );
	PathRemoveFileSpec( tempPath );
	std::string path( tempPath );
	path += "\\" + dllName;
	return path;
}

void Inject( int pID, std::string dllPath, bool can_manual_map ) {
	bool loadlibrayyyyyyyyyyyyy = true;
	bool did_inject = false;

	if ( can_manual_map ) {
		printf( "> Default injection method: Manual Map\n" );
		printf( "> Do you want to switch to LoadLibrary?\n" );
		printf( "> Press enter to continue..." );

		std::string input;
		getline( std::cin, input );
		loadlibrayyyyyyyyyyyyy = !input.empty( );
	}

	if ( loadlibrayyyyyyyyyyyyy )
		did_inject = LoadLib::Inject( pID, dllPath );
	else {
		manual_map mapper;
		did_inject = mapper.inject_from_path( pID, dllPath.c_str( ) );
	}


	if ( !did_inject )
		printf( "> Injection failed\n" );
}

void Close( ) {
	printf( "> Press enter to close\n" );
	printf( "> Automatically closing in 5 seconds\n" );

	Sleep( 500 );
	DWORD finished_inject = GetTickCount( );
	while ( !( GetAsyncKeyState( VK_RETURN ) < 0 ) ) {
		DWORD current = GetTickCount( );
		if ( current > finished_inject + 5000 )
			return;

		Sleep( 100 );
	}
}

int main( int argc, char* argv[] ) {
	SetupConsole( );

	TargetProcess target;
	int pid = Target::find_target( &target );
	if ( pid != -1 ) {
		printf( "> Target: %s \n", target.display_name.c_str( ) );
		printf( "> Process: %s \n", target.process_name.c_str( ) );
		printf( "> Process ID: %i \n", pid );
		printf( "> DLL: %s \n", target.dll_name.c_str( ) );

		std::string dllPath = GetDLLPath( target.dll_name );
		printf( "> DLL Path: %s \n", dllPath.c_str( ) );

		Inject( pid, dllPath, target.manual_map );
	}
	else {
		printf( "> Couldn't find any valid target...\n" );
	}

	Close( );
	return 0;
}
