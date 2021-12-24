#include "Target.h"

#include <Windows.h>
#include <tlhelp32.h> //snapshots

std::vector<TargetProcess> Target::potential_targets = {
	{ "Fortnite", "FortniteClient-Win64-Shipping.exe", "Fortnite.dll", X64, false },
	{ "Call of Duty: WWII", "s2_mp64_ship.exe", "ww2_hack.dll", X64, true },
	{ "Call of Duty® Infinite Warfare", "iw7_ship.exe", "ww2_hack.dll", X64, true },

	{ "Call of Duty®: Advanced Warfare Multiplayer", "s1_mp64_ship.exe", "aw_hack.dll", X64, true },
	{ "Call of Duty®: Advanced Warfare", "s1_sp64_ship.exe", "aw_hack.dll", X64, true },

	{ "Battlefield 4", "bf4.exe", "bf4.dll", X64, true },

	{ "Grand Theft Auto V", "GTA5.exe", "gta_internal.dll", X64, false },
	{ "Counter-Strike: Global Offensive", "csgo.exe", "csgo_internal.dll", X86, false },
	{ "Garry's Mod", "hl2.exe", "gmod_hack.dll", X86, false },
	{ "Left 4 Dead 2", "left4dead2.exe", "l4d2_hack.dll", X86, false },
	{ "Call of Duty: Modern Warfare 3", "iw5mp.exe", "mw3_hack.dll", X86, false },
	{ "Modern Warfare 2", "iw4mp.exe", "mw2_hack.dll", X86, false },
};

int Target::is_valid_target( std::string target_name ) {
	Architecture_t architecture = (Architecture_t)( sizeof( void* ) == 8 );

	for ( size_t i = 0; i < potential_targets.size( ); i++ ) {
		TargetProcess& potential_target = potential_targets.at( i );
		if ( potential_target.architecture == architecture && potential_target.process_name.compare( target_name ) == 0 ) {
			return (int)i;
		}
	}

	return -1;
}

int Target::find_target( TargetProcess* target ) {
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
	if ( hSnapshot == INVALID_HANDLE_VALUE )
		return -1;

	PROCESSENTRY32 entry = { NULL };
	entry.dwSize = sizeof( PROCESSENTRY32 );

	if ( !Process32First( hSnapshot, &entry ) ) {
		CloseHandle( hSnapshot );
		return -1;
	}

	do {
		int potential_target_id = is_valid_target( entry.szExeFile );
		if ( potential_target_id != -1 ) {
			CloseHandle( hSnapshot );
			*target = potential_targets.at( potential_target_id );
			return entry.th32ProcessID;
		}
	} while ( Process32Next( hSnapshot, &entry ) );

	CloseHandle( hSnapshot );
	return -1;
}